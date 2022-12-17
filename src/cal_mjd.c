#include <stdio.h>
#include <math.h>
#include "astro.h"

/* given a date in months, mn, days, dy, years, yr,
 * return the modified Julian date (number of days elapsed since 1900 jan 0.5),
 * *mjd.
 */
void cal_mjd (mn, dy, yr, mjd)
int mn, yr;
double dy;
double *mjd;
{
    int b, d, m, y;
    long c;

    m = mn;
    y = (yr < 0) ? yr + 1 : yr;
    if (mn < 3)
    {
        m += 12;
        y -= 1;
    }

    if (yr < 1582 || (yr == 1582 && (mn < 10 || (mn == 10 && dy < 15))))
        b = 0;
    else
    {
        int a;
        a = y/100;
        b = 2 - a + a/4;
    }

    if (y < 0)
        c = (long)((365.25*y) - 0.75) - 694025L;
    else
        c = (long)(365.25*y) - 694025L;

    d = 30.6001*(m+1);

    *mjd = b + c + d + dy - 0.5;
}

/* given the modified Julian date (number of days elapsed since 1900 jan 0.5,),
 * mjd, return the calendar date in months, *mn, days, *dy, and years, *yr.
 */
void mjd_cal (mjd, mn, dy, yr)
double mjd;
int *mn, *yr;
double *dy;
{
    double d, f;
    double i, a, b, ce, g;

    d = mjd + 0.5;
    i = floor(d);
    f = d-i;
    if (f == 1)
    {
        f = 0;
        i += 1;
    }

    if (i > -115860.0)
    {
        a = floor((i/36524.25)+.9983573)+14;
        i += 1 + a - floor(a/4.0);
    }

    b = floor((i/365.25)+.802601);
    ce = i - floor((365.25*b)+.750001)+416;
    g = floor(ce/30.6001);
    *mn = g - 1;
    *dy = ce - floor(30.6001*g)+f;
    *yr = b + 1899;

    if (g > 13.5)
        *mn = g - 13;
    if (*mn < 2.5)
        *yr = b + 1900;
    if (*yr < 1)
        *yr -= 1;
}

/* given an mjd, set *dow to 0..6 according to which dayof the week it falls
 * on (0=sunday) or set it to -1 if can't figure it out.
 */
void mjd_dow (mjd, dow)
double mjd;
int *dow;
{
    /* cal_mjd() uses Gregorian dates on or after Oct 15, 1582.
     * (Pope Gregory XIII dropped 10 days, Oct 5..14, and improved the leap-
     * year algorithm). however, Great Britian and the colonies did not
     * adopt it until Sept 14, 1752 (they dropped 11 days, Sept 3-13,
     * due to additional accumulated error). leap years before 1752 thus
     * can not easily be accounted for from the cal_mjd() number...
     */
    if (mjd < -53798.5)
    {
        /* pre sept 14, 1752 too hard to correct */
        *dow = -1;
        return;
    }
    *dow = ((long)floor(mjd-.5) + 1) % 7;/* 1/1/1900 (mjd 0.5) is a Monday*/
    if (*dow < 0)
        *dow += 7;
}

/* given a mjd, return the the number of days in the month.  */
void mjd_dpm (mjd, ndays)
double mjd;
int *ndays;
{
    static short dpm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int m, y;
    double d;

    mjd_cal (mjd, &m, &d, &y);
    *ndays = (m==2 && ((y%4==0 && y%100!=0)||y%400==0)) ? 29 : dpm[m-1];
}


/* given a mjd, return the year as a double. */
void mjd_year (mjd, yr)
double mjd;
double *yr;
{
    int m, y;
    double d;
    double e0, e1;	/* mjd of start of this year, start of next year */

    mjd_cal (mjd, &m, &d, &y);
    if (y == -1) y = -2;
    cal_mjd (1, 1.0, y, &e0);
    cal_mjd (1, 1.0, y+1, &e1);
    *yr = y + (mjd - e0)/(e1 - e0);
}

/* given a decimal year, return mjd */
void year_mjd (y, mjd)
double y;
double *mjd;
{
    double e0, e1;	/* mjd of start of this year, start of next year */
    int yf = floor (y);
    if (yf == -1) yf = -2;

    cal_mjd (1, 1.0, yf, &e0);
    cal_mjd (1, 1.0, yf+1, &e1);
    *mjd = e0 + (y - yf)*(e1-e0);
}
