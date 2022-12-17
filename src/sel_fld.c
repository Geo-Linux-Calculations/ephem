#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "screen.h"
#include "ephem.h"

/* define BANG if and only if your system supports the system() function.
 */
#define	BANG

#ifndef NCURSES_LARGE
/* if we're NOT using the NCURSES_LARGE option,
 * we can use static initializers ... */
F_t fields[] =
{
#define _RCFPAK(r,c,f) rcfpack(r,c,f)
#else
/* we ARE using NCURSES_LARGE option, so define an _init_fields() function
 * to initialize fields[] because NC and NR are no longer constant ...
 * and be sure to run _init_fields() each time the terminal resizes.
 */
#define MAXFIELDS 1000
int NFIELDS = 0;
F_t fields[MAXFIELDS];
#define _RCFPAK(r,c,f) (fields[NFIELDS++]=rcfpack(r,c,f))
void _init_fields()
{
    NFIELDS = 0;
#endif
    _RCFPAK (R_ALTM,	C_ALTM,		F_MMNU|F_CHG),
    _RCFPAK (R_CALLISTO,C_JMX,		F_MNUJ|F_PLT),
    _RCFPAK (R_CALLISTO,C_JMY,		F_MNUJ|F_PLT),
    _RCFPAK (R_CALLISTO,C_JMZ,		F_MNUJ|F_PLT),
    _RCFPAK (R_DAWN,	C_DAWN,		F_MMNU|F_CHG),
    _RCFPAK (R_DAWN,	C_DAWNV,	F_MMNU|F_PLT),
    _RCFPAK (R_DUSK,	C_DUSK,		F_MMNU|F_CHG),
    _RCFPAK (R_DUSK,	C_DUSKV,	F_MMNU|F_PLT),
    _RCFPAK (R_EPOCH,	C_EPOCHV,	F_MMNU|F_CHG),
    _RCFPAK (R_EUROPA,	C_JMX,		F_MNUJ|F_PLT),
    _RCFPAK (R_EUROPA,	C_JMY,		F_MNUJ|F_PLT),
    _RCFPAK (R_EUROPA,	C_JMZ,		F_MNUJ|F_PLT),
    _RCFPAK (R_GANYMEDE,C_JMX,		F_MNUJ|F_PLT),
    _RCFPAK (R_GANYMEDE,C_JMY,		F_MNUJ|F_PLT),
    _RCFPAK (R_GANYMEDE,C_JMZ,		F_MNUJ|F_PLT),
    _RCFPAK (R_HEIGHT,	C_HEIGHTV,	F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_IO,	C_JMX,		F_MNUJ|F_PLT),
    _RCFPAK (R_IO,	C_JMY,		F_MNUJ|F_PLT),
    _RCFPAK (R_IO,	C_JMZ,		F_MNUJ|F_PLT),
    _RCFPAK (R_JCML,	C_JCMLSI,	F_MNUJ|F_PLT),
    _RCFPAK (R_JCML,	C_JCMLSII,	F_MNUJ|F_PLT),
    _RCFPAK (R_JD,	C_JDV,		F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_JUPITER,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_JUPITER,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_JUPITER,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_JUPITER,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_JUPITER,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_JUPITER,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_JUPITER,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_JUPITER,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_JUPITER,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_JUPITER,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_JUPITER,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_JUPITER,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_JUPITER,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_JUPITER,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_JUPITER,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_JUPITER,	C_XTRA,		F_MNU1|F_CHG),
    _RCFPAK (R_JUPITER,	C_XTRA,		F_MNU2|F_CHG),
    _RCFPAK (R_JUPITER,	C_XTRA,		F_MNU3|F_CHG),
    _RCFPAK (R_LAT,	C_LATV,		F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_LD,	C_LD,		F_MMNU|F_PLT|F_CHG),
    _RCFPAK (R_LISTING,	C_LISTING,	F_MMNU|F_CHG),
    _RCFPAK (R_LON,	C_LON,		F_MMNU|F_CHG),
    _RCFPAK (R_LON,	C_LONV,		F_MMNU|F_PLT),
    _RCFPAK (R_LONG,	C_LONGV,	F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_LST,	C_LSTV,		F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_LT,	C_LT,		F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_MARS,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_MARS,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_MARS,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_MARS,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_MARS,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_MARS,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_MARS,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_MARS,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_MARS,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_MARS,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_MARS,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_MARS,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_MARS,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_MARS,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_MARS,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_MARS,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_MERCURY,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_MERCURY,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_MERCURY,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_MERCURY,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_MERCURY,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_MERCURY,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_MERCURY,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_MERCURY,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_MERCURY,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_MERCURY,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_MERCURY,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_MERCURY,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_MERCURY,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_MERCURY,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_MERCURY,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_MOON,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_MOON,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_MOON,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_MOON,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_MOON,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_MOON,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_MOON,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_MOON,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_MOON,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_MOON,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_MOON,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_MOON,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_MOON,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_MOON,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_MOON,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_NEPTUNE,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_NEPTUNE,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_NEPTUNE,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_NEPTUNE,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_NEPTUNE,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_NEPTUNE,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_NEPTUNE,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_NSTEP,	C_NSTEPV,	F_MMNU|F_CHG),
    _RCFPAK (R_OBJX,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_OBJX,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_OBJX,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_OBJX,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_OBJX,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_OBJX,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_OBJX,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJX,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJX,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJX,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJX,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJX,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJX,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJX,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_OBJX,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJX,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_OBJY,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_OBJY,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_OBJY,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_OBJY,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_OBJY,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_OBJY,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJY,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJY,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJY,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJY,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_OBJY,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJY,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_OBJY,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_OBJY,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_OBJY,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_PAUSE,	C_PAUSEV,	F_MMNU|F_CHG),
    _RCFPAK (R_PLOT,	C_PLOT,		F_MMNU|F_CHG),
    _RCFPAK (R_PLUTO,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_PLUTO,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_PLUTO,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_PLUTO,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_PLUTO,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_PLUTO,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_PLUTO,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_PLUTO,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_PLUTO,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_PLUTO,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_PLUTO,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_PLUTO,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_PLUTO,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_PLUTO,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_PLUTO,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_PLUTO,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_PRES,	C_PRESV,	F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_SATURN,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_SATURN,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_SATURN,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_SATURN,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_SATURN,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_SATURN,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_SATURN,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_SATURN,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_SATURN,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_SATURN,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_SATURN,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_SATURN,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_SATURN,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_SATURN,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_SATURN,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_SATURN,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_SRCH,	C_SRCH,		F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_STPSZ,	C_STPSZV,	F_MMNU|F_CHG),
    _RCFPAK (R_SUN,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_SUN,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_SUN,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_SUN,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_SUN,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_SUN,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_SUN,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_SUN,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_SUN,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_SUN,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_SUN,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_SUN,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_SUN,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_SUN,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_SUN,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_SUN,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_TEMP,	C_TEMPV,	F_MMNU|F_CHG|F_PLT),
    _RCFPAK (R_TZN,	C_TZN,		F_MMNU|F_CHG),
    _RCFPAK (R_TZONE,	C_TZONEV,	F_MMNU|F_CHG),
    _RCFPAK (R_UD,	C_UD,		F_MMNU|F_PLT|F_CHG),
    _RCFPAK (R_URANUS,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_URANUS,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_URANUS,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_URANUS,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_URANUS,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_URANUS,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_URANUS,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_URANUS,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_URANUS,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_URANUS,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_URANUS,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_URANUS,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_URANUS,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_URANUS,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_URANUS,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_URANUS,	C_VENUS,	F_MNU3|F_PLT),
    _RCFPAK (R_UT,	C_UTV,		F_MMNU|F_PLT|F_CHG),
    _RCFPAK (R_VENUS,	C_ALT,		F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_AZ,		F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_CONSTEL,	F_MNU1|F_CHG),
    _RCFPAK (R_VENUS,	C_CONSTEL,	F_MNU2|F_CHG),
    _RCFPAK (R_VENUS,	C_CONSTEL,	F_MNU3|F_CHG),
    _RCFPAK (R_VENUS,	C_DEC,		F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_EDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_ELONG,	F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_HLAT,		F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_HLONG,	F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_JUPITER,	F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_MAG,		F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_MARS,		F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_MERCURY,	F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_MOON,		F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_NEPTUNE,	F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_OBJ,		F_MNU1|F_CHG),
    _RCFPAK (R_VENUS,	C_OBJ,		F_MNU2|F_CHG),
    _RCFPAK (R_VENUS,	C_OBJ,		F_MNU3|F_CHG),
    _RCFPAK (R_VENUS,	C_OBJX,		F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_OBJY,		F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_PHASE,	F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_PLUTO,	F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_RA,		F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_RISEAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_VENUS,	C_RISETM,	F_MNU2|F_PLT),
    _RCFPAK (R_VENUS,	C_SATURN,	F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_SDIST,	F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_SETAZ,	F_MNU2|F_PLT),
    _RCFPAK (R_VENUS,	C_SETTM,	F_MNU2|F_PLT),
    _RCFPAK (R_VENUS,	C_SIZE,		F_MNU1|F_PLT),
    _RCFPAK (R_VENUS,	C_SUN,		F_MNU3|F_PLT),
    _RCFPAK (R_VENUS,	C_TRANSALT,	F_MNU2|F_PLT),
    _RCFPAK (R_VENUS,	C_TRANSTM,	F_MNU2|F_PLT),
    _RCFPAK (R_VENUS,	C_TUP,		F_MNU2|F_PLT),
    _RCFPAK (R_VENUS,	C_URANUS,	F_MNU3|F_PLT),
    _RCFPAK (R_WATCH,	C_WATCH,	F_MMNU|F_CHG)
#ifndef NCURSES_LARGE
};
#define	NFIELDS (sizeof(fields)/sizeof(fields[0]))
#else
    ;
}
#endif

static void move_cur (int dirchar, int flag, int *rp, int *cp);
static int nearestfld (int r, int c, int flag);

/* let op select a field by moving around and hitting RETURN or SPACE, or
 *   until see END. also allow moving directly to frequently used fields
 *   with some hot-keys.
 * only allow fields with the given flag mask.
 * return the rcfpack()'d field, or 0 if typed END.
 * N.B. we might also exit() entirely by calling bye() if op types QUIT.
 */
int sel_fld (f, flag, prmpt, help)
int f;		/* inital row, col */
int flag;
char *prmpt, *help;
{
    extern void bye();
    int r = unpackr(f), c = unpackc(f);
    char *lastp;
    int ch;

    lastp = 0;
    while (1)
    {
        if (lastp != prmpt)
        {
            lastp = prmpt;
            f_prompt (lastp);
        }
        c_pos (r, c);
        switch (ch = read_char())
        {
        case REDRAW:
            redraw_screen(2);	/* redraw all from scratch */
            lastp = 0;
            break;
        case VERSION:
            version();
            lastp = 0;
            break;
        case HELP:
            f_msg (help);
            lastp = 0;
            break;
        case QUIT:
            f_prompt ("Exit ephem? (y) ");
            if (read_char() == 'y')
                bye();	/* never returns */
            lastp = 0;
            break;
#ifdef BANG
        case '!':
        {
            char buf[NC];
            f_prompt ("!");
            if (read_line (buf, sizeof(buf)-2) > 0)
            {
                c_erase();
                byetty();
#ifdef CURSES
                endwin();
#endif
                int sys_rv = system (buf);
                printf ("\n exit code = %d [Hit any key to resume ephem...]",sys_rv);
                (void) read_char();
#ifdef CURSES
                refresh();
                keypad(stdscr,TRUE);
#endif
                redraw_screen(2);	/* redraw all from scratch */
            }
            lastp = 0;
        }
        break;
#endif
        case END:
            return (0);
        case '\r':
        case ' ':
            return (rcfpack (r, c, 0));
        default:
            move_cur (ch, flag, &r, &c);
            break;
        }
    }
}

/* move cursor to next field in given direction: hjkl, or directly to a
 * field, and set *rp and *cp.
 * limit eligible fields to those with given flag mask.
 */
static
void move_cur (dirchar, flag, rp, cp)
int dirchar;
int flag;
int *rp, *cp;
{
    int curr = *rp, curc = *cp;
    F_t f, newf, *fp;
    int d, newd;

wrapped:
    newf = 0;
    newd = 1000;

    switch (dirchar)
    {
    case 'h': /* left */
        /* go to next field to the left, or wrap.  */
        for (fp = fields+NFIELDS; --fp >= fields; )
        {
            f = *fp;
            if (tstpackf(f,flag) && unpackr(f) == curr)
            {
                d = curc - unpackc(f);
                if (d > 0 && d < newd)
                {
                    newf = f;
                    newd = d;
                }
            }
        }
        if (!newf)
        {
            curc = NC;
            goto wrapped;
        }
        break;

    case 'j': /* down */
        /* go to closest field on next row down with anything on it,
         * or wrap.
         */
        for (fp = fields+NFIELDS; --fp >= fields; )
        {
            f = *fp;
            if (tstpackf(f,flag))
            {
                d = unpackr(f) - curr;
                if (d > 0 && d < newd)
                {
                    newf = f;
                    newd = d;
                }
            }
        }
        if (newf)
        {
            /* now find the field closest to current col on that row */
            newf = nearestfld (unpackr(newf), curc, flag);
        }
        else
        {
            curr = 0;
            goto wrapped;
        }
        break;

    case 'k': /* up */
        /* go to closest field on next row up with anything on it,
         * or wrap.
         */
        for (fp = fields+NFIELDS; --fp >= fields; )
        {
            f = *fp;
            if (tstpackf(f,flag))
            {
                d = curr - unpackr(f);
                if (d > 0 && d < newd)
                {
                    newf = f;
                    newd = d;
                }
            }
        }
        if (newf)
        {
            /* now find the field closest to current col on that row */
            newf = nearestfld (unpackr(newf), curc, flag);
        }
        else
        {
            curr = NR+1;
            goto wrapped;
        }
        break;

    case 'l': /* right */
        /* go to next field to the right, or wrap.  */
        for (fp = fields+NFIELDS; --fp >= fields; )
        {
            f = *fp;
            if (tstpackf(f,flag) && unpackr(f) == curr)
            {
                d = unpackc(f) - curc;
                if (d > 0 && d < newd)
                {
                    newf = f;
                    newd = d;
                }
            }
        }
        if (!newf)
        {
            curc = 0;
            goto wrapped;
        }
        break;

    /* handy shorthands directly to a given spot.
     * calling nearestfld() automatically allows for which menu
     *   is up now and what is pickable. you can use rcfpack()
     *   directly for top half fields that are always up.
     * N.B. using nearestfld() can be too aggressive. it will try
     *   other fields entirely if one you intend is not eligible.
     */
    case 'S':
        newf = nearestfld (R_SUN, C_OBJ, flag);
        break;
    case 'M':
        newf = nearestfld (R_MOON, C_OBJ, flag);
        break;
    case 'e':
        newf = nearestfld (R_MERCURY, C_OBJ, flag);
        break;
    case 'v':
        newf = nearestfld (R_VENUS, C_OBJ, flag);
        break;
    case 'm':
        newf = nearestfld (R_MARS, C_OBJ, flag);
        break;
    case 'J':
        newf = nearestfld (R_JUPITER, C_OBJ, flag);
        break;
    case 's':
        newf = nearestfld (R_SATURN, C_OBJ, flag);
        break;
    case 'u':
        newf = nearestfld (R_URANUS, C_OBJ, flag);
        break;
    case 'n':
        newf = nearestfld (R_NEPTUNE, C_OBJ, flag);
        break;
    case 'p':
        newf = nearestfld (R_PLUTO, C_OBJ, flag);
        break;
    case 'x':
        newf = nearestfld (R_OBJX, C_OBJ, flag);
        break;
    case 'y':
        newf = nearestfld (R_OBJY, C_OBJ, flag);
        break;
    case 'c':
        newf = nearestfld (R_ALTM, C_ALTM, flag);
        break;
    case 'd':
        newf = nearestfld (R_UD, C_UD, flag);
        break;
    case 'o':
        newf = nearestfld (R_EPOCH, C_EPOCHV, flag);
        break;
    case 'z':
        newf = nearestfld (R_STPSZ, C_STPSZV, flag);
        break;
    case 'w':
        newf = nearestfld (R_WATCH, C_WATCH, flag);
        break;
    case 'L':
        newf = nearestfld (R_LISTING, C_LISTING, flag);
        break;
    }

    if (newf)
    {
        *rp = unpackr(newf);
        *cp = unpackc(newf);
    }
}

/* return the nearest field with given flag mask, either way, on this row,
 * else 0 if none.
 */
static int
nearestfld (r, c, flag)
int r, c, flag;
{
    F_t nf, f, *fp;
    int d, d0;

    nf = 0;
    d0 = 1000;

    for (fp = fields+NFIELDS; --fp >= fields; )
    {
        f = *fp;
        if (tstpackf(f,flag) && unpackr(f) == r)
        {
            d = abs(c - unpackc(f));
            if (d < d0)
            {
                nf = f;
                d0 = d;
            }
        }
    }
    return (nf);
}

#ifdef ANSI_COLORS
APP_COLOR *App_Colors = NULL;
int Colors_Enabled = 1;

void init_app_colors()
{
    App_Colors = (APP_COLOR *)malloc(sizeof(APP_COLOR) * N_COLORS);
#define _DEF_COLOR(d,n,x,y,z) \
	App_Colors[d].name = (char *)n; \
	App_Colors[d].c1= x; \
	App_Colors[d].c2= y; \
	App_Colors[d].c3= z;
    _DEF_COLOR( COLOR_VERSION,	"version", 1, 40, 93);
    _DEF_COLOR( COLOR_WATCH,	"watch", 0, 49, 93)
    _DEF_COLOR( COLOR_BORDERS,	"borders",30,46,0);
    _DEF_COLOR( COLOR_MM_LABELS,	"mm_labels",1,40,32);
    _DEF_COLOR( COLOR_SRCH_PRSTATE,	"srch_prstate",33,44,0);
    _DEF_COLOR( COLOR_PLOT_PRSTATE,	"plot_prstate",33,44,0);
    _DEF_COLOR( COLOR_LISTING_PRSTATE,"listing_prstate",33,44,0);
    _DEF_COLOR( COLOR_NOW,		"now",7,0,0);
    _DEF_COLOR( COLOR_TWILIGHT,	"twilight",0,49,37);
    _DEF_COLOR( COLOR_ALT_LABELS,	"alt_labels",36,40,0);
    _DEF_COLOR( COLOR_PHOON,	"phoon",7,40,90);
    _DEF_COLOR( COLOR_GLOBE,	"globe",32,1,0);
    _DEF_COLOR( COLOR_ALT_MENU,	"alt_menu",30,42,0);
    _DEF_COLOR( COLOR_SUN,		"sun", 7,49,93);
    _DEF_COLOR( COLOR_MOON,		"moon", 7,40,90);
    _DEF_COLOR( COLOR_MERCURY,	"mercury", 7,101,97);
    _DEF_COLOR( COLOR_VENUS,	"venus", 30,46,0);
    _DEF_COLOR( COLOR_MARS,		"mars",	5,101,93);
    _DEF_COLOR( COLOR_JUPITER,	"jupiter", 1,41,39);
    _DEF_COLOR( COLOR_SATURN,	"saturn", 44,22,0);
    _DEF_COLOR( COLOR_URANUS,	"uranus", 36,44,0);
    _DEF_COLOR( COLOR_NEPTUNE,	"neptune", 31,46,0);
    _DEF_COLOR( COLOR_PLUTO,	"pluto", 37,40,0);
    _DEF_COLOR( COLOR_OBJX,		"objx",	30,42,0);
    _DEF_COLOR( COLOR_OBJY,		"objy",	30,42,0);
    _DEF_COLOR( COLOR_EARTH,	"earth",32,0,0);
}

void app_color( app_color_code )
int app_color_code; /* a COLOR_* index into App_Colors[] */
{
    if ( Colors_Enabled )
    {
        if ( app_color_code >= 0
                && app_color_code < N_COLORS )
        {
            if ( App_Colors[app_color_code].c3 != 0 )
                fprintf(stdout,"\033[%d;%d;%dm",
                        App_Colors[app_color_code].c1,
                        App_Colors[app_color_code].c2,
                        App_Colors[app_color_code].c3 );
            else if ( App_Colors[app_color_code].c2 != 0 )
                fprintf(stdout,"\033[%d;%dm",
                        App_Colors[app_color_code].c1,
                        App_Colors[app_color_code].c2 );
            else
                fprintf(stdout,"\033[%dm",
                        App_Colors[app_color_code].c1 );
        }
    }
}


/* set application ANSI color code from config string
 * parse single set commands like:
 *	COLOR=entity,c1,c2
 * or
 *	COLOR=entity,c1,c2,c3
 *
 * for example,
 *
 *	COLOR=earth,32,44,1
 *	COLOR=version,33,1
 * etc.
 *
 * returns:
 *	-1 if not parsed ( >= 0 if parsed ok )
 *
 * if parsed ok, returns:
 *	APP_COLOR_CMD if a color-related command (that was not an application
 *	color definition) was parsed.
 *       ENABLE_COLORS=0
 *       ENABLE_COLORS=1
 * etc.
 *
 * otherwise it parsed an app color definition command;
 *	sets the corresponding App_Colors[] entry if parsed
 *	and returns the index (color code) if successfully parsed,
 */
int set_app_color( s )
char *s;
{
    /* s to lower case ==> buf[] */
    char buf[128];
    char *c = s;
    char *o = &buf[0];
    int l = 0;
    for(; l<128 && *c != '\0'; c++,l++)
    {
        *o = isupper(*c) ? tolower(*c) : *c;
        o++;
    }
    *o = '\0';

    /* parse color enable/disable command */
    if ( sscanf( buf,"enable_colors=%d", &Colors_Enabled )==1 )
        return APP_COLOR_CMD;


    /* parse define app color command */
    char name[32];
    int c1=0,c2=0,c3=0;
    int scanned_ok = 0;
    if ( sscanf( buf,"color=%[^,],%d,%d,%d", name, &c1,&c2,&c3) == 4 )
    {
        scanned_ok = 1;
    }
    else if ( sscanf( buf,"color=%[^,],%d,%d", name, &c1,&c2) == 3 )
    {
        scanned_ok = 1;
    }
    if ( scanned_ok != 0 )   /* parsed? */
    {

        /* known entity? */
        int i;
        for(i=0; i<N_COLORS; i++ )
        {
            if (strcmp(App_Colors[i].name, name) == 0)
            {
                App_Colors[i].c1 = c1;
                App_Colors[i].c2 = c2;
                App_Colors[i].c3 = c3;
                /* parsed, recognized entity, set color: */
                return i;
            }
        }
    }
    return -1;  /* didn't parse / recognize that entity */
}
#else
int set_app_color( s )
char *s;
{
    /* NOP if ANSI_COLORS isn't defined */
    return(0);
}
#endif /* ANSI_COLORS */
