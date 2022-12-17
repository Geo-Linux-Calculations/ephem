/* phoon - show the phase of the moon
 **
 ** ver  date   who remarks
 ** --- ------- --- -------------------------------------------------------------
 ** ??? 12dec15 DS  transplanted into ephem as Watch|Moon feature (removed main)
 **                 added HUBERT and PUMPKIN #ifdefs
 ** 03A 01apr95 JP  Updated to use date_parse.
 ** 02A 07jun88 JP  Changed the phase calculation to use code from John Walker's
 **                   "moontool", increasing accuracy tremendously.
 **                 Got rid of SINFORCOS, since Apple has probably fixed A/UX
 **                   by now.
 ** 01I 03feb88 JP  Added 32 lines.
 ** 01H 14jan88 JP  Added 22 lines.
 ** 01G 05dec87 JP  Added random sabotage to generate Hubert.
 **		   Defeated text stuff for moons 28 or larger.
 ** 01F 13oct87 JP  Added pumpkin19 in October.  Added hubert29.
 ** 01E 14may87 JP  Added #ifdef SINFORCOS to handle broken A/UX library.
 ** 01D 02apr87 JP  Added 21 lines.
 ** 01C 26jan87 JP  Added backgrounds for 29 and 18 lines.
 ** 01B 28dec86 JP  Added -l flag, and backgrounds for 19 and 24 lines.
 ** 01A 08nov86 JP  Translated from the ratfor version of 12nov85, which itself
 **                   was translated from the Pascal version of 05apr79.
 **
 ** Copyright (C) 1986,1987,1988,1995 by Jef Poskanzer <jef@mail.acme.com>.
 ** All rights reserved.
 **
 ** Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions
 ** are met:
 ** 1. Redistributions of source code must retain the above copyright
 **    notice, this list of conditions and the following disclaimer.
 ** 2. Redistributions in binary form must reproduce the above copyright
 **    notice, this list of conditions and the following disclaimer in the
 **    documentation and/or other materials provided with the distribution.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 ** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 ** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 ** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 ** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 ** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 ** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 ** SUCH DAMAGE.
 */

#ifdef GLOBE_PHOON

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "screen.h"
#include "phoonastro.h"
int c_pos ( int r, int c);


/* Global defines and declarations. */

#define SECSPERMINUTE 60
#define SECSPERHOUR (60 * SECSPERMINUTE)
#define SECSPERDAY (24 * SECSPERHOUR)

#define PI 3.1415926535897932384626433

#define DEFAULTNUMLINES 23

#define QUARTERLITLEN 16
#define QUARTERLITLENPLUSONE 17

/* If you change the aspect ratio, the canned backgrounds won't work. */
#define ASPECTRATIO 0.5


static void
putseconds( long secs )
{
    long days, hours, minutes;

    days = secs / SECSPERDAY;
    secs = secs - days * SECSPERDAY;
    hours = secs / SECSPERHOUR;
    secs = secs - hours * SECSPERHOUR;
    minutes = secs / SECSPERMINUTE;
    secs = secs - minutes * SECSPERMINUTE;

    printf( "%ld %2ld:%02ld:%02ld", days, hours, minutes, secs );
}


int
find_nearest_phoon_size( int nlines, int *pos_dif )
{
    int phoon_sizes[] = { 18, 19, 21, 22, 23, 24, 29, 32 };
    int n_sizes = 8;
    int new_nlines = -1;
    int i;
    for ( i=0 ; i < n_sizes; i++ )
    {
        if ( nlines <= phoon_sizes[i] )   // prev one fits
        {
            if ( i == 0 )
                //new_nlines = phoon_sizes[0]; // nearest is first
                new_nlines = nlines; // don't change
            else
                new_nlines = phoon_sizes[i-1];
            break;
        }
    }
    if ( new_nlines == -1 )
        new_nlines = phoon_sizes[n_sizes-1]; // nearest is last

    if ( new_nlines < nlines )
    {
        *pos_dif = nlines - new_nlines;
    }
    else
    {
        *pos_dif = 0;
    }
#ifdef DEBUG
    c_pos( nlines-1,0);
    printf("%d", nlines );
    c_pos( nlines-0,0);
    printf("%d", new_nlines );
#endif
    return new_nlines;
}


void
putmoon_jd( double jd, int numlines, char* atfiller )
{
    static char background18[18][37] =
    {
        "             .----------.            ",
        "         .--'   o    .   `--.        ",
        "       .'@  @@@@@@ O   .   . `.      ",
        "     .'@@  @@@@@@@@   @@@@   . `.    ",
        "   .'    . @@@@@@@@  @@@@@@    . `.  ",
        "  / @@ o    @@@@@@.   @@@@    O   @\\ ",
        "  |@@@@               @@@@@@     @@| ",
        " / @@@@@   `.-.    . @@@@@@@@  .  @@\\",
        " | @@@@   --`-'  .  o  @@@@@@@      |",
        " |@ @@                 @@@@@@ @@@   |",
        " \\      @@    @   . ()  @@   @@@@@  /",
        "  |   @      @@@         @@@  @@@  | ",
        "  \\  .   @@  @\\  .      .  @@    o / ",
        "   `.   @@@@  _\\ /     .      o  .'  ",
        "     `.  @@    ()---           .'    ",
        "       `.     / |  .    o    .'      ",
        "         `--./   .       .--'        ",
        "             `----------'            "
    };
    static char background19[19][39] =
    {
        "              .----------.             ",
        "          .--'   o    .   `--.         ",
        "       .-'@  @@@@@@ O   .   . `-.      ",
        "     .' @@  @@@@@@@@   @@@@   .  `.    ",
        "    /     . @@@@@@@@  @@@@@@     . \\   ",
        "   /@@  o    @@@@@@.   @@@@    O   @\\  ",
        "  /@@@@                @@@@@@     @@@\\ ",
        " . @@@@@   `.-./    . @@@@@@@@  .  @@ .",
        " | @@@@   --`-'  .      @@@@@@@       |",
        " |@ @@        `      o  @@@@@@ @@@@   |",
        " |      @@        o      @@   @@@@@@  |",
        " ` .  @       @@     ()   @@@  @@@@   '",
        "  \\     @@   @@@@        . @@   .  o / ",
        "   \\   @@@@  @@\\  .           o     /  ",
        "    \\ . @@     _\\ /    .      .-.  /   ",
        "     `.    .    ()---        `-' .'    ",
        "       `-.    ./ |  .   o     .-'      ",
        "          `--./   .       .--'         ",
        "              `----------'             "
    };
#ifdef PUMPKIN
    static char pumpkin19[19][39] =
    {
        "              @@@@@@@@@@@@             ",
        "          @@@@@@@@@@@@@@@@@@@@         ",
        "       @@@@@@@@@@@@@@@@@@@@@@@@@@      ",
        "     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    ",
        "    @@@@        @@@@@@@@        @@@@   ",
        "   @@@@@@      @@@@@@@@@@      @@@@@@  ",
        "  @@@@@@@@    @@@@@@@@@@@@    @@@@@@@@ ",
        " @@@@@@@@@@  @@@@@@  @@@@@@  @@@@@@@@@@",
        " @@@@@@@@@@@@@@@@@    @@@@@@@@@@@@@@@@@",
        " @@@@@@@@@@@@@@@@      @@@@@@@@@@@@@@@@",
        " @@@@@@@@@@@@@@@        @@@@@@@@@@@@@@@",
        " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
        "  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ ",
        "   @@@@@                @@      @@@@@  ",
        "    @@@@@@                    @@@@@@   ",
        "     @@@@@@@@              @@@@@@@@    ",
        "       @@@@@@@@@        @@@@@@@@@      ",
        "          @@@@@@@@@@@@@@@@@@@@         ",
        "              @@@@@@@@@@@@             "
    };
#endif
    static char background21[21][43] =
    {
        "                .----------.               ",
        "           .---'   O   . .  `---.          ",
        "        .-'@ @@@@@@  .  @@@@@    `-.       ",
        "      .'@@  @@@@@@@@@  @@@@@@@   .  `.     ",
        "     /   o  @@@@@@@@@  @@@@@@@      . \\    ",
        "    /@  o   @@@@@@@@@.  @@@@@@@   O    \\   ",
        "   /@@@  .   @@@@@@o   @@@@@@@@@@     @@\\  ",
        "  /@@@@@            . @@@@@@@@@@@@@ o @@@\\ ",
        " .@@@@@ O  `.-./ .     @@@@@@@@@@@@    @@ .",
        " | @@@@   --`-'      o    @@@@@@@@ @@@@   |",
        " |@ @@@       `   o     .  @@  . @@@@@@@  |",
        " |      @@  @        .-.    @@@  @@@@@@@  |",
        " `  . @       @@@    `-'  . @@@@  @@@@  o '",
        "  \\     @@   @@@@@ .         @@  .       / ",
        "   \\   @@@@  @\\@@    /  . O   .    o  . /  ",
        "    \\o  @@     \\ \\  /       .   .      /   ",
        "     \\    .    .\\.-.___  .     .  .-. /    ",
        "      `.         `-'             `-'.'     ",
        "        `-.  o  / |    o   O  .  .-'       ",
        "           `---.    .    .  .---'          ",
        "                `----------'               "
    };
    static char background22[22][45] =
    {
        "                .------------.               ",
        "            .--'   o     . .  `--.           ",
        "         .-'    .    O   .      . `-.        ",
        "       .'@    @@@@@@@   .  @@@@@     `.      ",
        "     .'@@@  @@@@@@@@@@@   @@@@@@@  .   `.    ",
        "    /     o @@@@@@@@@@@   @@@@@@@      . \\   ",
        "   /@@  o   @@@@@@@@@@@.   @@@@@@@   O    \\  ",
        "  /@@@@   .   @@@@@@@o    @@@@@@@@@@    @@@\\ ",
        "  |@@@@@               . @@@@@@@@@@@@  @@@@| ",
        " /@@@@@  O  `.-./  .      @@@@@@@@@@@   @@  \\",
        " | @@@@    --`-'      o    . @@@@@@@ @@@@   |",
        " |@ @@@  @@  @ `   o  .-.     @@  . @@@@@@  |",
        " \\             @@@    `-'  .   @@@  @@@@@@  /",
        "  | . @  @@   @@@@@ .          @@@@  @@@ o | ",
        "  \\     @@@@  @\\@@    /  .  O   @@ .     . / ",
        "   \\  o  @@     \\ \\  /          . . o     /  ",
        "    \\      .    .\\.-.___   .  .  .  .-.  /   ",
        "     `.           `-'              `-' .'    ",
        "       `.    o   / |     o   O   .   .'      ",
        "         `-.    /     .      .    .-'        ",
        "            `--.        .     .--'           ",
        "                `------------'               "
    };
    static char background23[23][47] =
    {
        "                 .------------.                ",
        "             .--'  o     . .   `--.            ",
        "          .-'   .    O   .       . `-.         ",
        "       .-'@   @@@@@@@   .  @@@@@      `-.      ",
        "      /@@@  @@@@@@@@@@@   @@@@@@@   .    \\     ",
        "    ./    o @@@@@@@@@@@   @@@@@@@       . \\.   ",
        "   /@@  o   @@@@@@@@@@@.   @@@@@@@   O      \\  ",
        "  /@@@@   .   @@@@@@@o    @@@@@@@@@@     @@@ \\ ",
        "  |@@@@@               . @@@@@@@@@@@@@ o @@@@| ",
        " /@@@@@  O  `.-./  .      @@@@@@@@@@@@    @@  \\",
        " | @@@@    --`-'       o     @@@@@@@@ @@@@    |",
        " |@ @@@        `    o      .  @@   . @@@@@@@  |",
        " |       @@  @         .-.     @@@   @@@@@@@  |",
        " \\  . @        @@@     `-'   . @@@@   @@@@  o /",
        "  |      @@   @@@@@ .           @@   .       | ",
        "  \\     @@@@  @\\@@    /  .  O    .     o   . / ",
        "   \\  o  @@     \\ \\  /         .    .       /  ",
        "    `\\     .    .\\.-.___   .      .   .-. /'   ",
        "      \\           `-'                `-' /     ",
        "       `-.   o   / |     o    O   .   .-'      ",
        "          `-.   /     .       .    .-'         ",
        "             `--.       .      .--'            ",
        "                 `------------'                "
    };

    static char background24[24][49] =
    {
        "                  .------------.                 ",
        "             .---' o     .  .   `---.            ",
        "          .-'   .    O    .       .  `-.         ",
        "        .'@   @@@@@@@   .   @@@@@       `.       ",
        "      .'@@  @@@@@@@@@@@    @@@@@@@   .    `.     ",
        "     /    o @@@@@@@@@@@    @@@@@@@       .  \\    ",
        "    /@  o   @@@@@@@@@@@.    @@@@@@@   O      \\   ",
        "   /@@@   .   @@@@@@@o     @@@@@@@@@@     @@@ \\  ",
        "  /@@@@@               .  @@@@@@@@@@@@@ o @@@@ \\ ",
        "  |@@@@  O  `.-./  .       @@@@@@@@@@@@    @@  | ",
        " / @@@@    --`-'       o      @@@@@@@@ @@@@     \\",
        " |@ @@@     @  `           .   @@     @@@@@@@   |",
        " |      @           o          @      @@@@@@@   |",
        " \\       @@            .-.      @@@    @@@@  o  /",
        "  | . @        @@@     `-'    . @@@@           | ",
        "  \\      @@   @@@@@ .            @@   .        / ",
        "   \\    @@@@  @\\@@    /  .   O    .     o   . /  ",
        "    \\ o  @@     \\ \\  /          .    .       /   ",
        "     \\     .    .\\.-.___    .      .   .-.  /    ",
        "      `.          `-'                 `-' .'     ",
        "        `.   o   / |      o    O   .    .'       ",
        "          `-.   /      .       .     .-'         ",
        "             `---.       .      .---'            ",
        "                  `------------'                 "
    };
    static char background29[29][59] =
    {
        "                      .--------------.                     ",
        "                 .---'  o        .    `---.                ",
        "              .-'    .    O  .         .   `-.             ",
        "           .-'     @@@@@@       .             `-.          ",
        "         .'@@   @@@@@@@@@@@       @@@@@@@   .    `.        ",
        "       .'@@@  @@@@@@@@@@@@@@     @@@@@@@@@         `.      ",
        "      /@@@  o @@@@@@@@@@@@@@     @@@@@@@@@     O     \\     ",
        "     /        @@@@@@@@@@@@@@  @   @@@@@@@@@ @@     .  \\    ",
        "    /@  o      @@@@@@@@@@@   .  @@  @@@@@@@@@@@     @@ \\   ",
        "   /@@@      .   @@@@@@ o       @  @@@@@@@@@@@@@ o @@@@ \\  ",
        "  /@@@@@                  @ .      @@@@@@@@@@@@@@  @@@@@ \\ ",
        "  |@@@@@    O    `.-./  .        .  @@@@@@@@@@@@@   @@@  | ",
        " / @@@@@        --`-'       o        @@@@@@@@@@@ @@@    . \\",
        " |@ @@@@ .  @  @    `    @            @@      . @@@@@@    |",
        " |   @@                         o    @@   .     @@@@@@    |",
        " |  .     @   @ @       o              @@   o   @@@@@@.   |",
        " \\     @    @       @       .-.       @@@@       @@@      /",
        "  |  @    @  @              `-'     . @@@@     .    .    | ",
        "  \\ .  o       @  @@@@  .              @@  .           . / ",
        "   \\      @@@    @@@@@@       .                   o     /  ",
        "    \\    @@@@@   @@\\@@    /        O          .        /   ",
        "     \\ o  @@@       \\ \\  /  __        .   .     .--.  /    ",
        "      \\      .     . \\.-.---                   `--'  /     ",
        "       `.             `-'      .                   .'      ",
        "         `.    o     / | `           O     .     .'        ",
        "           `-.      /  |        o             .-'          ",
        "              `-.          .         .     .-'             ",
        "                 `---.        .       .---'                ",
        "                      `--------------'                     "
    };
#ifdef HUBERT
    static char hubert29[29][59] =
    {
        "                      .--------------.                     ",
        "                 .---'  o        .    `---.                ",
        "              .-'    .    O  .         .   `-.             ",
        "           .-'     @@@@@@       .             `-.          ",
        "         .'@@   @@@@@@@@@@@       @@@@@@@   .    `.        ",
        "       .'@@@  @@@@@ ___====-_  _-====___ @         `.      ",
        "      /@@@  o _--~~~#####//      \\\\#####~~~--_ O     \\     ",
        "     /     _-~##########// (    ) \\\\##########~-_  .  \\    ",
        "    /@  o -############//  :\\^^/:  \\\\############-  @@ \\   ",
        "   /@@@ _~############//   (@::@)   \\\\############~_ @@ \\  ",
        "  /@@@ ~#############((     \\\\//     ))#############~ @@ \\ ",
        "  |@@ -###############\\\\    (oo)    //###############- @ | ",
        " / @ -#################\\\\  / \"\" \\  //#################- . \\",
        " |@ -###################\\\\/      \\//###################-  |",
        " | _#/:##########/\\######(   /\\   )######/\\##########:\\#_ |",
        " | :/ :#/\\#/\\#/\\/  \\#/\\##\\  :  :  /##/\\#/  \\/\\#/\\#/\\#: \\: |",
        " \\ \"  :/  V  V  \"   V  \\#\\: :  : :/#/  V   \"  V  V  \\:  \" /",
        "  | @ \"   \"  \"      \"   / : :  : : \\   \"      \"  \"   \"   | ",
        "  \\ .  o       @  @@@@ (  : :  : :  )  @@  .           . / ",
        "   \\      @@@    @@@@ __\\ : :  : : /__            o     /  ",
        "    \\    @@@@@   @@\\@(vvv(VVV)(VVV)vvv)       .        /   ",
        "     \\ o  @@@       \\ \\  /  __        .   .     .--.  /    ",
        "      \\      .     . \\.-.---                   `--'  /     ",
        "       `.             `-'      .                   .'      ",
        "         `.    o     / | `           O     .     .'        ",
        "           `-.      /  |        o             .-'          ",
        "              `-.          .         .     .-'             ",
        "                 `---.        .       .---'                ",
        "                      `--------------'                     "
    };
#endif
    static char background32[32][65] =
    {
        "                         .--------------.                        ",
        "                   .----'  o        .    `----.                  ",
        "                .-'     .    O  .          .   `-.               ",
        "             .-'      @@@@@@       .              `-.            ",
        "           .'@     @@@@@@@@@@@       @@@@@@@@    .   `.          ",
        "         .'@@    @@@@@@@@@@@@@@     @@@@@@@@@@         `.        ",
        "       .'@@@ o   @@@@@@@@@@@@@@     @@@@@@@@@@      o    `.      ",
        "      /@@@       @@@@@@@@@@@@@@  @   @@@@@@@@@@  @@     .  \\     ",
        "     /            @@@@@@@@@@@   .  @@   @@@@@@@@@@@@     @@ \\    ",
        "    /@  o     .     @@@@@@ o       @   @@@@@@@@@@@@@@ o @@@@ \\   ",
        "   /@@@                        .       @@@@@@@@@@@@@@@  @@@@@ \\  ",
        "  /@@@@@                     @      .   @@@@@@@@@@@@@@   @@@   \\ ",
        "  |@@@@@     o      `.-./  .             @@@@@@@@@@@@ @@@    . | ",
        " / @@@@@           __`-'       o          @@       . @@@@@@     \\",
        " |@ @@@@ .        @    `    @            @@    .     @@@@@@     |",
        " |   @@       @                    o       @@@   o   @@@@@@.    |",
        " |          @                             @@@@@       @@@       |",
        " |  . .  @      @  @       o              @@@@@     .    .      |",
        " \\            @                .-.      .  @@@  .           .   /",
        "  |    @   @   @      @        `-'                     .       / ",
        "  \\   .      @   @                   .            o            / ",
        "   \\     o          @@@@   .                .                 /  ",
        "    \\       @@@    @@@@@@        .                    o      /   ",
        "     \\     @@@@@   @@\\@@    /         o           .         /    ",
        "      \\  o  @@@       \\ \\  /  ___         .   .     .--.   /     ",
        "       `.      .       \\.-.---                     `--'  .'      ",
        "         `.             `-'       .                    .'        ",
        "           `.    o     / |              O      .     .'          ",
        "             `-.      /  |         o              .-'            ",
        "                `-.           .         .      .-'               ",
        "                   `----.        .       .----'                  ",
        "                         `--------------'                        "
    };

    static char qlits[4][16] =
    {
        "New Moon +     ",
        "First Quarter +",
        "Full Moon +    ",
        "Last Quarter + ",
    };
    static char nqlits[4][16] =
    {
        "New Moon -     ",
        "First Quarter -",
        "Full Moon -    ",
        "Last Quarter - ",
    };

    double pctphase, angphase, cphase, aom, cdist, cangdia, csund, csuang;
    double phases[2], which[2];
    long clocknow;
    int atflrlen, atflridx, numcols, lin, col, midlin;
    double mcap, yrad, xrad, y, xright, xleft;
    int colright, colleft;
    char c;

    /* Find the length of the atfiller string. */
    atflrlen = strlen( atfiller );

    /* Figure out the phase. */

    pctphase = phase( jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang );
    angphase = pctphase * 2.0 * PI;
    mcap = -cos( angphase );

    /* Get now for use as a random number. */
    (void) time( &clocknow );

    /* possibly adjust numlines to nearest smaller phoon size */
    int line_difference = 0;
    numlines = find_nearest_phoon_size( numlines, &line_difference);
    int top_marg = line_difference / 2;

    /* Figure out how big the moon is. */
    yrad = numlines / 2.0;
    xrad = yrad / ASPECTRATIO;
    numcols = xrad * 2;

    /* length of info text to right */
    int text_len = 20;

    /* left margin to center moon wrt NC */
    int leftmarg = (NC - (numcols + text_len))/2;
    if (leftmarg<0) leftmarg = 0;

    /* Figure out some other random stuff. */
    midlin = numlines / 2;
    phasehunt2( jd, phases, which );

    /* Now output the moon, a slice at a time. */
    atflridx = 0;
    for ( lin = 0; lin < numlines; lin = lin + 1 )
    {
        c_pos( 2    // ephem watch header (2 lines)
               + top_marg      // to vertically center phoon
               + lin,  // current phoon slice line
               leftmarg ); // to horizontally center phoon + info text

        /* Compute the edges of this slice. */
        y = lin + 0.5 - yrad;
        xright = xrad * sqrt( 1.0 - ( y * y ) / ( yrad * yrad ) );
        xleft = -xright;
        if ( angphase >= 0.0 && angphase < PI )
            xleft = mcap * xleft;
        else
            xright = mcap * xright;
        colleft = (int) (xrad + 0.5) + (int) (xleft + 0.5);
        colright = (int) (xrad + 0.5) + (int) (xright + 0.5);

        /* Now output the slice. */
        for ( col = 0; col < colleft; ++col )
            putchar( ' ' );
        COLOR_CODE(COLOR_PHOON);
        for ( ; col <= colright; ++col )
        {
            switch ( numlines )
            {
            case 18:
                c = background18[lin][col];
                break;
            case 19:
            {
#ifdef PUMPKIN
                time_t t;
                t = julian_to_unix( jd );
                tmP = localtime( &t );
                if ( tmP->tm_mon == 9 &&
                        clocknow % ( 33 - tmP->tm_mday ) == 1 )
                    c = pumpkin19[lin][col];
                else
#endif
                    c = background19[lin][col];
            }
            break;
            case 21:
                c = background21[lin][col];
                break;
            case 22:
                c = background22[lin][col];
                break;
            case 23:
                c = background23[lin][col];
                break;
            case 24:
                c = background24[lin][col];
                break;
            case 29:
#ifdef HUBERT
                if ( clocknow % 23 == 3 && cphase > 0.8 && numlines >= 29 )
                    c = hubert29[lin][col];
                else
#endif
                    c = background29[lin][col];
                break;
            case 32:
                c = background32[lin][col];
                break;
            default:
                c = '@';
            }
            if ( c != '@' )
                putchar( c );
            else
            {
                putchar( atfiller[atflridx] );
                atflridx = ( atflridx + 1 ) % atflrlen;
            }
        }
        COLOR_OFF;

        for ( ; col <= numcols; ++col )
            putchar( ' ' );



        /* Output the end-of-line information, if any. */
        if ( lin == midlin - 2 )
        {
            fputs( "\t ", stdout );
            fputs( qlits[(int) (which[0] * 4.0 + 0.001)], stdout );
        }
        else if ( lin == midlin - 1)
        {
            fputs( "\t ", stdout );
            putseconds( (int) ( ( jd - phases[0] ) * SECSPERDAY ) );
            fputs( "    ", stdout );
        }
        else if ( lin == midlin )
        {
            fputs( "\t ", stdout );
            fputs( nqlits[(int) (which[1] * 4.0 + 0.001)], stdout );
        }
        else if ( lin == midlin + 1 )
        {
            fputs( "\t ", stdout );
            putseconds( (int) ( ( phases[1] - jd ) * SECSPERDAY ) );
            fputs( "    ", stdout );
        }
    }

}

void
putmoon( time_t t, int numlines, char* atfiller )
{
    double jd = unix_to_julian( t );
    putmoon_jd( jd, numlines, atfiller );
}
#endif /* GLOBE_PHOON */
