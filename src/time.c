/* get the time from the os.
 *
 * here are two methods I was able to verify; pick one for your system and
 *   define exactly one of TZA or TZB:
 * TZA works on our ibm-pc/turbo-c and at&t systems,
 * TZB works on our 4.2 BSD vax.
 *
 * I'm told that on Sun OS 4.0.3 (BSD 4.3?) and Apollo SR 10.1 TZB works if
 *   you use <sys/time.h> in place of <time.h>.
 * 
 * On VMS, you DON'T want to define EITHER TZA nor TZB since it can't handle
 *   time zones, period. time_fromsys() will detect that fact based on gmtime()
 *   returning 0.
 */

#define	TZA

#ifdef VMS
#undef TZA
#undef TZB
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "astro.h"
#include "circum.h"
#include "ephem.h"

static long c0;
static double mjd0;

static void settzstuff (int dst, Now *np);

/* save current mjd and corresponding system clock for use by inc_mjd().
 * this establishes the base correspondence between the mjd and system clock.
 */
void set_t0 (np)
Now *np;
{
	mjd0 = mjd;
	(void) time (&c0);
}

/* fill in n_mjd/tz/tznm from system clock.
 */
void time_fromsys (np)
Now *np;
{
	struct tm *tp;
	time_t c;
	double day, hr;

	(void) time (&c);

	tp = gmtime (&c);
	if (tp) {
	    cal_mjd (tp->tm_mon+1, (double)tp->tm_mday, tp->tm_year+1900, &day);
	    sex_dec (tp->tm_hour, tp->tm_min, tp->tm_sec, &hr);
	    mjd = day + hr/24.0;
	    tp = localtime (&c);
	    settzstuff (tp->tm_isdst ? 1 : 0, np);
	} else {
	    /* if gmtime() doesn't work, we assume the timezone stuff won't
	     * either, so we just use what it is and leave it alone. Some
	     * systems (like VMS) do not know about time zones, so this is the
	     * best guess in that case.
	     */
	    tp = localtime (&c);
	    cal_mjd (tp->tm_mon+1, (double)tp->tm_mday, tp->tm_year+1900, &day);
	    sex_dec (tp->tm_hour, tp->tm_min, tp->tm_sec, &hr);
	    mjd = day + hr/24.0 + tz/24.0;
	}
}

/* given whether dst is now in effect (must be strictly 0 or 1), fill in
 * tzname and tz within np.
 */
static
void settzstuff (dst, np)
int dst;
Now *np;
{
#ifdef TZA
	extern long timezone;
	extern char *tzname[2];

	tzset();
	tz = timezone/3600;
	if (dst)
	    tz -= 1.0;
	(void) strncpy (tznm, tzname[dst], sizeof(tznm)-1);
#endif
#ifdef TZB
	extern char *timezone();
	struct timeval timev;
	struct timezone timez;

	gettimeofday (&timev, &timez);
	tz = timez.tz_minuteswest/60;
	if (dst)
	    tz -= 1.0;
	(void) strncpy (tznm, timezone(timez.tz_minuteswest, dst),
								sizeof(tznm)-1);
#endif
	tznm[sizeof(tznm)-1] = '\0';	/* insure string is terminated */
}

void inc_mjd (np, inc)
Now *np;
double inc;
{
	if (inc == RTC) {
	    long c;
	    (void) time (&c);
	    mjd = mjd0 + (c - c0)/SPD;
	} else
	    mjd += inc/24.0;

	/* round to nearest whole second.
	 * without this, you can get fractional days so close to .5 but
	 * not quite there that mjd_hr() can return 24.0
	 */
	rnd_second (&mjd);
}
