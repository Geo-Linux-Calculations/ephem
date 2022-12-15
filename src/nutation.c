#include <stdio.h>
#include <math.h>
#include "astro.h"

/* given the modified JD, mjd, find the nutation in obliquity, *deps, and
 * the nutation in longitude, *dpsi, each in radians.
 */
void nutation (mjd, deps, dpsi)
double mjd;
double *deps, *dpsi;
{
	static double lastmjd = -10000, lastdeps, lastdpsi;
	double ls, ld;	/* sun's mean longitude, moon's mean longitude */
	double ms, md;	/* sun's mean anomaly, moon's mean anomaly */
	double nm;	/* longitude of moon's ascending node */
	double t, t2;	/* number of Julian centuries of 36525 days since
			 * Jan 0.5 1900.
			 */
	double tls, tnm, tld;	/* twice above */
	double a, b;	/* temps */

	if (mjd == lastmjd) {
	    *deps = lastdeps;
	    *dpsi = lastdpsi;
	    return;
	}
	    
	t = mjd/36525.;
	t2 = t*t;

	a = 100.0021358*t;
	b = 360.*(a-(long)a);
	ls = 279.697+.000303*t2+b;

	a = 1336.855231*t;
	b = 360.*(a-(long)a);
	ld = 270.434-.001133*t2+b;

	a = 99.99736056000026*t;
	b = 360.*(a-(long)a);
	ms = 358.476-.00015*t2+b;

	a = 13255523.59*t;
	b = 360.*(a-(long)a);
	md = 296.105+.009192*t2+b;

	a = 5.372616667*t;
	b = 360.*(a-(long)a);
	nm = 259.183+.002078*t2-b;

	/* convert to radian forms for use with trig functions.
	 */
	tls = 2*degrad(ls);
	nm = degrad(nm);
	tnm = 2*nm;
	ms = degrad(ms);
	tld = 2*degrad(ld);
	md = degrad(md);

	/* find delta psi and eps, in arcseconds.
	 */
	lastdpsi = (-17.2327-.01737*t)*sin(nm)+(-1.2729-.00013*t)*sin(tls)
		   +.2088*sin(tnm)-.2037*sin(tld)+(.1261-.00031*t)*sin(ms)
		   +.0675*sin(md)-(.0497-.00012*t)*sin(tls+ms)
		   -.0342*sin(tld-nm)-.0261*sin(tld+md)+.0214*sin(tls-ms)
		   -.0149*sin(tls-tld+md)+.0124*sin(tls-nm)+.0114*sin(tld-md);
	lastdeps = (9.21+.00091*t)*cos(nm)+(.5522-.00029*t)*cos(tls)
		   -.0904*cos(tnm)+.0884*cos(tld)+.0216*cos(tls+ms)
		   +.0183*cos(tld-nm)+.0113*cos(tld+md)-.0093*cos(tls-ms)
		   -.0066*cos(tls-nm);

	/* convert to radians.
	 */
	lastdpsi = degrad(lastdpsi/3600);
	lastdeps = degrad(lastdeps/3600);

	lastmjd = mjd;
	*deps = lastdeps;
	*dpsi = lastdpsi;
}
