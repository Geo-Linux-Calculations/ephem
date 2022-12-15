On any modern UNIX just type
    make ephem
    sudo make install
    ephem

Now read Man.txt to use it.



For older systems the following legacy notes might prove helpful.


1) io.c:
   define UNIX, VMS or TURBO_C in io.c depending on your system. Note that
   TURBO_C also seems to work ok with Microsoft and Lattice C too but these
   have not been tested recently. Note also that the VMS C compiler defines VMS
   automatically so you don't really have to #define it.

   Also in io.c and if you use UNIX, you have four choices of methods for doing
   non-blocking reads and two choices for controlling tty modes. #define one
   of USE_FIONREAD, USE_NDELAY, USE_ATTSELECT and USE_BSDSELECT and one of
   USE_TERMIO and USE_SGTTY.

   And also in io.c MSDOS users may do cursor control with direct BIOS calls
   (the default), or with ANSI.SYS by defining USE_ANSISYS.

2) time.c:
   Select from two methods of dealing with time from the operating system
   with the TZA/TZB defines in time.c. If you get link undefines related to
   time functions try changing to the other form.

   On Sun systems running OS 4.0.3 (or BSD 4.3) or Apollo SR 10.1 use TZB and
   change <time.h> to <sys/time.h>.

   For VMS, since it does not support time zone info, do NOT #define EITHER
   of TZA or TZB. This will have the effect of leaving the time zone unchanged
   whenever you set the time via the "Now" option. (This is taken care of
   automatically in time.c by #undef'ining TZA and TZB if VMS is defined.)

3) mainmenu.c:
   if you are compiling for an IBM-PC then #define PC_GRAPHICS for a nicer
   looking way to draw the screen boundry lines using character graphics.

4) sel_fld.c:
   if your runtime library supports the system() function (to run a shell
   command) then leave #define BANG in sel_fld.c, else undefine it. When
   defined, this will allow you to jump out of ephem and run any command,
   then resume where you left off.

5) beware that I have not used string.h or strings.h. if your library's
   strlen() and str.*cmp() functions don't return int (such as long), then you
   will have to hand add string.h or your own extern declarations. I have
   included all the necessary declarations for the functions that return
   (char *) such as strcpy(), etc, though.

6) main.c calls sleep() which is not in some IBM-PC runtime C libraries. You
   might kludge up your own call that does a cpu countdown loop. The accuracy
   will only effect the Pause feature, not ephem's actual time mechanisms.

7) Ephem can now be built by simply compiling all the .c files and linking them
   all together. On Unix systems, you must also link with the termcap library
   (-ltermcap) and possibly the auxiliary math library (-lm) if your default C
   library does not include all the required transcendental functions. At the
   end of this file I have included a VMS build script for those building
   ephem on a that system.

The following files are pretty much just pure transliterations from BASIC
into C from machine-readable copies of the programs in Duffett-Smith's book.
They have nothing to do with the rest of ephem so they may be used for
completely different applications if so desired.

   aa_hadec.c anomaly.c astro.h cal_mjd.c comet.c eq_ecl.c moon.c moonnf.c
   nutation.c obliq.c parallax.c pelement.c plans.c reduce.c refract.c
   sex_dec.c sun.c utc_gst.c

If you would like to gut ephem for just its astronomical functionality,
start with body_cir().

$!========================================================================
$!
$! Name      : BUILD.COM
$!
$! Purpose   : compile and link ephem under VMS
$!
$! Arguments : P1/P2 = DEBUG: compile with DEBUG info
$!             P1/P2 = LINK : link only
$!             P1    = nn   : start compiling at list element "nn" (0-36)
$!
$! Created  23-MAR-1990   Karsten Spang
$! Modified 31-AUG-1990   Rick Dyson
$!                        added listing to FILES for v4.19
$!                        added P1 = nn option
$!
$!========================================================================
$   VERIFY = F$Verify (0)
$   On ERROR     Then GoTo EXIT
$   On CONTROL_Y Then GoTo EXIT
$   If P1 .eqs. "DEBUG" .or. P2 .eqs. "DEBUG"
$       Then
$           CC      := Cc /Debug /NoOptimize /NoList
$           LINK    := Link /Debug /NoMap
$       Else
$           CC      := Cc /NoList
$           LINK    := Link /NoMap
$   EndIf
$   FILES = "MAIN,AA_HADEC,ALTJ,ALTMENUS,ANOMALY,CAL_MJD,CIRCUM,COMET,"+ -
        "COMPILER,CONSTEL,EQ_ECL,FLOG,FORMATS,IO,LISTING,MAINMENU,"+ -
	"MOON,MOONNF,NUTATION,OBJX,OBLIQ,PARALLAX,PELEMENT,PLANS,PLOT,"+ -
        "POPUP,PRECESS,REDUCE,REFRACT,RISET,RISET_C,SEL_FLD,SEX_DEC,SRCH,"+ -
        "SUN,TIME,UTC_GST,VERSION,WATCH"
$   If P1 .eqs. "LINK" .or. P2 .eqs. "LINK" Then GoTo LINK
$   FILE_NUM = F$Integer (P1)
$   If (FILE_NUM .ge. 37 .or. FILE_NUM .lt. 0) Then FILE_NUM = 0
$COMPILE_LOOP:
$   FILE = F$Element (FILE_NUM, "," ,FILES)
$   If FILE .eqs. "," Then GoTo COMPILE_END
$   Write Sys$Output "Compiling file number ''FILE_NUM' = ''FILE' ..."
$   CC 'FILE'
$   FILE_NUM = FILE_NUM + 1
$   GoTo COMPILE_LOOP
$COMPILE_END:
$LINK:
$   Write Sys$Output "Linking ephem now...^G"
$   LINK /Exe = EPHEM 'FILES',Sys$Input/Opt
Sys$Library:VAXCRTL/Share
$EXIT:
$   Set NoOn
$   Purge *.OBJ,*.EXE
$   If VERIFY Then Set Verify
$   Exit
