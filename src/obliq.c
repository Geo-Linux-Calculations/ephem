#include <stdio.h>
#include "astro.h"

/* given the modified Julian date, mjd, find the obliquity of the
 * ecliptic, *eps, in radians.
 */
void obliquity (mjd, eps)
double mjd;
double *eps;
{
    static double lastmjd = -10000, lasteps;

    if (mjd != lastmjd)
    {
        double t;
        t = mjd/36525.;
        lasteps = degrad(2.345229444E1
                         - ((((-1.81E-3*t)+5.9E-3)*t+4.6845E1)*t)/3600.0);
        lastmjd = mjd;
    }
    *eps = lasteps;
}
