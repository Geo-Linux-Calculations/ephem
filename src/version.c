/* N.B. please increment version and date and note each change. */
#include <stdio.h>
#include <string.h>
#include "screen.h"
#include "ephem.h"

static char vmsg[] = "Version 4.32 December 10, 2021";

/*
 * 4.32 12/10/21 globe.c watch.c sel_fld.c plot.c main.c tags formats.c ephem.h listing.c io.c
 *            lint to run on macOS; no changes in functionality
 * 4.31 12/14/15 screen.h main.c sel_fld.c version.c watch.c mods
 *            "Doug" <semaphore_2000@yahoo.com>
 *              - -DNCURSES_LARGE uses ncurses LINES,COLS SIGWINCH for
 *                 non- 25x80 terminals and dynamic resizing
 *              - -DANSI_COLORS use ANSI color codes, reverse, bold, etc;
 *                 also, colors may be customized with COLOR=... config
 *                 and/or command-line settings (see Man.txt)
 *              - cmake support, added CMakeLists.txt  ... build in usual
 *                cmake way, i.e.:
 *                      cmake .
 *                      make
 *                      make install
 *              - switched from termcap to ncurses
 *              - added Watch|Earth,Moon features, by splicing in "phoon"
 *                and "globe" written by Jef Poskanzer <jef@mail.acme.com>;
 *                http://acme.com/software (optional; -DGLOBE_PHOON)
 *              - added -K switch (enter sky dome directly)
 *              - minor code changes (added some .h headers, etc)
 * 4.30 4/25/11 io.c mods for Windows, courtesy "Mosh" <mosh.ahmed@gmail.com>
 *              very minor syntax cleanups.
 * 4.29 9/3/00  Jonathan Woithe, jwoithe@physics.adelaide.edu.au
 *              Changes motivated by the program freezing after several
 *              "q" operations when compiled on Linux 2.2.13/egcs 1.1.2.
 *              (When compiled with -g the program also segfaulted whenever
 *              the date was changed.)
 *                - creation of ephem.h to contain all extern function
 *                  prototypes
 *                - inserted explicit typing on all functions
 *                - use system-provided includes rather than either manually
 *                  prototyping libc functions or not prototyping at all
 *                - change the type of some variables to match libc function
 *                  prototypes
 *              Verified clean compile and run on egcs 1.1.2/Linux 2.2.13.
 *              The effect of these changes on other systems is unknown.
 * 4.28 2/25/92	post to comp.sources.misc
 * 4.27 1/10	allow full plotting accuracy for LD/UD fields.
 *		check better for bad temp/height/pressure/stpsz formats.
 * 4.26	11/27	fix bug in nutation.c (two successive degrad() calls for tnm)
 *		allow for earth being >1 au from sun in watch_solarsystem().
 *	12/8	use unsigned short for fields[] in sel_fld.c
 * 4.25	11/6	fix problem redrawing first page of object lookup.
 * 4.24	11/5	fix initial cursor loc in jupiter extra menu.
 * 4.23	10/11	switch to Meeus' algorithm for jupiter's moons.
 *	10/13	some constellation values had leading 0: octal!
 *		add casts to setting srch_f to 0 via ?: for better portability.
 *	10/16	bona fide menu option for moons.
 *	10/18	add SPACE as an alternative to RETURN for picking.
 *	10/19	wrap long listing file lines.
 *	10/24	disregard Pause while listing too.
 *	10/25	obj lookup now lets you pick from a table.
 *	10/27	add sqrt as a builtin search function
 *		add jupiter's central meridian longitudes to jup aux menu.
 *	10/29	ignore all .cfg/.db lines not starting with alpha char.
 *		use ctype.h for all alpha/digit/print tests.
 *	10/30	park the watch cursor after each screen-full.
 *	10/31	allow for retrograde rates in body_cir()'s conservation effort.
 * 4.22	8/29/90	add options for using select() in io.c.
 *	9/6	add checks for termcap keypad start/end codes (ke/ks).
 *		guard against a 0 entry from tgetstr() in io.c/egetstr().
 *	9/11	add hyperbolic objx type.
 *	9/12	check for missing termcap codes better.
 *	9/13	a few more #ifdef VMS tweaks.
 *	9/14	add more horizon marks to sky dome.
 *	10/3	add optional size to user objects (fixed rise/set problem too).
 *		add constellations support.
 *		add jupiter's moons. (to be much improved some day)
 *	10/4	switch to precession routine from Astro Almanac.
 *	10/5	use J for jupiter hot key, not ^j (more portable).
 *		fix year 0 problem.
 * 4.21	8/23/90	fix dawn/dusk near vernal equinox.
 * 4.20 8/15/90	add g/k and H/G magnitude model options for elliptical objects.
 *	8/17	put moon's geocentric long/lat under Hlong/Hlat columns.
 *		allow entering negative decimal years.
 *	8/20	init all static mjd storage to unlike times.
 *	8/21	add USE_TERMIO option to io.c.
 * 4.19 8/7/90	add listing feature, with 'L' hot-key.
 *		add title for plot file (as well as listing file).
 *		add some (void) casts for lint sake.
 * 4.18 8/2/90	fix parabolic comet bug in objx.c (bad lam computation).
 * 4.17 7/2/90	add 'c' short cut to Menu field.
 *		display full Dec precision for fixed objx setup.
 *		increase pluto auscale in watch.c, and guard screen boundries.
 *		add Pause feature.
 *	7/27	further improve rise/set and dawn/dusk times.
 *		add MENU={DATA,RISET,SEP} config/arg option.
 * 4.16 5/30/90	watch popup now allows changing formats without returning.
 *		add 'w' short cut to watch field.
 *		improve labeling a bit in Dome display.
 * 4.15	5/2/90	move setjmp() in main so it catches fp errs from ephem.cfg too.
 *	5/15	maintain name of objx/y.
 *		clean up objx.c.
 *	5/16	fix bug circum.c related to phase of fixed objects.
 *	5/22	add "Sky dome" watch display format (idea from Jeffery Cook).
 *	5/23	remember last selection in watch, search, and plot popup menus.
 *		cleanup layout and add labels in the watch screens.
 * 4.14 4/9/90	add y to body_tags[] in watch.c.
 *	4/10	add ! support (#ifdef BANG in sel_fld()).
 *      4/17	add #ifdef VMS and allow for no time zones (Karsten Spang).
 *	4/23	switch to EPHEMCFG (no more HOME).
 *		add #include <stdlib.h> #ifdef VMS wherever atof() is used.
 *	4/24	fix phase so it works for objects out of the ecliptic.
 * 4.13 3/9/90	add support for second user-def object: "object y"
 *		fix bug updating obj ref epoch (always used PARABOLIC's)
 *		fix Turbo C workaround that prevented plotting sun dist.
 *      3/13	fix bug preventing searching on separation column for objx
 *	3/22	revamp elliptical object definition parameters to match AA.
 *		permit exiting/redrawing properly from within a popup too.
 *		add a bit more precision to plot labels.
 *		let plot files have comments too, ie, leading *'s
 *	3/23	add "Lookup" to search config file for objects.
 *	3/30	separate database from config file; add -d and EPHEMDB env var.
 *		catch SIGFPE and longjmp() back into main interation loop.
 *	4/3	add magnitude to fixed-object database fields.
 * 4.12 1/12/90	lay framework for orbital element support for object x.
 *	1/15	fix slight bug related to nutation.
 *		plot fields in the same order they were selected.
 *	1/18   	add copywrite notice in main.c.
 *		note <sys/time.h> in time.c for BSD 4.3.
 *	1/20	work on fixed and parabolic orbital elements.
 *      1/25	work on elliptical orbital elements.
 *	2/1	work on objx's magnitude models.
 *		add confirmation question before quitting.
 *      2/6	add d,o,z special speed move chars.
 *	2/8	watch: add LST to night sky and maintain RTC time back in main.
 *	2/12	fix bug in timezone related to daytime flag.
 *		add w (week) watch advance key code.
 *		add cautionary note about no string[s].h to Readme
 *	2/15	allow for precession moving dec outside range -90..90.
 *	2/19	fix bug that wiggled cursor during plotting in rise/set menu.
 *	2/20	fix bug preventing DAWN/DUSK/LON from being used in search func.
 * 4.11 12/29	add PC_GRAPHICS option in mainmenu.c (no effect on unix version)
 *      1/3/90	fix twilight error when sun never gets as low as -18 degs.
 *      1/4/90	always find alt/az from eod ra/dec, not from precessed values.
 *	1/9/90	lastmjd in plans.c was an int: prevented needless recalcs.
 * 4.10 12/6/89 fix transit times of circumpolar objects that don't rise.
 *              fix plotting search function when searching is not on.
 *	12/12	fix Objx rise/set bug.
 *      12/21	don't erase last watch positions until computed all new ones.
 *      12/23   added USE_BIOSCALLS to io.c: Doug McDonald's BIOS calls
 *	12/27	allow dates to be entered as decimal years (for help with plots)
 *	12/27	remove literal ESC chars in strings in io.c.
 * 4.9 11/28/89 provide two forms of non-blocking reads for unix in io.c
 *     11/30/89 take out superfluous ESC testing in chk_arrow().
 *              guard better against bogus chars in sel_fld().
 *		use %lf in scanf's.
 *              command-line arg PROPTS+ adds to settings from config file.
 *		change (int) casts in moduloes to (long) for 16bit int systems.
 * 4.8 10/28/89 use doubles everywhere
 *     10/31/89	add direct planet row selection codes.
 *     11/2/89  improve compiler's fieldname parser.
 *     11/3/89	switch from ESC to q for "go on" (CBREAK ESC not very portable)
 *     11/6/89	allow plotting the search function too.
 *     11/8/89  suppress screen updates while plotting and nstep > 1.
 *     11/9/89	fix bug prohibiting plotting venus' sdist and objx's transit.
 *     11/9/89	add option to plot in polar coords.
 *     11/12/89	fix bug related to updating timezone name when it is changed.
 *     11/21/89 fix bug in when to print info about object-x
 *     11/21/89	increase MAXPLTLINES to 10 (to ease plotting all planet seps)
 *     11/22/89 allow setting fields from command line too.
 * 4.7 10/13/89 start adding general searching feature. start with flogging.
 *     10/17/89 add compiler, first menu ideas, get binary srch working.
 *     10/18/89 add parabolic-extrema and secant-0 solvers.
 *     10/23/89 finish up new idea of one-line control and set-up "popup" menus.
 * 4.6 10/29/89 improve transit circumstances by iterating as with rise/set.
 *		allow changing lst.
 *		show Updating message at better times.
 *		avoid overstrikes while watching and add trails option.
 *		allow for Turbo-C 2.0 printf bug using %?.0f".
 * 4.5  9/24/89 add third table of all mutual planet angular distances.
 * 4.4  9/21/89 add second planet table with rise/set times.
 *		all rise/set times may now use standard or adaptive horizons.
 * 4.3   9/6/89 NM/FM calendar overstikes now use local time (was ut).
 *		display elongation of object x.
 *		better handling of typo when asking for refraction model.
 * 4.2	7/24/89	specify 7 digits to plot file (not just default to 6)
 * 4.1  7/18/89 use buffered output and fflush in read_char().
 * 4.0   7/8/89	add simple sky and solarsystem plotting (and rearrange fields)
 *		change mars' .cfg mnemonic from a to m.
 *		clean up nstep/NEW CIR handling
 *		quit adding our own .cfg suffixes, but...
 *		add looking for $HOME/.ephemrc (Ronald Florence)
 *		drop -b
 *		no longer support SITE
 * 3.17 6/15/89 misspelt temperature prompt; sun -/= bug. (Mike McCants)
 *		change sun() to sunpos() for sake of Sun Microsystems.
 * 3.16  6/9/89 allow JD to be set and plotted.
 *		c_clear (LATTIC_C) should use J not j (Alex Pruss)
 *		support SIGINT (Ronald Florence)
 * 3.15  6/8/89 forget SIMPLETZ: now TZA and TZB.
 * 3.14  6/6/89 add back borders but allow turning off via -b
 * 3.13 5/26/89 fix Day/Nite picking loc bug.
 * 3.12 5/25/89 add SIMPLETZ option to time.c for systems w/o tzset()
 *		files; couldn't plot dayln or niteln.
 * 3.11 5/16/89 local time prompt said utc; add NiteLn; check for bad plot
 * 3.10 4/27/89 allow caps for moving cursor around too
 * 3.9   4/5/89 discard leading termcap delay digits, for now
 * 3.8   3/2/89 shorten displayed precision, add heliocentric lat/long
 * 3.7  2/13/89 change to ^d to quit program.
 * 3.6   2/7/89 try adding .cfg suffix if can't find config file
 * 3.5   2/2/89 sunrise/set times based on actual sun size and pressure/temp
 * 3.4  1/22/89 calendar and all rise/set times based on local date, not utc
 * 3.3   1/6/89 add z to plot files (still don't plot it however)
 * 3.2   1/3/89 if set date/time then time does not inc first step
 * 3.1   1/1/89 user def. graph labels; nstep/stpsz control; genuine c_eol
 * 3.0 12/31/88 add graphing; add version to credits.
 * 2.7 12/30/88 add version to credits.
 * 2.6 12/28/88 twilight defined as 18 rather than 15 degrees below horizon
 * 2.5 12/26/88 remove trace capability; add screen shadowing: ^l.
 * 2.4 10/31/88 add credits banner, -s turns it off; savings time fix.
 * 2.3  9/23/88 exchange Altitude/Elevation titles (no code changes)
 * 2.2  9/20/88 more caution in aaha_aux() guarding acos() arg range
 * 2.1  9/14/88 moon phase always >= 0 to match planets convention
 * 2.0  9/13/88 add version ^v option
 */

void version()
{
    f_msg (vmsg);
}

static char *cre[] =
{
    "Ephem - an interactive astronomical ephemeris program",
    vmsg,
    "",
    "Copyright (c) 1990,1991,1992,2021 by Elwood Charles Downey, ecdowney@clearskyinstitute.com",
    "Copyright (C) 1986,1987,1988,1995 by Jef Poskanzer <jef@mail.acme.com>",
    "Copyright (C) 2015 by Doug Snead, semaphore_2000@yahoo.com",
    "(Code cleanup by Jonathan Woithe, March 2000)",
    "",
    "Permission is granted to make and distribute copies of this program free of",
    "charge, provided the copyright notices and this permission notice are",
    "preserved on all copies.  All other rights reserved.  No representation is",
    "made about the suitability of this software for any purpose.  It is provided",
    "\"as is\" without express or implied warranty, to the extent permitted by",
    "applicable law.",
    "",
    /*
    "Many formulas and tables are based, with permission, on material found in",
    "\"Astronomy with your Personal Computer\" by Dr. Peter Duffett-Smith,",
    "Cambridge University Press, (c) 1985.  Constellation algorithm from a paper",
    "by Nancy G.  Roman, \"Identification of a constellation from a position\",",
    "Publications of the Astronomical Society of the Pacific, Vol.  99, p.",
    "695-699, July 1987.  Precession routine from 1989 Astronomical Almanac.",
    "Jupiter's moons based on information in \"Astronomical Formulae for",
    "Calculators\" by Jean Meeus.  Richmond, Va., U.S.A., Willmann-Bell, (c) 1982.",
    */
    "See the manual (Man.txt) for a list of references.",
    "",
    "type any key to continue..."
};

void credits()
{
    int r;
    int l;
    int nr;

    c_erase();
    COLOR_CODE(COLOR_VERSION);

    nr = sizeof(cre)/sizeof(cre[0]);
    r = (NR - nr)/2 + 1;
    for (l = 0; l < nr; l++)
        f_string (r++, (NC - strlen(cre[l]))/2, cre[l]);

    COLOR_OFF;
    (void) read_char();	/* wait for any char to continue */
}
