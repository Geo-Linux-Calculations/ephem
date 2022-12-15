/* screen layout details
 *
 * it looks better if the fields are drawn in some nice order so it you
 * rearrange the fields, check the menu printing functions.
 * NB: all row/col values are 1 based, with upper left at [1,1]
 */

/* size of screen */
#ifdef NCURSES_LARGE
 #include <ncurses.h>
 #define	NR	LINES
 #define	NC	COLS
#else
 #define	NR	24
 #define	NC	80
#endif

/* optional ANSI colors */
#define COLOR_VERSION		0
#define COLOR_WATCH		1
#define COLOR_BORDERS		2
#define COLOR_MM_LABELS		3
#define COLOR_SRCH_PRSTATE	4
#define COLOR_PLOT_PRSTATE	5
#define COLOR_LISTING_PRSTATE	6
#define COLOR_NOW		7
#define COLOR_TWILIGHT		8
#define COLOR_ALT_LABELS	9
#define COLOR_ALT_MENU		10
#define COLOR_PHOON 		11
#define COLOR_GLOBE		12
#define COLOR_SUN		13
#define COLOR_MOON		14
#define COLOR_MERCURY		15
#define COLOR_VENUS		16
#define COLOR_MARS		17
#define COLOR_JUPITER		18
#define COLOR_SATURN		19
#define COLOR_URANUS		20
#define COLOR_NEPTUNE		21
#define COLOR_PLUTO		22
#define COLOR_OBJX		23
#define COLOR_OBJY		24
#define COLOR_EARTH		25
#define N_COLORS	26
#ifdef ANSI_COLORS
typedef struct {
   char   *name;
   int c1,c2,c3;
} APP_COLOR;
APP_COLOR *App_Colors;
void app_color();
#define APP_COLOR_CMD  (N_COLORS+1000)
int Colors_Enabled;
 #define COLOR_CODE(c) app_color(c)
 #define COLOR_OFF    if(Colors_Enabled)(void)fputs("\033[0m",stdout);
#else
 #define COLOR_CODE(c) {}
 #define COLOR_OFF    {}
#endif


#define	ASPECT	(4./3.)	/* screen width to height dimensions ratio */

#define	GAP	6	/* gap between field name and value */

#define	COL1		1
#define	COL2		27
#define	COL3		44
#define	COL4		61	/* calendar */

#define	R_PROMPT	1	/* prompt row */
#define	C_PROMPT	COL1

#define	R_NEWCIR	2
#define	C_NEWCIR	((NC-17)/2) /* 17 is length of the message */

#define	R_TOP		3	/* first row of top menu items */

#define	R_TZN	(R_TOP+0)
#define	C_TZN	COL1
#define	R_LT	R_TZN
#define	C_LT	(C_TZN+GAP-2)
#define	R_LD	R_TZN
#define	C_LD	(C_TZN+13)

#define	R_UT	(R_TOP+1)
#define	C_UT	COL1
#define	C_UTV	(C_UT+GAP-2)
#define	R_UD	R_UT
#define	C_UD	(C_UT+13)

#define	R_JD	(R_TOP+2)
#define	C_JD	COL1
#define	C_JDV	(C_JD+GAP+3)

#define	R_LST	(R_TOP)
#define	C_LST	COL2
#define	C_LSTV	(C_LST+GAP)

#define	R_LAT	(R_TOP+0)
#define	C_LAT	COL3
#define	C_LATV	(C_LAT+4)

#define	R_DAWN	(R_TOP+2)
#define	C_DAWN	COL2
#define	C_DAWNV	(C_DAWN+GAP+3)

#define	R_STPSZ	(R_TOP+7)
#define	C_STPSZ	COL2
#define	C_STPSZV (C_STPSZ+GAP-1)

#define	R_HEIGHT (R_TOP+2)
#define	C_HEIGHT COL3
#define	C_HEIGHTV (C_HEIGHT+GAP)

#define	R_PRES	(R_TOP+4)
#define	C_PRES	COL3
#define	C_PRESV	(C_PRES+GAP)

#define	R_WATCH	(R_TOP+3)
#define	C_WATCH	COL1

#define	R_LISTING (R_TOP+4)
#define	C_LISTING COL1
#define	C_LISTINGV (C_LISTING+20)

#define	R_SRCH	(R_TOP+5)
#define	C_SRCH	COL1
#define	C_SRCHV	(C_SRCH+16)

#define	R_PLOT	(R_TOP+6)
#define	C_PLOT	COL1
#define	C_PLOTV (C_PLOT+20)

#define	R_ALTM	(R_TOP+7)
#define	C_ALTM	COL1
#define	C_ALTMV	(C_ALTM+10)

#define	R_TZONE	(R_TOP+5)
#define	C_TZONE	COL3
#define	C_TZONEV (C_TZONE+GAP-1)

#define	R_LONG	(R_TOP+1)
#define	C_LONG	COL3
#define	C_LONGV	(C_LONG+4)

#define	R_DUSK	(R_TOP+3)
#define	C_DUSK	COL2
#define	C_DUSKV	(C_DUSK+GAP+3)

#define	R_NSTEP (R_TOP+6)
#define	C_NSTEP	COL2
#define	C_NSTEPV (C_NSTEP+GAP)

#define	R_TEMP	(R_TOP+3)
#define	C_TEMP	COL3
#define	C_TEMPV	(C_TEMP+GAP)

#define	R_EPOCH		(R_TOP+6)
#define	C_EPOCH		COL3
#define	C_EPOCHV	(C_EPOCH+GAP)

#define	R_PAUSE (R_TOP+7)
#define	C_PAUSE	COL3
#define	C_PAUSEV (C_PAUSE+GAP)

#define	R_MNUDEP	(R_TOP+6)
#define	C_MNUDEP	COL3
#define	C_MNUDEPV	(C_EPOCH+GAP)

#define	R_LON	(R_TOP+4)
#define	C_LON	COL2
#define	C_LONV	(C_LON+GAP+3)

#define	R_CAL	R_TOP
#define	C_CAL   COL4

/* planet rows */
#define	R_PLANTAB	(R_TOP+9)
#define	R_SUN		(R_PLANTAB+1)
#define	R_MOON		(R_PLANTAB+2)
#define	R_MERCURY	(R_PLANTAB+3)
#define	R_VENUS		(R_PLANTAB+4)
#define	R_MARS		(R_PLANTAB+5)
#define	R_JUPITER	(R_PLANTAB+6)
#define	R_SATURN	(R_PLANTAB+7)
#define	R_URANUS	(R_PLANTAB+8)
#define	R_NEPTUNE	(R_PLANTAB+9)
#define	R_PLUTO		(R_PLANTAB+10)
#define	R_OBJX		(R_PLANTAB+11)
#define	R_OBJY		(R_PLANTAB+12)

#define	C_OBJ		1
#define	C_CONSTEL	2
#define	C_XTRA		3

/* menu 1 info table */
#define	C_RA		4
#define	C_DEC		12
#define	C_AZ		19
#define	C_ALT		26
#define	C_HLONG		33
#define	C_HLAT		40
#define	C_EDIST		47
#define C_SDIST 	54
#define	C_ELONG		61
#define	C_SIZE		68
#define	C_MAG		73
#define	C_PHASE		78

/* menu 2 screen items */
#define	C_RISETM	7
#define	C_RISEAZ	18
#define	C_TRANSTM	29
#define	C_TRANSALT	40
#define	C_SETTM		51
#define	C_SETAZ		62
#define	C_TUP		73

/* menu 3 items */
#define	C_SUN		4
#define	C_MOON		10
#define	C_MERCURY	17
#define	C_VENUS		23
#define	C_MARS		30
#define	C_JUPITER	36
#define	C_SATURN	43
#define	C_URANUS	49
#define	C_NEPTUNE	56
#define	C_PLUTO		62
#define	C_OBJX		69
#define	C_OBJY		75

/* menu for jupiter aux info items */
#define	R_JCML		(R_TOP+10)
#define	C_JCMLSI	43
#define	C_JCMLSII	60
#define	C_JMNAMES	9
#define	C_JMX		28
#define	C_JMY		43
#define	C_JMZ		58
#define	R_JMAP		23
#define	C_JMAP		1
#define	R_JCOLHDNGS	17
#define	R_IO		(R_JCOLHDNGS+1)
#define	R_EUROPA	(R_JCOLHDNGS+2)
#define	R_GANYMEDE	(R_JCOLHDNGS+3)
#define	R_CALLISTO	(R_JCOLHDNGS+4)

#define	PW	(NC-C_PROMPT+1)	/* total prompt line width */

/* macros to pack a row/col and menu selection flags all into 16-bits.
 * (use this rather than a structure because we can compare them so easily.
 * could use bit fields and a union, but then can't init them or use switch.)
 * bit field defs: [15..13]=menu [12..11]=flags [10..0]=NC*row+column(0 based).
 * see sel_fld.c.
 * F_MNUX also used in main to manage which bottom menu is up.
 */
#define	F_MMNU		(0<<13)	/* field is on main menu (or on all menus) */
#define	F_MNU1		(1<<13)	/* field is on menu 1 */
#define	F_MNU2	 	(2<<13)	/* field is on menu 2 */
#define	F_MNU3		(3<<13)	/* field is on menu 3 */
#define	F_MNUJ		(4<<13)	/* field is on jupiter menu */
#define	F_PLT		(1<<12)	/* field may be picked for plotting or listng */
#define	F_CHG		(1<<11)	/* field may be picked for changing */
#define	rcfpack(r,c,f)	((f) | (((r)-1)*NC + ((c)-1)))
#define	unpackr(p)	(((p) & 0x7ff)/NC+1)
#define	unpackc(p)	(((p) & 0x7ff)%NC+1)
#define	unpackrc(p)	((p) & 0x7ff)
#define	tstpackf(p,f)	(((p) & ((f)&0x1800)) && \
		    (((p)&0xe000) == ((f)&0xe000) || ((p)&0xe000) == F_MMNU))

/* additions to the planet defines from astro.h.
 * must not conflict, and must fit in range 0..15.
 */
#define	SUN	(PLUTO+1)
#define	MOON	(PLUTO+2)
#define	OBJX	(PLUTO+3)	/* the user-defined object */
#define	OBJY	(PLUTO+4)	/* the user-defined object */
#define	NOBJ	(OBJY+1)	/* total number of objects */

#define	cntrl(x)	((x) & 037)
#define	QUIT		cntrl('d')	/* char to exit program */
#define	HELP		'?'		/* char to give help message */
#define	REDRAW		cntrl('l')	/* char to redraw (like vi) */
#define	VERSION		cntrl('v')	/* char to display version number */
#define	END		'q'		/* char to quit current mode */


/* table of the fields, with flags indicating which menu(s) they are on and
 * whether pickable for changing or plotting.
 * N.B. type must be long enough to hold 16 bits.
 */
typedef unsigned short F_t;

