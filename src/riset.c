#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

/* given the true geocentric ra and dec of an object, the observer's latitude,
 *   lat, and a horizon displacement correction, dis, all in radians, find the
 *   local sidereal times and azimuths of rising and setting, lstr/s
 *   and azr/s, also all in radians, respectively.
 * dis is the vertical displacement from the true position of the horizon. it
 *   is positive if the apparent position is higher than the true position.
 *   said another way, it is positive if the shift causes the object to spend
 *   longer above the horizon. for example, atmospheric refraction is typically
 *   assumed to produce a vertical shift of 34 arc minutes at the horizon; dis
 *   would then take on the value +9.89e-3 (radians). On the other hand, if
 *   your horizon has hills such that your apparent horizon is, say, 1 degree
 *   above sea level, you would allow for this by setting dis to -1.75e-2
 *   (radians).
 *
 * algorithm:
 *   the situation is described by two spherical triangles with two equal angles
 *    (the right horizon intercepts, and the common horizon transverse):
 *   given lat, d(=d1+d2), and dis find z(=z1+z2) and rho, where      /| eq pole
 *     lat = latitude,                                              /  |
 *     dis = horizon displacement (>0 is below ideal)             / rho|
 *     d = angle from pole = PI/2 - declination                /       |
 *     z = azimuth east of north                            /          |
 *     rho = polar rotation from down = PI - hour angle    /           | 
 *   solve simultaneous equations for d1 and d2:         /             |
 *     1) cos(d) = cos(d1+d2)                           / d2           | lat
 *            = cos(d1)cos(d2) - sin(d1)sin(d2)        /               |
 *     2) sin(d2) = sin(lat)sin(d1)/sin(dis)          /                |
 *   then can solve for z1, z2 and rho, taking       /                 |
 *     care to preserve quadrant information.       /                 -|
 *                                              z1 /        z2       | |
 *                      ideal horizon ------------/--------------------| 
 *                                         | |   /                     N
 *                                          -|  / d1
 *                                       dis | /
 *                                           |/
 *                  apparent horizon  ---------------------------------
 *
 * note that when lat=0 this all breaks down (because d2 and z2 degenerate to 0)
 *   but fortunately then we can solve for z and rho directly.
 *
 * status: 0: normal; 1: never rises; -1: circumpolar; 2: trouble.
 */
void riset (ra, dec, lat, dis, lstr, lsts, azr, azs, status)
double ra, dec;
double lat, dis;
double *lstr, *lsts;
double *azr, *azs;
int *status;
{
#define	EPS	(1e-6)	/* math rounding fudge - always the way, eh? */
	double d;	/* angle from pole */
	double h;	/* hour angle */
	double crho;	/* cos hour-angle complement */
	int shemi;	/* flag for southern hemisphere reflection */

	d = PI/2 - dec;

	/* reflect if in southern hemisphere.
	 * (then reflect azimuth back after computation.)
	 */
	if ((shemi = (lat < 0))) {
	    lat = -lat;
	    d = PI - d;
	}

	/* do the easy ones (and avoid violated assumptions) if d arc never
	 * meets horizon. 
	 */
	if (d <= lat + dis + EPS) {
	    *status = -1; /* never sets */
	    return;
	}
	if (d >= PI - lat + dis - EPS) {
	    *status = 1; /* never rises */
	    return;
	}

	/* find rising azimuth and cosine of hour-angle complement */
	if (lat > EPS) {
	    double d2, d1; /* polr arc to ideal hzn, and corrctn for apparent */
	    double z2, z1; /* azimuth to ideal horizon, and " */
	    double a;	   /* intermediate temp */
	    double sdis, slat, clat, cz2, cd2;	/* trig temps */
	    sdis = sin(dis);
	    slat = sin(lat);
	    a = sdis*sdis + slat*slat + 2*cos(d)*sdis*slat;
	    if (a <= 0) {
		*status = 2; /* can't happen - hah! */
		return;
	    }
	    d1 = asin (sin(d) * sdis / sqrt(a));
	    d2 = d - d1;
	    cd2 = cos(d2);
	    clat = cos(lat);
	    cz2 = cd2/clat;
	    z2 = acos (cz2);
	    z1 = acos (cos(d1)/cos(dis));
	    if (dis < 0)
		z1 = -z1;
	    *azr = z1 + z2;
	    range (azr, PI);
	    crho = (cz2 - cd2*clat)/(sin(d2)*slat);
	} else {
	    *azr = acos (cos(d)/cos(dis));
	    crho = sin(dis)/sin(d);
	}

	if (shemi)
	    *azr = PI - *azr;
        *azs = 2*PI - *azr;
	
	/* find hour angle */
	h = PI - acos (crho);
        *lstr = radhr(ra-h);
	*lsts = radhr(ra+h);
	range (lstr, 24.0);
	range (lsts, 24.0);

	*status = 0;
}
