#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

#define	DCOS(x)		cos(degrad(x))
#define	DSIN(x)		sin(degrad(x))
#define	DASIN(x)	raddeg(asin(x))
#define	DATAN2(y,x)	raddeg(atan2((y),(x)))

/* corrects ra and dec, both in radians, for precession from epoch 1 to epoch 2.
 * the epochs are given by their modified JDs, mjd1 and mjd2, respectively.
 * N.B. ra and dec are modifed IN PLACE.
 */

/*
 * Copyright (c) 1990 by Craig Counterman. All rights reserved.
 *
 * This software may be redistributed freely, not sold.
 * This copyright notice and disclaimer of warranty must remain
 *    unchanged. 
 *
 * No representation is made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty, to the extent permitted by applicable law.
 *
 * Rigorous precession. From Astronomical Ephemeris 1989, p. B18
 */

void precess (mjd1, mjd2, ra, dec)
double mjd1, mjd2;	/* initial and final epoch modified JDs */
double *ra, *dec;	/* ra/dec for mjd1 in, for mjd2 out */
{
	double zeta_A, z_A, theta_A;
	double T;
	double A, B, C;
	double alpha, delta;
	double alpha_in, delta_in;
	double from_equinox, to_equinox;
	double alpha2000, delta2000;

	mjd_year (mjd1, &from_equinox);
	mjd_year (mjd2, &to_equinox);
	alpha_in = raddeg(*ra);
	delta_in = raddeg(*dec);

	/* From from_equinox to 2000.0 */
	if (from_equinox != 2000.0) {
	    T = (from_equinox - 2000.0)/100.0;
	    zeta_A  = 0.6406161* T + 0.0000839* T*T + 0.0000050* T*T*T;
	    z_A     = 0.6406161* T + 0.0003041* T*T + 0.0000051* T*T*T;
	    theta_A = 0.5567530* T - 0.0001185* T*T + 0.0000116* T*T*T;

	    A = DSIN(alpha_in - z_A) * DCOS(delta_in);
	    B = DCOS(alpha_in - z_A) * DCOS(theta_A) * DCOS(delta_in)
	      + DSIN(theta_A) * DSIN(delta_in);
	    C = -DCOS(alpha_in - z_A) * DSIN(theta_A) * DCOS(delta_in)
	      + DCOS(theta_A) * DSIN(delta_in);

	    alpha2000 = DATAN2(A,B) - zeta_A;
	    range (&alpha2000, 360.0);
	    delta2000 = DASIN(C);
	} else {
	    /* should get the same answer, but this could improve accruacy */
	    alpha2000 = alpha_in;
	    delta2000 = delta_in;
	};


	/* From 2000.0 to to_equinox */
	if (to_equinox != 2000.0) {
	    T = (to_equinox - 2000.0)/100.0;
	    zeta_A  = 0.6406161* T + 0.0000839* T*T + 0.0000050* T*T*T;
	    z_A     = 0.6406161* T + 0.0003041* T*T + 0.0000051* T*T*T;
	    theta_A = 0.5567530* T - 0.0001185* T*T + 0.0000116* T*T*T;

	    A = DSIN(alpha2000 + zeta_A) * DCOS(delta2000);
	    B = DCOS(alpha2000 + zeta_A) * DCOS(theta_A) * DCOS(delta2000)
	      - DSIN(theta_A) * DSIN(delta2000);
	    C = DCOS(alpha2000 + zeta_A) * DSIN(theta_A) * DCOS(delta2000)
	      + DCOS(theta_A) * DSIN(delta2000);

	    alpha = DATAN2(A,B) + z_A;
	    range(&alpha, 360.0);
	    delta = DASIN(C);
	} else {
	    /* should get the same answer, but this could improve accruacy */
	    alpha = alpha2000;
	    delta = delta2000;
	};

	*ra = degrad(alpha);
	*dec = degrad(delta);
}
