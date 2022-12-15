/* code to support the listing capabilities.
 * idea is to let the operator name a listing file and mark some fields for
 * logging. then after each screen update, the logged fields are written to
 * the listing file in the same manner as they appeared on the screen.
 * 
 * format of the listing file is one line per screen update.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
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

#define	MAXLSTFLDS	132	/* max number of fields we can track.
				 * note we can't store more than NFLOGS fields
				 * anyway (see flog.c).
				 */
#define	FNLEN		(14+1)	/* longest filename; plus 1 for \0 */

static char lst_filename[FNLEN] = "ephem.lst";	/* default plot file name */
static FILE *lst_fp;		/* the plot file; == 0 means don't plot */

/* store rcfpack()s for each field to track, in l-to-r order */
static int lstflds[MAXLSTFLDS];
static int nlstflds;		/* number of lstflds[] in actual use */

static int lstsrchfld;		/* set when the Search field is to be listed */

static void lst_select_fields();
static void lst_turn_off ();
static void lst_turn_on ();
static void lst_file ();
static void display_listing_file (FILE *lfp);

/* picked the Listing label:
 * if on, just turn it off.
 * if off, turn on, define fields or select name of file to list to and do it.
 * TODO: more flexibility, more relevance.
 */
void listing_setup()
{
	if (lst_fp)
	    lst_turn_off();
	else {
	    static char *chcs[] = {
		"Select fields", "Display a listing file", "Begin listing"
	    };
	    static int fn;	/* start with 0, then remember for next time */
    ask:
	    switch (popup(chcs, fn, nlstflds > 0 ? 3 : 2)) {
	    case 0: fn = 0; lst_select_fields(); goto ask;
	    case 1: fn = 1; lst_file(); goto ask;
	    case 2: fn = 2; lst_turn_on(); break;
	    default: break;
	    }
	}
}

/* write the active listing to the current listing file, if one is open. */
void listing()
{
	if (lst_fp) {
	    int n;
	    double flx;
	    char flstr[32];
	    if (!srch_ison() && lstsrchfld) {
		/* if searching is not on but we are listing the search
		 * funtion we must evaluate and log it ourselves here and now.
		 * lst_turn_on() insured there is a good function to eval.
		 * N.B. if searching IS on, we rely on main() having called
		 * srch_eval() BEFORE plot() so it is already evaluated.
		 */
		double e;
		char errmsg[128];
		if (execute_expr (&e, errmsg) < 0) {
		    f_msg (errmsg);
		    lst_turn_off();
		    return;
		} else {
		    (void) sprintf (flstr, "%g", e);
		    (void) flog_log (R_SRCH, C_SRCH, e, flstr);
		}
	    }

	    /* list in order of original selection */
	    for (n = 0; n < nlstflds; n++)
		if (flog_get (lstflds[n], &flx, flstr) == 0)
		    (void) fprintf (lst_fp, "%s  ", flstr);
	    (void) fprintf (lst_fp, "\n");
	}
}

void listing_prstate (force)
int force;
{
	static int last;
	int this = lst_fp != 0;

	if (force || this != last) {
	    f_string (R_LISTING, C_LISTINGV, this ? " on" : "off");
	    last = this;
	}
}

int listing_ison()
{
	return (lst_fp != 0);
}

static
void lst_reset()
{
	int *lp;

	for (lp = lstflds; lp < &lstflds[nlstflds]; lp++) {
	    (void) flog_delete (*lp);
	    *lp = 0;
	}
	nlstflds = 0;
	lstsrchfld = 0;
}

/* let operator select the fields he wants to have in his listing.
 * register them with flog and keep rcfpack() in lstflds[] array.
 * as a special case, set lstsrchfld if Search field is selected.
 */
static
void lst_select_fields()
{
	static char hlp[] = "move and RETURN to select a field, or q to quit";
	static char sry[] = "Sorry; can not list any more fields.";
	int f = rcfpack(R_UT,C_UTV,0); /* TODO: start where main was? */
	int sf = rcfpack (R_SRCH,C_SRCH,0);
	char buf[64];
	int i;

	lst_reset();
	for (i = 0; i < MAXLSTFLDS; i++) {
	    (void) sprintf(buf,"select field for column %d or q to quit", i+1);
	    f = sel_fld (f, alt_menumask()|F_PLT, buf, hlp);
	    if (!f)
		break;
	    if (flog_add (f) < 0) {
		f_msg (sry);
		break;
	    }
	    lstflds[i] = f;
	    if (f == sf)
		lstsrchfld = 1;
	}
	if (i == MAXLSTFLDS)
	    f_msg (sry);
	nlstflds = i;
}

static
void lst_turn_off ()
{
	(void) fclose (lst_fp);
	lst_fp = 0;
	listing_prstate(0);
}

/* turn on listing facility.
 * establish a file to use (and thereby set lst_fp, the "listing-is-on" flag).
 * also check that there is a srch function if it is being used.
 */
static
void lst_turn_on ()
{
	int sf = rcfpack(R_SRCH, C_SRCH, 0);
	char fn[FNLEN], fnq[NC];
	char *optype;
	int n;

	/* insure there is a valid srch function if we are to list it */
	for (n = 0; n < nlstflds; n++)
	    if (lstflds[n] == sf && !prog_isgood()) {
		f_msg ("Listing search function but it is not defined.");
		return;
	    }

	/* prompt for file name, giving current as default */
	(void) sprintf (fnq, "file to write <%s>: ", lst_filename);
	f_prompt (fnq);
	n = read_line (fn, sizeof(fn)-1);

	/* leave plotting off if type END.
	 * reuse same fn if just type \n
	 */
	if (n < 0)
	    return;
	if (n > 0)
	    (void) strcpy (lst_filename, fn);

	/* give option to append if file already exists */
	optype = "w";
	if (access (lst_filename, 2) == 0) {
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

	/* listing is on if file opens ok */
	lst_fp = fopen (lst_filename, optype);
	if (!lst_fp) {
	    (void) sprintf (fnq, "can not open %s: %s", lst_filename, errsys);
	    f_msg (fnq);
	} else {
	    /* add a title if desired */
	    static char tp[] = "Title (q to skip): ";
	    f_prompt (tp);
	    if (read_line (fnq, PW - sizeof(tp)) > 0)
		(void) fprintf (lst_fp, "%s\n", fnq);
	}

	listing_prstate (0);
}

/* ask operator for a listing file to show. if it's ok, do it.
 */
static
void lst_file ()
{
	char fn[FNLEN], fnq[64];
	FILE *lfp;
	int n;

	/* prompt for file name, giving current as default */
	(void) sprintf (fnq, "file to read <%s>: ", lst_filename);
	f_prompt (fnq);
	n = read_line (fn, sizeof(fn)-1);

	/* forget it if type END.
	 * reuse same fn if just type \n
	 */
	if (n < 0)
	    return;
	if (n > 0)
	    (void) strcpy (lst_filename, fn);

	/* show it if file opens ok */
	lfp = fopen (lst_filename, "r");
	if (lfp) {
	    display_listing_file (lfp);
	    (void) fclose (lfp);
	} else {
	    char buf[NC];
	    (void) sprintf (buf, "can not open %s: %s", lst_filename, errsys);
	    f_prompt (buf);
	    (void)read_char();
	}
}

/* display the given listing file on the screen.
 * allow for files longer than the screen.
 * N.B. do whatever you like but redraw the screen when done.
 */
static
void display_listing_file (lfp)
FILE *lfp;
{
	static const char eofp[] = "[End-of-file. Hit any key to resume...] ";
	static const char p[] =    "[Hit any key to continue or q to quit...] ";
	char buf[NC+2];	/* screen width plus for '\n' and '\0' */
	int nc, nl;

	c_erase();
	nl = 0;
	while (1) {
        char *rv = fgets (buf, sizeof(buf), lfp);
	    if (rv==NULL || feof(lfp)) {
		printf (eofp);
		(void) read_char();
		break;
	    }
	    /* make sure last char is \n, even if it's a long line */
	    nc = strlen (buf);
	    if (nc == NC+1) {
		(void) ungetc (buf[NC], lfp);
		buf[NC] = '\n';
	    }
	    printf ("%s\r", buf);
	    if (++nl == NR-1) {
		/* read-ahead one char to check for eof */
		int rach = getc (lfp);
		if (feof(lfp)) {
		    (void) printf (eofp);
		    (void) read_char();
		    break;
		} else
		    (void) ungetc (rach, lfp);
		(void) printf (p);
		if (read_char() == END)
		    break;
		c_erase();
		nl = 0;
	    }
	}

	redraw_screen (2);	/* full redraw */
}
