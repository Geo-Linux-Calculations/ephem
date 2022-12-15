/* routines for managing the alternative bottom half menus.
 * planet-specific menus are in their own files.
 */

#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "circum.h"
#include "screen.h"
#include "ephem.h"

static void alt1_labels();
static void alt2_labels();
static void alt3_labels();
static void alt1_body(int p, int force, Now *np);
static void alt2_body(int p, int force, Now *np);
static void alt3_body(int p, int force, Now *np);

static int altmenu = F_MNU1;	/* which alternate menu is up; one of F_MNUi */
static int alt2_stdhzn;	/* whether to use STDHZN (aot ADPHZN) horizon algthm  */
static int alt3_geoc;	/* whether to use geocentric (aot topocentric) vantage*/

/* table of screen rows given a body #define from astro/h or screen.h */
static short bodyrow[NOBJ] = {
	R_MERCURY, R_VENUS, R_MARS, R_JUPITER, R_SATURN,
	R_URANUS, R_NEPTUNE, R_PLUTO, R_SUN, R_MOON, R_OBJX, R_OBJY
};
/* table of screen cols for third menu format, given body #define ... */
static short bodycol[NOBJ] = {
	C_MERCURY, C_VENUS, C_MARS, C_JUPITER, C_SATURN,
	C_URANUS, C_NEPTUNE, C_PLUTO, C_SUN, C_MOON, C_OBJX, C_OBJY
};

/* initialize altmenu; used by main from cracking the ephem startup file.
 */
void altmenu_init (n)
int n;
{
	altmenu = n;
}

/* let op decide which alternate menu should be up,
 * including any menu-specific setup they might require.
 * return 0 if things changed to require updating the alt menu; else -1.
 */
int altmenu_setup()
{
	static char *flds[5] = {
	    "Data", "(Rise/Set", "", "(Separations"
	};
	int newmenu = altmenu, newhzn = alt2_stdhzn, newgeoc = alt3_geoc;
	int new;
	int fn = altmenu == F_MNU3 ? 3 : altmenu == F_MNU2 ? 1 : 0;

    ask:
	flds[2]= newhzn ? "Standard hzn)" : "Adaptive hzn)";
	flds[4]= newgeoc? "Geocentric)" : "Topocentric)";

	switch (popup (flds, fn, 5)) {
	case 0: newmenu = F_MNU1; break;
	case 1: newmenu = F_MNU2; break;
	case 2: newhzn ^= 1; fn = 2; goto ask;
	case 3: newmenu = F_MNU3; break;
	case 4: newgeoc ^= 1; fn = 4; goto ask;
	default: return (-1);
	}

	new = 0;
	if (newmenu != altmenu) {
	    altmenu = newmenu;
	    new++;
	}
	if (newhzn != alt2_stdhzn) {
	    alt2_stdhzn = newhzn;
	    if (newmenu == F_MNU2)
		new++;
	}
	if (newgeoc != alt3_geoc) {
	    alt3_geoc = newgeoc;
	    if (newmenu == F_MNU3)
		new++;
	}
	return (new ? 0 : -1);
}

/* erase the info for the given planet */
void alt_nobody (p)
int p;
{
	f_eol (bodyrow[p], C_RA);
}

void alt_body (b, force, np)
int b;		/* which body, ala astro.h and screen.h defines */
int force;	/* if !0 then draw for sure, else just if changed since last */
Now *np;
{
	switch (altmenu) {
	case F_MNU1: alt1_body (b, force, np); break;
	case F_MNU2: alt2_body (b, force, np); break;
	case F_MNU3: alt3_body (b, force, np); break;
	}
}

/* draw the labels for the current alternate menu format */
void alt_labels ()
{
	switch (altmenu) {
	case F_MNU1: alt1_labels (); break;
	case F_MNU2: alt2_labels (); break;
	case F_MNU3: alt3_labels (); break;
	case F_MNUJ: altj_labels (); break;
	}
}

void alt_erase ()
{
	int i;

	for (i = R_PLANTAB; i <= NR; i++)
	    f_eol (i, 1);
	f_string (R_ALTM, C_ALTMV, "             ");
}

int  alt_menumask()
{
	return (altmenu);
}

/* handy function to return the next planet in the order in which they are
 * displayed in the lower half of the screen.
 * input is a given planet, return is the next planet.
 * if input is not legal, then first planet is returned; when input is the
 * last planet, then -1 is returned.
 * typical usage is something like:
 *   for (p = nxtbody(-1); p != -1; p = nxtbody(p))
 */
int nxtbody(p)
int p;
{
	static short nxtpl[NOBJ] = {
	    VENUS, MARS, JUPITER, SATURN, URANUS,
	    NEPTUNE, PLUTO, OBJX, MOON, MERCURY, OBJY, -1
	};

	if (p < MERCURY || p >= NOBJ)
	    return (SUN);
	else
	    return (nxtpl[p]);
}

void alt_plnames()
{
	f_string (R_PLANTAB,	C_OBJ,	"OCX");
	f_string (R_SUN,	C_OBJ,	"Su");
	f_string (R_MOON,	C_OBJ,	"Mo");
	f_string (R_MERCURY,	C_OBJ,	"Me");
	f_string (R_VENUS,	C_OBJ,	"Ve");
	f_string (R_MARS,	C_OBJ,	"Ma");
	f_string (R_JUPITER,	C_OBJ,	"Ju");
	f_string (R_SATURN,	C_OBJ,	"Sa");
	f_string (R_URANUS,	C_OBJ,	"Ur");
	f_string (R_NEPTUNE,	C_OBJ,	"Ne");
	f_string (R_PLUTO,	C_OBJ,	"Pl");
	f_string (R_OBJX,	C_OBJ,	"X");
	f_string (R_OBJY,	C_OBJ,	"Y");
}

static
void alt1_labels()
{
	f_string (R_ALTM, C_ALTMV, "  Planet Data");

	alt_plnames();
	f_string (R_PLANTAB,	C_RA+2,	"R.A.");
	f_string (R_PLANTAB,	C_DEC+2,"Dec");
	f_string (R_PLANTAB,	C_AZ+2,	"Az");
	f_string (R_PLANTAB,	C_ALT+2,"Alt");
	f_string (R_PLANTAB,	C_HLONG,"H Long");
	f_string (R_PLANTAB,	C_HLAT,	"H Lat");
	f_string (R_PLANTAB,	C_EDIST,"Ea Dst");
	f_string (R_PLANTAB,	C_SDIST,"Sn Dst");
	f_string (R_PLANTAB,	C_ELONG,"Elong");
	f_string (R_PLANTAB,	C_SIZE,	"Size");
	f_string (R_PLANTAB,	C_MAG,	"VMag");
	f_string (R_PLANTAB,	C_PHASE,"Phs");
}

static
void alt2_labels()
{
	f_string (R_ALTM, C_ALTMV, "Rise/Set Info");

	alt_plnames();
	f_string (R_PLANTAB,	C_RISETM-2,	"Rise Time");
	f_string (R_PLANTAB,	C_RISEAZ,	"Rise Az");
	f_string (R_PLANTAB,	C_TRANSTM-2,	"Trans Time");
	f_string (R_PLANTAB,	C_TRANSALT-1,	"Trans Alt");
	f_string (R_PLANTAB,	C_SETTM-1,	"Set Time");
	f_string (R_PLANTAB,	C_SETAZ,	"Set Az");
	f_string (R_PLANTAB,	C_TUP-1,	"Hours Up");
}

static
void alt3_labels()
{
	f_string (R_ALTM, C_ALTMV, "  Separations");

	alt_plnames();
	f_string (R_PLANTAB,	C_SUN,		" Sun");
	f_string (R_PLANTAB,	C_MOON,		"Moon");
	f_string (R_PLANTAB,	C_MERCURY,	"Merc");
	f_string (R_PLANTAB,	C_VENUS,	"Venus");
	f_string (R_PLANTAB,	C_MARS,		"Mars");
	f_string (R_PLANTAB,	C_JUPITER,	" Jup");
	f_string (R_PLANTAB,	C_SATURN,	" Sat");
	f_string (R_PLANTAB,	C_URANUS,	"Uranus");
	f_string (R_PLANTAB,	C_NEPTUNE,	" Nep");
	f_string (R_PLANTAB,	C_PLUTO,	"Pluto");
	f_string (R_PLANTAB,	C_OBJX,		"  X");
	f_string (R_PLANTAB,	C_OBJY,		"  Y");
}

/* print body info in first menu format */
static
void alt1_body (p, force, np)
int p;		/* which body, as in astro.h/screen.h defines */
int force;	/* whether to print for sure or only if things have changed */
Now *np;
{
	Sky sky;
	double as = plot_ison() || srch_ison() ? 0.0 : 60.0;
	int row = bodyrow[p];

	if (body_cir (p, as, np, &sky) || force) {
	    f_ra (row, C_RA, sky.s_ra);
	    f_angle (row, C_DEC, sky.s_dec);
	    if (sky.s_hlong != NOHELIO) {
		f_angle (row, C_HLONG, sky.s_hlong);
		if (p != SUN)
		    f_angle (row, C_HLAT, sky.s_hlat);
	    }

	    if (p == MOON) {
		/* distance is on km, show in miles */
		f_double (R_MOON, C_EDIST, "%6.0f", sky.s_edist/1.609344);
	    } else if (sky.s_edist > 0.0) {
		/* show distance in au */
		f_double (row, C_EDIST,(sky.s_edist>=10.0)?"%6.3f":"%6.4f",
								sky.s_edist);
	    }
	    if (sky.s_sdist > 0.0)
		f_double (row, C_SDIST, (sky.s_sdist>=9.99995)?"%6.3f":"%6.4f",
								sky.s_sdist);
	    if (p != SUN)
		f_double (row, C_ELONG, "%6.1f", sky.s_elong);
	    f_double (row, C_SIZE, sky.s_size >= 99.95 ?"%4.0f":"%4.1f",
								sky.s_size);
	    f_double (row, C_MAG, sky.s_mag <= -9.95 ? "%4.0f" : "%4.1f",
								sky.s_mag);
	    if (sky.s_sdist > 0.0) {
		/* some terminals scroll when write a char in low-right corner.
		 * TODO: is there a nicer way to handle this maybe?
		 */
		int col = row == NR ? C_PHASE - 1 : C_PHASE;
		/* would just do this if Turbo-C 2.0 "%?.0f" worked:
		 * f_double (row, col, "%3.0f", sky.s_phase);
		 */
		f_int (row, col, "%3d", sky.s_phase);
	    }
	}

	f_angle (row, C_AZ, sky.s_az);
	f_angle (row, C_ALT, sky.s_alt);
}

/* print body info in the second menu format */
static
void alt2_body (p, force, np)
int p;		/* which body, as in astro.h/screen.h defines */
int force;	/* whether to print for sure or only if things have changed */
Now *np;
{
	double ltr, lts, ltt, azr, azs, altt;
	int row = bodyrow[p];
	int status;
	double tmp;
	int today_tup = 0;

	/* always recalc OBJX and Y since we don't know it's the same object */
	if (!riset_cir (p, np, p==OBJX || p==OBJY, alt2_stdhzn?STDHZN:ADPHZN,
		&ltr, &lts, &ltt, &azr, &azs, &altt, &status) && !force)
	    return;

	alt_nobody (p);

	if (status & RS_ERROR) {
	    /* can not find where body is! */
	    f_string (row, C_RISETM, "?Error?");
	    return;
	}
	if (status & RS_CIRCUMPOLAR) {
	    /* body is up all day */
	    f_string (row, C_RISETM, "Circumpolar");
	    if (status & RS_NOTRANS)
		f_string (row, C_TRANSTM, "No transit");
	    else {
		f_mtime (row, C_TRANSTM, ltt);
		if (status & RS_2TRANS)
		    f_char (row, C_TRANSTM+5, '+');
		f_angle (row, C_TRANSALT, altt);
	    }
	    f_string (row, C_TUP, "24:00"); /*f_mtime() changes to 0:00 */
	    return;
	}
	if (status & RS_NEVERUP) {
	    /* body never up at all today */
	    f_string (row, C_RISETM, "Never up");
	    f_mtime (row, C_TUP, 0.0);
	    return;
	}

	if (status & RS_NORISE) {
	    /* object does not rise as such today */
	    f_string (row, C_RISETM, "Never rises");
	    ltr = 0.0; /* for TUP */
	    today_tup = 1;
	} else {
	    f_mtime (row, C_RISETM, ltr);
	    if (status & RS_2RISES) {
		/* object rises more than once today */
		f_char (row, C_RISETM+5, '+');
	    }
	    f_angle (row, C_RISEAZ, azr);
	}

	if (status & RS_NOTRANS)
	    f_string (row, C_TRANSTM, "No transit");
	else {
	    f_mtime (row, C_TRANSTM, ltt);
	    if (status & RS_2TRANS)
		f_char (row, C_TRANSTM+5, '+');
	    f_angle (row, C_TRANSALT, altt);
	}

	if (status & RS_NOSET) {
	    /* object does not set as such today */
	    f_string (row, C_SETTM, "Never sets");
	    lts = 24.0;	/* for TUP */
	    today_tup = 1;
	} else {
	    f_mtime (row, C_SETTM, lts);
	    if (status & RS_2SETS)
		f_char (row, C_SETTM+5, '+');
	    f_angle (row, C_SETAZ, azs);
	}

	tmp = lts - ltr;
	if (tmp < 0)
	    tmp = 24.0 + tmp;
	f_mtime (row, C_TUP, tmp);
	if (today_tup)
	    f_char (row, C_TUP+5, '+');
}

/* print body info in third menu format. this may be either the geocentric
 *   or topocentric angular separation between object p and each of the others.
 *   the latter, of course, includes effects of refraction and so can change
 *   quite rapidly near the time of each planets rise or set.
 * for now, we don't save old values so we always redo everything and ignore
 *  the "force" argument. this isn't that bad since body_cir() has memory and
 *   will avoid most computations as we hit them again in the lower triangle.
 * we are limited to only 5 columns per object. to make it fit, we display
 *   degrees:minutes if less than 100 degrees, otherwise just whole degrees.
 */
/*ARGSUSED*/
static
void alt3_body (p, force, np)
int p;		/* which body, as in astro.h/screen.h defines */
int force;	/* whether to print for sure or only if things have changed */
Now *np;
{
	int row = bodyrow[p];
	Sky skyp, skyq;
	double spy, cpy, px, *qx, *qy;
	int wantx = obj_ison(OBJX);
	int wanty = obj_ison(OBJY);
	double as = plot_ison() || srch_ison() ? 0.0 : 60.0;
	int q;

	(void) body_cir (p, as, np, &skyp);
	if (alt3_geoc) {
	    /* use ra for "x", dec for "y". */
	    spy = sin (skyp.s_dec);
	    cpy = cos (skyp.s_dec);
	    px = skyp.s_ra;
	    qx = &skyq.s_ra;
	    qy = &skyq.s_dec;
	} else {
	    /* use azimuth for "x", altitude for "y". */
	    spy = sin (skyp.s_alt);
	    cpy = cos (skyp.s_alt);
	    px = skyp.s_az;
	    qx = &skyq.s_az;
	    qy = &skyq.s_alt;
	}
	for (q = nxtbody(-1); q != -1; q = nxtbody(q))
	    if (q != p && (q != OBJX || wantx) && (q != OBJY || wanty)) {
		double sep, dsep;
		(void) body_cir (q, as, np, &skyq);
		sep = acos(spy*sin(*qy) + cpy*cos(*qy)*cos(px-*qx));
		dsep = raddeg(sep);
		if (dsep >= (100.0 - 1.0/60.0/2.0))
		    f_int (row, bodycol[q], "%5d:", dsep);
		else
		    f_angle (row, bodycol[q], sep);
	    }
}
