/* find rise and set circumstances, ie, riset_cir() and related functions. */

#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "circum.h"
#include "screen.h"	/* just for SUN and MOON */
#include "ephem.h"

#define	TRACE(x)	{FILE *fp = fopen("trace","a"); fprintf x; fclose(fp);}

#define	STDREF	degrad(34./60.)	/* nominal horizon refraction amount */
#define	TWIREF	degrad(18.)	/* twilight horizon displacement */
#define	TMACC	(15./3600.)	/* convergence accuracy, hours */

static void iterative_riset (int p, Now *np, int hzn, double *ltr, double *lts,
                             double *ltt, double *azr, double *azs, double *altt, int *status);
static void stationary_riset (int p, double mjd0, Now *np, int hzn, double *lstr,
                              double *lsts, double *lstt, double *azr, double *azs, double *altt, int *status);
static void transit (double r, double d, Now *np, double *lstt, double *altt);

/* find where and when a body, p, will rise and set and
 *   it's transit circumstances. all times are local, angles rads e of n.
 * return 0 if just returned same stuff as previous call, else 1 if new.
 * status is set from the RS_* #defines in circum.h.
 * also used to find astro twilight by calling with hzn TWILIGHT.
 */
int riset_cir (p, np, force, hzn, ltr, lts, ltt, azr, azs, altt, status)
int p;		/* one of the body defines in astro.h or screen.h */
Now *np;
int force;	/* set !=0 to force computations */
int hzn;	/* STDHZN or ADPHZN or TWILIGHT */
double *ltr, *lts; /* local rise and set times */
double *ltt;	/* local transit time */
double *azr, *azs; /* local rise and set azimuths, rads e of n */
double *altt;	/* local altitude at transit */
int *status;	/* one or more of the RS_* defines */
{
    typedef struct
    {
        Now l_now;
        double l_ltr, l_lts, l_ltt, l_azr, l_azs, l_altt;
        int l_hzn;
        int l_status;
    } Last;
    /* must be in same order as the astro.h/screen.h #define's */
    static Last last[NOBJ] =
    {
        {{NOMJD}}, {{NOMJD}}, {{NOMJD}}, {{NOMJD}}, {{NOMJD}}, {{NOMJD}},
        {{NOMJD}}, {{NOMJD}}, {{NOMJD}}, {{NOMJD}}, {{NOMJD}}, {{NOMJD}}
    };
    Last *lp;
    int new;

    lp = last + p;
    if (!force && same_cir (np, &lp->l_now) && same_lday (np, &lp->l_now)
            && lp->l_hzn == hzn)
    {
        *ltr = lp->l_ltr;
        *lts = lp->l_lts;
        *ltt = lp->l_ltt;
        *azr = lp->l_azr;
        *azs = lp->l_azs;
        *altt = lp->l_altt;
        *status = lp->l_status;
        new = 0;
    }
    else
    {
        *status = 0;
        iterative_riset (p, np, hzn, ltr, lts, ltt, azr, azs, altt, status);
        lp->l_ltr = *ltr;
        lp->l_lts = *lts;
        lp->l_ltt = *ltt;
        lp->l_azr = *azr;
        lp->l_azs = *azs;
        lp->l_altt = *altt;
        lp->l_status = *status;
        lp->l_hzn = hzn;
        lp->l_now = *np;
        new = 1;
    }
    return (new);
}

static
void iterative_riset (p, np, hzn, ltr, lts, ltt, azr, azs, altt, status)
int p;
Now *np;
int hzn;
double *ltr, *lts, *ltt;	/* local times of rise, set and transit */
double *azr, *azs, *altt;/* local azimuths of rise, set and transit altitude */
int *status;
{
#define	MAXPASSES	6
    double lstr, lsts, lstt; /* local sidereal times of rising/setting */
    double mjd0;		/* mjd estimates of rise/set event */
    double lnoon;		/* mjd of local noon */
    double x;		/* discarded tmp value */
    Now n;			/* just used to call now_lst() */
    double lst;		/* lst at local noon */
    double diff, lastdiff;	/* iterative improvement to mjd0 */
    int pass;
    int rss;

    /* first approximation is to find rise/set times of a fixed object
     * in its position at local noon.
     */
    lnoon = mjd_day(mjd - tz/24.0) + (12.0 + tz)/24.0; /*mjd of local noon*/
    n.n_mjd = lnoon;
    n.n_lng = lng;
    now_lst (&n, &lst);	/* lst at local noon */
    mjd0 = lnoon;
    stationary_riset (p,mjd0,np,hzn,&lstr,&lsts,&lstt,&x,&x,&x,&rss);
chkrss:
    switch (rss)
    {
    case  0:
        break;
    case  1:
        *status = RS_NEVERUP;
        return;
    case -1:
        *status = RS_CIRCUMPOLAR;
        goto transit;
    default:
        *status = RS_ERROR;
        return;
    }

    /* find a better approximation to the rising circumstances based on
     * more passes, each using a "fixed" object at the location at
     * previous approximation of the rise time.
     */
    lastdiff = 1000.0;
    for (pass = 1; pass < MAXPASSES; pass++)
    {
        diff = (lstr - lst)*SIDRATE; /* next guess at rise time wrt noon */
        if (diff > 12.0)
            diff -= 24.0*SIDRATE;	/* not tomorrow, today */
        else if (diff < -12.0)
            diff += 24.0*SIDRATE;	/* not yesterday, today */
        mjd0 = lnoon + diff/24.0;	/* next guess at mjd of rise */
        stationary_riset (p,mjd0,np,hzn,&lstr,&x,&x,azr,&x,&x,&rss);
        if (rss != 0) goto chkrss;
        if (fabs (diff - lastdiff) < TMACC)
            break;
        lastdiff = diff;
    }
    if (pass == MAXPASSES)
        *status |= RS_NORISE;	/* didn't converge - no rise today */
    else
    {
        *ltr = 12.0 + diff;
        if (p != MOON &&
                (*ltr <= 24.0*(1.0-SIDRATE) || *ltr >= 24.0*SIDRATE))
            *status |= RS_2RISES;
    }

    /* find a better approximation to the setting circumstances based on
     * more passes, each using a "fixed" object at the location at
     * previous approximation of the set time.
     */
    lastdiff = 1000.0;
    for (pass = 1; pass < MAXPASSES; pass++)
    {
        diff = (lsts - lst)*SIDRATE; /* next guess at set time wrt noon */
        if (diff > 12.0)
            diff -= 24.0*SIDRATE;	/* not tomorrow, today */
        else if (diff < -12.0)
            diff += 24.0*SIDRATE;	/* not yesterday, today */
        mjd0 = lnoon + diff/24.0;	/* next guess at mjd of set */
        stationary_riset (p,mjd0,np,hzn,&x,&lsts,&x,&x,azs,&x,&rss);
        if (rss != 0) goto chkrss;
        if (fabs (diff - lastdiff) < TMACC)
            break;
        lastdiff = diff;
    }
    if (pass == MAXPASSES)
        *status |= RS_NOSET;	/* didn't converge - no set today */
    else
    {
        *lts = 12.0 + diff;
        if (p != MOON &&
                (*lts <= 24.0*(1.0-SIDRATE) || *lts >= 24.0*SIDRATE))
            *status |= RS_2SETS;
    }

transit:
    /* find a better approximation to the transit circumstances based on
     * more passes, each using a "fixed" object at the location at
     * previous approximation of the transit time.
     */
    lastdiff = 1000.0;
    for (pass = 1; pass < MAXPASSES; pass++)
    {
        diff = (lstt - lst)*SIDRATE; /*next guess at transit time wrt noon*/
        if (diff > 12.0)
            diff -= 24.0*SIDRATE;	/* not tomorrow, today */
        else if (diff < -12.0)
            diff += 24.0*SIDRATE;	/* not yesterday, today */
        mjd0 = lnoon + diff/24.0;	/* next guess at mjd of transit */
        stationary_riset (p,mjd0,np,hzn,&x,&x,&lstt,&x,&x,altt,&rss);
        if (fabs (diff - lastdiff) < TMACC)
            break;
        lastdiff = diff;
    }
    if (pass == MAXPASSES)
        *status |= RS_NOTRANS;	/* didn't converge - no transit today */
    else
    {
        *ltt = 12.0 + diff;
        if (p != MOON &&
                (*ltt <= 24.0*(1.0-SIDRATE) || *ltt >= 24.0*SIDRATE))
            *status |= RS_2TRANS;
    }
}

static
void stationary_riset (p, mjd0, np, hzn, lstr, lsts, lstt, azr, azs, altt, status)
int p;
double mjd0;
Now *np;
int hzn;
double *lstr, *lsts, *lstt;
double *azr, *azs, *altt;
int *status;
{
    extern void bye();
    double dis;
    Now n;
    Sky s;

    /* find object p's topocentric ra/dec at mjd0
     * (this must include parallax)
     */
    n = *np;
    n.n_mjd = mjd0;
    (void) body_cir (p, 0.0, &n, &s);
    if (epoch != EOD)
        precess (epoch, mjd0, &s.s_ra, &s.s_dec);
    if (s.s_edist > 0)
    {
        /* parallax, if we can */
        double ehp, lst, ha;
        if (p == MOON)
            ehp = asin (6378.14/s.s_edist);
        else
            ehp = (2.*6378./146e6)/s.s_edist;
        now_lst (&n, &lst);
        ha = hrrad(lst) - s.s_ra;
        ta_par (ha, s.s_dec, lat, height, ehp, &ha, &s.s_dec);
        s.s_ra = hrrad(lst) - ha;
        range (&s.s_ra, 2*PI);
    }

    switch (hzn)
    {
    case STDHZN:
        /* nominal atmospheric refraction.
         * then add nominal moon or sun semi-diameter, as appropriate.
         * other objects assumes to be negligibly small.
         */
        dis = STDREF;
        if (p == MOON || p == SUN)
            dis += degrad (32./60./2.);
        break;
    case TWILIGHT:
        if (p != SUN)
        {
            f_msg ("Non-sun twilight bug!");
            bye();
        }
        dis = TWIREF;
        break;
    case ADPHZN:
        /* adaptive includes actual refraction conditions and also
         * includes object's semi-diameter.
         */
        unrefract (pressure, temp, 0.0, &dis);
        dis = -dis;
        dis += degrad(s.s_size/3600./2.0);
        break;
    }

    riset (s.s_ra, s.s_dec, lat, dis, lstr, lsts, azr, azs, status);
    transit (s.s_ra, s.s_dec, np, lstt, altt);
}


/* find when and how hi object at (r,d) is when it transits. */
static
void transit (r, d, np, lstt, altt)
double r, d;	/* ra and dec, rads */
Now *np;	/* for refraction info */
double *lstt;	/* local sidereal time of transit */
double *altt;	/* local, refracted, altitude at time of transit */
{
    *lstt = radhr(r);
    *altt = PI/2 - lat + d;
    if (*altt > PI/2)
        *altt = PI - *altt;
    refract (pressure, temp, *altt, altt);
}
