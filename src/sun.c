#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

/* given the modified JD, mjd, return the true geocentric ecliptic longitude
 *   of the sun for the mean equinox of the date, *lsn, in radians, and the
 *   sun-earth distance, *rsn, in AU. (the true ecliptic latitude is never more
 *   than 1.2 arc seconds and so may be taken to be a constant 0.)
 * if the APPARENT ecliptic longitude is required, correct the longitude for
 *   nutation to the true equinox of date and for aberration (light travel time,
 *   approximately  -9.27e7/186000/(3600*24*365)*2*pi = -9.93e-5 radians).
 */
void sunpos (mjd, lsn, rsn)
double mjd;
double *lsn, *rsn;
{
	double t, t2;
	double ls, ms;    /* mean longitude and mean anomoay */
	double s, nu, ea; /* eccentricity, true anomaly, eccentric anomaly */
	double a, b, a1, b1, c1, d1, e1, h1, dl, dr;

	t = mjd/36525.;
	t2 = t*t;
	a = 100.0021359*t;
	b = 360.*(a-(long)a);
	ls = 279.69668+.0003025*t2+b;
	a = 99.99736042000039*t;
	b = 360*(a-(long)a);
	ms = 358.47583-(.00015+.0000033*t)*t2+b;
	s = .016751-.0000418*t-1.26e-07*t2;
	anomaly (degrad(ms), s, &nu, &ea);
	a = 62.55209472000015*t;
	b = 360*(a-(long)a);
	a1 = degrad(153.23+b);
	a = 125.1041894*t;
	b = 360*(a-(long)a);
	b1 = degrad(216.57+b);
	a = 91.56766028*t;
	b = 360*(a-(long)a);
	c1 = degrad(312.69+b);
	a = 1236.853095*t;
	b = 360*(a-(long)a);
	d1 = degrad(350.74-.00144*t2+b);
	e1 = degrad(231.19+20.2*t);
	a = 183.1353208*t;
	b = 360*(a-(long)a);
	h1 = degrad(353.4+b);
	dl = .00134*cos(a1)+.00154*cos(b1)+.002*cos(c1)+.00179*sin(d1)+
								.00178*sin(e1);
	dr = 5.43e-06*sin(a1)+1.575e-05*sin(b1)+1.627e-05*sin(c1)+
					    3.076e-05*cos(d1)+9.27e-06*sin(h1);
	*lsn = nu+degrad(ls-ms+dl);
	*rsn = 1.0000002*(1-s*cos(ea))+dr;
	range (lsn, 2*PI);
}
