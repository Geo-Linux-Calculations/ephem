#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

/* this array contains polynomial coefficients to find the various orbital
 *   elements for the mean orbit at any instant in time for each major planet.
 *   the first five elements are in the form a0 + a1*t + a2*t**2 + a3*t**3,
 *   where t is the number of Julian centuries of 36525 Julian days since 1900
 *   Jan 0.5. the last three elements are constants.
 *
 * the orbital element (column) indeces are:
 *   [ 0- 3]: coefficients for mean longitude, in degrees;
 *   [ 4- 7]: coefficients for longitude of the perihelion, in degrees;
 *   [ 8-11]: coefficients for eccentricity;
 *   [12-15]: coefficients for inclination, in degrees;
 *   [16-19]: coefficients for longitude of the ascending node, in degrees;
 *      [20]: semi-major axis, in AU;
 *      [21]: angular diameter at 1 AU, in arcsec;
 *      [22]: standard visual magnitude, ie, the visual magnitude of the planet
 *	      when at a distance of 1 AU from both the Sun and the Earth and
 *	      with zero phase angle.
 *
 * the planent (row) indeces are:
 *   [0]: Mercury; [1]: Venus;   [2]: Mars;  [3]: Jupiter; [4]: Saturn;
 *   [5]: Uranus;  [6]: Neptune; [7]: Pluto.
 */
#define	NPELE	(5*4 + 3)	/* 4 coeffs for ea of 5 elems, + 3 constants */
static double elements[8][NPELE] = {

	{   /*     mercury... */

	    178.179078,	415.2057519,	3.011e-4,	0.0,
	    75.899697,	1.5554889,	2.947e-4,	0.0,
	    .20561421,	2.046e-5,	3e-8,		0.0,
	    7.002881,	1.8608e-3,	-1.83e-5,	0.0,
	    47.145944,	1.1852083,	1.739e-4,	0.0,
	    .3870986,	6.74, 		-0.42
	},

	{   /*     venus... */

	    342.767053,	162.5533664,	3.097e-4,	0.0,
	    130.163833,	1.4080361,	-9.764e-4,	0.0,
	    6.82069e-3,	-4.774e-5,	9.1e-8,		0.0,
	    3.393631,	1.0058e-3,	-1e-6,		0.0,
	    75.779647,	.89985,		4.1e-4,		0.0,
	    .7233316,	16.92,		-4.4
	},

	{   /*     mars... */

	    293.737334,	53.17137642,	3.107e-4,	0.0,
	    3.34218203e2, 1.8407584,	1.299e-4,	-1.19e-6,
	    9.33129e-2,	9.2064e-5,	7.7e-8,		0.0,
	    1.850333,	-6.75e-4,	1.26e-5,	0.0,
	    48.786442,	.7709917,	-1.4e-6,	-5.33e-6,
	    1.5236883,	9.36,		-1.52
	},

	{   /*     jupiter... */

	    238.049257,	8.434172183,	3.347e-4,	-1.65e-6,
	    1.2720972e1, 1.6099617,	1.05627e-3,	-3.43e-6,
	    4.833475e-2, 1.6418e-4,	-4.676e-7,	-1.7e-9,
	    1.308736,	-5.6961e-3,	3.9e-6,		0.0,
	    99.443414,	1.01053,	3.5222e-4,	-8.51e-6,
	    5.202561,	196.74,		-9.4
	},

	{   /*     saturn... */

	    266.564377,	3.398638567,	3.245e-4,	-5.8e-6,
	    9.1098214e1, 1.9584158,	8.2636e-4,	4.61e-6,
	    5.589232e-2, -3.455e-4,	-7.28e-7,	7.4e-10,
	    2.492519,	-3.9189e-3,	-1.549e-5,	4e-8,
	    112.790414,	.8731951,	-1.5218e-4,	-5.31e-6,
	    9.554747,	165.6,		-8.88
	},

	{   /*     uranus... */

	    244.19747,	1.194065406,	3.16e-4,	-6e-7,
	    1.71548692e2, 1.4844328,	2.372e-4,	-6.1e-7,
	    4.63444e-2,	-2.658e-5,	7.7e-8,		0.0,
	    .772464,	6.253e-4,	3.95e-5,	0.0,
	    73.477111,	.4986678,	1.3117e-3,	0.0,
	    19.21814,	65.8,		-7.19
	},

	{   /*     neptune... */

	    84.457994,	.6107942056,	3.205e-4,	-6e-7,
	    4.6727364e1, 1.4245744,	3.9082e-4,	-6.05e-7,
	    8.99704e-3,	6.33e-6,	-2e-9,		0.0,
	    1.779242,	-9.5436e-3,	-9.1e-6,	0.0,
	    130.681389,	1.098935,	2.4987e-4,	-4.718e-6,
	    30.10957,	62.2,		-6.87
	},

	{   /*     pluto...(osculating 1984 jan 21) */

	    95.3113544,	.3980332167,	0.0,		0.0,
	    224.017,	0.0,		0.0,		0.0,
	    .25515,	0.0,		0.0,		0.0,
	    17.1329,	0.0,		0.0,		0.0,
	    110.191,	0.0,		0.0,		0.0,
	    39.8151,	8.2,		-1.0
	}
};

/* given a modified Julian date, mjd, return the elements for the mean orbit
 *   at that instant of all the major planets, together with their
 *   mean daily motions in longitude, angular diameter and standard visual
 *   magnitude.
 * plan[i][j] contains all the values for all the planets at mjd, such that
 *   i = 0..7: mercury, venus, mars, jupiter, saturn, unranus, neptune, pluto;
 *   j = 0..8: mean longitude, mean daily motion in longitude, longitude of 
 *     the perihelion, eccentricity, inclination, longitude of the ascending
 *     node, length of the semi-major axis, angular diameter from 1 AU, and
 *     the standard visual magnitude (see elements[][] comment, above).
 */
void pelement (mjd, plan)
double mjd;
double plan[8][9];
{
	register double *ep, *pp;
	register double t = mjd/36525.;
	double aa;
	int planet, i;

	for (planet = 0; planet < 8; planet++) {
	    ep = elements[planet];
	    pp = plan[planet];
	    aa = ep[1]*t;
	    pp[0] = ep[0] + 360.*(aa-(long)aa) + (ep[3]*t + ep[2])*t*t;
	    range (pp, 360.);
	    pp[1] = (ep[1]*9.856263e-3) + (ep[2] + ep[3])/36525;

	    for (i = 4; i < 20; i += 4)
		pp[i/4+1] = ((ep[i+3]*t + ep[i+2])*t + ep[i+1])*t + ep[i+0];

	    pp[6] = ep[20];
	    pp[7] = ep[21];
	    pp[8] = ep[22];
	}
}
