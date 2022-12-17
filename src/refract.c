#include <stdio.h>
#include <math.h>
#include "astro.h"

/* correct the true altitude, ta, for refraction to the apparent altitude, aa,
 * each in radians, given the local atmospheric pressure, pr, in mbars, and
 * the temperature, tr, in degrees C.
 */
void refract (pr, tr, ta, aa)
double pr, tr;
double ta;
double *aa;
{
    double r;	/* refraction correction*/

    if (ta >= degrad(15.))
    {
        /* model for altitudes at least 15 degrees above horizon */
        r = 7.888888e-5*pr/((273+tr)*tan(ta));
    }
    else if (ta > degrad(-5.))
    {
        /* hairier model for altitudes at least -5 and below 15 degrees */
        double a, b, tadeg = raddeg(ta);
        a = ((2e-5*tadeg+1.96e-2)*tadeg+1.594e-1)*pr;
        b = (273+tr)*((8.45e-2*tadeg+5.05e-1)*tadeg+1);
        r = degrad(a/b);
    }
    else
    {
        /* do nothing if more than 5 degrees below horizon.
         */
        r = 0;
    }

    *aa  =  ta + r;
}

/* correct the apparent altitude, aa, for refraction to the true altitude, ta,
 * each in radians, given the local atmospheric pressure, pr, in mbars, and
 * the temperature, tr, in degrees C.
 */
void unrefract (pr, tr, aa, ta)
double pr, tr;
double aa;
double *ta;
{
    double err;
    double appar;
    double true;

    /* iterative solution: search for the true that refracts to the
     *   given apparent.
     * since refract() is discontinuous at -5 degrees, there is a range
     *   of apparent altitudes between about -4.5 and -5 degrees that are
     *   not invertable (the graph of ap vs. true has a vertical step at
     *   true = -5). thus, the iteration just oscillates if it gets into
     *   this region. if this happens the iteration is forced to abort.
     *   of course, this makes unrefract() discontinuous too.
     */
    true = aa;
    do
    {
        refract (pr, tr, true, &appar);
        err = appar - aa;
        true -= err;
    }
    while (fabs(err) >= 1e-6 && true > degrad(-5));

    *ta = true;
}
