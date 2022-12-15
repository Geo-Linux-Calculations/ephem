#include "astro.h"
#include "ephem.h"

static double tnaught (double mjd);

/* given a modified julian date, mjd, and a universally coordinated time, utc,
 * return greenwich mean siderial time, *gst.
 */
void utc_gst (mjd, utc, gst)
double mjd;
double utc;
double *gst;
{
	double tnaught();
	static double lastmjd = -10000;
	static double t0;

	if (mjd != lastmjd) {
	    t0 = tnaught (mjd);
	    lastmjd = mjd;
	}
	*gst = (1.0/SIDRATE)*utc + t0;
	range (gst, 24.0);
}

/* given a modified julian date, mjd, and a greenwich mean siderial time, gst,
 * return universally coordinated time, *utc.
 */
void gst_utc (mjd, gst, utc)
double mjd;
double gst;
double *utc;
{
	double tnaught();
	static double lastmjd = -10000;
	static double t0;

	if (mjd != lastmjd) {
	    t0 = tnaught (mjd);
	    range (&t0, 24.0);
	    lastmjd = mjd;
	}
	*utc = gst - t0;
	range (utc, 24.0);
	*utc *= SIDRATE;
}

static double
tnaught (mjd)
double mjd;	/* julian days since 1900 jan 0.5 */
{
	double dmjd;
	int m, y;
	double d;
	double t, t0;

	mjd_cal (mjd, &m, &d, &y);
	cal_mjd (1, 0., y, &dmjd);
	t = dmjd/36525;
	t0 = 6.57098e-2 * (mjd - dmjd) - 
	     (24 - (6.6460656 + (5.1262e-2 + (t * 2.581e-5))*t) -
		   (2400 * (t - (((double)y - 1900)/100))));
	return (t0);
}
