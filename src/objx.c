/* functions to save the user-definable objects, referred to as "x" and "y".
 * this way, once defined, the objects can be quieried for position just like
 * the other bodies, with obj_cir(). 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifdef VMS
#include <stdlib.h>
#endif
#include "astro.h"
#include "circum.h"
#include "screen.h"
#include "ephem.h"

static char *dbfile;			/* !0 if set by -d option */
static char dbfdef[] = "ephem.db"; 	/* default database file name */

/* structures to describe objects of various types.
 */
#define	MAXNM		16	/* longest allowed object name, inc \0 */
typedef struct {
    double m_m1, m_m2;	/* either g/k or H/G, depending on... */
    int m_whichm;	/* one of MAG_gk or MAG_HG */
} Mag;
typedef struct {
    double f_ra;	/* ra, rads, at given epoch */
    double f_dec;	/* dec, rads, at given epoch */
    double f_mag;	/* visual magnitude */
    double f_siz;	/* angular size, in arc seconds */
    double f_epoch;	/* the given epoch, as an mjd */
    char   f_name[MAXNM]; /* name */
} ObjF;			/* fixed object */
typedef struct {
    double e_inc;	/* inclination, degrees */
    double e_Om;	/* longitude of ascending node, degrees */
    double e_om;	/* argument of perihelion, degress */
    double e_a;		/* mean distance, aka, semi-maj axis, in AU */
    double e_n;		/* daily motion, degrees/day */
    double e_e;		/* eccentricity */
    double e_M;		/* mean anomaly, ie, degrees from perihelion at... */
    double e_cepoch;	/* epoch date (M reference), as an mjd */
    double e_epoch;	/* equinox year (inc/Om/om reference), as an mjd */
    Mag    e_mag;	/* magnitude */
    double e_siz;	/* angular size, in arc seconds at 1 AU */
    char   e_name[MAXNM]; /* name */
} ObjE;			/* object in heliocentric elliptical orbit */
typedef struct {
    double h_ep;	/* epoch of perihelion, as an mjd */
    double h_inc;	/* inclination, degs */
    double h_Om;	/* longitude of ascending node, degs */
    double h_om;	/* argument of perihelion, degs. */
    double h_e;		/* eccentricity */
    double h_qp;	/* perihelion distance, AU */
    double h_epoch;	/* equinox year (inc/Om/om reference), as an mjd */
    double h_g, h_k;	/* magnitude model coefficients */
    double h_siz;	/* angular size, in arc seconds at 1 AU */
    char   h_name[MAXNM]; /* name */
} ObjH;			/* object in heliocentric parabolic trajectory */
typedef struct {
    double p_ep;	/* epoch of perihelion, as an mjd */
    double p_inc;	/* inclination, degs */
    double p_qp;	/* perihelion distance, AU */
    double p_om;	/* argument of perihelion, degs. */
    double p_Om;	/* longitude of ascending node, degs */
    double p_epoch;	/* reference epoch, as an mjd */
    double p_g, p_k;	/* magnitude model coefficients */
    double p_siz;	/* angular size, in arc seconds at 1 AU */
    char   p_name[MAXNM]; /* name */
} ObjP;			/* object in heliocentric parabolic trajectory */

typedef struct {
    int  o_type;	/* current object type; see flags, below */
    int  o_on;		/* !=0 while current object is active */
    ObjF o_f;		/* the fixed object */
    ObjE o_e;		/* the elliptical orbit object */
    ObjH o_h;		/* the hyperbolic orbit object */
    ObjP o_p;		/* the parabolic orbit object */
} Obj;

/* o_type */
#define	FIXED		1
#define	ELLIPTICAL	2
#define	HYPERBOLIC	3
#define	PARABOLIC	4

/* m_whichm */
#define	MAG_HG		0	/* using 0 makes HG the initial default */
#define	MAG_gk		1

static Obj objx;
static Obj objy;

#define	DY	0		/* decimal year flag for set_year() */
#define	YMD	1		/* year/mon/day flag for set_year() */

static int nxt_db (char buf[], int blen, FILE *fp);
static void obj_dfixed (Obj *op, int ac, char *av[]);
static void obj_delliptical(Obj *op, int ac, char *av[]);
static void obj_dhyperbolic (Obj *op, int ac, char *av[]);
static void obj_dparabolic(Obj *op, int ac, char *av[]);
static int set_double (char *av[], int vn, char *pr, double *fp);
static int set_name (char *av[], char *np);
static int set_year (char *av[], int vn, char *pr, int type, double *yp);
static void crack_year (char *bp, double *p);
static int set_mag (char *av[], int vn, Mag *mp);

/* run when Objx or y is picked from menu.
 * we tell which by the planet code.
 * let op define object and turn it on and off.
 */
void obj_setup(p)
int p;
{
	static char *pr[6] = { /* leave a slot for "On"/"Off" */
	    "Fixed", "Elliptical", "Hyperbolic", "Parabolic", "Lookup"
	};
	int f;
	Obj *op;

	op = (p == OBJX) ? &objx : &objy;

    rechk:
	/* map o_type to popup choice.
	 */
	switch (op->o_type) {
	case FIXED: f = 0; break;
	case ELLIPTICAL: f = 1; break;
	case HYPERBOLIC: f = 2; break;
	case PARABOLIC: f = 3; break;
	default: f = 4; break;
	}

    ask:
	pr[5] = op->o_on ? "On" : "Off";
	switch (f = popup (pr, f, 6)) {
	case 0: obj_dfixed(op, 0, (char**)0); goto ask;
	case 1: obj_delliptical(op, 0, (char**)0); goto ask;
	case 2: obj_dhyperbolic(op, 0, (char**)0); goto ask;
	case 3: obj_dparabolic(op, 0, (char**)0); goto ask;
	case 4: if (obj_filelookup(p, (char *)0) == 0) obj_on(p); goto rechk;
	case 5: op->o_on ^= 1; break;
	}
}

/* turn "on" or "off" but don't forget facts about object the object.
 */
void obj_on (p)
int p;
{
	if (p == OBJX)
	    objx.o_on = 1;
	else
	    objy.o_on = 1;
}
void obj_off (p)
int p;
{
	if (p == OBJX)
	    objx.o_on = 0;
	else
	    objy.o_on = 0;
}

/* return true if object is now on, else 0.
 */
int obj_ison(p)
int p;
{
	return ((p == OBJX) ? objx.o_on : objy.o_on);
}

/* set an alternate database file name.
 * N.B. we assume the storage pointed to by name is permanent.
 */
void obj_setdbfilename (name)
char *name;
{
	dbfile = name;
}

/* fill in info about object x or y.
 * most arguments and conditions are the same as for plans().
 * only difference is that mag is already apparent, not absolute magnitude.
 * this is called by body_cir() for object x and y just like plans() is called
 * for the planets.
 */
void obj_cir (jd, p, lpd0, psi0, rp0, rho0, lam, bet, siz, mag)
double jd;	/* mjd now */
int p;		/* OBJX or OBJY */
double *lpd0;	/* heliocentric longitude, or NOHELIO  */
double *psi0;	/* heliocentric latitude, or 0 if *lpd0 set to NOHELIO */
double *rp0;	/* distance from the sun, or 0 */
double *rho0;	/* true distance from the Earth, or 0 */
double *lam;	/* apparent geocentric ecliptic longitude */
double *bet;	/* apparent geocentric ecliptic latitude */
double *siz;	/* angular size of object, arc seconds */
double *mag;	/* APPARENT magnitude */
{
	Obj *op = (p == OBJX) ? &objx : &objy;

	switch (op->o_type) {
	case FIXED: {
	    double xr, xd;
	    xr = op->o_f.f_ra;
	    xd = op->o_f.f_dec;
	    if (op->o_f.f_epoch != jd)
		precess (op->o_f.f_epoch, jd, &xr, &xd);
	    eq_ecl (jd, xr, xd, bet, lam);

	    *lpd0 = NOHELIO;
	    *psi0 = *rp0 = *rho0 = 0.0;
	    *mag = op->o_f.f_mag;
	    *siz = op->o_f.f_siz;
	    }
	    break;

	case ELLIPTICAL: {
	    /* this is basically the same code as pelement() and plans()
	     * combined and simplified for the special case of osculating
	     * (unperturbed) elements.
	     * inputs have been changed to match the Astronomical Almanac.
	     * we have added reduction of elements using reduce_elements().
	     */
	    double dt, lg, lsn, rsn;
	    double nu, ea;
	    double ma, rp, lo, slo, clo;
	    double inc, psi, spsi, cpsi;
	    double y, lpd, rpd, ll, rho, sll, cll;
	    double om;		/* arg of perihelion */
	    double Om;		/* long of ascending node. */
	    double e;
	    int pass;

	    dt = 0;
	    sunpos (jd, &lsn, &rsn);
	    lg = lsn + PI;
	    e = op->o_e.e_e;

	    for (pass = 0; pass < 2; pass++) {

		reduce_elements (op->o_e.e_epoch, jd-dt, degrad(op->o_e.e_inc),
				degrad (op->o_e.e_om), degrad (op->o_e.e_Om),
				&inc, &om, &Om);

		ma = degrad (op->o_e.e_M
				+ (jd - op->o_e.e_cepoch - dt) * op->o_e.e_n);
		anomaly (ma, e, &nu, &ea);
		rp = op->o_e.e_a * (1-e*e) / (1+e*cos(nu));
		lo = nu + om;
		slo = sin(lo);
		clo = cos(lo);
		spsi = slo*sin(inc);
		y = slo*cos(inc);
		psi = asin(spsi);
		lpd = atan(y/clo)+Om;
		if (clo<0) lpd += PI;
		range (&lpd, 2*PI);
		cpsi = cos(psi);
		rpd = rp*cpsi;
		ll = lpd-lg;
		rho = sqrt(rsn*rsn+rp*rp-2*rsn*rp*cpsi*cos(ll));
		dt = rho*5.775518e-3;	/* light travel time, in days */
		if (pass == 0) {
		    *lpd0 = lpd;
		    *psi0 = psi;
		    *rp0 = rp;
		    *rho0 = rho;
		}
	    }

	    sll = sin(ll);
	    cll = cos(ll);
	    if (rpd < rsn)
		*lam = atan(-1*rpd*sll/(rsn-rpd*cll))+lg+PI;
	    else
		*lam = atan(rsn*sll/(rpd-rsn*cll))+lpd;
	    range (lam, 2*PI);
	    *bet = atan(rpd*spsi*sin(*lam-lpd)/(cpsi*rsn*sll));

	    if (op->o_e.e_mag.m_whichm == MAG_HG) {
		/* the H and G parameters from the Astro. Almanac.
		 */
		double psi_t, Psi_1, Psi_2, beta;
		beta = acos((rp*rp + rho*rho - rsn*rsn)/ (2*rp*rho));
		psi_t = exp(log(tan(beta/2.0))*0.63);
		Psi_1 = exp(-3.33*psi_t);
		psi_t = exp(log(tan(beta/2.0))*1.22);
		Psi_2 = exp(-1.87*psi_t);
		*mag = op->o_e.e_mag.m_m1 + 5.0*log10(rp*rho)
		    - 2.5*log10((1-op->o_e.e_mag.m_m2)*Psi_1
		    + op->o_e.e_mag.m_m2*Psi_2);
	    } else {
		/* the g/k model of comets */
		*mag = op->o_e.e_mag.m_m1 + 5*log10(rho)
					+ 2.5*op->o_e.e_mag.m_m2*log10(rp);
	    }
	    *siz = op->o_e.e_siz / rho;
	    }
	    break;

	case HYPERBOLIC: {
	    double dt, lg, lsn, rsn;
	    double nu, ea;
	    double ma, rp, lo, slo, clo;
	    double inc, psi, spsi, cpsi;
	    double y, lpd, rpd, ll, rho, sll, cll;
	    double om;		/* arg of perihelion */
	    double Om;		/* long of ascending node. */
	    double e;
	    double a, n;	/* semi-major axis, mean daily motion */
	    int pass;

	    dt = 0;
	    sunpos (jd, &lsn, &rsn);
	    lg = lsn + PI;
	    e = op->o_h.h_e;
	    a = op->o_h.h_qp/(e - 1.0);
	    n = .98563/sqrt(a*a*a);

	    for (pass = 0; pass < 2; pass++) {

		reduce_elements (op->o_h.h_epoch, jd-dt, degrad(op->o_h.h_inc),
				degrad (op->o_h.h_om), degrad (op->o_h.h_Om),
				&inc, &om, &Om);

		ma = degrad ((jd - op->o_h.h_ep - dt) * n);
		anomaly (ma, e, &nu, &ea);
		rp = a * (e*e-1.0) / (1.0+e*cos(nu));
		lo = nu + om;
		slo = sin(lo);
		clo = cos(lo);
		spsi = slo*sin(inc);
		y = slo*cos(inc);
		psi = asin(spsi);
		lpd = atan(y/clo)+Om;
		if (clo<0) lpd += PI;
		range (&lpd, 2*PI);
		cpsi = cos(psi);
		rpd = rp*cpsi;
		ll = lpd-lg;
		rho = sqrt(rsn*rsn+rp*rp-2*rsn*rp*cpsi*cos(ll));
		dt = rho*5.775518e-3;	/* light travel time, in days */
		if (pass == 0) {
		    *lpd0 = lpd;
		    *psi0 = psi;
		    *rp0 = rp;
		    *rho0 = rho;
		}
	    }

	    sll = sin(ll);
	    cll = cos(ll);
	    if (rpd < rsn)
		*lam = atan(-1*rpd*sll/(rsn-rpd*cll))+lg+PI;
	    else
		*lam = atan(rsn*sll/(rpd-rsn*cll))+lpd;
	    range (lam, 2*PI);
	    *bet = atan(rpd*spsi*sin(*lam-lpd)/(cpsi*rsn*sll));

	    *mag = op->o_h.h_g + 5*log10(rho) + 2.5*op->o_h.h_k*log10(rp);
	    *siz = op->o_h.h_siz / rho;
	    }
	    break;

	case PARABOLIC: {
	    double inc, om, Om;
	    double lpd, psi, rp, rho;
	    double dt;
	    int pass;

	    /* two passes to correct lam and bet for light travel time. */
	    dt = 0.0;
	    for (pass = 0; pass < 2; pass++) {
		reduce_elements (op->o_p.p_epoch, jd-dt, degrad(op->o_p.p_inc),
		    degrad(op->o_p.p_om), degrad(op->o_p.p_Om), &inc, &om, &Om);
		comet (jd-dt, op->o_p.p_ep, inc, om, op->o_p.p_qp, Om,
					&lpd, &psi, &rp, &rho, lam, bet);
		if (pass == 0) {
		    *lpd0 = lpd;
		    *psi0 = psi;
		    *rp0 = rp;
		    *rho0 = rho;
		}
		dt = rho*5.775518e-3;	/* au to light-days */
	    }
	    *mag = op->o_p.p_g + 5*log10(rho) + 2.5*op->o_p.p_k*log10(rp);
	    *siz = op->o_p.p_siz / rho;
	    }
	    break;

	default:
	    f_msg ((p == OBJX) ? "Obj X is not defined"
			       : "Obj Y is not defined");
	    break;
	}
}

/* define obj based on the ephem.db line, s.
 * p is one of OBJX or OBJY.
 * format: name,type,[other fields, as per corresponding ObjX typedef]
 * N.B. we replace all ',' within s with '\0' IN PLACE.
 * return 0 if ok, else print reason why not with f_msg() and return -1.
 */
int obj_define (p, s)
int p;	/* OBJX or OBJY */
char *s;
{
#define	MAXARGS	20
	char *av[MAXARGS];	/* point to each field for easy reference */
	char c;
	int ac;
	Obj *op = (p == OBJX) ? &objx : &objy;

	/* parse into comma separated fields */
	ac = 0;
	av[0] = s;
	do {
	    c = *s++;
	    if (c == ',' || c == '\0') {
		s[-1] = '\0';
		av[++ac] = s;
	    }
	} while (c);

	if (ac < 2) {
	    char buf[NC];
	    if (ac > 0)
		(void) sprintf (buf, "No type for Object %s", av[0]);
	    else
		(void) sprintf (buf, "No fields in %s", s);
	    f_msg (buf);
	    return (-1);
	}

	/* switch out on type of object - the second field */
	switch (av[1][0]) {
	case 'f':
	    if (ac != 6 && ac != 7) {
		char buf[NC];
		(void) sprintf(buf,
		    "Need ra,dec,mag,D[,siz] for fixed object %s", av[0]);
		f_msg (buf);
		return (-1);
	    }
	    obj_dfixed (op, ac, av);
	    break;

	case 'e':
	    if (ac != 13 && ac != 14) {
		char buf[NC];
		(void) sprintf (buf,
		    "Need i,O,o,a,n,e,M,E,D,H/g,G/k[,siz] for elliptical object %s",
								    av[0]);
		f_msg (buf);
		return (-1);
	    }
	    obj_delliptical (op, ac, av);
	    break;

	case 'h':
	    if (ac != 11 && ac != 12) {
		char buf[NC];
		(void) sprintf (buf,
		    "Need T,i,O,o,e,q,D,g,k[,siz] for hyperbolic object %s", av[0]);
		f_msg (buf);
		return (-1);
	    }
	    obj_dhyperbolic (op, ac, av);
	    break;

	case 'p':
	    if (ac != 10 && ac != 11) {
		char buf[NC];
		(void) sprintf (buf,
		    "Need T,i,o,q,O,D,g,k[,siz] for parabolic object %s", av[0]);
		f_msg (buf);
		return (-1);
	    }
	    obj_dparabolic (op, ac, av);
	    break;

	default: {
		char buf[NC];
		(void) sprintf (buf, "Unknown type for Object %s: %s",
								av[0], av[1]);
		f_msg (buf);
		return (-1);
	    }
	}

	return (0);
}

/* if name, then look it up in the ephem database file and set p.
 * else display a table of all objects and let op pick one.
 * p is either OBJX or OBJY.
 * if -d was used use it; else if EPHEMDB env set use it, else use default.
 * return 0 if successfully set object p, else -1.
 */
int obj_filelookup (p, name)
int p;			/* OBJX or OBJY */
char *name;
{
/* redefine RCTN,NTR,NTC for column-major order if you prefer */
#define	NLR		(NR-1)		/* number of rows of names.
					 * leave 1 for prompt
					 */
#define	LCW		9		/* screen columns per name */
#define	NLC		9		/* total number of name columns */
#define	NL		(NLR*NLC)	/* total number of names per screen */
#define	RCTN(r,c)	((r)*NLC+(c))	/* row/col to index */
#define	NTR(n)		((n)/NLC)	/* index to row */
#define	NTC(n)		((n)%NLC)	/* index to col (0 based) */
					/* N.B. all these are 0-based */
	static char prompt[] =
		    "RETURN to select, p/n for previous/next page, q to quit";
	FILE *fp;
	char *fn;
	int i, pgn;	/* index on current screen, current page number */
	int r, c;
	char buf[160];	/* longer than any one database line */
	char pb[NC];	/* prompt buffer */
	int readahd;	/* 1 if buffer set from previous loop */
	int choice;	/* index to selection; -1 until set */
	int roaming;	/* 1 while just roaming around screen */
	int abandon;	/* 1 if decide to not pick afterall */

	/* open the database file */
	if (dbfile)
	    fn = dbfile;
	else {
	    fn = getenv ("EPHEMDB");
	    if (!fn)
		fn = dbfdef;
	}
	fp = fopen (fn, "r");
	if (!fp) {
	    (void) sprintf (buf, "Can not open database file %s", fn);
	    f_msg(buf);
	    return(-1);
	}

	/* name is specified so just search for it without any op interaction */
	if (name) {
	    int nl = strlen (name);
	    int ret = 0;
	    while (nxt_db(buf, sizeof(buf), fp) == 0 && strncmp(buf, name, nl))
		continue;
	    if (feof(fp)) {
		(void) sprintf (buf, "Object %s not found", name);
		f_msg (buf);
		ret = -1;
	    } else
		(void) obj_define (p, buf);
	    (void) fclose (fp);
	    return (ret);
	}
	    
	pgn = 0;
	readahd = 0;
	choice = -1;
	abandon = 0;

	/* continue until a choice is made or op abandons the attempt */
	do {
	    /* put up next screen full of names.
	     * leave top row open for messages.
	     */
	    c_erase();
	    for (i = 0; i < NL; )
		if (readahd || nxt_db (buf, sizeof(buf), fp) == 0) {
		    char objname[LCW];
		    int ii;
		    for (ii = 0; ii < sizeof(objname)-1; ii++)
			if ((objname[ii] = buf[ii]) == ',')
			    break;
		    objname[ii] = '\0';
		    if (i == NL-1)
			objname[LCW-2] = '\0'; /* avoid scroll in low-r corner*/
		    f_string (NTR(i)+2, NTC(i)*LCW+1, objname);
		    i++;
		    readahd = 0;
		} else
		    break;

	    /* read another to check for eof. if valid, set readahd for next
	     * time.
	     */
	    if (nxt_db (buf, sizeof(buf), fp) == 0)
		readahd = 1;

	    /* let op pick one. set cursor on first one.
	     * remember these r/c are 0-based, but c_pos() is 1-based 
	     */
	    (void) sprintf (pb, "Page %d%s. %s", pgn+1,
					    feof(fp) ? " (last)" : "", prompt);
	    f_prompt(pb);
	    r = c = 0;
	    roaming = 1;
	    do {
		c_pos (r+2, c*LCW+1);
		switch (read_char()) {
		case 'h': /* left */
		    if (c == 0) c = NLC;
		    c -= 1;
		    if (RCTN(r,c) >= i)
			c = NTC(i-1);
		    break;
		case 'j': /* down */
		    if (++r == NLR) r = 0;
		    if (RCTN(r,c) >= i)
			r = 0;
		    break;
		case 'k': /* up */
		    if (r == 0) r = NLR;
		    r -= 1;
		    while (RCTN(r,c) >= i)
			r -= 1;
		    break;
		case 'l': /* right */
		    if (++c == NLC) c = 0;
		    if (RCTN(r,c) >= i)
			c = 0;
		    break;
		case REDRAW:
		    /* start over and skip over prior pages' entries */
		    rewind(fp);
		    for (i = 0; i < NL*pgn; i++)
			(void) nxt_db (buf, sizeof(buf), fp);
		    readahd = 0;
		    roaming = 0;
		    break;
		case 'p':
		    /* if not at first page, start over and skip back one
		     * pages' entries
		     */
		    if (pgn > 0) {
			rewind(fp);
			pgn--;
			for (i = 0; i < NL*pgn; i++)
			    (void) nxt_db (buf, sizeof(buf), fp);
			readahd = 0;
			roaming = 0;
		    }
		    break;
		case 'n':
		    /* if not already at eof, we can go ahead another page */
		    if (!feof (fp)) {
			pgn++;
			roaming = 0;
		    }
		    break;
		case END:
		    abandon = 1;
		    roaming = 0;
		    break;
		case ' ': case '\r':
		    choice = NL*pgn + RCTN(r,c);
		    roaming = 0;
		    break;
		}
	    } while (roaming);
	} while (choice < 0 && !abandon);

	if (choice >= 0) {
	    /* skip first choice entries; selection is the next one */
	    (void) rewind (fp);
	    for (i = 0; i < choice; i++)
		(void) nxt_db (buf, sizeof(buf), fp);
	    (void) nxt_db (buf, sizeof(buf), fp);
	    (void) obj_define (p, buf);
	}
	(void) fclose (fp);
	redraw_screen (2);
	return (choice >= 0 ? 0 : -1);
}

/* read database file fp and put next valid entry into buf.
 * return 0 if ok, else -1
 */
static
int nxt_db (buf, blen, fp)
char buf[];
int blen;
FILE *fp;
{
	char s;
	while (1) {
	    if (fgets (buf, blen, fp) == 0)
		return (-1);
	    s = buf[0];
	    if (isalpha(s))
		return (0);
	}
}

/* define a fixed object.
 * args in av, in order, are name, type, ra, dec, magnitude, reference epoch
 *   and optional angular size.
 * if av then it is a list of strings to use for each parameter, else must
 * ask for each (but type). the av option is for cracking the ephem.db line.
 * if asking show current settings and leave unchanged if hit RETURN.
 * END aborts without making any more changes.
 * o_type is set to FIXED.
 * N.B. we don't error check av in any way, not even for length.
 */
static
void obj_dfixed (op, ac, av)
Obj *op;
int ac;
char *av[];
{
	char buf[NC];
	char *bp;
	int sts;

	op->o_type = FIXED;

	if (set_name (av, op->o_f.f_name) < 0)
	    return;

	if (av) {
	    bp = av[2];
	    sts = 1;
	} else {
	    static char p[] = "RA (h:m:s): (";
	    f_prompt (p);
	    f_ra (R_PROMPT, C_PROMPT+sizeof(p)-1, op->o_f.f_ra);
	    (void) printf (") ");
	    sts = read_line (buf, 8+1);
	    if (sts < 0)
		return;
	    bp = buf;
	}
	if (sts > 0) {
	    int h, m, s;
	    f_dec_sexsign (radhr(op->o_f.f_ra), &h, &m, &s);
	    f_sscansex (bp, &h, &m, &s);
	    sex_dec (h, m, s, &op->o_f.f_ra);
	    op->o_f.f_ra = hrrad(op->o_f.f_ra);
	}

	if (av) {
	    bp = av[3];
	    sts = 1;
	} else {
	    static char p[] = "Dec (d:m:s): (";
	    f_prompt (p);
	    f_gangle (R_PROMPT, C_PROMPT+sizeof(p)-1, op->o_f.f_dec);
	    (void) printf (") ");
	    sts = read_line (buf, 9+1);
	    if (sts < 0)
		return;
	    bp = buf;
	}
	if (sts > 0) {
	    int dg, m, s;
	    f_dec_sexsign (raddeg(op->o_f.f_dec), &dg, &m, &s);
	    f_sscansex (bp, &dg, &m, &s);
	    sex_dec (dg, m, s, &op->o_f.f_dec);
	    op->o_f.f_dec = degrad(op->o_f.f_dec);
	}

	if (set_double (av, 4, "Magnitude: ", &op->o_f.f_mag) < 0)
	    return;

	if (set_year (av, 5,"Reference epoch (UT Date, m/d.d/y or year.d): ",
						    DY, &op->o_f.f_epoch) < 0)
	    return;

	if (ac == 7 || !av)
	    (void) set_double (av, 6, "Angular Size: ", &op->o_f.f_siz);
	else
	    op->o_f.f_siz = 0.0;

}

/* define an object in an elliptical heliocentric orbit.
 * 13 or 14 args in av, in order, are name, type, inclination, longitude of
 *   ascending node, argument of perihelion, mean distance (aka semi-major
 *   axis), daily motion, eccentricity, mean anomaly (ie, degrees from
 *   perihelion), epoch date (ie, time of the mean anomaly value), equinox year
 *   (ie, time of inc/lon/aop), two magnitude coefficients and optional size.
 * the mag may be H/G or g/k model, set by leading g or H (use H/G if none).
 * if av then it is a list of strings to use for each parameter, else must
 * ask for each. the av option is for cracking the ephem.db line.
 * if asking show current settings and leave unchanged if hit RETURN.
 * END aborts without making any more changes.
 * o_type is set to ELLIPTICAL.
 * N.B. we don't error check av in any way, not even for length.
 */
static
void obj_delliptical(op, ac, av)
Obj *op;
int ac;
char *av[];
{
	op->o_type = ELLIPTICAL;

	if (set_name (av, op->o_e.e_name) < 0)
	    return;

	if (set_double (av, 2, "Inclination (degs):", &op->o_e.e_inc) < 0)
	    return;

	if (set_double (av, 3, "Longitude of ascending node (degs):",
				&op->o_e.e_Om) < 0)
	    return;

	if (set_double (av, 4, "Argument of Perihelion (degs):",
				&op->o_e.e_om) < 0)
	    return;

	if (set_double (av, 5, "Mean distance (AU):", &op->o_e.e_a) < 0)
	    return;

	if (set_double (av, 6, "Daily motion (degs/day):", &op->o_e.e_n) < 0)
	    return;

	if (set_double (av, 7, "Eccentricity:", &op->o_e.e_e) < 0)
	    return;

	if (set_double (av, 8, "Mean anomaly (degs):", &op->o_e.e_M) < 0)
	    return;

	if (set_year (av, 9, "Epoch date (UT Date, m/d.d/y or year.d): ",
						    YMD, &op->o_e.e_cepoch) < 0)
	    return;

	if (set_year (av, 10, "Equinox year (UT Date, m/d.d/y or year.d): ",
						    DY, &op->o_e.e_epoch) < 0)
	    return;

	if (set_mag (av, 11, &op->o_e.e_mag) < 0)
	    return;

	if (ac == 14 || !av)
	    (void) set_double (av, 13, "Angular Size @ 1 AU: ", &op->o_e.e_siz);
	else
	    op->o_e.e_siz = 0.0;

}

/* define an object in heliocentric hyperbolic orbit.
 * 11 or 12 args in av, in order, are name, type, epoch of perihelion,
 *   inclination, longitude of ascending node, argument of perihelion,
 *    eccentricity, perihelion distance, reference epoch, absolute magnitude
 *    and luminosity index, and optional size.
 * if av then it is a list of strings to use for each parameter, else must
 * ask for each. the av option is for cracking the ephem.db line.
 * if asking show current settings and leave unchanged if hit RETURN.
 * END aborts without making any more changes.
 * o_type is set to HYPERBOLIC.
 * N.B. we don't error check av in any way, not even for length.
 */
static
void obj_dhyperbolic (op, ac, av)
Obj *op;
int ac;
char *av[];
{
	op->o_type = HYPERBOLIC;

	if (set_name (av, op->o_h.h_name) < 0)
	    return;

	if (set_year(av,2,"Epoch of perihelion (UT Date, m/d.d/y or year.d): ",
						    YMD, &op->o_h.h_ep) < 0)
	    return;

	if (set_double (av, 3, "Inclination (degs):", &op->o_h.h_inc) < 0)
	    return;

	if (set_double (av, 4,
		"Longitude of ascending node (degs):", &op->o_h.h_Om) < 0)
	    return;

	if (set_double(av,5,"Argument of perihelion (degs):", &op->o_h.h_om) <0)
	    return;

	if (set_double (av, 6, "Eccentricity:", &op->o_h.h_e) < 0)
	    return;

	if (set_double (av, 7, "Perihelion distance (AU):", &op->o_h.h_qp) < 0)
	    return;

	if (set_year (av, 8, "Reference epoch (UT Date, m/d.d/y or year.d): ",
						    DY, &op->o_h.h_epoch) < 0)
	    return;

	if (set_double (av, 9, "g:", &op->o_h.h_g) < 0)
	    return;

	if (set_double (av, 10, "k:", &op->o_h.h_k) < 0)
	    return;

	if (ac == 12 || !av)
	    (void) set_double (av, 11, "Angular Size @ 1 AU: ", &op->o_h.h_siz);
	else
	    op->o_h.h_siz = 0.0;
}

/* define an object in heliocentric parabolic orbit.
 * 10 or 11 args in av, in order, are name, type, epoch of perihelion,
 *   inclination, argument of perihelion, perihelion distance, longitude of
 *   ascending node, reference epoch, absolute magnitude and luminosity index,
 *   and optional size.
 * if av then it is a list of strings to use for each parameter, else must
 * ask for each. the av option is for cracking the ephem.db line.
 * if asking show current settings and leave unchanged if hit RETURN.
 * END aborts without making any more changes.
 * o_type is set to PARABOLIC.
 * N.B. we don't error check av in any way, not even for length.
 */
static
void obj_dparabolic(op, ac, av)
Obj *op;
int ac;
char *av[];
{
	op->o_type = PARABOLIC;

	if (set_name (av, op->o_p.p_name) < 0)
	    return;

	if (set_year(av,2,"Epoch of perihelion (UT Date, m/d.d/y or year.d): ",
						    YMD, &op->o_p.p_ep) < 0)
	    return;

	if (set_double (av, 3, "Inclination (degs):", &op->o_p.p_inc) < 0)
	    return;

	if (set_double(av,4,"Argument of perihelion (degs):", &op->o_p.p_om) <0)
	    return;

	if (set_double (av, 5, "Perihelion distance (AU):", &op->o_p.p_qp) < 0)
	    return;

	if (set_double (av, 6,
		"Longitude of ascending node (degs):", &op->o_p.p_Om) < 0)
	    return;

	if (set_year (av, 7, "Reference epoch (UT Date, m/d.d/y or year.d): ",
						    DY, &op->o_p.p_epoch) < 0)
	    return;

	if (set_double (av, 8, "g:", &op->o_p.p_g) < 0)
	    return;

	if (set_double (av, 9, "k:", &op->o_p.p_k) < 0)
	    return;

	if (ac == 11 || !av)
	    (void) set_double (av, 10, "Angular Size @ 1 AU: ", &op->o_p.p_siz);
	else
	    op->o_p.p_siz = 0.0;
}

static
int set_double (av, vn, pr, fp)
char *av[];	/* arg list */
int vn;		/* which arg */
char *pr;	/* prompt */
double *fp;	/* ptr to double to be set */
{
	int sts;
	char buf[NC];
	char *bp;

	if (av) {
	    bp = av[vn];
	    sts = 1;
	} else {
	    f_prompt (pr);
	    f_double (R_PROMPT, C_PROMPT+1+strlen(pr), "(%g) ", *fp);
	    sts = read_line (buf, 20);
	    if (sts < 0)
		return (-1);
	    bp = buf;
	}
	if (sts > 0)
	    *fp = atof (bp);
	return (0);
}

static
int set_name (av, np)
char *av[];	/* arg list */
char *np;	/* name to be set */
{
	int sts;
	char buf[NC];
	char *bp;

	if (av) {
	    bp = av[0];
	    sts = 1;
	} else {
	    (void) sprintf (buf, "Name: (%s) ", np);
	    f_prompt (buf);
	    sts = read_line (buf, MAXNM-1);
	    if (sts < 0)
		return (-1);
	    bp = buf;
	}
	if (sts > 0)
	    (void) strcpy (np, bp);
	return (0);
}

static
int set_year (av, vn, pr, type, yp)
char *av[];	/* arg list */
int vn;		/* which arg */
char *pr;	/* prompt */
int type;	/* display type: YMD or DY */
double *yp;	/* ptr to year to be set */
{
	int sts;
	char buf[NC];
	char *bp;

	if (av) {
	    bp = av[vn];
	    sts = 1;
	} else {
	    f_prompt (pr);
	    if (type == DY) {
		double y;
		mjd_year (*yp, &y);
		(void) printf ("(%g) ", y);
	    } else {
		int m, y;
		double d;
		mjd_cal (*yp, &m, &d, &y);
		(void) printf ("(%d/%g/%d) ", m, d, y);
	    }
	    sts = read_line (buf, 20);
	    if (sts < 0)
		return (-1);
	    bp = buf;
	}
	if (sts > 0)
	    crack_year (bp, yp);
	return (0);
}

/* given either a decimal year (xxxx. something) or a calendar (x/x/x)
 * convert it to an mjd and store it at *p;
 */
static
void crack_year (bp, p)
char *bp;
double *p;
{
	if (decimal_year(bp)) {
	    double y = atof (bp);
	    year_mjd (y, p);
	} else {
	    int m, y;
	    double d;
	    mjd_cal (*p, &m, &d, &y);	/* init with current */
	    f_sscandate (bp, &m, &d, &y);
	    cal_mjd (m, d, y, p);
	}
}

/* read next two args from av and load the magnitude members m_m1 and m_m2.
 * also set m_whichm to default if this is from the .db file, ie, if av!=0.
 * #,#     -> model is unchanged
 * g#,[k]# -> g/k
 * H#,[G]# -> H/G
 */
static
int set_mag (av, vn, mp)
char *av[];	/* arg list */
int vn;		/* which arg. we use av[vn] and av[vn+1] */
Mag *mp;
{
	int sts;
	char buf[NC];
	char *bp;

	if (av) {
	    mp->m_whichm = MAG_HG;	/* always the default for the db file */
	    bp = av[vn];
	    sts = 1;
	} else {
	    /* show both the value and the type of the first mag param,
	     * as well as a hint as to how to set the type if desired.
	     */
	    (void) sprintf (buf, "%c: (%g) (g# H# or #) ",
				mp->m_whichm == MAG_HG ? 'H' : 'g', mp->m_m1);
	    f_prompt (buf);
	    sts = read_line (buf, 9);
	    if (sts < 0)
		return (-1);
	    bp = buf;
	}
	if (sts > 0) {
	    switch (bp[0]) {
	    case 'g':
		mp->m_whichm = MAG_gk;
		bp++;
		break;
	    case 'H':
		mp->m_whichm = MAG_HG;
		bp++;
	    default:
		/* leave type unchanged if no prefix */
		break;
	    }
	    mp->m_m1 = atof (bp);
	}

	if (av) {
	    bp = av[vn+1];
	    sts = 1;
	} else {
	    /* can't change the type in the second param */
	    (void) sprintf (buf, "%c: (%g) ",
				mp->m_whichm == MAG_HG ? 'G' : 'k', mp->m_m2);
	    f_prompt (buf);
	    sts = read_line (buf, 9);
	    if (sts < 0)
		return (-1);
	    bp = buf;
	}
	if (sts > 0) {
	    int ok = 0;
	    switch (bp[0]) {
	    case 'k':
		if (mp->m_whichm == MAG_gk) {
		    bp++;
		    ok = 1;
		}
		break;
	    case 'G':
		if (mp->m_whichm == MAG_HG) {
		    bp++;
		    ok = 1;
		}
		break;
	    default:
		ok = 1;
		break;
	    }
	    if (ok)
		mp->m_m2 = atof (bp);
	    else {
		f_msg ("Can't switch magnitude models at second parameter.");
		return (-1);
	    }
	}
	return (0);
}
