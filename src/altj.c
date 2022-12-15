/* management and computional support for jupiter's detail menu.
 */

#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "circum.h"
#include "screen.h"
#include "ephem.h"

static void
jupinfo (double d, double *ix, double *ex, double *gx, double *cx, 
         double *iy, double *ey, double *gy, double *cy, double *iz, 
         double *ez, double *gz, double *cz, double *sIcml, double *sIIcml);

void altj_labels()
{
	static char grs[] = "(GRS is at approximately 30 degs in System II)";

	f_string (R_ALTM, C_ALTMV, "  Jupiter Aux");

	f_string (R_JCML, 6, "Central Meridian Longitude (degs):");
	f_string (R_JCML, 51, "(Sys I)");
	f_string (R_JCML, 68, "(Sys II)");
	f_string (R_JCML+1, (NC-sizeof(grs))/2, grs);

	f_string (R_IO,		C_JMNAMES,	"I   Io");
	f_string (R_EUROPA,	C_JMNAMES,	"II  Europa");
	f_string (R_GANYMEDE,	C_JMNAMES,	"III Ganymede");
	f_string (R_CALLISTO,	C_JMNAMES,	"IV  Callisto");
	f_string (R_JCOLHDNGS-1,C_JMY-2,	"Jupiter Radii");
	f_string (R_JCOLHDNGS,	C_JMX+1,	"X (+E)");
	f_string (R_JCOLHDNGS,	C_JMY+1,	"Y (+S)");
	f_string (R_JCOLHDNGS,	C_JMZ-2,	"Z (+towards)");
	f_string (R_JMAP+1,	2,		"West");
	f_string (R_JMAP+1,	NC-5,		"East");
}

/* display jupiter's details. */
/* ARGSUSED */
void altj_display (force, np)
int force;	/* whether to print for sure or only if things have changed */
Now *np;
{
#define	NJM	5	/* number of moons, plus + for Jupiter itself */
#define	NORM	26.6	/* max callisto orbit radius; used to normalize */
	static char fmt[] = "%7.3f";
	struct moonxlocs {
	    double x;
	    char mid;
	} raw_ml[NJM], sorted_ml[NJM];
	int nml;	/* number of sorted_ml[] elements in use */
	char buf[NC];
	double iy, ey, gy, cy;
	double iz, ez, gz, cz;
	double sIcml, sIIcml;
	int i;

	/* compute jupiter info.
	 * put moons' x loc into raw_ml[] so it can be sorted for graphic.
	 */
	jupinfo (mjd, &raw_ml[0].x, &raw_ml[1].x, &raw_ml[2].x, &raw_ml[3].x,
		    &iy, &ey, &gy, &cy, &iz, &ez, &gz, &cz, &sIcml, &sIIcml);

	f_double (R_JCML, C_JCMLSI, fmt, sIcml);
	f_double (R_JCML, C_JCMLSII, fmt, sIIcml);
	f_double (R_IO, C_JMX, fmt, raw_ml[0].x);
	f_double (R_EUROPA, C_JMX, fmt, raw_ml[1].x);
	f_double (R_GANYMEDE, C_JMX, fmt, raw_ml[2].x);
	f_double (R_CALLISTO, C_JMX, fmt, raw_ml[3].x);
	f_double (R_IO, C_JMY, fmt, iy);
	f_double (R_EUROPA, C_JMY, fmt, ey);
	f_double (R_GANYMEDE, C_JMY, fmt, gy);
	f_double (R_CALLISTO, C_JMY, fmt, cy);
	f_double (R_IO, C_JMZ, fmt, iz);
	f_double (R_EUROPA, C_JMZ, fmt, ez);
	f_double (R_GANYMEDE, C_JMZ, fmt, gz);
	f_double (R_CALLISTO, C_JMZ, fmt, cz);

	raw_ml[0].mid = 'I';
	raw_ml[1].mid = 'E';
	raw_ml[2].mid = 'G';
	raw_ml[3].mid = 'C';
	raw_ml[4].x = 0.0;
	raw_ml[4].mid = 'J';

	/* insert in increasing order into sorted_ml[]
	 */
	nml = 0;
	for (i = 0; i < NJM; i++) {
	    int j;
	    /* exit loop with next sort_ml location to use at index j+1 */
	    for (j = nml; --j >= 0; )
		if (raw_ml[i].x < sorted_ml[j].x)
		    sorted_ml[j+1] = sorted_ml[j];
		else
		    break;
	    sorted_ml[j+1] = raw_ml[i];
	    nml++;
	}

	/* blank-fill and terminate buf */
	(void) sprintf (buf, "%*s", NC-1, "");

	/* convert to screen columns, maintaining correct left-to-right
	 * order based on x when there are collisions.
	 */
	for (i = 0; i < NJM; i++) {
	    int col = (int)(NC/2-1 + (NC/2-1)*sorted_ml[i].x/NORM + 0.5);
	    while (buf[col] != ' ')
		col++;
	    buf[col] = sorted_ml[i].mid;
	}

	f_string (R_JMAP, C_JMAP, buf);
}

#define	dsin(x)	sin(degrad(x))
#define	dcos(x)	cos(degrad(x))

/* given a modified julian date (ie, days since Jan .5 1900), d, return x, y, z
 *   location of each Galilean moon as a multiple of Jupiter's radius. on this
 *   scale, Callisto is never more than 26.5593. +x is easterly, +y is
 *   southerly, +z is towards earth. x and z are relative to the equator
 *   of Jupiter; y is further corrected for earth's position above or below
 *   this plane. also, return the system I and II central meridian longitude,
 *   in degress, relative to the true disk of jupiter and corrected for light
 *   travel time.
 * from "Astronomical Formulae for Calculators", 2nd ed, by Jean Meeus,
 *   Willmann-Bell, Richmond, Va., U.S.A. (c) 1982, chapters 35 and 36.
 */
static void
jupinfo (d, ix, ex, gx, cx, iy, ey, gy, cy, iz, ez, gz, cz, sIcml, sIIcml)
double d;
double *ix, *ex, *gx, *cx;
double *iy, *ey, *gy, *cy;
double *iz, *ez, *gz, *cz;
double *sIcml, *sIIcml;
{
	double A, B, Del, J, K, M, N, R, V;
	double cor_u1, cor_u2, cor_u3, cor_u4;
	double solc, tmp, G, H, psi, r, r1, r2, r3, r4;
	double u1, u2, u3, u4;
	double lam, Ds;
	double z1, z2, z3,  z4;
	double De, dsinDe;

	V = 134.63 + 0.00111587 * d;

	M = (358.47583 + 0.98560003*d);
	N = (225.32833 + 0.0830853*d) + 0.33 * dsin (V);

	J = 221.647 + 0.9025179*d - 0.33 * dsin(V);;

	A = 1.916*dsin(M) + 0.02*dsin(2*M);
	B = 5.552*dsin(N) + 0.167*dsin(2*N);
	K = (J+A-B);
	R = 1.00014 - 0.01672 * dcos(M) - 0.00014 * dcos(2*M);
	r = 5.20867 - 0.25192 * dcos(N) - 0.00610 * dcos(2*N);
	Del = sqrt (R*R + r*r - 2*R*r*dcos(K));
	psi = raddeg (asin (R/Del*dsin(K)));

	solc = (d - Del/173.);	/* speed of light correction */
	tmp = psi - B;

	u1 = 84.5506 + 203.4058630 * solc + tmp;
	u2 = 41.5015 + 101.2916323 * solc + tmp;
	u3 = 109.9770 + 50.2345169 * solc + tmp;
	u4 = 176.3586 + 21.4879802 * solc + tmp;

	G = 187.3 + 50.310674 * solc;
	H = 311.1 + 21.569229 * solc;
      
	cor_u1 =  0.472 * dsin (2*(u1-u2));
	cor_u2 =  1.073 * dsin (2*(u2-u3));
	cor_u3 =  0.174 * dsin (G);
	cor_u4 =  0.845 * dsin (H);
      
	r1 = 5.9061 - 0.0244 * dcos (2*(u1-u2));
	r2 = 9.3972 - 0.0889 * dcos (2*(u2-u3));
	r3 = 14.9894 - 0.0227 * dcos (G);
	r4 = 26.3649 - 0.1944 * dcos (H);

	*ix = -r1 * dsin (u1+cor_u1);
	*ex = -r2 * dsin (u2+cor_u2);
	*gx = -r3 * dsin (u3+cor_u3);
	*cx = -r4 * dsin (u4+cor_u4);

	lam = 238.05 + 0.083091*d + 0.33*dsin(V) + B;
	Ds = 3.07*dsin(lam + 44.5);
	De = Ds - 2.15*dsin(psi)*dcos(lam+24.)
		- 1.31*(r-Del)/Del*dsin(lam-99.4);
	dsinDe = dsin(De);

	z1 = r1 * dcos(u1+cor_u1);
	z2 = r2 * dcos(u2+cor_u2);
	z3 = r3 * dcos(u3+cor_u3);
	z4 = r4 * dcos(u4+cor_u4);

	*iy = z1*dsinDe;
	*ey = z2*dsinDe;
	*gy = z3*dsinDe;
	*cy = z4*dsinDe;

	*iz = z1;
	*ez = z2;
	*gz = z3;
	*cz = z4;

	*sIcml  = 268.28 + 877.8169088*(d - Del/173) + psi - B;
	range (sIcml, 360.0);
	*sIIcml = 290.28 + 870.1869088*(d - Del/173) + psi - B;
	range (sIIcml, 360.0);
}
