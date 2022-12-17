/* main "ephem" program.
 * -------------------------------------------------------------------
 * Copyright (c) 1990,1991,1992 by Elwood Charles Downey
 *
 * Permission is granted to make and distribute copies of this program
 * free of charge, provided the copyright notice and this permission
 * notice are preserved on all copies.  All other rights reserved.
 * -------------------------------------------------------------------
 * set options.
 * init screen and circumstances.
 * enter infinite loop updating screen and allowing operator input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "astro.h"
#include "circum.h"
#include "ephem.h"
#include "screen.h"

extern char *getenv();

/* shorthands for fields of a Now structure, now.
 * first undo the ones for a Now pointer from circum.h.
 */
#undef mjd
#undef lat
#undef lng
#undef tz
#undef temp
#undef pressure
#undef height
#undef epoch
#undef tznm

#define mjd	now.n_mjd
#define lat	now.n_lat
#define lng	now.n_lng
#define tz	now.n_tz
#define temp	now.n_temp
#define pressure now.n_pressure
#define height	now.n_height
#define epoch	now.n_epoch
#define tznm	now.n_tznm

static jmp_buf fpe_err_jmp;	/* used to recover from SIGFPE */
static char *cfgfile;		/* !0 if -c used */
static char *watchmode;	/* !0 if -W used */
static char cfgdef[] = "ephem.cfg"; /* default configuration file name */
static Now now;		/* where when and how, right now */
static double tminc;	/* hrs to inc time by each loop; RTC means use clock */
static int nstep;	/* steps to go before stopping */
static int spause;	/* secs to pause between steps */
static int optwi;	/* set when want to display dawn/dusk/len-of-night */
static int oppl;	/* mask of (1<<planet) bits; set when want to show it */

static void on_fpe();
static void read_fieldargs ();
static void read_cfgfile();
static int crack_fieldset ();
static int chg_fld ();
static void print_tminc();
static void print_alt ();
static void print_nstep();
static void print_spause();
static void toggle_body ();
int set_app_color();
static void usage(char *why);
#ifdef NCURSES_LARGE
void _init_fields();
void sky_dome_labels();
void handle_winch();
struct sigaction sa;    /* signal handling struct for SIGWINCH screen resize */
int watching = 0;  /* true if we're in a watch mode */
#endif
#ifdef ANSI_COLORS
void init_app_colors();
#endif

int main (ac, av)
int ac;
char *av[];
{
    void bye();
    static char freerun[] =
        "Running... press any key to stop to make changes.";
    static char prmpt[] =
        "Move to another field, RETURN to change this field, ? for help, or q to run";
    static char hlp[] =
        "arrow keys move to field; any key stops running; ^d exits; ^l redraws";
    int fld = rcfpack(R_NSTEP, C_NSTEPV, 0); /* initial cursor loc */
    int sflag = 0;	/* not silent, by default */
    int main_menu_loop = 1; /* continue mm looping ? */
    int srchdone = 0; /* true when search funcs say so */
    int newcir = 2;	/* set when circumstances change - means don't tminc */

#if defined(WIN32) && defined(_CONSOLE)
    initscr();
#endif
#ifdef NCURSES_LARGE
    initscr();
    _init_fields();
#endif
#ifdef ANSI_COLORS
    init_app_colors();
#endif

    while ((--ac > 0) && (**++av == '-'))
    {
        char *s;
        for (s = *av+1; *s != '\0'; s++)
            switch (*s)
            {
            case 'W': /* start in a watch mode 1-5 */
                if (--ac <= 0) usage("-W but no mode");
                watchmode = *++av;
                break;
            case 'c': /* set name of config file to use */
                if (--ac <= 0) usage("-c but no config file");
                cfgfile = *++av;
                break;
            case 'd': /* set alternate database file name */
                if (--ac <= 0) usage("-d but no database file");
                obj_setdbfilename (*++av);
                break;
            case 's': /* no credits "silent" (don't publish this) */
                sflag++;
                break;
            default:
                usage("Bad - option");
            }
    }

    int watch_mode_code = 0; /* n.b.: the WATCH_* code +1 */
    if (watchmode)
    {
        watch_mode_code = atoi(watchmode);
        if ( watch_mode_code < 1 || watch_mode_code > 5 )
            usage( "watch mode: -W m\n"
                   "Where m is 1-5; 1=Sky Dome 2=AltAz 3=Solar Sys 4=Earth 5=Moon");
    }

    if (!sflag)
        credits();

    /* fresh screen.
     * crack config file, THEN args so args may override.
     */
    c_erase();
    read_cfgfile ();
    read_fieldargs (ac, av);

    /* set up to clean up screen and tty if interrupted.
     * also set up to stop if get floating error.
     */
    (void) signal (SIGINT, bye);
    (void) signal (SIGFPE, on_fpe);

#ifdef NCURSES_LARGE
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);
#endif

    /* optional watch mode from command line */
    if (watch_mode_code)
    {
        watch_function(watch_mode_code-1, &now, tminc, oppl);
        watch_menu (&now, tminc, oppl);
        set_t0 (&now);
    }

    /* update screen forever (until QUIT) */
    while (main_menu_loop)
    {

        /* if get a floating error, longjmp() here and stop looping */
        if (setjmp (fpe_err_jmp))
            nstep = 0;
        else
        {
            nstep -= 1;

            /* recalculate everything and update all the fields */
            redraw_screen (newcir);
            mm_newcir (0);

            /* let searching functions change tminc and check for done */
            srchdone = srch_eval (mjd, &tminc) < 0;
            print_tminc(0);	/* to show possibly new search increment */

            /* update plot and listing files, now that all fields are up
             * to date and search function has been evaluated.
             */
            plot();
            listing();

            /* handle spause if we are really looping */
            if (nstep > 0)
                slp_sync();
        }

        /* stop loop to allow op to change parameters:
         * if a search evaluation converges (or errors out),
         * or if steps are done,
         * or if op hits any key.
         */
        newcir = 0;
        if (srchdone || nstep <= 0 || (chk_char()==0 && read_char()!=0))
        {
            int nfld;

            /* update screen with the current stuff if stopped during
             * unattended plotting or listing since last redraw_screen()
             * didn't.
             */
            if ((plot_ison() || listing_ison()) && nstep > 0)
                redraw_screen (1);

            /* return nstep to default of 1 */
            if (nstep <= 0)
            {
                nstep = 1;
                print_nstep (0);
            }

            /* change fields until END.
             * update all time fields if any are changed
             * and print NEW CIRCUMSTANCES if any have changed.
             * QUIT causes bye() to be called and we never return.
             */
            while((nfld = sel_fld(fld,alt_menumask()|F_CHG,prmpt,hlp)) != 0)
            {
                if (chg_fld ((char *)0, &nfld))
                {
                    mm_now (&now, 1);
                    mm_newcir(1);
                    newcir = 1;
                }
                fld = nfld;
            }
            if (nstep > 1)
                f_prompt (freerun);
        }

        /* increment time only if op didn't change cirumstances */
        if (!newcir)
            inc_mjd (&now, tminc);
    }

#ifdef NCURSES_LARGE
    endwin();
#endif
    return (0);
}

/* read in ephem's configuration file, if any.
 * if errors in file, call usage() (which exits).
 * if use -d, require it; else try $EPHEMCFG and ephem.cfg but don't
 *   complain if can't find these since, after all, one is not required.
 * skip all lines that doesn't begin with an alpha char.
 */
static void
read_cfgfile()
{
    char buf[128];
    FILE *fp;
    char *fn;

    /* open the config file.
     * only REQUIRED if used -d option.
     * if succcessful, fn points to file name.
     */
    if (cfgfile)
    {
        fn = cfgfile;
        fp = fopen (fn, "r");
        if (!fp)
        {
            (void) sprintf (buf, "Can not open %s", fn);
            usage (buf);	/* does not return */
        }
    }
    else
    {
        fn = getenv ("EPHEMCFG");
        if (!fn)
            fn = cfgdef;
    }
    fp = fopen (fn, "r");
    if (!fp)
        return;	/* oh well; after all, it's not required */

    while (fgets (buf, sizeof(buf), fp))
    {
        if (!isalpha(buf[0]))
            continue;
        buf[strlen(buf)-1] = '\0';		/* discard trailing \n */
        if (crack_fieldset (buf) < 0)
        {
            if ( set_app_color(buf) < 0 )
            {
                char why[NC];
                (void) sprintf (why, "Bad field spec in %s: %s\n", fn, buf);
                usage (why);
            }
        }
    }
    (void) fclose (fp);
}


/* draw all the stuff on the screen, using the current menu.
 * if how_much == 0 then just update fields that need it;
 * if how_much == 1 then redraw all fields;
 * if how_much == 2 then erase the screen and redraw EVERYTHING.
 */
void redraw_screen (how_much)
int how_much;
{
    if (how_much == 2)
        c_erase();

    /* print the single-step message if this is the last loop */
    if (nstep < 1)
        print_updating();

    if (how_much == 2)
    {

        COLOR_CODE(COLOR_BORDERS);
        mm_borders();
        COLOR_OFF;

        COLOR_CODE(COLOR_MM_LABELS);
        mm_labels();
        COLOR_OFF;

        COLOR_CODE(COLOR_SRCH_PRSTATE);
        srch_prstate(1);
        COLOR_OFF;

        COLOR_CODE(COLOR_PLOT_PRSTATE);
        plot_prstate(1);
        COLOR_OFF;

        COLOR_CODE(COLOR_LISTING_PRSTATE);
        listing_prstate(1);
        COLOR_OFF;
    }

    /* if just updating changed fields while plotting or listing
     * unattended then suppress most screen updates except
     * always show nstep to show plot loops to go and
     * always show tminc to show search convergence progress.
     */
    print_nstep(how_much);
    print_tminc(how_much);
    print_spause(how_much);
    if (how_much == 0 && (plot_ison() || listing_ison()) && nstep > 0)
        f_off();

    /* print all the time-related fields */
    COLOR_CODE(COLOR_NOW);
    mm_now (&now, how_much);
    COLOR_OFF;

    COLOR_CODE(COLOR_TWILIGHT);
    if (optwi)
        mm_twilight (&now, how_much);
    COLOR_OFF;

    /* print stuff on bottom menu */
    print_alt (how_much);

    f_on();
}

/* clean up and exit.
 */
void
bye()
{
    c_erase();
    byetty();
#ifdef NCURSES_LARGE
    endwin();
#endif
    exit (0);
}

/* this gets called when a floating point error occurs.
 * we force a jump back into main() with looping terminated.
 */
static void
on_fpe()
{
    extern void longjmp();

    (void) signal (SIGFPE, on_fpe);
    f_msg ("Floating point error has occurred - computations aborted.");
    longjmp (fpe_err_jmp, 1);
}

static void
usage(why)
char *why;
{
    /* don't advertise -s (silent) option */
    c_erase();
    f_string (1, 1, why);
    char msg[] =
        "Usage:\n ephem "
#ifdef _CIRCUM_H
        "[-W m] "
#endif
        "[-c <configfile>] [-d <database>] [field=value ...]\r\n";
    f_string (2, 1, msg );
    byetty();
#ifdef NCURSES_LARGE
    endwin();
    fprintf(stderr,"\n%s\n%s\n",why,msg);
#endif
    exit (1);
}

/* process the field specs from the command line.
 * if trouble call usage() (which exits).
 */
static void
read_fieldargs (ac, av)
int ac;		/* number of such specs */
char *av[];	/* array of strings in form <field_name value> */
{
    while (--ac >= 0)
    {
        char *fs = *av++;
        if (crack_fieldset (fs) < 0)
        {
            if ( set_app_color(fs) < 0 )
            {
                char why[NC];
                (void) sprintf (why, "Bad command line spec: %.*s",
                                (int)(sizeof(why)-26), fs);
                usage (why);
            }
        }
    }
}

/* process a field spec in buf, either from config file or argv.
 * return 0 if recognized ok, else -1.
 */
static int
crack_fieldset (buf)
char *buf;
{
#define	ARRAY_SIZ(a)	(sizeof(a)/sizeof((a)[0]))
#define	MAXKW		6	/* longest keyword, not counting trailing 0 */
    /* N.B. index of item is its case value, below.
     * N.B. if add an item, keep it no longer than MAXKW chars.
     */
    static char keywords[][MAXKW+1] =
    {
        /*  0 */	"LAT",
        /*  1 */	"LONG",
        /*  2 */	"UT",
        /*  3 */	"UD",
        /*  4 */	"TZONE",
        /*  5 */	"TZNAME",
        /*  6 */	"HEIGHT",
        /*  7 */	"NSTEP",
        /*  8 */	"PAUSE",
        /*  9 */	"STPSZ",
        /* 10 */	"TEMP",
        /* 11 */	"PRES",
        /* 12 */	"EPOCH",
        /* 13 */	"JD",
        /* 14 */	"OBJX",
        /* 15 */	"OBJY",
        /* 16 */	"PROPTS",
        /* 17 */	"MENU"
    };
    int i;
    int l;
    int f;

    for (i = 0; i < ARRAY_SIZ(keywords); i++)
        if (strncmp (keywords[i], buf, l = strlen(keywords[i])) == 0)
        {
            buf += l+1;	/* skip keyword and its subsequent delimiter */
            break;
        }

    switch (i)
    {
    case 0:
        f = rcfpack (R_LAT,C_LATV,0);
        (void) chg_fld (buf, &f);
        break;
    case 1:
        f = rcfpack (R_LONG,C_LONGV,0), (void) chg_fld (buf, &f);
        break;
    case 2:
        f = rcfpack (R_UT,C_UTV,0), (void) chg_fld (buf, &f);
        break;
    case 3:
        f = rcfpack (R_UD,C_UD,0), (void) chg_fld (buf, &f);
        break;
    case 4:
        f = rcfpack (R_TZONE,C_TZONEV,0), (void) chg_fld (buf, &f);
        break;
    case 5:
        f = rcfpack (R_TZN,C_TZN,0), (void) chg_fld (buf, &f);
        break;
    case 6:
        f = rcfpack (R_HEIGHT,C_HEIGHTV,0), (void) chg_fld (buf, &f);
        break;
    case 7:
        f = rcfpack (R_NSTEP,C_NSTEPV,0), (void) chg_fld (buf, &f);
        break;
    case 8:
        f = rcfpack (R_PAUSE,C_PAUSEV,0), (void) chg_fld (buf, &f);
        break;
    case 9:
        f = rcfpack (R_STPSZ,C_STPSZV,0), (void) chg_fld (buf, &f);
        break;
    case 10:
        f = rcfpack (R_TEMP,C_TEMPV,0), (void) chg_fld (buf, &f);
        break;
    case 11:
        f = rcfpack (R_PRES,C_PRESV,0), (void) chg_fld (buf, &f);
        break;
    case 12:
        f = rcfpack (R_EPOCH,C_EPOCHV,0), (void) chg_fld (buf, &f);
        break;
    case 13:
        f = rcfpack (R_JD,C_JDV,0), (void) chg_fld (buf, &f);
        break;
    case 14:
        (void) obj_filelookup (OBJX, buf);
        break;
    case 15:
        (void) obj_filelookup (OBJY, buf);
        break;
    case 16:
        if (buf[-1] != '+')
            optwi = oppl = 0;
        while (*buf)
            switch (*buf++)
            {
            case 'T':
                optwi = 1;
                break;
            case 'S':
                oppl |= (1<<SUN);
                break;
            case 'M':
                oppl |= (1<<MOON);
                break;
            case 'e':
                oppl |= (1<<MERCURY);
                break;
            case 'v':
                oppl |= (1<<VENUS);
                break;
            case 'm':
                oppl |= (1<<MARS);
                break;
            case 'j':
            case 'J':
                oppl |= (1<<JUPITER);
                break;
            case 's':
                oppl |= (1<<SATURN);
                break;
            case 'u':
                oppl |= (1<<URANUS);
                break;
            case 'n':
                oppl |= (1<<NEPTUNE);
                break;
            case 'p':
                oppl |= (1<<PLUTO);
                break;
            case 'x':
                oppl |= (1<<OBJX);
                obj_on(OBJX);
                break;
            case 'y':
                oppl |= (1<<OBJY);
                obj_on(OBJY);
                break;
            }
        break;
    case 17:
        if (strncmp (buf, "DATA", 4) == 0)
            altmenu_init (F_MNU1);
        else if (strncmp (buf, "RISET", 5) == 0)
            altmenu_init (F_MNU2);
        else if (strncmp (buf, "SEP", 3) == 0)
            altmenu_init (F_MNU3);
        else if (strncmp (buf, "JUP", 3) == 0)
            altmenu_init (F_MNUJ);
        break;
    default:
        return (-1);
    }
    return (0);
}

/* react to the field at *fld according to the optional string input at bp.
 * if bp is != 0 use it, else issue read_line() and use buffer.
 * then sscanf the buffer and update the corresponding (global) variable(s)
 * or do whatever a pick at that field should do.
 * we might also change *fld if we want to change the current cursor location.
 * return 1 if we change a field that invalidates any of the times or
 * to update all related fields.
 */
static int
chg_fld (bp, fld)
char *bp;
int *fld;
{
    char buf[NC];
    int deghrs = 0, mins = 0, secs = 0;
    int new = 0;

    /* switch on just the row/col portion */

#ifndef NCURSES_LARGE
    /* NR,NC & rfcpack are constant, so use a C switch statement */
#define M_SWITCH(e) switch(e)
#define M_CASE(c) case c:
#define M_BREAK1     break;
#define M_BREAK     break;
#define M_DONE  {}
#else
    /* NR,NC & rfcpack are variable, so fake a C switch */
#define M_SWITCH(e) int _e = e;
#define M_CASE(c) if (_e == (c)) {
#define M_BREAK1     goto _done
#define M_BREAK     goto _done; }
#define M_DONE  _done:{}
#endif

    M_SWITCH (unpackrc(*fld))
    {
        M_CASE( rcfpack (R_ALTM, C_ALTM, 0) )
        if (altmenu_setup() == 0)
        {
            print_updating();
            alt_erase();
            print_alt(2);
        }
        M_BREAK;
        M_CASE( rcfpack (R_JD, C_JDV, 0) )
        if (!bp)
        {
            static char p[] = "Julian Date (or n for Now): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'n' || bp[0] == 'N')
            time_fromsys (&now);
        else
            mjd = atof(bp) - 2415020L;
        set_t0 (&now);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_UD, C_UD, 0) )
        if (!bp)
        {
            static char p[] = "utc date (m/d/y, or year.d, or n for Now): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'n' || bp[0] == 'N')
            time_fromsys (&now);
        else
        {
            if (decimal_year(bp))
            {
                double y = atof (bp);
                year_mjd (y, &mjd);
            }
            else
            {
                double day, newmjd0;
                int month, year;
                mjd_cal (mjd, &month, &day, &year); /* init with now */
                f_sscandate (bp, &month, &day, &year);
                cal_mjd (month, day, year, &newmjd0);
                /* if don't give a fractional part to days
                 * then retain current hours.
                 */
                if ((long)day == day)
                    mjd = newmjd0 + mjd_hr(mjd)/24.0;
                else
                    mjd = newmjd0;
            }
        }
        set_t0 (&now);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_UT, C_UTV, 0) )
        if (!bp)
        {
            static char p[] = "utc time (h:m:s, or n for Now): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'n' || bp[0] == 'N')
            time_fromsys (&now);
        else
        {
            double newutc = (mjd-mjd_day(mjd)) * 24.0;
            f_dec_sexsign (newutc, &deghrs, &mins, &secs);
            f_sscansex (bp, &deghrs, &mins, &secs);
            sex_dec (deghrs, mins, secs, &newutc);
            mjd = mjd_day(mjd) + newutc/24.0;
        }
        set_t0 (&now);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_LD, C_LD, 0) )
        if (!bp)
        {
            static char p[] = "local date (m/d/y, or year.d, n for Now): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'n' || bp[0] == 'N')
            time_fromsys (&now);
        else
        {
            if (decimal_year(bp))
            {
                double y = atof (bp);
                year_mjd (y, &mjd);
                mjd += tz/24.0;
            }
            else
            {
                double day, newlmjd0;
                int month, year;
                mjd_cal (mjd-tz/24.0, &month, &day, &year); /* now */
                f_sscandate (bp, &month, &day, &year);
                cal_mjd (month, day, year, &newlmjd0);
                /* if don't give a fractional part to days
                 * then retain current hours.
                 */
                if ((long)day == day)
                    mjd = newlmjd0 + mjd_hr(mjd-tz/24.0)/24.0;
                else
                    mjd = newlmjd0;
                mjd += tz/24.0;
            }
        }
        set_t0 (&now);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_LT, C_LT, 0) )
        if (!bp)
        {
            static char p[] = "local time (h:m:s, or n for Now): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'n' || bp[0] == 'N')
            time_fromsys (&now);
        else
        {
            double newlt = (mjd-mjd_day(mjd)) * 24.0 - tz;
            range (&newlt, 24.0);
            f_dec_sexsign (newlt, &deghrs, &mins, &secs);
            f_sscansex (bp, &deghrs, &mins, &secs);
            sex_dec (deghrs, mins, secs, &newlt);
            mjd = mjd_day(mjd-tz/24.0) + (newlt + tz)/24.0;
        }
        set_t0 (&now);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_LST, C_LSTV, 0) )
        if (!bp)
        {
            static char p[] = "local sidereal time (h:m:s, or n for Now): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'n' || bp[0] == 'N')
            time_fromsys (&now);
        else
        {
            double lst, utc;
            now_lst (&now, &lst);
            f_dec_sexsign (lst, &deghrs, &mins, &secs);
            f_sscansex (bp, &deghrs, &mins, &secs);
            sex_dec (deghrs, mins, secs, &lst);
            lst -= radhr(lng); /* convert to gst */
            range (&lst, 24.0);
            gst_utc (mjd_day(mjd), lst, &utc);
            mjd = mjd_day(mjd) + utc/24.0;
        }
        set_t0 (&now);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_TZN, C_TZN, 0) )
        if (!bp)
        {
            static char p[] = "timezone abbreviation (3 char max): ";
            f_prompt (p);
            if (read_line (buf, 3) <= 0)
                M_BREAK1;
            bp = buf;
        }
        (void) memcpy (tznm, bp, sizeof(tznm)-1);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_TZONE, C_TZONEV, 0) )
        if (!bp)
        {
            static char p[] = "hours behind utc: ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        f_dec_sexsign (tz, &deghrs, &mins, &secs);
        f_sscansex (bp, &deghrs, &mins, &secs);
        sex_dec (deghrs, mins, secs, &tz);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_LONG, C_LONGV, 0) )
        if (!bp)
        {
            static char p[] = "longitude (+ west) (d:m:s): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        f_dec_sexsign (-raddeg(lng), &deghrs, &mins, &secs);
        f_sscansex (bp, &deghrs, &mins, &secs);
        sex_dec (deghrs, mins, secs, &lng);
        lng = degrad (-lng);		/* want - radians west */
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_LAT, C_LATV, 0) )
        if (!bp)
        {
            static char p[] = "latitude (+ north) (d:m:s): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        f_dec_sexsign (raddeg(lat), &deghrs, &mins, &secs);
        f_sscansex (bp, &deghrs, &mins, &secs);
        sex_dec (deghrs, mins, secs, &lat);
        lat = degrad (lat);
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_HEIGHT, C_HEIGHTV, 0) )
        if (!bp)
        {
            static char p[] = "height above sea level (ft): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (sscanf (bp, "%lf", &height) == 1)
        {
            height /= 2.093e7; /*convert ft to earth radii above sea level*/
            new = 1;
        }
        M_BREAK;
        M_CASE( rcfpack (R_NSTEP, C_NSTEPV, 0) )
        if (!bp)
        {
            static char p[] = "number of steps to run: ";
            f_prompt (p);
            if (read_line (buf, 8) <= 0)
                M_BREAK1;
            bp = buf;
        }
        (void) sscanf (bp, "%d", &nstep);
        print_nstep (0);
        M_BREAK;
        M_CASE( rcfpack (R_PAUSE, C_PAUSEV, 0) )
        if (!bp)
        {
            static char p[] = "seconds to pause between steps: ";
            f_prompt (p);
            if (read_line (buf, 8) <= 0)
                M_BREAK1;
            bp = buf;
        }
        (void) sscanf (bp, "%d", &spause);
        print_spause (0);
        M_BREAK;
        M_CASE( rcfpack (R_TEMP, C_TEMPV, 0) )
        if (!bp)
        {
            static char p[] = "temperature (deg.F): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (sscanf (bp, "%lf", &temp) == 1)
        {
            temp = 5./9.*(temp - 32.0);	/* want degs C */
            new = 1;
        }
        M_BREAK;
        M_CASE( rcfpack (R_PRES, C_PRESV, 0) )
        if (!bp)
        {
            static char p[] =
                "atmos pressure (in. Hg; 0 for no refraction correction): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (sscanf (bp, "%lf", &pressure) == 1)
        {
            pressure *= 33.86;		/* want mBar */
            new = 1;
        }
        M_BREAK;
        M_CASE( rcfpack (R_EPOCH, C_EPOCHV, 0) )
        if (!bp)
        {
            static char p[] = "epoch (year, or e for Equinox of Date): ";
            f_prompt (p);
            if (read_line (buf, PW-strlen(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'e' || bp[0] == 'E')
            epoch = EOD;
        else
        {
            double e;
            e = atof(bp);
            year_mjd (e, &epoch);
        }
        new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_STPSZ, C_STPSZV, 0) )
        if (!bp)
        {
            static char p[] =
                "step size increment (h:m:s, or <x>d for x days, or r for RTC): ";
            f_prompt (p);
            if (read_line (buf, PW-sizeof(p)) <= 0)
                M_BREAK1;
            bp = buf;
        }
        if (bp[0] == 'r' || bp[0] == 'R')
            tminc = RTC;
        else
        {
            int last = strlen (bp) - 1;
            if (bp[last] == 'd')
            {
                /* ends in d so treat as a number of days */
                double x;
                if (sscanf (bp, "%lf", &x) == 1)
                    tminc = x * 24.0;
            }
            else
            {
                if (tminc == RTC)
                    deghrs = mins = secs = 0;
                else
                    f_dec_sexsign (tminc, &deghrs, &mins, &secs);
                f_sscansex (bp, &deghrs, &mins, &secs);
                sex_dec (deghrs, mins, secs, &tminc);
            }
        }
        print_tminc(0);
        set_t0 (&now);
        M_BREAK;
        M_CASE( rcfpack (R_PLOT, C_PLOT, 0) )
        plot_setup();
        if (plot_ison())
            new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_LISTING, C_LISTING, 0) )
        listing_setup();
        if (listing_ison())
            new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_WATCH, C_WATCH, 0) )
        watch_menu (&now, tminc, oppl);
        /* set new reference time to what watch left it.
         * no need to set new since watch just did a redraw.
         */
        set_t0 (&now);
        M_BREAK;
        M_CASE( rcfpack (R_DAWN, C_DAWN, 0) )
        if (optwi ^= 1)
        {
            print_updating();
            mm_twilight (&now, 1);
        }
        else
        {
            f_blanks (R_DAWN, C_DAWNV, 5);
            f_blanks (R_DUSK, C_DUSKV, 5);
            f_blanks (R_LON, C_LONV, 5);
        }
        M_BREAK;
        M_CASE( rcfpack (R_DUSK, C_DUSK, 0) )
        if (optwi ^= 1)
        {
            print_updating();
            mm_twilight (&now, 1);
        }
        else
        {
            f_blanks (R_DAWN, C_DAWNV, 5);
            f_blanks (R_DUSK, C_DUSKV, 5);
            f_blanks (R_LON, C_LONV, 5);
        }
        M_BREAK;
        M_CASE( rcfpack (R_LON, C_LON, 0) )
        if (optwi ^= 1)
        {
            print_updating();
            mm_twilight (&now, 1);
        }
        else
        {
            f_blanks (R_DAWN, C_DAWNV, 5);
            f_blanks (R_DUSK, C_DUSKV, 5);
            f_blanks (R_LON, C_LONV, 5);
        }
        M_BREAK;
        M_CASE( rcfpack (R_SRCH, C_SRCH, 0) )
        srch_setup();
        if (srch_ison())
            new = 1;
        M_BREAK;
        M_CASE( rcfpack (R_SUN, C_OBJ, 0) )
        toggle_body (SUN);
        M_BREAK;
        M_CASE( rcfpack (R_SUN, C_CONSTEL, 0) )
        if (oppl & (1<<SUN))
            constellation_msg (SUN, &now);
        M_BREAK;
        M_CASE( rcfpack (R_MOON, C_OBJ, 0) )
        toggle_body (MOON);
        M_BREAK;
        M_CASE( rcfpack (R_MOON, C_CONSTEL, 0) )
        if (oppl & (1<<MOON))
            constellation_msg (MOON, &now);
        M_BREAK;
        M_CASE( rcfpack (R_MERCURY, C_OBJ, 0) )
        toggle_body (MERCURY);
        M_BREAK;
        M_CASE( rcfpack (R_MERCURY, C_CONSTEL, 0) )
        if (oppl & (1<<MERCURY))
            constellation_msg (MERCURY, &now);
        M_BREAK;
        M_CASE( rcfpack (R_VENUS, C_OBJ, 0) )
        toggle_body (VENUS);
        M_BREAK;
        M_CASE( rcfpack (R_VENUS, C_CONSTEL, 0) )
        if (oppl & (1<<VENUS))
            constellation_msg (VENUS, &now);
        M_BREAK;
        M_CASE( rcfpack (R_MARS, C_OBJ, 0) )
        toggle_body (MARS);
        M_BREAK;
        M_CASE( rcfpack (R_MARS, C_CONSTEL, 0) )
        if (oppl & (1<<MARS))
            constellation_msg (MARS, &now);
        M_BREAK;
        M_CASE( rcfpack (R_JUPITER, C_OBJ, 0) )
        toggle_body (JUPITER);
        M_BREAK;
        M_CASE( rcfpack (R_JUPITER, C_CONSTEL, 0) )
        if (oppl & (1<<JUPITER))
            constellation_msg (JUPITER, &now);
        M_BREAK;
        M_CASE( rcfpack (R_JUPITER, C_XTRA, 0) )
        if (oppl & (1<<JUPITER))
        {
            print_updating();
            alt_erase();
            altmenu_init (F_MNUJ);
            print_alt (2);
            *fld = rcfpack(R_NSTEP, C_NSTEPV, 0);
        }
        M_BREAK;
        M_CASE( rcfpack (R_SATURN, C_OBJ, 0) )
        toggle_body (SATURN);
        M_BREAK;
        M_CASE( rcfpack (R_SATURN, C_CONSTEL, 0) )
        if (oppl & (1<<SATURN))
            constellation_msg (SATURN, &now);
        M_BREAK;
        M_CASE( rcfpack (R_URANUS, C_OBJ, 0) )
        toggle_body (URANUS);
        M_BREAK;
        M_CASE( rcfpack (R_URANUS, C_CONSTEL, 0) )
        if (oppl & (1<<URANUS))
            constellation_msg (URANUS, &now);
        M_BREAK;
        M_CASE( rcfpack (R_NEPTUNE, C_OBJ, 0) )
        toggle_body (NEPTUNE);
        M_BREAK;
        M_CASE( rcfpack (R_NEPTUNE, C_CONSTEL, 0) )
        if (oppl & (1<<NEPTUNE))
            constellation_msg (NEPTUNE, &now);
        M_BREAK;
        M_CASE( rcfpack (R_PLUTO, C_OBJ, 0) )
        toggle_body (PLUTO);
        M_BREAK;
        M_CASE( rcfpack (R_PLUTO, C_CONSTEL, 0) )
        if (oppl & (1<<PLUTO))
            constellation_msg (PLUTO, &now);
        M_BREAK;
        M_CASE( rcfpack (R_OBJX, C_OBJ, 0) )
        /* this might change which columns are used so erase all when
         * returns and redraw if still on.
         */
        obj_setup (OBJX);
        alt_nobody (OBJX);
        if (obj_ison (OBJX))
        {
            oppl |= 1 << OBJX;
            print_updating();
            alt_body (OBJX, 1, &now);
        }
        else
            oppl &= ~(1 << OBJX);	/* already erased; just clear flag */
        M_BREAK;
        M_CASE( rcfpack (R_OBJX, C_CONSTEL, 0) )
        if (oppl & (1<<OBJX))
            constellation_msg (OBJX, &now);
        M_BREAK;
        M_CASE( rcfpack (R_OBJY, C_OBJ, 0) )
        /* this might change which columns are used so erase all when
         * returns and redraw if still on.
         */
        obj_setup (OBJY);
        alt_nobody (OBJY);
        if (obj_ison (OBJY))
        {
            oppl |= 1 << OBJY;
            print_updating();
            alt_body (OBJY, 1, &now);
        }
        else
            oppl &= ~(1 << OBJY);	/* already erased; just clear flag */
        M_BREAK;
        M_CASE( rcfpack (R_OBJY, C_CONSTEL, 0) )
        if (oppl & (1<<OBJY))
            constellation_msg (OBJY, &now);
        M_BREAK;
    }
    M_DONE;

    return (new);
}

static void
print_tminc(force)
int force;
{
    static double last = -123.456;	/* anything unlikely */

    if (force || tminc != last)
    {
        if (tminc == RTC)
            f_string (R_STPSZ, C_STPSZV, " RT CLOCK");
        else if (fabs(tminc) >= 24.0)
            f_double (R_STPSZ, C_STPSZV, "%6.4g dy", tminc/24.0);
        else
            f_signtime (R_STPSZ, C_STPSZV, tminc);
        last = tminc;
    }
}

/* print stuff on bottom menu */
static void
print_alt (howmuch)
int howmuch;
{
    COLOR_CODE(COLOR_ALT_LABELS);
    if (howmuch == 2)
        alt_labels();
    COLOR_OFF;

    COLOR_CODE(COLOR_ALT_MENU);
    if (alt_menumask() == F_MNUJ)
        altj_display (howmuch, &now);
    else
    {
        int p;
        for (p = nxtbody(-1); p != -1; p = nxtbody(p))
            if (oppl & (1<<p))
                alt_body (p, howmuch, &now);
    }
    COLOR_OFF;
}

void print_updating()
{
    f_prompt ("Updating...");
}

static void
print_nstep(force)
int force;
{
    static int last;

    if (force || nstep != last)
    {
        char buf[16];
        (void) sprintf (buf, "%8d", nstep);
        f_string (R_NSTEP, C_NSTEPV, buf);
        last = nstep;
    }
}

static void
print_spause(force)
int force;
{
    static int last;

    if (force || spause != last)
    {
        char buf[16];
        (void) sprintf (buf, "%8d", spause);
        f_string (R_PAUSE, C_PAUSEV, buf);
        last = spause;
    }
}

/* if not plotting/listing/searching then sleep spause seconds.
 * if time is being based on the real-time clock, sync on the next
 *   integral multiple of spause seconds after the minute.
 * check for keyboard action once each second to let it break out early.
 */
void slp_sync()
{
    extern long time();

    if (spause > 0 && !plot_ison() && !srch_ison() && !listing_ison())
    {
        int n;
        if (tminc == RTC)
        {
            long t;
            (void) time (&t);
            n = spause - (t % spause);
        }
        else
            n = spause;
        while (--n >= 0)
            if (chk_char() == 0)
                break;
            else
                (void) sleep (1);
    }
}

static void
toggle_body (p)
int p;
{
    if ((oppl ^= (1<<p)) & (1<<p))
    {
        print_updating();
        alt_body (p, 1, &now);
    }
    else
        alt_nobody (p);
}

#ifdef NCURSES_LARGE
/* state needed for sky dome redraw */
int LastSkyStyle;
Now *Last_np;
/* handle SIGWINCH for screen resize */
void handle_winch(sig)
int sig;
{
    /* ncurses re-inits */
    endwin();
    refresh();
    keypad(stdscr,TRUE);

    _init_fields();

    c_erase();

    /* redraw labels, screen */
    if ( ! watching )
        redraw_screen(2);
    else if ( LastSkyStyle == 0 )
        sky_dome_labels( LastSkyStyle, Last_np );

    /* reset SIGWINCH handling */
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);
}
#endif

