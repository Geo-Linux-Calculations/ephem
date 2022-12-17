#include <stdio.h>
#include <math.h>
#include "astro.h"

#define	TWOPI	(2*PI)

/* given the mean anomaly, ma, and the eccentricity, s, of elliptical motion,
 * find the true anomaly, *nu, and the eccentric anomaly, *ea.
 * all angles in radians.
 */
void anomaly (ma, s, nu, ea)
double ma, s;
double *nu, *ea;
{
    double m, fea;

    m = ma-TWOPI*(long)(ma/TWOPI);
    if (m > PI) m -= TWOPI;
    if (m < -PI) m += TWOPI;
    fea = m;

    if (s < 1.0)
    {
        /* elliptical */
        double dla;
        while (1)
        {
            dla = fea-(s*sin(fea))-m;
            if (fabs(dla)<1e-6)
                break;
            dla /= 1-(s*cos(fea));
            fea -= dla;
        }
        *nu = 2*atan(sqrt((1+s)/(1-s))*tan(fea/2));
    }
    else
    {
        /* hyperbolic */
        double corr = 1;
        while (fabs(corr) > 0.000001)
        {
            corr = (m - s * sinh(fea) + fea) / (s*cosh(fea) - 1);
            fea += corr;
        }
        *nu = 2*atan(sqrt((s+1)/(s-1))*tanh(fea/2));
    }
    *ea = fea;
}
