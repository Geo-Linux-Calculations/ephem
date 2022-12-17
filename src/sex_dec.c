#include <math.h>

/* given hours (or degrees), hd, minutes, m, and seconds, s,
 * return decimal hours (or degrees), *d.
 * in the case of hours (angles) < 0, only the first non-zero element should
 *   be negative.
 */
void sex_dec (hd, m, s, d)
int hd, m, s;
double *d;
{
    int sign = 1;

    if (hd < 0)
    {
        sign = -1;
        hd = -hd;
    }
    else if (m < 0)
    {
        sign = -1;
        m = -m;
    }
    else if (s < 0)
    {
        sign = -1;
        s = -s;
    }

    *d = (((double)s/60.0 + (double)m)/60.0 + (double)hd) * sign;
}

/* given decimal hours (or degrees), d.
 * return nearest hours (or degrees), *hd, minutes, *m, and seconds, *s,
 * each always non-negative; *isneg is set to 1 if d is < 0, else to 0.
 */
void dec_sex (d, hd, m, s, isneg)
double d;
int *hd, *m, *s, *isneg;
{
    double min;

    if (d < 0)
    {
        *isneg = 1;
        d = -d;
    }
    else
        *isneg = 0;

    *hd = (int)d;
    min = (d - *hd)*60.;
    *m = (int)min;
    *s = (int)((min - *m)*60. + 0.5);

    if (*s == 60)
    {
        if ((*m += 1) == 60)
        {
            *hd += 1;
            *m = 0;
        }
        *s = 0;
    }
    /* no  negative 0's */
    if (*hd == 0 && *m == 0 && *s == 0)
        *isneg = 0;
}

/* insure 0 <= *v < r.
 */
void range (v, r)
double *v, r;
{
    *v -= r*floor(*v/r);
}
