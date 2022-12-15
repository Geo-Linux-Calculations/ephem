/* these functions allow you to watch the sky or the solar system via a
 * simple character-graphics representation on the screen. 
 * the interaction starts by using the current time. then control with
 *    END returns to table form; or
 *    RETURN advances time by one StpSz; or
 *    h advances once by 1 hour; or
 *    d advances once by 24 hours (1 day); or
 *    w advances once by 7 days (1 week); or
 *    any other key free runs by StpSz until any key is hit.
 */

#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "circum.h"
#include "screen.h"
#include "ephem.h"

#define	SSZCOL	1		/* column to show solar system z coords */
#define	PARK_ROW	1	/* cursor park loc after each screen */
#define	PARK_COL	NC	/* cursor park loc after each screen */

#define	NO_SKY		-1	/* none */
#define	DOME_SKY	0	/* flags for watch_sky() */
#define	ALTAZ_SKY	1	/* flags for watch_sky() */

int LastSkyStyle = NO_SKY;	/* remember sky style for interrupts */
Now *Last_np;			/* remember Now ptr for interrupts */
#ifdef NCURSES_LARGE
#include <ncurses.h>
int watching;			/* are we in a watch mode ? */
#endif
#ifdef GLOBE_PHOON
#include <time.h>
#endif

#define	SKYACC	3600.	/* desired sky plot accuracy, in arc seconds */
#define	SSACC	3600.	/* desired solar system plot accuracy, in arc secs */

/* macros to convert row(col) in range 1..NR(1..NC) to fraction in range 0..1 */
#define	NEARONE		0.9999999
#define	r2fr(r)		(((r)-1)/(NEARONE*NR)+1/NC/2)
#define	c2fc(c)		(((c)-1)/(NEARONE*NC)+1/NC/2)
#define	fr2r(fr)	((int)((fr)*(NEARONE*NR))+1)
#define	fc2c(fc)	((int)((fc)*(NEARONE*NC))+1)

/* single-character tag for each body.
 * order must match the #defines in astro.h and screen.h additions.
 */
static char body_tags[] = "evmjsunpSMxy";

/* multiple and single loop prompts */
static char frprompt[] = "Running... press any key to stop.";
static char qprompt[]  =
"q to quit, RETURN/h/d/w to step by StpSz/hr/day/wk, or any other to freerun";

/* used to locate, record and then erase last plotted chars */
typedef struct {
    double l_fr, l_fc;	/* 2d coords as 0..1 (upper left corner is (0,0)) */
    int	 l_r, l_c;	/* screen 2d coords (upper left corner is [1,1]) */
    char l_tag;		/* char to use to print on screen */
} LastDraw;

static int trails;	/* !0 if want to leave trails */

void watch_sky (int style, Now *np, double tminc, int wbodies);
static void watch_solarsystem (Now *np, double tminc, int wbodies);
static void set_ss (LastDraw *lp, double dist, double lg, double lt, int tag);
static void set_screencoords (LastDraw lp[], int np);
static void safe_f_char (int r, int c, int tag);
static int readwcmd (double tminc0, double *tminc, int *once);

#ifdef GLOBE_PHOON
static void watch_earth(Now *np, double tminc);
static void watch_moon (Now *np, double tminc);
#endif


int
watch_function( nf, np, tminc, wbodies)
	int nf;         /* function number: WATCH_* options */
	Now *np;	/* time now and on each step */
	double tminc;	/* hrs to increment time by each step */
	int wbodies;	/* each bit is !=0 if want that body */
{
	int didone = 0;

#ifdef NCURSES_LARGE
	    watching = 1;
#endif
	switch (nf) {
	    case WATCH_DOME:
		watch_sky (DOME_SKY, np, tminc, wbodies); didone = 1; break;
	    case WATCH_ALTAZ:
		watch_sky (ALTAZ_SKY, np, tminc, wbodies); didone = 1; break;
	    case WATCH_SOLAR_SYS:
		watch_solarsystem (np, tminc, wbodies); didone = 1; break;
#ifdef GLOBE_PHOON
	    case WATCH_EARTH:
		watch_earth(np, tminc); didone = 1; break;
	    case WATCH_MOON:
		watch_moon(np, tminc); didone = 1; break;
#endif
	}
#ifdef NCURSES_LARGE
	    watching = 0;
#endif

	return didone;
}


/* present menu, make selection, enter watch mode  */
void
watch_menu (np, tminc, wbodies)
Now *np;	/* time now and on each step */
double tminc;	/* hrs to increment time by each step */
int wbodies;	/* each bit is !=0 if want that body */
{
	int NFields = 4;
#define NT_FLD     (NFields-1)	/* trails/no-trails menu index */
#ifdef GLOBE_PHOON
	NFields += 2;
#endif
	char *flds[8] = {
	    "Sky dome", "Alt/az sky", "Solar system"
#ifdef GLOBE_PHOON
		    , "Earth", "Moon"
#endif
	};
	static int fn;	/* begin with 0, then remember for next time */
	int didone = 0;

	while (1) {
	    int nf;
	    flds[ NT_FLD ] = trails ? "Leave trails" : "No trails";
	    if ((nf = popup (flds, fn, NFields)) < 0)
		break;
	    fn = nf;
	    if ( nf == NT_FLD) {
		    trails ^= 1;
	    }
	    else {
		    didone = watch_function( nf, np, tminc, wbodies);
	    }
	}
	if (didone)
	    redraw_screen(2);
}


static char east[] = "East";
static char west[] = "West";
static char north[] = "North";
static char south[] = "South";


/* like f_char(), but optionally applies ANSI colors to various watched bodies */
void print_body(row,col,body_symbol)
int row, col;
char body_symbol;
{
#ifdef ANSI_COLORS
	switch (body_symbol) {
		case 'S':  COLOR_CODE(COLOR_SUN);  break; // black/yellow
		case 'M':  COLOR_CODE(COLOR_MOON); break; // lt gray/gray
		case 'e':  COLOR_CODE(COLOR_MERCURY); break; // blue/gray
		case 'v':  COLOR_CODE(COLOR_VENUS); break; // black/aqua
		case 'E':  COLOR_CODE(COLOR_EARTH); break; // green/blue
		case 'm':  COLOR_CODE(COLOR_MARS); break; // yellow/red
		case 'j': case 'J':
			   COLOR_CODE(COLOR_JUPITER); break; // aqua/red
		case 's':  COLOR_CODE(COLOR_SATURN); break; // yellow/black
		case 'u':  COLOR_CODE(COLOR_URANUS); break; // blue/aqua
		case 'n':  COLOR_CODE(COLOR_NEPTUNE); break; // magenta/aqua
		case 'p':  COLOR_CODE(COLOR_PLUTO); break; // grey/white
		case 'x':  COLOR_CODE(COLOR_OBJX); ; break; // black/green
		case 'y':  COLOR_CODE(COLOR_OBJY); ; break; // green/black
	}
#endif
	f_char (row, col, body_symbol);
	COLOR_OFF;
}


/* clear screen and put up the permanent labels */
void sky_dome_labels (style,np)
int style;	/* DOME_SKY or ALTAZ_SKY */
Now *np;	/* time now and on each step */
{
	c_erase();
	COLOR_CODE(COLOR_WATCH);

	if (style == DOME_SKY) {
		double a;
		for (a = 0.0; a < 2*PI; a += PI/8)
			f_char (fr2r(.5-sin(a)/2.),
			    fc2c(.5+cos(a)/2./ASPECT) + ((a>PI/2 && a<3*PI/2) ? -1 : 1),
			    '*');
		f_string (fr2r(.5), fc2c(.5-.5/ASPECT)-7, "East");
		f_string (fr2r(1.), fc2c(.5)-2, south);
		f_string (fr2r(.5), fc2c(.5+.5/ASPECT)+4, "West");
		f_string (2, NC/2-2, north);
	} else {
		f_string (NR, 1, north);
		f_string (NR, NC/4, east);
		f_string (NR, NC/2, south);
		f_string (NR, 3*NC/4, west);
		f_string (NR, NC-5, north);   /* -1 more to avoid scrolling */
		f_string (2, NC/2-3, "Zenith");
	}
	f_string (2, 1, tznm);
	f_string (3, 1, "LST");

	COLOR_OFF;
}


	static 
void sky_dome_update (style,np,wbodies,ld0,ld1,newp,lastp,s,tmp,nlast)
int style;	/* DOME_SKY or ALTAZ_SKY */
Now *np;	/* time now and on each step */
int wbodies;	/* each bit is !=0 if want */
LastDraw ld0[NOBJ], ld1[NOBJ];
LastDraw **newp,**lastp;
Sky *s;
double *tmp;
int *nlast;
{
        int p;
	double lmjd;
        LastDraw *lp;

        /* calculate desired stuff into newp[] */
        int nnew = 0;
        for (p = nxtbody(-1); p != -1; p = nxtbody(p))
            if (wbodies & (1<<p)) {
                (void) body_cir (p, SKYACC, np, s);
                if ((*s).s_alt > 0.0) {
                    LastDraw *lnp = (*newp) + nnew;
                    if (style == DOME_SKY) {
                        *tmp = 0.5 - (*s).s_alt/PI;
                        lnp->l_fr = 0.5 - (*tmp)*cos((*s).s_az);
                        lnp->l_fc = 0.5 - (*tmp)*sin((*s).s_az)/ASPECT;
                    } else {
                        lnp->l_fr = 1.0 - (*s).s_alt/(PI/2);
                        lnp->l_fc = (*s).s_az/(2*PI);
                    }
                    lnp->l_tag = body_tags[p];
                    nnew++;
                }
            }
        set_screencoords ( (*newp), nnew);

        /* unless we want trails,
         * erase any previous tags (in same order as written) from lastp[].
         */
        if (!trails)
            for (lp = (*lastp); --(*nlast) >= 0; lp++)
                f_char (lp->l_r, lp->l_c, ' ');

        /* print LOCAL time and date we will be using */
        lmjd = mjd - tz/24.0;
        f_time (2, 5, mjd_hr(lmjd));
        f_date (2, 14, mjd_day(lmjd));
        now_lst (np, tmp);
        f_time (3, 5, (*tmp));

        /* now draw new stuff from newp[] and park the cursor */
        for (lp = (*newp); lp < (*newp) + nnew; lp++)
                print_body(lp->l_r, lp->l_c, lp->l_tag);

        c_pos (PARK_ROW, PARK_COL);
        fflush (stdout);

        /* swap new and last roles and save new count */
        if ( (*newp) == ld0)
            (*newp) = ld1, (*lastp) = ld0;
        else
            (*newp) = ld0, (*lastp) = ld1;
        *nlast = nnew;
}


/* full alt/az or dome sky view (like the popular astro mags).
 * alt/az: north is at left and right of screen, south at center.
 *   0 elevation is at bottom of screen, zenith at the top.
 * dome: east is left, north is up.
 */
void watch_sky (style, np, tminc, wbodies)
int style;	/* DOME_SKY or ALTAZ_SKY */
Now *np;	/* time now and on each step */
double tminc;	/* hrs to increment time by each step */
int wbodies;	/* each bit is !=0 if want */
{
	double tminc0 = tminc;	/* remember the original */
	/* two draw buffers so we can leave old up while calc new then
	 * erase and draw in one quick operation. always calc new in newp
	 * buffer and erase previous from lastp. buffers alternate roles.
	 */
	LastDraw ld0[NOBJ], ld1[NOBJ], *lastp = ld0, *newp = ld1;
	int nlast = 0;
	int once = 1;
	double tmp;
	Sky s;

	LastSkyStyle = style;
	Last_np = np;

	/* clear screen and put up the permanent labels */
	sky_dome_labels(style, np);

	while (1) {
	    if (once)
		    print_updating();

	    sky_dome_update(style,np,wbodies,
			    ld0,ld1,&newp,&lastp,
			    &s,&tmp,&nlast);

	    if (!once)
		    slp_sync();

	    if (once || (chk_char()==0 && read_char()!=0)) {
		if (readwcmd (tminc0, &tminc, &once) < 0)
		    break;
	    }

	    /* advance time */
	    inc_mjd (np, tminc);
	    Last_np = np;
	}

	LastSkyStyle = NO_SKY;
}


/* solar system view, "down from the top", first point of aries to the right.
 * always include earth.
 */
static
void watch_solarsystem (np, tminc, wbodies)
Now *np;	/* time now and on each step */
double tminc;	/* hrs to increment time by each step */
int wbodies;
{
	/* max au of each planet from sun; in astro.h #defines order */
	static double auscale[] = {.38, .75, 1.7, 5.2, 11., 20., 31., 50.};
	double tminc0 = tminc;	/* remember the original */
	/* two draw buffers so we can leave old up while calc new then
	 * erase and draw in one quick operation. always calc new in newp
	 * buffer and erase previous from lastp. buffers alternate roles.
	 */
	LastDraw ld0[2*NOBJ], ld1[2*NOBJ], *lp, *lastp = ld0, *newp = ld1;
	int nlast = 0, nnew;
	int once = 1;
	double lmjd;
	double scale;
	Sky s;
	int p;
	Last_np = np;

	/* set screen scale: largest au we will have to plot.
	 * never make it less than 1 au (with fudge) since we always show earth.
	 */
	scale = 1.1;
	for (p = MARS; p <= PLUTO; p++)
	    if ((wbodies & (1<<p)) && auscale[p] > scale)
		scale = auscale[p];

	/* clear screen and put up the permanent labels */
	c_erase();
	COLOR_CODE(COLOR_WATCH);
	f_string (2, 1, tznm);
	COLOR_OFF;

	while (1) {
	    if (once)
		print_updating();

	    /* calculate desired stuff into newp[].
	     * fake a sun at center and add earth first.
	     * (we get earth's loc when ask for sun)
	     */
	    nnew = 0;
	    set_ss (newp+nnew, 0.0, 0.0, 0.0, 'S');
	    nnew += 2;
	    (void) body_cir (SUN, SSACC, np, &s);
	    set_ss (newp+nnew, s.s_edist/scale, s.s_hlong, 0.0, 'E');
	    nnew += 2;
	    for (p = MERCURY; p <= PLUTO; p++)
		if (p != MOON && (wbodies & (1<<p))) {
		    (void) body_cir (p, SSACC, np, &s);
		    set_ss (newp+nnew, s.s_sdist/scale, s.s_hlong, s.s_hlat,
							    body_tags[p]);
		    nnew += 2;
		}
	    for (p = OBJX; p != -1; p = (p == OBJX) ? OBJY : -1)
		if (wbodies & (1<<p)) {
		    (void) body_cir (p, SSACC, np, &s);
		    if (s.s_hlong != NOHELIO && s.s_sdist <= scale) {
			set_ss (newp+nnew, s.s_sdist/scale, s.s_hlong, s.s_hlat,
								body_tags[p]);
			nnew += 2;
		    }
		}

	    set_screencoords (newp, nnew);

	    /* unless we want trails,
	     * erase any previous tags (in same order as written) from lastp[].
	     */
	    if (!trails)
		for (lp = lastp; --nlast >= 0; lp++)
		    safe_f_char (lp->l_r, lp->l_c, ' ');

	    /* print LOCAL time and date we will be using */
	    lmjd = mjd - tz/24.0;
	    f_time (2, 5, mjd_hr(lmjd));
	    f_date (2, 14, mjd_day(lmjd));

	    /* now draw new stuff from newp[] and park the cursor */
	    for (lp = newp; lp < newp + nnew; lp++)
		safe_f_char (lp->l_r, lp->l_c, lp->l_tag);
	    c_pos (PARK_ROW, PARK_COL);
	    fflush (stdout);

	    /* swap new and last roles and save new count */
	    if (newp == ld0)
		newp = ld1, lastp = ld0;
	    else
		newp = ld0, lastp = ld1;
	    nlast = nnew;

	    if (!once)
		slp_sync();

	    if (once || (chk_char()==0 && read_char()!=0)) {
		if (readwcmd (tminc0, &tminc, &once) < 0)
		    break;
	    }

	    /* advance time */
	    inc_mjd (np, tminc);
        Last_np = np;
	}
}

/* fill in two LastDraw solar system entries,
 * one for the x/y display, one for the z.
 */
static
void set_ss (lp, dist, lg, lt, tag)
LastDraw *lp;
double dist, lg, lt;	/* scaled heliocentric distance, longitude and lat */
int tag;
{
	lp->l_fr = 0.5 - dist*sin(lg)*0.5;
	lp->l_fc = 0.5 + dist*cos(lg)*0.5/ASPECT;
	lp->l_tag = tag;
	lp++;
	/* row is to show course helio altitude but since we resolve collisions
	 * by adjusting columns we can get more detail by smaller variations
	 * within one column.
	 */
	lp->l_fr = 0.5 - dist*sin(lt)*0.5;
	lp->l_fc = c2fc(SSZCOL) + (1 - lp->l_fr)/NC;
	lp->l_tag = tag;
}

/* given a list of LastDraw structs with their l_{fr,fc} filled in,
 * fill in their l_{r,c}.
 * TODO: better collision avoidance.
 */
static
void set_screencoords (lp, np)
LastDraw lp[];
int np;
{
	LastDraw *lpi;	/* the current basis for comparison */
	LastDraw *lpj;	/* the sweep over other existing cells */
	int i;		/* index of the current basis cell, lpi */
	int j;		/* index of sweep cell, lpj */
	int n;		/* total cells placed so far (ie, # to check) */

	/* idea is to place each new item onto the screen.
	 * after each placement, look for collisions.
	 * if find a colliding pair, move the one with the greater l_fc to
	 * the right one cell, then rescan for more collisions.
	 * this will yield a result that is sorted by columns by l_fc.
	 * TODO: don't just move to the right, try up too for true 2d adjusts.
	 */
	for (n = 0; n < np; n++) {
	    lpi = lp + n;
	    i = n;
	    lpi->l_r = fr2r(lpi->l_fr);
	    lpi->l_c = fc2c(lpi->l_fc);
	  chk:
	    for (j = 0; j < n; j++) {
		lpj = lp + j;
		if (i!=j && lpi->l_r == lpj->l_r && lpi->l_c == lpj->l_c) {
		    if (lpj->l_fc > lpi->l_fc) {
			/* move lpj and use it as basis for checks now */
			lpi = lpj;
			i = j;
		    }
		    if (++lpi->l_c > NC)
			lpi->l_c = 1;
		    goto chk;
		}
	    }
	}
}

/* since the solar system scaling is only approximate, and doesn't include
 * object x/y at all, characters might get mapped off screen. this funtion
 * guards against drawing chars off screen. it also moves a char being drawn
 * on the lower right corner of the screem left one to avoid scrolling.
 */
static
void safe_f_char (r, c, tag)
int r, c;
int tag;
{
	if (r >= 1 && r <= NR && c >= 1 && c <= NC) {
	    if (r == NR && c == NC)
		c -= 1;
	    print_body(r, c, tag);
	}
}

/* see what the op wants to do now and update prompt/times accordingly.
 * return -1 if we are finished, else 0.
 */
static int
readwcmd (tminc0, tminc, once)
double tminc0;
double *tminc;
int *once;
{
	f_prompt (qprompt);

	switch (read_char()) {
	case END:		/* back to table */
	    return (-1);
	case '\r': case ' ':	/* one StpSz step */
	    *tminc = tminc0;
	    *once = 1;
	    break;
	case 'h':		/* one 1-hour step */
	    *tminc = 1.0;
	    *once = 1;
	    break;
	case 'd':		/* one 24-hr step */
	    *tminc = 24.0;
	    *once = 1;
	    break;
	case 'w':		/* 7 day step */
	    *tminc = 7*24.0;
	    *once = 1;
	    break;
	default:		/* free-run */
	    *once = 0;
	    f_prompt (frprompt);
	}
	return (0);
}


#ifdef GLOBE_PHOON
/* assimilated ASCII graphic earth and moon from http://acme.com/software/ */
void putmoon_jd( double jd, int numlines, char* atfiller );
void print_globe_jd( double jd );
time_t julian_to_unix( double jd );

/* watch Earth as seen from Sun */
static
void watch_earth(np, tminc)
	Now *np;	/* time now and on each step */
	double tminc;	/* hrs to increment time by each step */
{
	int once = 1;
	double tminc0 = tminc;	/* remember the original */
	Last_np = np;

	/* clear screen and put up the permanent (phoon) labels */
	c_erase();

	while (1) {

		if (once)
			print_updating();

		double jd = mjd + 2415020L;
		print_globe_jd( jd );

		/* print LOCAL time and date we will be using */
		COLOR_CODE(COLOR_WATCH);
		f_string (2, 1, tznm);
		f_string (3, 1, "LST");
		COLOR_OFF;
		double lmjd = mjd - tz/24.0;
		f_time (2, 5, mjd_hr(lmjd));
		f_date (2, 14, mjd_day(lmjd));
		double tmp;
		now_lst (np, &tmp);
		f_time (3, 5, tmp);

		c_pos (PARK_ROW, PARK_COL);
		fflush(stdout);

		if (!once)
			slp_sync();

		if (once || (chk_char()==0 && read_char()!=0)) {
			if (readwcmd (tminc0, &tminc, &once) < 0)
				break;
		}

		/* advance time */
		inc_mjd (np, tminc);
		Last_np = np;
	}
}


/* watch phases of Moon as seen from Earth */
static
void watch_moon (np, tminc)
	Now *np;	/* time now and on each step */
	double tminc;	/* hrs to increment time by each step */
{
	int once = 1;
	double tminc0 = tminc;	/* remember the original */
	Last_np = np;

	/* clear screen and put up the permanent (phoon) labels */
	c_erase();

	while (1) {

		if (once)
			print_updating();

		// draw moon first, then time/date
		double jd = mjd + 2415020L;

		// TODO: adjust lines to nearest non-@ phoon draw size
		putmoon_jd( jd, (int)(NR-1), "@" ); // vs, putmoon(jd,NR,"GREENCHEESE");

		/* print LOCAL time and date we will be using */
		COLOR_CODE(COLOR_WATCH);
		f_string (2, 1, tznm);
		f_string (3, 1, "LST");
		COLOR_OFF;
		double lmjd = mjd - tz/24.0;
		f_time (2, 5, mjd_hr(lmjd));
		f_date (2, 14, mjd_day(lmjd));
		double tmp;
		now_lst (np, &tmp);
		f_time (3, 5, tmp);

		c_pos (PARK_ROW, PARK_COL);
		fflush(stdout);

		if (!once)
			slp_sync();

		if (once || (chk_char()==0 && read_char()!=0)) {
			if (readwcmd (tminc0, &tminc, &once) < 0)
				break;
		}

		/* advance time */
		inc_mjd (np, tminc);
		Last_np = np;
	}
}
#endif
