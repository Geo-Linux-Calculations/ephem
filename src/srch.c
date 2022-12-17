/* this file contains functions to support iterative ephem searches.
 * we support several kinds of searching and solving algorithms.
 * values used in the evaluations come from the field logging flog.c system.
 * the expressions being evaluated are compiled and executed from compiler.c.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "screen.h"
#include "ephem.h"

static int (*srch_f)();
static int srch_tmscalled;
static char expbuf[80];		/* [0] == '\0' when expression is invalid */
static double tmlimit = 1./60.;	/* search accuracy, in hrs; def is one minute */

static void set_function();
static void set_accuracy();
static int srch_minmax(double mjd, double v, double *tmincp);
static int srch_solve0(double mjd, double v, double *tmincp);
static int srch_binary(double mjd, double v, double *tmincp);

void srch_setup()
{
    int srch_minmax(), srch_solve0(), srch_binary();
    static char *chcs[] =
    {
        "Find extreme", "Find 0", "Binary", "New function", "Accuracy",
        "Stop"
    };
    static int fn;	/* start with 0, then remember for next time */

    /* let op select algorithm, edit, set accuracy
     * or stop if currently searching
     * algorithms require a function.
     */
ask:
    switch (popup(chcs, fn, srch_f ? 6 : 5))
    {
    case 0:
        fn = 0;
        if (expbuf[0] == '\0')
            set_function();
        srch_f = expbuf[0] ? srch_minmax : (int (*)())0;
        if (srch_f)
            break;
        else
            goto ask;
    case 1:
        fn = 1;
        if (expbuf[0] == '\0')
            set_function();
        srch_f = expbuf[0] ? srch_solve0 : (int (*)())0;
        if (srch_f)
            break;
        else
            goto ask;
    case 2:
        fn = 2;
        if (expbuf[0] == '\0')
            set_function();
        srch_f = expbuf[0] ? srch_binary : (int (*)())0;
        if (srch_f)
            break;
        else
            goto ask;
    case 3:
        fn = 3;
        srch_f = 0;
        set_function();
        goto ask;
    case 4:
        fn = 4;
        srch_f = 0;
        set_accuracy();
        goto ask;
    case 5:
        srch_f = 0;
        srch_prstate(0);
        return;
    default:
        return;
    }

    /* new search */
    srch_tmscalled = 0;
    srch_prstate (0);
}

/* if searching is in effect call the search type function.
 * it might modify *tmincp according to where it next wants to eval.
 * (remember tminc is in hours, not days).
 * if searching ends for any reason it is also turned off.
 * also, flog the new value.
 * return 0 if caller can continue or -1 if it is time to stop.
 */
int srch_eval(mjd, tmincp)
double mjd;
double *tmincp;
{
    char errbuf[128];
    int s;
    double v;

    if (!srch_f)
        return (0);

    if (execute_expr (&v, errbuf) < 0)
    {
        s = -1;
        srch_f = 0;
        f_msg (errbuf);
    }
    else
    {
        s = (*srch_f)(mjd, v, tmincp);
        if (s < 0)
            srch_f = 0;
        (void) flog_log (R_SRCH, C_SRCH, v, "");
        srch_tmscalled++;
    }

    srch_prstate (0);
    return (s);
}

/* print state of searching. */
void srch_prstate (force)
int force;
{
    int srch_minmax(), srch_solve0(), srch_binary();
    static int (*last)();

    if (force || srch_f != last)
    {
        f_string (R_SRCH, C_SRCHV,
                  srch_f == srch_minmax   ? "Extrema" :
                  srch_f == srch_solve0   ? " Find 0" :
                  srch_f == srch_binary ?   " Binary" :
                  "    off");
        last = srch_f;
    }
}

int srch_ison()
{
    return (srch_f != 0);
}

/* display current expression. then if type in at least one char make it the
 * current expression IF it compiles ok.
 * TODO: editing?
 */
static
void set_function()
{
    static char prompt[] = "Function: ";
    char newexp[NC];
    int s;

    f_prompt (prompt);
    (void) fputs (expbuf, stdout);
    c_pos (R_PROMPT, sizeof(prompt));

    s = read_line (newexp, PW-sizeof(prompt));
    if (s >= 0)
    {
        char errbuf[NC];
        if (s > 0 && compile_expr (newexp, errbuf) < 0)
            f_msg (errbuf);
        else
            (void) strcpy (expbuf, newexp);
    }
}

static
void set_accuracy()
{
    static char p[] = "Desired accuracy (         hrs): ";
    int hrs, mins, secs;
    char buf[NC];

    f_prompt (p);
    f_time (R_PROMPT, C_PROMPT+18, tmlimit); /* place in blank spot */
    c_pos (R_PROMPT, sizeof(p));
    if (read_line (buf, PW-sizeof(p)) > 0)
    {
        f_dec_sexsign (tmlimit, &hrs, &mins, &secs);
        f_sscansex (buf, &hrs, &mins, &secs);
        sex_dec (hrs, mins, secs, &tmlimit);
    }
}

/* use successive paraboloidal fits to find when expression is at a
 * local minimum or maximum.
 */
static
int srch_minmax(mjd, v, tmincp)
double mjd;
double v;
double *tmincp;
{
    static double base;		/* for better stability */
    static double x_1, x_2, x_3;	/* keep in increasing order */
    static double y_1, y_2, y_3;
    double xm, a, b;

    if (srch_tmscalled == 0)
    {
        base = mjd;
        x_1 = 0.0;
        y_1 = v;
        return (0);
    }
    mjd -= base;
    if (srch_tmscalled == 1)
    {
        /* put in one of first two slots */
        if (mjd < x_1)
        {
            x_2 = x_1;
            y_2 = y_1;
            x_1 = mjd;
            y_1 = v;
        }
        else
        {
            x_2 = mjd;
            y_2 = v;
        }
        return (0);
    }
    if (srch_tmscalled == 2 || fabs(mjd - x_1) < fabs(mjd - x_3))
    {
        /* closer to x_1 so discard x_3.
         * or if it's our third value we know to "discard" x_3.
         */
        if (mjd > x_2)
        {
            x_3 = mjd;
            y_3 = v;
        }
        else
        {
            x_3 = x_2;
            y_3 = y_2;
            if (mjd > x_1)
            {
                x_2 = mjd;
                y_2 = v;
            }
            else
            {
                x_2 = x_1;
                y_2 = y_1;
                x_1 = mjd;
                y_1 = v;
            }
        }
        if (srch_tmscalled == 2)
            return (0);
    }
    else
    {
        /* closer to x_3 so discard x_1 */
        if (mjd < x_2)
        {
            x_1 = mjd;
            y_1 = v;
        }
        else
        {
            x_1 =  x_2;
            y_1 = y_2;
            if (mjd < x_3)
            {
                x_2 = mjd;
                y_2 = v;
            }
            else
            {
                x_2 =  x_3;
                y_2 = y_3;
                x_3 = mjd;
                y_3 = v;
            }
        }
    }

#ifdef TRACEMM
    {
        char buf[NC];
        sprintf (buf, "x_1=%g y_1=%g x_2=%g y_2=%g x_3=%g y_3=%g",
                 x_1, y_1, x_2, y_2, x_3, y_3);
        f_msg (buf);
    }
#endif
    a = y_1*(x_2-x_3) - y_2*(x_1-x_3) + y_3*(x_1-x_2);
    if (fabs(a) < 1e-10)
    {
        /* near-0 zero denominator, ie, curve is pretty flat here,
         * so assume we are done enough.
         * signal this by forcing a 0 tminc.
         */
        *tmincp = 0.0;
        return (-1);
    }
    b = (x_1*x_1)*(y_2-y_3) - (x_2*x_2)*(y_1-y_3) + (x_3*x_3)*(y_1-y_2);
    xm = -b/(2.0*a);
    *tmincp = (xm - mjd)*24.0;
    return (fabs (*tmincp) < tmlimit ? -1 : 0);
}

/* use secant method to solve for time when expression passes through 0.
 */
static
int srch_solve0(mjd, v, tmincp)
double mjd;
double v;
double *tmincp;
{
    static double x0, x_1;	/* x(n-1) and x(n) */
    static double y_0, y_1;	/* y(n-1) and y(n) */
    double x_2;		/* x(n+1) */
    double df;		/* y(n) - y(n-1) */

    switch (srch_tmscalled)
    {
    case 0:
        x0 = mjd;
        y_0 = v;
        return(0);
    case 1:
        x_1 = mjd;
        y_1 = v;
        break;
    default:
        x0 = x_1;
        y_0 = y_1;
        x_1 = mjd;
        y_1 = v;
        break;
    }

    df = y_1 - y_0;
    if (fabs(df) < 1e-10)
    {
        /* near-0 zero denominator, ie, curve is pretty flat here,
         * so assume we are done enough.
         * signal this by forcing a 0 tminc.
         */
        *tmincp = 0.0;
        return (-1);
    }
    x_2 = x_1 - y_1*(x_1-x0)/df;
    *tmincp = (x_2 - mjd)*24.0;
    return (fabs (*tmincp) < tmlimit ? -1 : 0);
}

/* binary search for time when expression changes from its initial state.
 * if the change is outside the initial tminc range, then keep searching in that
 *    direction by tminc first before starting to divide down.
 */
static
int srch_binary(mjd, v, tmincp)
double mjd;
double v;
double *tmincp;
{
    static double lb, ub;		/* lower and upper bound */
    static int initial_state;
    int this_state = v >= 0.5;

#define	FLUNDEF	-9e10

    if (srch_tmscalled == 0)
    {
        if (*tmincp >= 0.0)
        {
            /* going forwards in time so first mjd is lb and no ub yet */
            lb = mjd;
            ub = FLUNDEF;
        }
        else
        {
            /* going backwards in time so first mjd is ub and no lb yet */
            ub = mjd;
            lb = FLUNDEF;
        }
        initial_state = this_state;
        return (0);
    }

    if (ub != FLUNDEF && lb != FLUNDEF)
    {
        if (this_state == initial_state)
            lb = mjd;
        else
            ub = mjd;
        *tmincp = ((lb + ub)/2.0 - mjd)*24.0;
#ifdef TRACEBIN
        {
            char buf[NC];
            sprintf (buf, "lb=%g ub=%g tminc=%g mjd=%g is=%d ts=%d",
                     lb, ub, *tmincp, mjd, initial_state, this_state);
            f_msg (buf);
        }
#endif
        /* signal to stop if asking for time change less than TMLIMIT */
        return (fabs (*tmincp) < tmlimit ? -1 : 0);
    }
    else if (this_state != initial_state)
    {
        /* gone past; turn around half way */
        if (*tmincp >= 0.0)
            ub = mjd;
        else
            lb = mjd;
        *tmincp /= -2.0;
        return (0);
    }
    else
    {
        /* just keep going, looking for first state change but we keep
         * learning the lower (or upper, if going backwards) bound.
         */
        if (*tmincp >= 0.0)
            lb = mjd;
        else
            ub = mjd;
        return (0);
    }
}
