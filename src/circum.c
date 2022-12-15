/* fill in a Sky struct with all we know about each object.
 *(the user defined objects are in obj.c)
 */

#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "circum.h"
#include "screen.h"	/* just for SUN and MOON */
#include "ephem.h"

static int about_now (Now *n1, Now *n2, double dt);

/* find body p's circumstances now.
 * to save some time the caller may specify a desired accuracy, in arc seconds.
 * if, based on its mean motion, it would not have moved this much since the
 * last time we were called we only recompute altitude and azimuth and avoid
 * recomputing the planet's heliocentric position. use 0.0 for best possible.
 * we always recompute the user-defined objects' position regardless.
 * return 0 if only alt/az changes, else 1 if all other stuff updated too.
 * N.B: values are for opposition, ie, at fastest retrograde.
 */
int body_cir (p, as, np, sp)
int p;
double as;
Now *np;
Sky *sp;
{
	typedef struct {
	    double l_dpas;	/* mean days per arc second */
	    Now l_now;		/* when l_sky was found */
	    double l_ra, l_dec;	/* the eod, ie, unprecessed, ra/dec values */
	    Sky l_sky;
	} Last;
	/* must be in same order as the astro.h object #define's */
	static Last last[8] = {
	    {.000068, {NOMJD}},	/* mercury */
	    {.00017, {NOMJD}},	/* venus */
	    {.00015, {NOMJD}},	/* mars */
	    {.0012, {NOMJD}},	/* jupiter */
	    {.0024, {NOMJD}},	/* saturn */
	    {.0051, {NOMJD}},	/* uranus */
	    {.0081, {NOMJD}},	/* neptune */
	    {.011, {NOMJD}}	/* pluto */
	};
	Last objxlast, objylast;
	double lst, alt, az;
	double ehp, ha, dec;	/* ehp: angular dia of earth from body */
	Last *lp;
	int new;

	switch (p) {
	case SUN: return (sun_cir (as, np, sp));
	case MOON: return (moon_cir (as, np, sp));
	case OBJX: lp = &objxlast; break;
	case OBJY: lp = &objylast; break;
	default: lp = last + p; break;
	}

	/* if less than l_every days from last time for this planet
	 * just redo alt/az.
	 * ALWAYS redo objects x and y.
	 */
	if (p != OBJX && p != OBJY && same_cir (np, &lp->l_now)
		      && about_now (np, &lp->l_now, as*lp->l_dpas)) {
	    *sp = lp->l_sky;
	    new = 0;
	} else {
	    double lpd0, psi0;	/* heliocentric ecliptic long and lat */
	    double rp0;		/* dist from sun */
	    double rho0;	/* dist from earth */
	    double lam, bet;	/* geocentric ecliptic long and lat */
	    double dia, mag;	/* angular diameter at 1 AU and magnitude */
	    double lsn, rsn;	/* true geoc lng of sun, dist from sn to earth*/
	    double el;	/* elongation */
	    double f;   /* phase from earth */

	    lp->l_now = *np;
	    sunpos (mjd, &lsn, &rsn);
	    if (p == OBJX || p == OBJY)
		obj_cir(mjd, p, &lpd0, &psi0, &rp0, &rho0, &lam, &bet,
						&sp->s_size, &sp->s_mag);
	    else {
		double deps, dpsi;
		double a;
		plans(mjd, p, &lpd0, &psi0, &rp0, &rho0, &lam, &bet, &dia,&mag);
		nutation (mjd, &deps, &dpsi);	/* correct for nutation */
		lam += dpsi;
		a = lsn-lam;			/* and 20.4" aberation */
		lam -= degrad(20.4/3600)*cos(a)/cos(bet);
		bet -= degrad(20.4/3600)*sin(a)*sin(bet);
	    }

	    ecl_eq (mjd, bet, lam, &lp->l_ra, &lp->l_dec);

	    sp->s_ra = lp->l_ra;
	    sp->s_dec = lp->l_dec;
	    if (epoch != EOD)
		precess (mjd, epoch, &sp->s_ra, &sp->s_dec);
	    sp->s_edist = rho0;
	    sp->s_sdist = rp0;
	    elongation (lam, bet, lsn, &el);
	    el = raddeg(el);
	    sp->s_elong = el;
	    f = (rp0 > 0.0)
		? 0.25 * (((rp0+rho0)*(rp0+rho0) - rsn*rsn)/(rp0*rho0)) : 0.0;
	    sp->s_phase = f*100.0; /* percent */
	    if (p != OBJX && p != OBJY) {
		sp->s_size = dia/rho0;
		sp->s_mag = mag + 5.0*log(rp0*rho0/sqrt(f))/log(10.0);
	    }
	    sp->s_hlong = lpd0;
	    sp->s_hlat = psi0;
	    new = 1;
	}

	/* alt, az; correct for parallax and refraction; use eod ra/dec */
	now_lst (np, &lst);
	ha = hrrad(lst) - lp->l_ra;
	if (sp->s_edist > 0.0) {
	    ehp = (2.0*6378.0/146.0e6) / sp->s_edist;
	    ta_par (ha, lp->l_dec, lat, height, ehp, &ha, &dec);
	} else
	    dec = lp->l_dec;
	hadec_aa (lat, ha, dec, &alt, &az);
	refract (pressure, temp, alt, &alt);
	sp->s_alt = alt;
	sp->s_az = az;
	lp->l_sky = *sp;
	return (new);
}

/* find local times when sun is 18 degrees below horizon.
 * return 0 if just returned same stuff as previous call, else 1 if new.
 */
int twilight_cir (np, dawn, dusk, status)
Now *np;
double *dawn, *dusk;
int *status;
{
	static Now last_now = {NOMJD};
	static double last_dawn, last_dusk;
	static int last_status;
	int new;

	if (same_cir (np, &last_now) && same_lday (np, &last_now)) {
	    *dawn = last_dawn;
	    *dusk = last_dusk;
	    *status = last_status;
	    new = 0;
	} else {
	    double x;
	    (void) riset_cir (SUN,np,0,TWILIGHT,dawn,dusk,&x,&x,&x,&x,status);
	    last_dawn = *dawn;
	    last_dusk = *dusk;
	    last_status = *status;
	    last_now = *np;
	    new = 1;
	}
	return (new);
}

/* find sun's circumstances now.
 * as is the desired accuracy, in arc seconds; use 0.0 for best possible.
 * return 0 if only alt/az changes, else 1 if all other stuff updated too.
 */
int sun_cir (as, np, sp)
double as;
Now *np;
Sky *sp;
{
	static Sky last_sky;
	static Now last_now = {NOMJD};
	static double last_ra, last_dec;	/* unprecessed ra/dec */
	double lst, alt, az;
	double ehp, ha, dec;	/* ehp: angular dia of earth from body */
	int new;

	if (same_cir (np, &last_now) && about_now (np, &last_now, as*.00028)) {
	    *sp = last_sky;
	    new = 0;
	} else {
	    double lsn, rsn;
	    double deps, dpsi;

	    last_now = *np;
	    sunpos (mjd, &lsn, &rsn);		/* sun's true ecliptic long
						 * and dist
						 */
	    nutation (mjd, &deps, &dpsi);	/* correct for nutation */
	    lsn += dpsi;
	    lsn -= degrad(20.4/3600);		/* and light travel time */

	    sp->s_edist = rsn;
	    sp->s_sdist = 0.0;
	    sp->s_elong = 0.0;
	    sp->s_size = raddeg(4.65242e-3/rsn)*3600*2;
	    sp->s_mag = -26.8;
	    sp->s_hlong = lsn-PI;	/* geo- to helio- centric */
	    range (&sp->s_hlong, 2*PI);
	    sp->s_hlat = 0.0;

	    ecl_eq (mjd, 0.0, lsn, &last_ra, &last_dec);
	    sp->s_ra = last_ra;
	    sp->s_dec = last_dec;
	    if (epoch != EOD)
		precess (mjd, epoch, &sp->s_ra, &sp->s_dec);
	    new = 1;
	}

	now_lst (np, &lst);
	ha = hrrad(lst) - last_ra;
	ehp = (2.0 * 6378.0 / 146.0e6) / sp->s_edist;
	ta_par (ha, last_dec, lat, height, ehp, &ha, &dec);
	hadec_aa (lat, ha, dec, &alt, &az);
	refract (pressure, temp, alt, &alt);
	sp->s_alt = alt;
	sp->s_az = az;
	last_sky = *sp;
	return (new);
}

/* find moon's circumstances now.
 * as is the desired accuracy, in arc seconds; use 0.0 for best possible.
 * return 0 if only alt/az changes, else 1 if all other stuff updated too.
 */
int moon_cir (as, np, sp)
double as;
Now *np;
Sky *sp;
{
	static Sky last_sky;
	static Now last_now = {NOMJD};
	static double ehp;
	static double last_ra, last_dec;	/* unprecessed */
	double lst, alt, az;
	double ha, dec;
	int new;

	if (same_cir (np, &last_now) && about_now (np, &last_now, as*.000021)) {
	    *sp = last_sky;
	    new = 0;
	} else {
	    double lam, bet;
	    double deps, dpsi;
	    double lsn, rsn;	/* sun long in rads, earth-sun dist in au */
	    double edistau;	/* earth-moon dist, in au */
	    double el;		/* elongation, rads east */

	    last_now = *np;
	    moon (mjd, &lam, &bet, &ehp);	/* moon's true ecliptic loc */
	    nutation (mjd, &deps, &dpsi);	/* correct for nutation */
	    lam += dpsi;
	    range (&lam, 2*PI);

	    sp->s_edist = 6378.14/sin(ehp);	/* earth-moon dist, want km */
	    sp->s_size = 3600*31.22512*sin(ehp);/* moon angular dia, seconds */
	    sp->s_hlong = lam;			/* save geo in helio fields */
	    sp->s_hlat = bet;

	    ecl_eq (mjd, bet, lam, &last_ra, &last_dec);
	    sp->s_ra = last_ra;
	    sp->s_dec = last_dec;
	    if (epoch != EOD)
		precess (mjd, epoch, &sp->s_ra, &sp->s_dec);

	    sunpos (mjd, &lsn, &rsn);
	    range (&lsn, 2*PI);
	    elongation (lam, bet, lsn, &el);

	    /* solve triangle of earth, sun, and elongation for moon-sun dist */
	    edistau = sp->s_edist/1.495979e8; /* km -> au */
	    sp->s_sdist =
		sqrt (edistau*edistau + rsn*rsn - 2.0*edistau*rsn*cos(el));

	    /* TODO: improve mag; this is based on a flat moon model. */
	    sp->s_mag = -12.7 + 2.5*(log10(PI) - log10(PI/2*(1+1.e-6-cos(el))));

	    sp->s_elong = raddeg(el);	/* want degrees */
	    sp->s_phase = fabs(el)/PI*100.0;	/* want non-negative % */
	    new = 1;
	}

	/* show topocentric alt/az by correcting ra/dec for parallax 
	 * as well as refraction.
	 */
	now_lst (np, &lst);
	ha = hrrad(lst) - last_ra;
	ta_par (ha, last_dec, lat, height, ehp, &ha, &dec);
	hadec_aa (lat, ha, dec, &alt, &az);
	refract (pressure, temp, alt, &alt);
	sp->s_alt = alt;
	sp->s_az = az;
	last_sky = *sp;
	return (new);
}

/* given geocentric ecliptic longitude and latitude, lam and bet, of some object
 * and the longitude of the sun, lsn, find the elongation, el. this is the
 * actual angular separation of the object from the sun, not just the difference
 * in the longitude. the sign, however, IS set simply as a test on longitude
 * such that el will be >0 for an evening object <0 for a morning object.
 * to understand the test for el sign, draw a graph with lam going from 0-2*PI
 *   down the vertical axis, lsn going from 0-2*PI across the hor axis. then
 *   define the diagonal regions bounded by the lines lam=lsn+PI, lam=lsn and
 *   lam=lsn-PI. the "morning" regions are any values to the lower left of the
 *   first line and bounded within the second pair of lines.
 * all angles in radians.
 */
void elongation (lam, bet, lsn, el)
double lam, bet, lsn;
double *el;
{
	*el = acos(cos(bet)*cos(lam-lsn));
	if (lam>lsn+PI || (lam>lsn-PI && lam<lsn)) *el = - *el;
}

/* return whether the two Nows are for the same observing circumstances. */
int same_cir (n1, n2)
register Now *n1, *n2;
{
	return (n1->n_lat == n2->n_lat
		&& n1->n_lng == n2->n_lng
		&& n1->n_temp == n2->n_temp
		&& n1->n_pressure == n2->n_pressure
		&& n1->n_height == n2->n_height
		&& n1->n_tz == n2->n_tz
		&& n1->n_epoch == n2->n_epoch);
}

/* return whether the two Nows are for the same LOCAL day */
int same_lday (n1, n2)
Now *n1, *n2;
{
	return (mjd_day(n1->n_mjd - n1->n_tz/24.0) ==
		       mjd_day(n2->n_mjd - n2->n_tz/24.0)); 
}

/* return whether the mjd of the two Nows are within dt */
static
int about_now (n1, n2, dt)
Now *n1, *n2;
double dt;
{
	return (fabs (n1->n_mjd - n2->n_mjd) <= dt/2.0);
}

void now_lst (np, lst)
Now *np;
double *lst;
{
	utc_gst (mjd_day(mjd), mjd_hr(mjd), lst);
	*lst += radhr(lng);
	range (lst, 24.0);
}

/* round a time in days, *t, to the nearest second, IN PLACE. */
void rnd_second (t)
double *t;
{
	*t = floor(*t*SPD+0.5)/SPD;
}
	
double
mjd_day(jd)
double jd;
{
	return (floor(jd-0.5)+0.5);
}

double
mjd_hr(jd)
double jd;
{
	return ((jd-mjd_day(jd))*24.0);
}
