/* printing routines for the main (upper) screen.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "astro.h"
#include "circum.h"
#include "screen.h"
#include "ephem.h"

/* #define PC_GRAPHICS */
#ifdef PC_GRAPHICS
#define	JOINT	207
#define	VERT	179
#define	HORIZ	205
#else
#define	JOINT	'-'
#define	VERT	'|'
#define	HORIZ	'-'
#endif

void mm_borders()
{
	char line[NC+1], *lp;
	register int i;

	lp = line;
	for (i = 0; i < NC; i++)
	    *lp++ = HORIZ;
	*lp = '\0';
	f_string (R_PLANTAB-1, 1, line);
	for (i = R_TOP; i < R_PLANTAB-1; i++)
	    f_char (i, COL2-2, VERT);
	f_char (R_PLANTAB-1, COL2-2, JOINT);
	for (i = R_TOP; i < R_PLANTAB-1; i++)
	    f_char (i, COL3-2, VERT);
	f_char (R_PLANTAB-1, COL3-2, JOINT);
	for (i = R_LST; i < R_PLANTAB-1; i++)
	    f_char (i, COL4-2, VERT);
	f_char (R_PLANTAB-1, COL4-2, JOINT);
}

/* print the permanent labels on the top menu */
void mm_labels()
{
	f_string (R_TZN,	C_TZN,		"LT");
	f_string (R_UT,		C_UT,		"UTC");
	f_string (R_JD,		C_JD,		"JulianDate");
	f_string (R_LISTING,	C_LISTING,	"Listing");
	f_string (R_WATCH,	C_WATCH,	"Watch");
	f_string (R_SRCH,	C_SRCH,		"Search");
	f_string (R_PLOT,	C_PLOT,		"Plot");
	f_string (R_ALTM,	C_ALTM,		"Menu");

	f_string (R_LST,	C_LST,		"LST");
	f_string (R_DAWN,	C_DAWN,		"Dawn");
	f_string (R_DUSK,	C_DUSK,		"Dusk");
	f_string (R_LON,	C_LON,		"NiteLn");
	f_string (R_PAUSE,	C_PAUSE,	"Pause");
	f_string (R_NSTEP,	C_NSTEP,	"NStep");
	f_string (R_STPSZ,	C_STPSZ,	"StpSz");

	f_string (R_LAT,	C_LAT,		"Lat");
	f_string (R_LONG,	C_LONG,		"Long");
	f_string (R_HEIGHT,	C_HEIGHT,	"Elev");
	f_string (R_TEMP,	C_TEMP,		"Temp");
	f_string (R_PRES,	C_PRES,		"AtmPr");
	f_string (R_TZONE,	C_TZONE,	"TZ");
	f_string (R_EPOCH,	C_EPOCH,	"Epoch");
}

static void mm_calendar (Now *np, int force);
static void mm_nfmoon (double jd, double tzone, int m, int f);

/* print all the time/date/where related stuff: the Now structure.
 * print in a nice order, based on the field locations, as much as possible.
 */
void mm_now (np, all)
Now *np;
int all;
{
	char buf[32];
	double lmjd = mjd - tz/24.0;
	double jd = mjd + 2415020L;
	double tmp;

	(void) sprintf (buf, "%-3.3s", tznm);
	f_string (R_TZN, C_TZN, buf);
	f_time (R_LT, C_LT, mjd_hr(lmjd));
	f_date (R_LD, C_LD, lmjd);

	f_time (R_UT, C_UTV, mjd_hr(mjd));
	f_date (R_UD, C_UD, mjd);

	(void) sprintf (buf, "%14.5f", jd);
	(void) flog_log (R_JD, C_JDV, jd, buf);
	f_string (R_JD, C_JDV, buf);

	now_lst (np, &tmp);
	f_time (R_LST, C_LSTV, tmp);

	if (all) {
	    f_gangle (R_LAT, C_LATV, lat);
	    f_gangle (R_LONG, C_LONGV, -lng);	/* + west */

	    tmp = height * 2.093e7;	/* want to see ft, not earth radii */
	    (void) sprintf (buf, "%5g ft", tmp);
	    (void) flog_log (R_HEIGHT, C_HEIGHTV, tmp, buf);
	    f_string (R_HEIGHT, C_HEIGHTV, buf);

	    tmp = 9./5.*temp + 32.0; 	/* want to see degrees F, not C */
	    (void) sprintf (buf, "%6g F", tmp);
	    (void) flog_log (R_TEMP, C_TEMPV, tmp, buf);
	    f_string (R_TEMP, C_TEMPV, buf);

	    tmp = pressure / 33.86;	/* want to see in. Hg, not mBar */
	    (void) sprintf (buf, "%5.2f in", tmp);
	    (void) flog_log (R_PRES, C_PRESV, tmp, buf);
	    f_string (R_PRES, C_PRESV, buf);

	    f_signtime (R_TZONE, C_TZONEV, tz);

	    if (epoch == EOD)
		f_string (R_EPOCH, C_EPOCHV, "(OfDate)");
	    else {
		mjd_year (epoch, &tmp);
		f_double (R_EPOCH, C_EPOCHV, "%8.1f", tmp);
	    }
	}

	/* print the calendar for local day, if new month/year.  */
	mm_calendar (np, all > 1);
}

/* display dawn/dusk/length-of-night times.
 */
void mm_twilight (np, force)
Now *np;
int force;
{
	double dusk, dawn;
	double tmp;
	int status;

	if (!twilight_cir (np, &dawn, &dusk, &status) && !force)
	    return;

	if (status != 0) {
	    f_blanks (R_DAWN, C_DAWNV, 5);
	    f_blanks (R_DUSK, C_DUSKV, 5);
	    f_string (R_LON, C_LONV, "-----");
	    return;
	}

	f_mtime (R_DAWN, C_DAWNV, dawn);
	f_mtime (R_DUSK, C_DUSKV, dusk);
	tmp = dawn - dusk; range (&tmp, 24.0);
	f_mtime (R_LON, C_LONV, tmp);
}

void mm_newcir (y)
int y;
{
	static char ncmsg[] = "NEW CIRCUMSTANCES";
	static char nomsg[] = "                 ";
	static int last_y = -1;

	if (y != last_y) {
	    f_string (R_NEWCIR, C_NEWCIR, y ? ncmsg : nomsg);
	    last_y = y;
	}
}

static
void mm_calendar (np, force)
Now *np;
int force;
{
	static char *mnames[] = {
	    "January", "February", "March", "April", "May", "June",
	    "July", "August", "September", "October", "November", "December"
	};
	static int last_m, last_y;
	static double last_tz = -100;
	char str[64];
	int m, y;
	double d;
	int f, nd;
	int r;
	double jd0;

	/* get local m/d/y. do nothing if still same month and not forced. */
	mjd_cal (mjd_day(mjd-tz/24.0), &m, &d, &y);
	if (m == last_m && y == last_y && tz == last_tz && !force)
	    return;
	last_m = m;
	last_y = y;
	last_tz = tz;

	/* find day of week of first day of month */
	cal_mjd (m, 1.0, y, &jd0);
	mjd_dow (jd0, &f);
	if (f < 0) {
	    /* can't figure it out - too hard before Gregorian */
	    int i;
	    for (i = 8; --i >= 0; )
		f_string (R_CAL+i, C_CAL, "                    ");
	    return;
	}

	/* print header */
	f_blanks (R_CAL, C_CAL, 20);
	(void) sprintf (str, "%s %4d", mnames[m-1], y);
	f_string (R_CAL, C_CAL + (20 - (strlen(mnames[m-1]) + 5))/2, str);
	f_string (R_CAL+1, C_CAL, "Su Mo Tu We Th Fr Sa");

	/* find number of days in this month */
	mjd_dpm (jd0, &nd);

	/* print the calendar */
	for (r = 0; r < 6; r++) {
	    char row[7*3+1], *rp = row;
	    int c;
	    for (c = 0; c < 7; c++) {
		int i = r*7+c;
		if (i < f || i >= f + nd)
		    (void) sprintf (rp, "   ");
		else
		    (void) sprintf (rp, "%2d ", i-f+1);
		rp += 3;
	    }
	    row[sizeof(row)-2] = '\0';	/* don't print last blank; causes wrap*/
	    f_string (R_CAL+2+r, C_CAL, row);
	}

	/* over print the new and full moons for this month.
	 * TODO: don't really know which dates to use here (see moonnf())
	 *   so try several to be fairly safe. have to go back to 4/29/1988
	 *   to find the full moon on 5/1 for example.
	 */
	mm_nfmoon (jd0-3, tz, m, f);
	mm_nfmoon (jd0+15, tz, m, f);
}

static
void mm_nfmoon (jd, tzone, m, f)
double jd, tzone;
int m, f;
{
	static char nm[] = "NM", fm[] = "FM";
	double dm;
	int mm, ym;
	double jdn, jdf;
	int di;

	moonnf (jd, &jdn, &jdf);
	mjd_cal (jdn-tzone/24.0, &mm, &dm, &ym);
	if (m == mm) {
	    di = dm + f - 1;
	    f_string (R_CAL+2+di/7, C_CAL+3*(di%7), nm);
	}
	mjd_cal (jdf-tzone/24.0, &mm, &dm, &ym);
	if (m == mm) {
	    di = dm + f - 1;
	    f_string (R_CAL+2+di/7, C_CAL+3*(di%7), fm);
	}
}
