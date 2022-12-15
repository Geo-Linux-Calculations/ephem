/* code to support the plotting capabilities.
 * idea is to let the operator name a plot file and mark some fields for
 * logging. then after each screen update, the logged fields are written to
 * the plot file. later, the file may be plotted (very simplistically by 
 * ephem, for now anyway, or by some other program entirely.).
 * 
 * format of the plot file is one line per coordinate: label,x,y
 * if z was specified, it is a fourth field.
 * x,y,z are plotted using %g format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "screen.h"
#include "ephem.h"

#ifdef VMS
#include <perror.h>
#include <errno.h>
#define	errsys	(sys_errlist[errno]) // ?
#else
#include <errno.h>
#define	errsys	(strerror(errno))
#endif



#define	TRACE(x)	{FILE *fp = fopen("trace","a"); fprintf x; fclose(fp);}

#define	MAXPLTLINES	10	/* max number of labeled lines we can track.
				 * note we can't store more than NFLOGS fields
				 * anyway (see flog.c).
				 */
#define	FNLEN		(14+1)	/* longest filename; plus 1 for \0 */

static char plt_filename[FNLEN] = "ephem.plt";	/* default plot file name */
static FILE *plt_fp;		/* the plot file; == 0 means don't plot */

/* store the label and rcfpack()s for each line to track. */
typedef struct {
    char pl_label;
    int pl_rcpx, pl_rcpy, pl_rcpz;
} PltLine;
static PltLine pltlines[MAXPLTLINES];
static int npltlines;		/* number of pltlines[] in actual use */

static int plt_in_polar;	/*if true plot in polar coords, else cartesian*/
static int pltsrchfld;		/* set when the Search field is to be plotted */

static void plt_select_fields();
static void plt_turn_off();
static void plt_turn_on();
static void plt_file();
static void plot_cartesian (FILE *pfp);
static void plot_polar (FILE *pfp);

/* picked the Plot label:
 * if on, just turn it off.
 * if off, turn on, define fields or select name of file to plot and do it.
 * TODO: more flexibility, more relevance.
 */
void plot_setup()
{
	if (plt_fp)
	    plt_turn_off();
	else {
	    static char *chcs[4] = {
		"Select fields", "Display a plot file", (char *)0,
		"Begin plotting"
	    };
	    static int fn;	/* start with 0, then remember for next time */
    ask:
	    chcs[2] = plt_in_polar ? "Polar coords" : "Cartesian coords";
	    switch (popup(chcs, fn, npltlines > 0 ? 4 : 3)) {
	    case 0: fn = 0; plt_select_fields(); goto ask;
	    case 1: fn = 1; plt_file(); goto ask;
	    case 2: fn = 2; plt_in_polar ^= 1; goto ask;
	    case 3: fn = 3; plt_turn_on(); break;
	    default: break;
	    }
	}
}

/* write the active plotfields to the current plot file, if one is open. */
void plot()
{
	if (plt_fp) {
	    PltLine *plp;
	    double x, y, z;
	    if (!srch_ison() && pltsrchfld) {
		/* if searching is not on but we are plotting the search
		 * funtion we must evaluate and log it ourselves here and now.
		 * plt_turn_on() insured there is a good function to eval.
		 * N.B. if searching IS on, we rely on main() having called
		 * srch_eval() BEFORE plot() so it is already evaluated.
		 */
		double e;
		char errmsg[128];
		if (execute_expr (&e, errmsg) < 0) {
		    f_msg (errmsg);
		    plt_turn_off();
		    return;
		} else
		    (void) flog_log (R_SRCH, C_SRCH, e, "");
	    }
	    /* plot in order of original selection */
	    for (plp = pltlines; plp < &pltlines[npltlines]; plp++) {
		if (flog_get (plp->pl_rcpx, &x, (char *)0) == 0 
			&& flog_get (plp->pl_rcpy, &y, (char *)0) == 0) {
		    (void) fprintf (plt_fp, "%c,%.12g,%.12g", plp->pl_label,
									x, y);
		    if (flog_get (plp->pl_rcpz, &z, (char *)0) == 0)
			(void) fprintf (plt_fp, ",%.12g", z);
		    (void) fprintf (plt_fp, "\n");
		}
	    }
	}
}

void plot_prstate (force)
int force;
{
	static int last;
	int this = plt_fp != 0;

	if (force || this != last) {
	    f_string (R_PLOT, C_PLOTV, this ? " on" : "off");
	    last = this;
	}
}

int plot_ison()
{
	return (plt_fp != 0);
}

static
void plt_reset()
{
	PltLine *plp;

	for (plp = &pltlines[npltlines]; --plp >= pltlines; ) {
	    (void) flog_delete (plp->pl_rcpx);
	    (void) flog_delete (plp->pl_rcpy);
	    (void) flog_delete (plp->pl_rcpz);
	    plp->pl_rcpx = plp->pl_rcpy = plp->pl_rcpz = 0;
	}
	npltlines = 0;
	pltsrchfld = 0;
}

/* let operator select the fields he wants to plot.
 * register them with flog and keep rcfpack() in pltlines[] array.
 * as a special case, set pltsrchfld if Search field is selected.
 */
static
void plt_select_fields()
{
	static char hlp[] = "move and RETURN to select a field, or q to quit";
	static char sry[] = "Sorry; can not log any more fields.";
	int f = rcfpack(R_UT,C_UTV,0); /* TODO: start where main was? */
	int sf = rcfpack (R_SRCH, C_SRCH, 0);
	char buf[64];
	int i;
	int tmpf;

	plt_reset();
	for (i = 0; i < MAXPLTLINES; i++) {
	    (void) sprintf (buf, "select x field for line %d", i+1);
	    f = sel_fld (f, alt_menumask()|F_PLT, buf, hlp);
	    if (!f)
		break;
	    if (flog_add (f) < 0) {
		f_msg (sry);
		break;
	    }
	    pltlines[i].pl_rcpx = f;
	    if (f == sf)
		pltsrchfld = 1;

	    (void) sprintf (buf, "select y field for line %d", i+1);
	    f = sel_fld (f, alt_menumask()|F_PLT, buf, hlp);
	    if (!f) {
		(void) flog_delete (pltlines[i].pl_rcpx);
		break;
	    }
	    if (flog_add (f) < 0) {
		(void) flog_delete (pltlines[i].pl_rcpx);
		f_msg (sry);
		break;
	    }
	    pltlines[i].pl_rcpy = f;
	    if (f == sf)
		pltsrchfld = 1;

	    (void) sprintf (buf, "select z field for line %d (q for no z)",i+1);
	    tmpf = sel_fld (f, alt_menumask()|F_PLT, buf, hlp);
	    if (tmpf) {
		if (flog_add (tmpf) < 0) {
		    (void) flog_delete (pltlines[i].pl_rcpx);
		    (void) flog_delete (pltlines[i].pl_rcpy);
		    f_msg (sry);
		    break;
		}
		pltlines[i].pl_rcpz = tmpf;
		if (tmpf == sf)
		    pltsrchfld = 1;
		f = tmpf;
	    }

	    do {
		(void) sprintf(buf,"enter a one-character label for line %d: ",
									i+1);
		f_prompt (buf);
	    } while (read_line (buf, 1) != 1);
	    pltlines[i].pl_label = *buf;
	}
	npltlines = i;
}

static
void plt_turn_off ()
{
	(void) fclose (plt_fp);
	plt_fp = 0;
	plot_prstate(0);
}

/* turn on plotting.
 * establish a file to use (and thereby set plt_fp, the plotting_is_on flag).
 * also check that there is a srch function if it is being plotted.
 */
static
void plt_turn_on ()
{
	int sf = rcfpack(R_SRCH, C_SRCH, 0);
	char fn[FNLEN], fnq[NC];
	char *optype;
	int n;
	PltLine *plp;

	/* insure there is a valid srch function if we are to plot it */
	for (plp = &pltlines[npltlines]; --plp >= pltlines; )
	    if ((plp->pl_rcpx == sf || plp->pl_rcpy == sf || plp->pl_rcpz == sf)
		    && !prog_isgood()) {
		f_msg ("Plotting search function but it is not defined.");
		return;
	    }

	/* prompt for file name, giving current as default */
	(void) sprintf (fnq, "file to write <%s>: ", plt_filename);
	f_prompt (fnq);
	n = read_line (fn, sizeof(fn)-1);

	/* leave plotting off if type END.
	 * reuse same fn if just type \n
	 */
	if (n < 0)
	    return;
	if (n > 0)
	    (void) strcpy (plt_filename, fn);

	/* give option to append if file already exists */
	optype = "w";
	if (access (plt_filename, 2) == 0) {
	    while (1) {
		f_prompt ("files exists; append or overwrite (a/o)?: ");
		n = read_char();
		if (n == 'a') {
		    optype = "a";
		    break;
		}
		if (n == 'o')
		    break;
	    }
	}

	/* plotting is on if file opens ok */
	plt_fp = fopen (plt_filename, optype);
	if (!plt_fp) {
	    char buf[NC];
	    (void) sprintf (buf, "can not open %s: %s", plt_filename, errsys);
	    f_prompt (buf);
	    (void)read_char();
	} else {
	    /* add a title if desired */
	    static char tp[] = "Title (q to skip): ";
	    f_prompt (tp);
	    if (read_line (fnq, PW - sizeof(tp)) > 0)
		(void) fprintf (plt_fp, "* %s\n", fnq);
	}
	plot_prstate (0);
}

/* ask operator for a file to plot. if it's ok, do it.
 */
static
void plt_file ()
{
	char fn[FNLEN], fnq[64];
	FILE *pfp;
	int n;

	/* prompt for file name, giving current as default */
	(void) sprintf (fnq, "file to read <%s>: ", plt_filename);
	f_prompt (fnq);
	n = read_line (fn, sizeof(fn)-1);

	/* forget it if type END.
	 * reuse same fn if just type \n
	 */
	if (n < 0)
	    return;
	if (n > 0)
	    (void) strcpy (plt_filename, fn);

	/* do the plot if file opens ok */
	pfp = fopen (plt_filename, "r");
	if (pfp) {
	    if (plt_in_polar)
		plot_polar (pfp);
	    else
		plot_cartesian (pfp);
	    (void) fclose (pfp);
	} else {
	    char buf[NC];
	    (void) sprintf (buf, "can not open %s: %s", plt_filename, errsys);
	    f_prompt (buf);
	    (void)read_char();
	}
}

/* plot the given file on the screen in cartesian coords.
 * TODO: add z tags somehow
 * N.B. do whatever you like but redraw the screen when done.
 */
static
void plot_cartesian (pfp)
FILE *pfp;
{
	static char fmt[] = "%c,%lf,%lf";
	double x, y;	/* N.B. be sure these match what scanf's %lf wants*/
	double minx, maxx, miny, maxy;
	char buf[128];
	int npts = 0;
	char c;

	/* find ranges and number of points */
	while (fgets (buf, sizeof(buf), pfp)) {
	    if (sscanf (buf, fmt, &c, &x, &y) != 3)
		continue;
	    if (npts++ == 0) {
		maxx = minx = x;
		maxy = miny = y;
	    } else {
		if (x > maxx) maxx = x;
		else if (x < minx) minx = x;
		if (y > maxy) maxy = y;
		else if (y < miny) miny = y;
	    }
	}

#define	SMALL	(1e-10)
	if (npts < 2 || fabs(minx-maxx) < SMALL || fabs(miny-maxy) < SMALL)
	    f_prompt ("At least two different points required to plot.");
	else {
	    /* read file again, this time plotting */
	    rewind (pfp);
	    c_erase();
	    while (fgets (buf, sizeof(buf), pfp)) {
		int row, col;
		if (sscanf (buf, fmt, &c, &x, &y) != 3)
		    continue;
		row = NR-(int)((NR-1)*(y-miny)/(maxy-miny)+0.5);
		col =  1+(int)((NC-1)*(x-minx)/(maxx-minx)+0.5);
		if (row == NR && col == NC)
		    col--;	/* avoid lower right scrolling corner */
		f_char (row, col, c);
	    }

	    /* label axes */
	    f_double (1, 1, "%g", maxy);
	    f_double (NR-1, 1, "%g", miny);
	    f_double (NR, 1, "%g", minx);
	    f_double (NR, NC-10, "%g", maxx);
	}

	/* hit any key to resume... */
	(void) read_char();
	redraw_screen (2);	/* full redraw */
}

/* plot the given file on the screen in polar coords.
 * first numberic field in plot file is r, second is theta in degrees.
 * TODO: add z tags somehow
 * N.B. do whatever you like but redraw the screen when done.
 */
static
void plot_polar (pfp)
FILE *pfp;
{
	static char fmt[] = "%c,%lf,%lf";
	double r, th;	/* N.B. be sure these match what scanf's %lf wants*/
	double maxr;
	char buf[128];
	int npts = 0;
	char c;

	/* find ranges and number of points */
	while (fgets (buf, sizeof(buf), pfp)) {
	    if (sscanf (buf, fmt, &c, &r, &th) != 3)
		continue;
	    if (npts++ == 0)
		maxr = r;
	    else
		if (r > maxr)
		    maxr = r;
	}

	if (npts < 2)
	    f_prompt ("At least two points required to plot.");
	else {
	    /* read file again, this time plotting */
	    rewind (pfp);
	    c_erase();
	    while (fgets (buf, sizeof(buf), pfp)) {
		int row, col;
		double x, y;
		if (sscanf (buf, fmt, &c, &r, &th) != 3)
		    continue;
		x = r * cos(th/57.2958);	/* degs to rads */
		y = r * sin(th/57.2958);
		row = NR-(int)((NR-1)*(y+maxr)/(2.0*maxr)+0.5);
		col =  1+(int)((NC-1)*(x+maxr)/(2.0*maxr)/ASPECT+0.5);
		if (row == NR && col == NC)
		    col--;	/* avoid lower right scrolling corner */
		f_char (row, col, c);
	    }

	    /* label radius */
	    f_double (NR/2, NC-10, "%g", maxr);
	}

	/* hit any key to resume... */
	(void) read_char();
	redraw_screen (2);	/* full redraw */
}
