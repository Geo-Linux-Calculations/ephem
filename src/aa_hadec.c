#include <stdio.h>
#include <math.h>
#include "astro.h"

/* the actual formula is the same for both transformation directions so
 * do it here once for each way.
 * N.B. all arguments are in radians.
 */
static void
aaha_aux (lat, x, y, p, q)
double lat;
double x, y;
double *p, *q;
{
	static double lastlat = -1000.;
	static double sinlastlat, coslastlat;
	double sy, cy;
	double sx, cx;
	double sq, cq;
	double a;
	double cp;

	/* latitude doesn't change much, so try to reuse the sin and cos evals.
	 */
	if (lat != lastlat) {
	    sinlastlat = sin (lat);
	    coslastlat = cos (lat);
	    lastlat = lat;
	}

	sy = sin (y);
	cy = cos (y);
	sx = sin (x);
	cx = cos (x);

/* define GOODATAN2 if atan2 returns full range -PI through +PI.
 */
#ifdef GOODATAN2
	*q = asin ((sy*sinlastlat) + (cy*coslastlat*cx));
	*p = atan2 (-cy*sx, -cy*cx*sinlastlat + sy*coslastlat);
#else
#define	EPS	(1e-20)
	sq = (sy*sinlastlat) + (cy*coslastlat*cx);
	*q = asin (sq);
	cq = cos (*q);
	a = coslastlat*cq;
	if (a > -EPS && a < EPS)
	    a = a < 0 ? -EPS : EPS; /* avoid / 0 */
	cp = (sy - (sinlastlat*sq))/a;
	if (cp >= 1.0)	/* the /a can be slightly > 1 */
	    *p = 0.0;
	else if (cp <= -1.0)
	    *p = PI;
	else
	    *p = acos ((sy - (sinlastlat*sq))/a);
	if (sx>0) *p = 2.0*PI - *p;
#endif
}

/* given latitude (n+, radians), lat, altitude (up+, radians), alt, and
 * azimuth (angle round to the east from north+, radians),
 * return hour angle (radians), ha, and declination (radians), dec.
 */
void aa_hadec (lat, alt, az, ha, dec)
double lat;
double alt, az;
double *ha, *dec;
{
	aaha_aux (lat, az, alt, ha, dec);
}

/* given latitude (n+, radians), lat, hour angle (radians), ha, and declination
 * (radians), dec,
 * return altitude (up+, radians), alt, and
 * azimuth (angle round to the east from north+, radians),
 */
void hadec_aa (lat, ha, dec, alt, az)
double lat;
double ha, dec;
double *alt, *az;
{
	aaha_aux (lat, ha, dec, az, alt);
}

