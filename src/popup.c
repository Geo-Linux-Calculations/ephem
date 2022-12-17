/* put up a one-line menu consisting of the given fields and let op move
 * between them with the same methods as sel_fld().
 * return index of which he picked, or -1 if hit END.
 */

#include <stdio.h>
#include <string.h>

#include "screen.h"
#include "ephem.h"

extern void bye();

#define	FLDGAP	2	/* inter-field gap */
#define	MAXFLDS	32	/* max number of fields we can handle */

static char pup[] = "Select: ";

/* put up an array of strings on prompt line and let op pick one.
 * start with field fn.
 * N.B. we do not do much error/bounds checking.
 */
int popup (fields, fn, nfields)
char *fields[];
int fn;
int nfields;
{
    int fcols[MAXFLDS];	/* column to use for each field */
    int i;

    if (nfields > MAXFLDS)
        return (-1);

again:
    /* erase the prompt line; we are going to take it over */
    c_pos (R_PROMPT, C_PROMPT);
    c_eol();

    /* compute starting column for each field */
    fcols[0] = sizeof(pup);
    for (i = 1; i < nfields; i++)
        fcols[i] = fcols[i-1] + strlen (fields[i-1]) + FLDGAP;

    /* draw each field, with comma after all but last */
    c_pos (R_PROMPT, 1);
    (void) fputs (pup, stdout);
    for (i = 0; i < nfields; i++)
    {
        c_pos (R_PROMPT, fcols[i]);
        printf (i < nfields-1 ? "%s," : "%s", fields[i]);
    }

    /* let op choose one now; begin at fn.
     */
    while (1)
    {
        c_pos (R_PROMPT, fcols[fn]);
        switch (read_char())
        {
        case END:
            return (-1);
        case QUIT:
            f_prompt ("Exit ephem? (y) ");
            if (read_char() == 'y')
                bye();	/* never returns */
            goto again;
        case REDRAW:
            redraw_screen(2);
            goto again;
        case VERSION:
            version();
            goto again;
        case '\r':
        case ' ':
            return (fn);
        case 'h':
            if (--fn < 0)
                fn = nfields - 1;
            break;
        case 'l':
            if (++fn >= nfields)
                fn = 0;
            break;
        }
    }
}
