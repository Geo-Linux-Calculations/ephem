/* this is a simple little package to manage the saving and retrieving of
 * field values, which we call field logging or "flogs". a flog consists of a
 * field location, ala rcfpack(), its value as a double and its value as
 * a string (ie, however it was printed). you can reset the list of flogs, add
 * to and remove from the list of registered fields and log a field if it has
 * been registered.
 *
 * this is used by the plotting and searching facilities of ephem to maintain
 * the values of the fields that are being plotted or used in search
 * expressions. it is used by the listing facility to generate listing files.
 *
 * a field can be in use for more than one
 * thing at a time (eg, all the X plot values may the same time field, or
 * searching and plotting might be on at one time using the same field) so
 * we consider the field to be in use as long a usage count is > 0.
 */

#include "string.h"
#include "screen.h"
#include "ephem.h"

#define	NFLOGS	132		/* max number of distinct simultaneous logged
				 * fields
				 */

typedef struct {
	int fl_usagecnt;	/* number of "users" logging to this field */
	int fl_fld;		/* an rcfpack(r,c,0) */
	double fl_val;		/* stored value as a double */
	char fl_str[16];	/* stored value as a formatted string.
				 * N.B.: never overwrite last char: keep as \0
				 */
} FLog;

static FLog flog[NFLOGS];

/* add fld to the list. if already there, just increment usage count.
 * return 0 if ok, else -1 if no more room.
 */
int flog_add (fld)
int fld;
{
	FLog *flp, *unusedflp = 0;

	/* scan for fld already in list, or find an unused one along the way */
	for (flp = &flog[NFLOGS]; --flp >= flog; ) {
	    if (flp->fl_usagecnt > 0) {
		if (flp->fl_fld == fld) {
		    flp->fl_usagecnt++;
		    return (0);
		}
	    } else
		unusedflp = flp;
	}
	if (unusedflp) {
	    unusedflp->fl_fld = fld;
	    unusedflp->fl_usagecnt = 1;
	    return (0);
	}
	return (-1);
}

/* decrement usage count for flog for fld. if goes to 0 take it out of list.
 * ok if not in list i guess...
 */
void flog_delete (fld)
int fld;
{
	FLog *flp;

	for (flp = &flog[NFLOGS]; --flp >= flog; )
	    if (flp->fl_fld == fld && flp->fl_usagecnt > 0) {
		if (--flp->fl_usagecnt <= 0) {
		    flp->fl_usagecnt = 0;
		}
		break;
	    }
}

/* if plotting, listing or searching is active then
 * if rcfpack(r,c,0) is in the fld list, set its value to val.
 * return 0 if ok, else -1 if not in list.
 */
int flog_log (r, c, val, str)
int r, c;
double val;
char *str;
{
	if (plot_ison() || listing_ison() || srch_ison()) {
	    FLog *flp;
	    int fld = rcfpack (r, c, 0);
	    for (flp = &flog[NFLOGS]; --flp >= flog; )
		if (flp->fl_fld == fld && flp->fl_usagecnt > 0) {
		    flp->fl_val = val;
		    (void) strncpy (flp->fl_str, str, sizeof(flp->fl_str)-1);
		    return(0);
		}
	    return (-1);
	} else
	    return (0);
}

/* search for fld in list. if find it, return its value and str, if str.
 * return 0 if found it, else -1 if not in list.
 */
int flog_get (fld, vp, str)
int fld;
double *vp;
char *str;
{
	FLog *flp;

	for (flp = &flog[NFLOGS]; --flp >= flog; )
	    if (flp->fl_fld == fld && flp->fl_usagecnt > 0) {
		*vp = flp->fl_val;
		if (str) 
		    (void) strcpy (str, flp->fl_str);
		return (0);
	    }
	return (-1);
}
