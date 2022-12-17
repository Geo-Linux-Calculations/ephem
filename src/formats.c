/* basic formating routines.
 * all the screen oriented printing should go through here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "astro.h"
#include "screen.h"
#include "ephem.h"

/* suppress screen io if this is true, but always flog stuff.
 */
static int f_scrnoff;
void f_on ()
{
    f_scrnoff = 0;
}
void f_off ()
{
    f_scrnoff = 1;
}

/* draw n blanks at the given cursor position.  */
void f_blanks (r, c, n)
int r, c, n;
{
    if (f_scrnoff)
        return;
    c_pos (r, c);
    while (--n >= 0)
        putchar (' ');
}

/* print the given value, v, in "sexadecimal" format at [r,c]
 * ie, in the form A:m.P, where A is a digits wide, P is p digits.
 * if p == 0, then no decimal point either.
 */
void f_sexad (r, c, a, p, mod, v)
int r, c;
int a, p;	/* left space, min precision */
int mod;	/* don't let whole portion get this big */
double v;
{
    char astr[32], str[32];
    long dec;
    double frac;
    int visneg;
    double vsav = v;

    if (v >= 0.0)
        visneg = 0;
    else
    {
        if (v <= -0.5/60.0*pow(10.0,-1.0*p))
        {
            v = -v;
            visneg = 1;
        }
        else
        {
            /* don't show as negative if less than the precision showing */
            v = 0.0;
            visneg = 0;
        }
    }

    dec = v;
    frac = (v - dec)*60.0;
    (void) sprintf (str, "59.%.*s5", p, "999999999");
    if (frac >= atof (str))
    {
        dec += 1;
        frac = 0.0;
    }
    dec %= mod;
    if (dec == 0 && visneg)
        (void) strcpy (str, "-0");
    else
        (void) sprintf (str, "%ld", visneg ? -dec : dec);

    /* would just do this if Turbo-C 2.0 %?.0f" worked:
     * sprintf (astr, "%*s:%0*.*f", a, str, p == 0 ? 2 : p+3, p, frac);
     */
    if (p == 0)
        (void) sprintf (astr, "%*s:%02d", a, str, (int)(frac+0.5));
    else
        (void) sprintf (astr, "%*s:%0*.*f", a, str, p+3, p, frac);

    (void) flog_log (r, c, vsav, astr);

    f_string (r, c, astr);
}

/* print the given value, t, in sexagesimal format at [r,c]
 * ie, in the form T:mm:ss, where T is nd digits wide.
 * N.B. we assume nd >= 2.
 */
void f_sexag (r, c, nd, t)
int r, c, nd;
double t;
{
    char tstr[32];
    int h, m, s;
    int tisneg;

    dec_sex (t, &h, &m, &s, &tisneg);
    if (h == 0 && tisneg)
        (void) sprintf (tstr, "%*s-0:%02d:%02d", nd-2, "", m, s);
    else
        (void) sprintf (tstr, "%*d:%02d:%02d", nd, tisneg ? -h : h, m, s);

    (void) flog_log (r, c, t, tstr);
    f_string (r, c, tstr);
}

/* print angle ra, in radians, in ra hours as hh:mm.m at [r,c]
 * N.B. we assume ra is >= 0.
 */
void f_ra (r, c, ra)
int r, c;
double ra;
{
    f_sexad (r, c, 2, 1, 24, radhr(ra));
}

/* print time, t, as hh:mm:ss */
void f_time (r, c, t)
int r, c;
double t;
{
    f_sexag (r, c, 2, t);
}

/* print time, t, as +/-hh:mm:ss (don't show leading +) */
void f_signtime (r, c, t)
int r, c;
double t;
{
    f_sexag (r, c, 3, t);
}

/* print time, t, as hh:mm */
void f_mtime (r, c, t)
int r, c;
double t;
{
    f_sexad (r, c, 2, 0, 24, t);
}

/* print angle, a, in rads, as degress at [r,c] in form ddd:mm */
void f_angle(r, c, a)
int r, c;
double a;
{
    f_sexad (r, c, 3, 0, 360, raddeg(a));
}

/* print angle, a, in rads, as degress at [r,c] in form dddd:mm:ss */
void f_gangle(r, c, a)
int r, c;
double a;
{
    f_sexag (r, c, 4, raddeg(a));
}

/* print the given modified Julian date, jd, as the starting date at [r,c]
 * in the form mm/dd/yyyy.
 */
void f_date (r, c, jd)
int r, c;
double jd;
{
    char dstr[32];
    int m, y;
    double d, tmp;

    mjd_cal (jd, &m, &d, &y);
    (void) sprintf (dstr, "%2d/%02d/%-4d", m, (int)(d), y);

    /* shadow to the plot subsystem as years. */
    mjd_year (jd, &tmp);
    (void) flog_log (r, c, tmp, dstr);
    f_string (r, c, dstr);
}

/* print the given double as a rounded int, with the given format.
 * this is used to plot full precision, but display far less.
 * N.B. caller beware that we really do expect fmt to refer to an int, not
 *   a long for example. also beware of range that implies.
 */
void f_int (row, col, fmt, f)
int row, col;
char fmt[];
double f;
{
    char str[80];
    int i;

    i = (f < 0) ? (int)(f-0.5) : (int)(f+0.5);
    (void) sprintf (str, fmt, i);

    (void) flog_log (row, col, f, str);
    f_string (row, col, str);
}

void f_char (row, col, c)
int row, col, c;
{
    if (f_scrnoff)
        return;
    c_pos (row, col);
    putchar (c);
}

void f_string (r, c, s)
int r, c;
char *s;
{
    if (f_scrnoff)
        return;
    c_pos (r, c);
    (void) fputs (s, stdout);
}

void f_double (r, c, fmt, f)
int r, c;
char *fmt;
double f;
{
    char str[80];
    (void) sprintf (str, fmt, f);
    (void) flog_log (r, c, f, str);
    f_string (r, c, str);
}

/* print prompt line */
void f_prompt (p)
char *p;
{
    c_pos (R_PROMPT, C_PROMPT);
    c_eol ();
    c_pos (R_PROMPT, C_PROMPT);
    (void) fputs (p, stdout);
}

/* clear from [r,c] to end of line, if we are drawing now. */
void f_eol (r, c)
int r, c;
{
    if (!f_scrnoff)
    {
        c_pos (r, c);
        c_eol();
    }
}

/* print a message and wait for op to hit any key */
void f_msg (m)
char *m;
{
    f_prompt (m);
    (void) read_char();
}

/* crack a line of the form X?X?X into its components,
 *   where X is an integer and ? can be any character except '0-9' or '-',
 *   such as ':' or '/'.
 * only change those fields that are specified:
 *   eg:  ::10	only changes *s
 *        10    only changes *d
 *        10:0  changes *d and *m
 * if see '-' anywhere, first non-zero component will be made negative.
 */
void f_sscansex (bp, d, m, s)
char *bp;
int *d, *m, *s;
{
    char c;
    int *p = d;
    int *nonzp = 0;
    int sawneg = 0;
    int innum = 0;

    while ((c = *bp++))
        if (isdigit(c))
        {
            if (!innum)
            {
                *p = 0;
                innum = 1;
            }
            *p = *p*10 + (c - '0');
            if (*p && !nonzp)
                nonzp = p;
        }
        else if (c == '-')
        {
            sawneg = 1;
        }
        else if (c != ' ')
        {
            /* advance to next component */
            p = (p == d) ? m : s;
            innum = 0;
        }

    if (sawneg && nonzp)
        *nonzp = -*nonzp;
}

/* crack a floating date string, bp, of the form m/d/y, where d may be a
 *   floating point number, into its components.
 * leave any component unspecified unchanged.
 * actually, the slashes may be anything but digits or a decimal point.
 * this is functionally the same as f_sscansex() exept we allow for
 *   the day portion to be real, and we don't handle negative numbers.
 *   maybe someday we could make a combined one and use it everywhere.
 */
void f_sscandate (bp, m, d, y)
char *bp;
int *m, *y;
double *d;
{
    char *bp0, c;

    bp0 = bp;
    while ((c = *bp++) && isdigit(c))
        continue;
    if (bp > bp0+1)
        *m = atoi (bp0);
    if (c == '\0')
        return;
    bp0 = bp;
    while ((c = *bp++) && (isdigit(c) || c == '.'))
        continue;
    if (bp > bp0+1)
        *d = atof (bp0);
    if (c == '\0')
        return;
    bp0 = bp;
    while ((c = *bp++))
        continue;
    if (bp > bp0+1)
        *y = atoi (bp0);
}

/* just like dec_sex() but makes the first non-zero element negative if
 * x is negative (instead of returning a sign flag).
 */
void f_dec_sexsign (x, h, m, s)
double x;
int *h, *m, *s;
{
    int n;
    dec_sex (x, h, m, s, &n);
    if (n)
    {
        if (*h)
            *h = -*h;
        else if (*m)
            *m = -*m;
        else
            *s = -*s;
    }
}

/* return 1 if bp looks like a decimal year; else 0.
 * any number greater than 12 or less than 0 is assumed to be a year, or any
 * string with exactly one decimal point, an optional minus sign, and nothing
 * else but digits.
 */
int decimal_year (bp)
char *bp;
{
    char c;
    int ndig = 0, ndp = 0, nneg = 0, nchar = 0;
    double y = atof(bp);

    while ((c = *bp++))
    {
        nchar++;
        if (isdigit(c))
            ndig++;
        else if (c == '.')
            ndp++;
        else if (c == '-')
            nneg++;
    }

    return (y > 12 || y < 0
            || (ndp == 1 && nneg <= 1 && nchar == ndig+ndp+nneg));
}
