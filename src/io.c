/* this file (in principle) contains all the device-dependent code for
 * handling screen movement and reading the keyboard. public routines are:
 *   c_pos(r,c), c_erase(), c_eol();
 *   chk_char(), read_char(), read_line (buf, max); and
 *   byetty().
 * N.B. we assume output may be performed by printf(), putchar() and
 *   fputs(stdout). since these are buffered we flush first in read_char().
 */

/* explanation of various conditional #define options:
 * UNIX: uses termcap for screen management.
 *   USE_TERMIO: use termio.h to control tty modes.
 *   USE_SGTTY: use sgtty.h to control tty modes.
 *   USE_NDELAY: do non-blocking tty reads with fcntl(O_NDELAY).
 *   USE_FIONREAD: do non-blocking tty reads with ioctl(FIONREAD).
 *   USE_ATTSELECT: do non-blocking reads with att's select(2) (4 args).
 *   USE_BSDSELECT: do non-blocking reads with bsd's select(2) (5 args).
 * TURBO_C: compiles for Turbo C 2.0. I'm told it works for Lattice and
 *     Microsoft too.
 *   USE_ANSISYS: default PC cursor control uses direct BIOS calls (thanks to
 *     Mr. Doug McDonald). If your PC does not work with this, however, add
 *     "device ANSI.SYS" to your config.sys file and build ephem with
 *     USE_ANSISYS.
 * VMS: uses QIO for input, TERMTABLE info for output. This code uses only
 *     standard VMS calls, i.e. it does not rely on any third-vendor termcap
 *     package or the like. The code includes recoqnition of arrow keys, it
 *     is easy to extend it to recoqnize other function keys. you don't
 *     need to #define VMS since it is inherent in the compiler.
 */

/* unless you are on VMS or WIN32 define one of these... */
#define UNIX
/* #define TURBO_C */

/* then if you defined UNIX you must use one of these ways to do non-blocking
 * tty reads
 */
//#define USE_FIONREAD
#define USE_NDELAY
/* #define USE_ATTSELECT */
/* #define USE_BSDSELECT */

/* and then if you defined UNIX you must also use one of these ways to control
 * the tty modes.
 */
#define USE_TERMIO
/* #define USE_SGTTY */

/* if you defined TURBO_C you might want this too if screen io looks garbled */
/* #define USE_ANSISYS */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "screen.h"

#ifdef UNIX
#include <signal.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#ifdef USE_TERMIO
#include <termios.h>
#include <ncurses.h>
#include <term.h>
#include <sys/ioctl.h>
#endif
#ifdef USE_SGTTY
#include <sgtty.h>
#endif
#ifdef USE_BSDSELECT
#include <sys/time.h>
#endif
#ifdef USE_NDELAY
#include <fcntl.h>
#endif

extern char *tgoto();
static char *cm, *ce, *cl, *ks, *ke, *kl, *kr, *ku, *kd; /* termcap sequences */
static int tloaded;
static int ttysetup;
#ifdef USE_TERMIO
static struct termios orig_tio;
#endif
#ifdef USE_SGTTY
static struct sgttyb orig_sgtty;
#endif

static void tload();
static void setuptty();

/* move cursor to row, col, 1-based.
 * we assume this also moves a visible cursor to this location.
 */
void c_pos (r, c)
int r, c;
{
    if (!tloaded) tload();
    fputs (tgoto (cm, c-1, r-1), stdout);
}

/* erase entire screen. */
void c_erase()
{
    if (!tloaded) tload();
    fputs (cl, stdout);
}

/* erase to end of line */
void c_eol()
{
    if (!tloaded) tload();
    fputs (ce, stdout);
}

#ifdef USE_NDELAY
static char sav_char;	/* one character read-ahead for chk_char() */
#endif

/* return 0 if there is a char that may be read without blocking, else -1 */
int chk_char()
{
#ifdef USE_NDELAY
    if (!ttysetup) setuptty();
    fflush (stdout);
    if (sav_char)
        return (0);
    fcntl (0, F_SETFL, O_NDELAY);	/* non-blocking read. FNDELAY on BSD */
    if (read (0, &sav_char, 1) != 1)
        sav_char = 0;
    return (sav_char ? 0 : -1);
#endif
#ifdef USE_ATTSELECT
    int nfds, rfds, wfds, to;
    if (!ttysetup) setuptty();
    fflush (stdout);
    rfds = 1 << 0;	/* reads are on fd 0 */
    wfds = 0;	/* not interested in any write fds */
    nfds = 1;	/* check only fd 0 */
    to = 0;		/* don't delay - return 0 if nothing pending */
    return (select (nfds, &rfds, &wfds, to) == 0 ? -1 : 0);
#endif
#ifdef USE_BSDSELECT
    int nfds, rfds, wfds, xfds;
    struct timeval to;
    if (!ttysetup) setuptty();
    fflush (stdout);
    rfds = 1 << 0;	/* reads are on fd 0 */
    wfds = 0;	/* not interested in any write fds */
    xfds = 0;	/* not interested in any exception fds */
    nfds = 1;	/* check only fd 0 */
    to.tv_sec = 0;	/* don't delay - return 0 if nothing pending */
    to.tv_usec = 0;	/* don't delay - return 0 if nothing pending */
    return (select (nfds, &rfds, &wfds, &xfds, &to) == 0 ? -1 : 0);
#endif
#ifdef USE_FIONREAD
    long n;
    if (!ttysetup) setuptty();
    fflush (stdout);
    ioctl (0, FIONREAD, &n);
    return (n > 0 ? 0 : -1);
#endif
}

/* used to time out of a read */
static int got_alrm;
static void
on_alrm()
{
    got_alrm = 1;
}

/* see if c is the first of any of the termcap arrow key sequences.
 * if it is, read the rest of the sequence, and return the hjkl code
 * that corresponds.
 * if no match, just return c.
 */
static char
chk_arrow (c)
char c;
{
    register char *seq;

    if (kl && kd && ku && kr &&
            (c == *(seq = kl) || c == *(seq = kd) || c == *(seq = ku)
             || c == *(seq = kr)))
    {
        char seqa[32]; /* maximum arrow escape sequence ever expected */
        unsigned l = strlen(seq);
        seqa[0] = c;
        if (l > 1)
        {
            extern unsigned alarm();
            /* cautiously read rest of arrow sequence */
            got_alrm = 0;
            (void) signal (SIGALRM, on_alrm);
            alarm(2);
            read (0, seqa+1, l-1);
            alarm(0);
            if (got_alrm)
                return (c);
        }
        seqa[l] = '\0';
        if (strcmp (seqa, kl) == 0)
            return ('h');
        if (strcmp (seqa, kd) == 0)
            return ('j');
        if (strcmp (seqa, ku) == 0)
            return ('k');
        if (strcmp (seqa, kr) == 0)
            return ('l');
    }
    return (c);
}

/* read the next char, blocking if necessary, and return it. don't echo.
 * map the arrow keys if we can too into hjkl
 */
char read_char()
{
    char c;
    if (!ttysetup) setuptty();
    fflush (stdout);
#ifdef USE_NDELAY
    fcntl (0, F_SETFL, 0);	/* blocking read */
    if (sav_char)
    {
        c = sav_char;
        sav_char = 0;
    }
    else
#endif
        read (0, &c, 1);
    c = chk_arrow (c & 0177); /* just ASCII, please */
    return (c);
}

/* do whatever might be necessary to get the screen and/or tty back into shape.
 */
void byetty()
{
    /* if keypad mode switch is used, turn it off now */
    if (ke)
    {
        fputs (ke, stdout);
        fflush (stdout);
    }

#ifdef USE_TERMIO
    tcsetattr (0, TCSANOW, &orig_tio);
#endif
#ifdef USE_SGTTY
    ioctl (0, TIOCSETP, &orig_sgtty);
#endif
#ifdef USE_NDELAY
    fcntl (0, F_SETFL, 0);	/* be sure to go back to blocking read */
#endif
    ttysetup = 0;
}

/* like tgetstr() but discard termcap delay codes, for now anyways */
static char *
egetstr (name, sptr)
char *name;
char **sptr;
{
    extern char *tgetstr();
    char *s;

    if ((s = tgetstr (name, sptr)) != NULL)
    {
        char c;
        while ((c = *s) == '*' || isdigit(c))
            s += 1;
    }
    return (s);
}

static void
tload()
{
    extern char *getenv(), *tgetstr();
    extern char *UP, *BC;
    char *egetstr();
    static char tbuf[512];
    char rawtbuf[1024];
    char *tp;
    char *ptr;

    if (!(tp = getenv ("TERM")))
    {
        printf ("no TERM\n");
        exit(1);
    }
    if (tgetent (rawtbuf, tp) != 1)
    {
        printf ("Can't find termcap for %s\n", tp);
        exit (1);
    }

    ptr = tbuf;

    ku = egetstr ("ku", &ptr);
    kd = egetstr ("kd", &ptr);
    kl = egetstr ("kl", &ptr);
    kr = egetstr ("kr", &ptr);

    if (!(cm = egetstr ("cm", &ptr)))
    {
        printf ("No termcap cm code\n");
        exit(1);
    }
    if (!(ce = egetstr ("ce", &ptr)))
    {
        printf ("No termcap ce code\n");
        exit(1);
    }
    if (!(cl = egetstr ("cl", &ptr)))
    {
        printf ("No termcap cl code\n");
        exit(1);
    }

    UP = egetstr ("up", &ptr);
    if (!tgetflag ("bs"))
        BC = egetstr ("bc", &ptr);

    if (!ttysetup) setuptty();

    /* if keypad mode switch is used, do it now */
    if ((ks = egetstr ("ks", &ptr)) != NULL)
    {
        ke = egetstr ("ke", &ptr);
        fputs (ks, stdout);
        fflush (stdout);
    }

    tloaded = 1;
}

/* set up tty for char-by-char read, non-blocking  */
static void setuptty()
{
#ifdef USE_TERMIO
    struct termios tio;

    tcgetattr (0, &orig_tio);
    tio = orig_tio;
    tio.c_iflag &= ~ICRNL;	/* leave CR unchanged */
    tio.c_oflag &= ~OPOST;	/* no output processing */
    tio.c_lflag &= ~(ICANON|ECHO); /* no input processing, no echo */
    tio.c_cc[VMIN] = 1;	/* return after each char */
    tio.c_cc[VTIME] = 0;	/* no read timeout */
    tcsetattr (0, TCSANOW, &tio);
#endif
#ifdef USE_SGTTY
    struct sgttyb sg;

    ioctl (0, TIOCGETP, &orig_sgtty);
    sg = orig_sgtty;
    sg.sg_flags &= ~ECHO;	/* do our own echoing */
    sg.sg_flags &= ~CRMOD;	/* leave CR and LF unchanged */
    sg.sg_flags |= XTABS;	/* no tabs with termcap */
    sg.sg_flags |= CBREAK;	/* wake up on each char but can still kill */
    ioctl (0, TIOCSETP, &sg);
#endif
    ttysetup = 1;
}
/* end of #ifdef UNIX */
#endif

#ifdef TURBO_C
#ifdef USE_ANSISYS
#define	ESC	'\033'
/* position cursor.
 * (ANSI: ESC [ r ; c f) (r/c are numbers given in ASCII digits)
 */
c_pos (r, c)
int r, c;
{
    printf ("%c[%d;%df", ESC, r, c);
}

/* erase entire screen. (ANSI: ESC [ 2 J) */
c_erase()
{
    printf ("%c[2J", ESC);
}

/* erase to end of line. (ANSI: ESC [ K) */
c_eol()
{
    printf ("%c[K", ESC);
}
#else
#include <dos.h>
union REGS rg;

/* position cursor.
 */
c_pos (r, c)
int r, c;
{
    rg.h.ah = 2;
    rg.h.bh = 0;
    rg.h.dh = r-1;
    rg.h.dl = c-1;
    int86(16,&rg,&rg);
}

/* erase entire screen.  */
c_erase()
{
    int cur_cursor, i;
    rg.h.ah = 3;
    rg.h.bh = 0;
    int86(16,&rg,&rg);
    cur_cursor = rg.x.dx;
    for(i = 0; i < 25; i++)
    {
        c_pos(i+1,1);
        rg.h.ah = 10;
        rg.h.bh = 0;
        rg.h.al = 32;
        rg.x.cx = 80;
        int86(16,&rg,&rg);
    }
    rg.h.ah = 2;
    rg.h.bh = 0;
    rg.x.dx = cur_cursor;
    int86(16,&rg,&rg);

}

/* erase to end of line.*/
c_eol()
{
    int cur_cursor, i;
    rg.h.ah = 3;
    rg.h.bh = 0;
    int86(16,&rg,&rg);
    cur_cursor = rg.x.dx;
    rg.h.ah = 10;
    rg.h.bh = 0;
    rg.h.al = 32;
    rg.x.cx = 80 - rg.h.dl;
    int86(16,&rg,&rg);
    rg.h.ah = 2;
    rg.h.bh = 0;
    rg.x.dx = cur_cursor;
    int86(16,&rg,&rg);

}
#endif

/* return 0 if there is a char that may be read without blocking, else -1 */
chk_char()
{
    return (kbhit() == 0 ? -1 : 0);
}

/* read the next char, blocking if necessary, and return it. don't echo.
 * map the arrow keys if we can too into hjkl
 */
read_char()
{
    int c;
    fflush (stdout);
    c = getch();
    if (c == 0)
    {
        /* get scan code; convert to direction hjkl if possible */
        c = getch();
        switch (c)
        {
        case 0x4b:
            c = 'h';
            break;
        case 0x50:
            c = 'j';
            break;
        case 0x48:
            c = 'k';
            break;
        case 0x4d:
            c = 'l';
            break;
        }
    }
    return (c);
}

/* do whatever might be necessary to get the screen and/or tty back into shape.
 */
byetty()
{
}
/* end of #ifdef TURBO_C */
#endif

#ifdef VMS
#include <string.h>
#include <iodef.h>
#include <descrip.h>
#include <dvidef.h>
#include <smgtrmptr.h>
#include <starlet.h>
#include <lib$routines.h>
#include <smg$routines.h>

/* Structured types for use in system calls */
typedef struct
{
    unsigned short status;
    unsigned short count;
    unsigned int info;
} io_status_block;
typedef struct
{
    unsigned short buffer_length;
    unsigned short item_code;
    void *buffer_address;
    unsigned short *return_length_address;
    unsigned long terminator;
} item_list;

static unsigned short ttchan = 0; /* channel number for terminal    */
volatile static io_status_block iosb; /* I/O status block for operation */
/* currently in progress          */
volatile static unsigned char input_buf; /* buffer to recieve input charac-*/
/* ter when operation completes   */
static void *term_entry;          /* pointer to TERMTABLE entry     */
#define MAXCAP 10
static char ce[MAXCAP];           /* ce and cl capability strings for  */
static char cl[MAXCAP];           /* this terminal type                */

/* Declaration of special keys to be recoqnized on input */
/* Number of special keys defined */
#define MAXKEY 4
/* TERMTABLE capability codes for the keys */
static long capcode[MAXKEY] = {SMG$K_KEY_UP_ARROW,SMG$K_KEY_DOWN_ARROW,
                               SMG$K_KEY_RIGHT_ARROW,SMG$K_KEY_LEFT_ARROW
                              };
/* character codes to be returned by read_char when a special key is presssed */
static int retcode[MAXKEY] = {'k','j','l','h'};
/* the actual capability strings from the key */
static char keycap[MAXKEY][MAXCAP];

static char special_buffer[MAXCAP];   /* buffer for reading special key */
static int chars_in_buffer;           /* number of characters in buffer */

/* set up the structures for this I/O module */
inittt()
{
    unsigned int status;   /* system routine return status */
    $DESCRIPTOR(tt,"TT");  /* terminal name */
    item_list itmlst;      /* item list for $getdvi obtaining term type */
    unsigned long devtype; /* terminal type returned form $getdvi */
    unsigned short retlen; /* return length from $getdvi */
    unsigned long lenret;  /* return length from smg$get_term_data */
    unsigned long maxlen;  /* maximum return length */
    unsigned long cap_code;/* capability code */
#define MAXINIT 20
    char init_string[MAXINIT];/* string to initialize terminal */
    int key;

    /* Assign a channel to the terminal */
    if (!((status = sys$assign(&tt,&ttchan,0,0))&1)) lib$signal(status);

    /* Get terminal type. Note that it is possible to use the same
     * iosb at this stage, because no I/O is initiated yet.
     */
    itmlst.buffer_length = 4;
    itmlst.item_code = DVI$_DEVTYPE;
    itmlst.buffer_address = &devtype;
    itmlst.return_length_address = &retlen;
    itmlst.terminator = 0;
    if (!((status = sys$getdviw(0,ttchan,0,&itmlst,&iosb,0,0,0))&1))
        lib$signal(status);
    if (!(iosb.status&1)) lib$signal(iosb.status);

    /* Get the TERMTABLE entry corresponding to the terminal type */
    if (!((status = smg$init_term_table_by_type(&devtype,
                    &term_entry))&1)) lib$signal(status);

    /* Get the initialisation string and initialize terminal */
    cap_code = SMG$K_INIT_STRING;
    maxlen = MAXINIT - 1;
    if (!((status = smg$get_term_data(&term_entry,&cap_code,&maxlen,
                                      &lenret,init_string))&1)) lib$signal(status);
    init_string[lenret] = '\0';
    fputs(init_string,stdout);
    fflush(stdout);

    /* Get ce and cl capabilities, these are static */
    cap_code = SMG$K_ERASE_TO_END_LINE;
    maxlen = MAXCAP-1;
    if (!((status = smg$get_term_data(&term_entry,&cap_code,&maxlen,
                                      &lenret,ce))&1)) lib$signal(status);
    ce[lenret] = '\0';

    cap_code = SMG$K_ERASE_WHOLE_DISPLAY;
    maxlen = MAXCAP-1;
    if (!((status = smg$get_term_data(&term_entry,&cap_code,&maxlen,
                                      &lenret,cl))&1)) lib$signal(status);
    cl[lenret] = '\0';

    /* Here one could obtain line drawing sequences, please feel free
       to implement it ... */

    /* Get special keys to be recoqnized on input */
    for (key = 0; key<MAXKEY; key++)
    {
        maxlen = MAXCAP-1;
        if (!((status = smg$get_term_data(&term_entry,&capcode[key],
                                          &maxlen,&lenret,keycap[key]))&1)) lib$signal(status);
        keycap[key][lenret] = '\0';
    }

    /* Initiate first input operation, NOECHO.
     * NOFILTR allows any character to get through, this makes it
     * possible to implement arrow recoqnition, and also makes
     * DEL and BS get through.
     * We don't wait for the operation to complete.
     * Note that stdout has already been fflush'ed above.
     */
    if (!((status = sys$qio(0,ttchan,
                            IO$_READVBLK|IO$M_NOECHO|IO$M_NOFILTR,
                            &iosb,0,0,&input_buf,1,0,0,0,0))&1)) lib$signal(status);

    /* Initialise special key buffer */
    chars_in_buffer = 0;
} /* inittt */


/* return 0 if there is a char that may be read without blocking, else -1 */
chk_char()
{
    if (!ttchan) inittt();

    return ( chars_in_buffer != 0 ? 0 :(iosb.status == 0 ? -1 : 0));
}

/* read the next char, blocking if necessary, and return it. don't echo.
 * map the arrow keys if we can too into hjkl
 */
read_char()
{
    unsigned int status;
    int buf;
    int i;
    int found_key;
    int key;
    int this_len;
    int match;

    if (!ttchan) inittt();

    /* If we attempted to read an special key previously, there are characters
     * left in the buffer, return these before doing more I/O
     */
    if (chars_in_buffer!=0)
    {
        buf = special_buffer[0];
        chars_in_buffer--;
        for (i = 0; i<chars_in_buffer; i++)
        {
            special_buffer[i] = special_buffer[i+1];
        }
        special_buffer[chars_in_buffer] = '\0';
    }
    else
    {

        /* Loop over characters read, the loop is terminated when the
         * characters read so far do not match any of the special keys
         * or when the characters read so far is identical to one of
         * the special keys.
         */

        do
        {
            /* Wait for I/O to complete */
            if (!((status = sys$synch(0,&iosb))&1)) lib$signal(status);
            special_buffer[chars_in_buffer] = input_buf;
            chars_in_buffer++;
            special_buffer[chars_in_buffer] = '\0';

            /* Initiate next input operation */
            fflush (stdout);
            if (!((status = sys$qio(0,ttchan,
                                    IO$_READVBLK|IO$M_NOECHO|IO$M_NOFILTR,
                                    &iosb,0,0,&input_buf,1,0,0,0,0))&1)) lib$signal(status);


            /* Check for match with all special strings */
            match = 0;
            found_key = MAXKEY;
            for (key = 0; key<MAXKEY; key++)
            {
                this_len = strlen(keycap[key]);
                if (this_len<chars_in_buffer) continue;
                if (!strncmp(keycap[key],special_buffer,chars_in_buffer))
                {
                    match = -1;
                    if (this_len == chars_in_buffer)
                    {
                        found_key = key;
                        break;
                    }
                }
            }
        }
        while (match && (found_key == MAXKEY));

        /* If one of the keys matches the input string, return the
         * corresponding  key code
         */
        if (found_key != MAXKEY)
        {
            buf = retcode[found_key];
            chars_in_buffer = 0;
        }
        else /* return first character and store the rest in the buffer */
        {
            buf = special_buffer[0];
            chars_in_buffer--;
            for (i = 0; i<chars_in_buffer; i++)
            {
                special_buffer[i] = special_buffer[i+1];
            }
        }
        special_buffer[chars_in_buffer] = '\0';
    }
    return(buf);
}

/* do whatever might be necessary to get the screen and/or tty back into shape.
 */
byetty()
{
    unsigned int status;

    if (ttchan)
    {
        /* There is no string in SMG to send to the terminal when
         * terminating, one could clear the screen, move the cursor to
         * the last line, or whatever. This program clears the screen
         * anyway before calling this routine, so we do nothing.
         */



        /* The following is not really neccessary, it will be done at program
         * termination anyway, but if someone tries to use the I/O routines agai
        n
         * it might prove useful...
         */
        if (!((status = smg$del_term_table())&1)) lib$signal(status);
        if (!((status = sys$dassgn(ttchan))&1)) lib$signal(status);
        /* This also cancels any outstanding I/O on the channel */
        ttchan = 0; /* marks terminal I/O as not initialized */
    }
}

/* position cursor. */
c_pos (r, c)
int r, c;
{
    unsigned long vector[3]; /* argument vector (position)   */
    unsigned long status;    /* system service return status */
    long lenret;             /* length of returned string    */
    long maxlen;             /* maximum return length        */
    unsigned long capcode;   /* capability code              */
    char seq[2*MAXCAP];      /* returned string              */

    if (!ttchan) inittt();

    /* Set cursor depends on the position, therefore we have to call
     * get_term_data for each operation
     */
    vector[0] = 2;
    vector[1] = r;
    vector[2] = c;
    capcode = SMG$K_SET_CURSOR_ABS;
    maxlen = 2*MAXCAP-1;
    if (!((status = smg$get_term_data(&term_entry,&capcode,&maxlen,
                                      &lenret,seq,vector))&1)) lib$signal(status);
    seq[lenret] = '\0';

    fputs(seq,stdout);
}

/* erase entire screen. */
c_erase()
{
    if (!ttchan) inittt();

    fputs(cl,stdout);
}

/* erase to end of line. */
c_eol()
{
    if (!ttchan) inittt();

    fputs(ce,stdout);
}
/* end of #ifdef VMS */
#endif

#if defined(WIN32) && defined(_CONSOLE)
#include <windows.h>
static HANDLE hStdout, hStdin;

void initscr()
{
    hStdout = GetStdHandle ( STD_OUTPUT_HANDLE );
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if ( INVALID_HANDLE_VALUE == hStdout ||
            INVALID_HANDLE_VALUE == hStdin)
    {
        printf("INVALID_HANDLE_VALUE\n");
        exit(1);
    }
    SetConsoleTitle("Ephem MSVC");
}

void clrtoeol()
{
    int i=80;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
    GetConsoleScreenBufferInfo( hStdout, &csbi );
    for(i=csbi.dwCursorPosition.X; i<csbi.srWindow.Right; i++)
    {
        printf(" ");
    }
}

/* From microsoft support. */
/* Standard error macro for reporting API errors */
#define PERR(ok, api){if(!(ok)) \
     printf("%s:Error %d from %s on line %d\n", \
        __FILE__, GetLastError(), api, __LINE__);}

void clear()
{
    COORD home = { 0, 0 };  /* here's where we'll home the cursor */
    BOOL ok;
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
    DWORD dwConSize; /* number of character cells in the current buffer */

    /* Get the number of character cells in the current buffer */
    ok = GetConsoleScreenBufferInfo( hStdout, &csbi );
    PERR( ok, "GetConsoleScreenBufferInfo" );
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* Fill the entire screen with blanks */
    ok = FillConsoleOutputCharacter( hStdout, (TCHAR) ' ',
                                     dwConSize, home, &cCharsWritten );
    PERR( ok, "FillConsoleOutputCharacter" );

    /* Get the current text attribute */
    ok = GetConsoleScreenBufferInfo( hStdout, &csbi );
    PERR( ok, "ConsoleScreenBufferInfo" );

    /* Now set the buffer's attributes accordingly */
    ok = FillConsoleOutputAttribute( hStdout, csbi.wAttributes,
                                     dwConSize, home, &cCharsWritten );
    PERR( ok, "FillConsoleOutputAttribute" );

    /* Put the cursor at (0, 0) */
    ok = SetConsoleCursorPosition( hStdout, home );
    PERR( ok, "SetConsoleCursorPosition" );
    return;
}

void endwin(void)
{
    clear();
}

void gotoxy(int x, int y)
{
    COORD pos;
    pos.X = (short) y; /* swap x y. */
    pos.Y = (short) x;
    SetConsoleCursorPosition ( hStdout, pos );
}

c_pos (r, c)
int r, c;
{
    gotoxy(r,c-1);
}

/* erase entire screen. (ANSI: ESC [ 2 J) */
c_erase()
{
    clear();
}

/* erase to end of line. (ANSI: ESC [ K) */
c_eol()
{
    clrtoeol();
}

/* return 0 if there is a char that may be read without blocking, else -1 */
chk_char()
{
    return (kbhit() == 0 ? -1 : 0);
}

/* read the next char, blocking if necessary, and return it. don't echo.
 * map the arrow keys if we can too into hjkl
 */
read_char()
{
    int c;
    fflush (stdout);
    c = getch();
    if (c==224)
    {
        /* get scan code; convert to direction hjkl if possible */
        c = getch();
        switch (c)
        {
        case 72:
            c = 'k';
            break;
        case 75:
            c = 'h';
            break;
        case 77:
            c = 'l';
            break;
        case 80:
            c = 'j';
            break;
        }
    }
    return (c);
}

/* do whatever might be necessary to get the screen and/or tty back into shape.
 */
byetty()
{
    endwin();
}


sleep(int seconds)
{
    (void) _sleep (1000 * seconds);
}

#endif	/* WIN32 */

/* read up to max chars into buf, with cannonization.
 * add trailing '\0' (buf is really max+1 chars long).
 * return count of chars read (not counting '\0').
 * assume cursor is already positioned as desired.
 * if type END when n==0 then return -1.
 */
int read_line (buf, max)
char buf[];
int max;
{
    static char erase[] = "\b \b";
    int n, c;
    int done;

#ifdef UNIX
    if (!ttysetup) setuptty();
#endif

    for (done = 0, n = 0; !done; )
        switch (c = read_char())  	/* does not echo */
        {
        case cntrl('h'):	/* backspace or */
        case 0177:		/* delete are each char erase */
            if (n > 0)
            {
                fputs (erase, stdout);
                n -= 1;
            }
            break;
        case cntrl('u'):		/* line erase */
            while (n > 0)
            {
                fputs (erase, stdout);
                n -= 1;
            }
            break;
        case '\r':	/* EOL */
            done++;
            break;
        default:			/* echo and store, if ok */
            if (n == 0 && c == END)
                return (-1);
            if (n >= max)
                putchar (cntrl('g'));
            else if (isprint(c))
            {
                putchar (c);
                buf[n++] = c;
            }
        }

    buf[n] = '\0';
    return (n);
}

