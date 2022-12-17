#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

#define	EQtoECL	1
#define	ECLtoEQ	(-1)

static
void ecleq_aux(int sw, double mjd, double x, double y, double *p, double *q);

/* given the modified Julian date, mjd, and an equitorial ra and dec, each in
 * radians, find the corresponding geocentric ecliptic latitude, *lat, and
 * longititude, *lng, also each in radians.
 * correction for the effect on the angle of the obliquity due to nutation is
 * included.
 */
void eq_ecl (mjd, ra, dec, lat, lng)
double mjd, ra, dec;
double *lat, *lng;
{
    ecleq_aux (EQtoECL, mjd, ra, dec, lng, lat);
}

/* given the modified Julian date, mjd, and a geocentric ecliptic latitude,
 * *lat, and longititude, *lng, each in radians, find the corresponding
 * equitorial ra and dec, also each in radians.
 * correction for the effect on the angle of the obliquity due to nutation is
 * included.
 */
void ecl_eq (mjd, lat, lng, ra, dec)
double mjd, lat, lng;
double *ra, *dec;
{
    ecleq_aux (ECLtoEQ, mjd, lng, lat, ra, dec);
}

static
void ecleq_aux (sw, mjd, x, y, p, q)
int sw;			/* +1 for eq to ecliptic, -1 for vv. */
double mjd, x, y;	/* sw==1: x==ra, y==dec.  sw==-1: x==lng, y==lat. */
double *p, *q;		/* sw==1: p==lng, q==lat. sw==-1: p==ra, q==dec. */
{
    static double lastmjd = -10000;	/* last mjd calculated */
    static double seps, ceps;	/* sin and cos of mean obliquity */
    double sx, cx, sy, cy, ty;

    if (mjd != lastmjd)
    {
        double eps;
        double deps, dpsi;
        obliquity (mjd, &eps);		/* mean obliquity for date */
        nutation (mjd, &deps, &dpsi);
        eps += deps;
        seps = sin(eps);
        ceps = cos(eps);
        lastmjd = mjd;
    }

    sy = sin(y);
    cy = cos(y);				/* always non-negative */
    if (fabs(cy)<1e-20) cy = 1e-20;		/* insure > 0 */
    ty = sy/cy;
    cx = cos(x);
    sx = sin(x);
    *q = asin((sy*ceps)-(cy*seps*sx*sw));
    *p = atan(((sx*ceps)+(ty*seps*sw))/cx);
    if (cx<0) *p += PI;		/* account for atan quad ambiguity */
    range (p, 2*PI);
}
