#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

#define	TWOPI		(2*PI)
#define	mod2PI(x)	((x) - (long)((x)/TWOPI)*TWOPI)

static void masun (double mjd, double *mas);
static void p_mercury (double map[], double *dl, double *dr);
static void p_venus (double t, double mas, double map[], double *dl, double *dr,
                     double *dml, double *dm);
static void p_mars (double mas, double map[], double *dl, double *dr,
                    double *dml, double *dm);
static void p_jupiter (double t, double s, double *dml, double *ds,
                       double *dm, double *da);
static void p_saturn (double t, double s, double *dml, double *ds, double *dm,
                      double *da, double *dhl);
static void p_uranus (double t, double s, double *dl, double *dr, double *dml,
                      double *ds, double *dm, double *da, double *dhl);
static void p_neptune (double t, double s, double *dl, double *dr, double *dml,
                       double *ds, double *dm, double *da, double *dhl);


/* given a modified Julian date, mjd, and a planet, p, find:
 *   lpd0: heliocentric longitude,
 *   psi0: heliocentric latitude,
 *   rp0:  distance from the sun to the planet,
 *   rho0: distance from the Earth to the planet,
 *         none corrected for light time, ie, they are the true values for the
 *         given instant.
 *   lam:  geocentric ecliptic longitude,
 *   bet:  geocentric ecliptic latitude,
 *         each corrected for light time, ie, they are the apparent values as
 *	   seen from the center of the Earth for the given instant.
 *   dia:  angular diameter in arcsec at 1 AU,
 *   mag:  visual magnitude when 1 AU from sun and earth at 0 phase angle.
 *
 * all angles are in radians, all distances in AU.
 * the mean orbital elements are found by calling pelement(), then mutual
 *   perturbation corrections are applied as necessary.
 *
 * corrections for nutation and abberation must be made by the caller. The RA
 *   and DEC calculated from the fully-corrected ecliptic coordinates are then
 *   the apparent geocentric coordinates. Further corrections can be made, if
 *   required, for atmospheric refraction and geocentric parallax although the
 *   intrinsic error herein of about 10 arcseconds is usually the dominant
 *   error at this stage.
 * TODO: combine the several intermediate expressions when get a good compiler.
 */
void plans (mjd, p, lpd0, psi0, rp0, rho0, lam, bet, dia, mag)
double mjd;
int p;
double *lpd0, *psi0, *rp0, *rho0, *lam, *bet, *dia, *mag;
{
    static double plan[8][9];
    static double lastmjd = -10000;
    double dl;	/* perturbation correction for longitude */
    double dr;	/*  "   orbital radius */
    double dml;	/*  "   mean longitude */
    double ds;	/*  "   eccentricity */
    double dm;	/*  "   mean anomaly */
    double da;	/*  "   semi-major axis */
    double dhl;	/*  "   heliocentric longitude */
    double lsn, rsn;/* true geocentric longitude of sun and sun-earth rad */
    double mas;	/* mean anomaly of the sun */
    double re;	/* radius of earth's orbit */
    double lg;	/* longitude of earth */
    double map[8];	/* array of mean anomalies for each planet */
    double lpd, psi, rp, rho;
    double ll, sll, cll;
    double t;
    double dt;
    int pass;
    int j;
    double s, ma;
    double nu, ea;
    double lp, om;
    double lo, slo, clo;
    double inc, y;
    double spsi, cpsi;
    double rpd;

    /* only need to fill in plan[] once for a given mjd */
    if (mjd != lastmjd)
    {
        pelement (mjd, plan);
        lastmjd = mjd;
    }

    dt = 0;
    t = mjd/36525.;
    sunpos (mjd, &lsn, &rsn);
    masun (mjd, &mas);
    re = rsn;
    lg = lsn+PI;

    /* first find the true position of the planet at mjd.
     * then repeat a second time for a slightly different time based
     * on the position found in the first pass to account for light-travel
     * time.
     */
    for (pass = 0; pass < 2; pass++)
    {

        for (j = 0; j < 8; j++)
            map[j] = degrad(plan[j][0]-plan[j][2]-dt*plan[j][1]);

        /* set initial corrections to 0.
         * then modify as necessary for the planet of interest.
         */
        dl = 0;
        dr = 0;
        dml = 0;
        ds = 0;
        dm = 0;
        da = 0;
        dhl = 0;

        switch (p)
        {

        case MERCURY:
            p_mercury (map, &dl, &dr);
            break;

        case VENUS:
            p_venus (t, mas, map, &dl, &dr, &dml, &dm);
            break;

        case MARS:
            p_mars (mas, map, &dl, &dr, &dml, &dm);
            break;

        case JUPITER:
            p_jupiter (t, plan[p][3], &dml, &ds, &dm, &da);
            break;

        case SATURN:
            p_saturn (t, plan[p][3], &dml, &ds, &dm, &da, &dhl);
            break;

        case URANUS:
            p_uranus (t, plan[p][3], &dl, &dr, &dml, &ds, &dm, &da, &dhl);
            break;

        case NEPTUNE:
            p_neptune (t, plan[p][3], &dl, &dr, &dml, &ds, &dm, &da, &dhl);
            break;

        case PLUTO:
            /* no perturbation theory for pluto */
            break;
        }

        s = plan[p][3]+ds;
        ma = map[p]+dm;
        anomaly (ma, s, &nu, &ea);
        rp = (plan[p][6]+da)*(1-s*s)/(1+s*cos(nu));
        lp = raddeg(nu)+plan[p][2]+raddeg(dml-dm);
        lp = degrad(lp);
        om = degrad(plan[p][5]);
        lo = lp-om;
        slo = sin(lo);
        clo = cos(lo);
        inc = degrad(plan[p][4]);
        rp = rp+dr;
        spsi = slo*sin(inc);
        y = slo*cos(inc);
        psi = asin(spsi)+dhl;
        spsi = sin(psi);
        lpd = atan(y/clo)+om+degrad(dl);
        if (clo<0) lpd += PI;
        range (&lpd, TWOPI);
        cpsi = cos(psi);
        rpd = rp*cpsi;
        ll = lpd-lg;
        rho = sqrt(re*re+rp*rp-2*re*rp*cpsi*cos(ll));

        /* when we view a planet we see it in the position it occupied
         * dt days ago, where rho is the distance between it and earth,
         * in AU. use this as the new time for the next pass.
         */
        dt = rho*5.775518e-3;

        if (pass == 0)
        {
            /* save heliocentric coordinates after first pass since, being
             * true, they are NOT to be corrected for light-travel time.
             */
            *lpd0 = lpd;
            range (lpd0, TWOPI);
            *psi0 = psi;
            *rp0 = rp;
            *rho0 = rho;
        }
    }

    sll = sin(ll);
    cll = cos(ll);
    if (p < MARS)
        *lam = atan(-1*rpd*sll/(re-rpd*cll))+lg+PI;
    else
        *lam = atan(re*sll/(rpd-re*cll))+lpd;
    range (lam, TWOPI);
    *bet = atan(rpd*spsi*sin(*lam-lpd)/(cpsi*re*sll));
    *dia = plan[p][7];
    *mag = plan[p][8];
}

/* set auxilliary variables used for jupiter, saturn, uranus, and neptune */
static
void aux_jsun (t, x1, x2, x3, x4, x5, x6)
double t;
double *x1, *x2, *x3, *x4, *x5, *x6;
{
    *x1 = t/5+0.1;
    *x2 = mod2PI(4.14473+5.29691e1*t);
    *x3 = mod2PI(4.641118+2.132991e1*t);
    *x4 = mod2PI(4.250177+7.478172*t);
    *x5 = 5 * *x3 - 2 * *x2;
    *x6 = 2 * *x2 - 6 * *x3 + 3 * *x4;
}

/* find the mean anomaly of the sun at mjd.
 * this is the same as that used in sun() but when it was converted to C it
 * was not known it would be required outside that routine.
 * TODO: add an argument to sun() to return mas and eliminate this routine.
 */
static
void masun (mjd, mas)
double mjd;
double *mas;
{
    double t, t2;
    double a, b;

    t = mjd/36525;
    t2 = t*t;
    a = 9.999736042e1*t;
    b = 360.*(a-(long)a);
    *mas = degrad (3.5847583e2-(1.5e-4+3.3e-6*t)*t2+b);
}

/* perturbations for mercury */
static
void p_mercury (map, dl, dr)
double map[];
double *dl, *dr;
{
    *dl = 2.04e-3*cos(5*map[2-1]-2*map[1-1]+2.1328e-1)+
          1.03e-3*cos(2*map[2-1]-map[1-1]-2.8046)+
          9.1e-4*cos(2*map[3]-map[1-1]-6.4582e-1)+
          7.8e-4*cos(5*map[2-1]-3*map[1-1]+1.7692e-1);

    *dr = 7.525e-6*cos(2*map[3]-map[1-1]+9.25251e-1)+
          6.802e-6*cos(5*map[2-1]-3*map[1-1]-4.53642)+
          5.457e-6*cos(2*map[2-1]-2*map[1-1]-1.24246)+
          3.569e-6*cos(5*map[2-1]-map[1-1]-1.35699);
}

/* ....venus */
static
void p_venus (t, mas, map, dl, dr, dml, dm)
double t, mas, map[];
double *dl, *dr, *dml, *dm;
{
    *dml = degrad (7.7e-4*sin(4.1406+t*2.6227));
    *dm = *dml;

    *dl = 3.13e-3*cos(2*mas-2*map[2-1]-2.587)+
          1.98e-3*cos(3*mas-3*map[2-1]+4.4768e-2)+
          1.36e-3*cos(mas-map[2-1]-2.0788)+
          9.6e-4*cos(3*mas-2*map[2-1]-2.3721)+
          8.2e-4*cos(map[3]-map[2-1]-3.6318);

    *dr = 2.2501e-5*cos(2*mas-2*map[2-1]-1.01592)+
          1.9045e-5*cos(3*mas-3*map[2-1]+1.61577)+
          6.887e-6*cos(map[3]-map[2-1]-2.06106)+
          5.172e-6*cos(mas-map[2-1]-5.08065e-1)+
          3.62e-6*cos(5*mas-4*map[2-1]-1.81877)+
          3.283e-6*cos(4*mas-4*map[2-1]+1.10851)+
          3.074e-6*cos(2*map[3]-2*map[2-1]-9.62846e-1);
}

/* ....mars */
static
void p_mars (mas, map, dl, dr, dml, dm)
double mas, map[];
double *dl, *dr, *dml, *dm;
{
    double a;

    a = 3*map[3]-8*map[2]+4*mas;
    *dml = degrad (-1*(1.133e-2*sin(a)+9.33e-3*cos(a)));
    *dm = *dml;

    *dl = 7.05e-3*cos(map[3]-map[2]-8.5448e-1)+
          6.07e-3*cos(2*map[3]-map[2]-3.2873)+
          4.45e-3*cos(2*map[3]-2*map[2]-3.3492)+
          3.88e-3*cos(mas-2*map[2]+3.5771e-1)+
          2.38e-3*cos(mas-map[2]+6.1256e-1)+
          2.04e-3*cos(2*mas-3*map[2]+2.7688)+
          1.77e-3*cos(3*map[2]-map[2-1]-1.0053)+
          1.36e-3*cos(2*mas-4*map[2]+2.6894)+
          1.04e-3*cos(map[3]+3.0749e-1);

    *dr = 5.3227e-5*cos(map[3]-map[2]+7.17864e-1)+
          5.0989e-5*cos(2*map[3]-2*map[2]-1.77997)+
          3.8278e-5*cos(2*map[3]-map[2]-1.71617)+
          1.5996e-5*cos(mas-map[2]-9.69618e-1)+
          1.4764e-5*cos(2*mas-3*map[2]+1.19768)+
          8.966e-6*cos(map[3]-2*map[2]+7.61225e-1);
    *dr += 7.914e-6*cos(3*map[3]-2*map[2]-2.43887)+
           7.004e-6*cos(2*map[3]-3*map[2]-1.79573)+
           6.62e-6*cos(mas-2*map[2]+1.97575)+
           4.93e-6*cos(3*map[3]-3*map[2]-1.33069)+
           4.693e-6*cos(3*mas-5*map[2]+3.32665)+
           4.571e-6*cos(2*mas-4*map[2]+4.27086)+
           4.409e-6*cos(3*map[3]-map[2]-2.02158);
}

/* ....jupiter */
static
void p_jupiter (t, s, dml, ds, dm, da)
double t, s;
double *dml, *ds, *dm, *da;
{
    double dp;
    double x1, x2, x3, x4, x5, x6, x7;
    double sx3, cx3, s2x3, c2x3;
    double sx5, cx5, s2x5;
    double sx6;
    double sx7, cx7, s2x7, c2x7, s3x7, c3x7, s4x7, c4x7, c5x7;

    aux_jsun (t, &x1, &x2, &x3, &x4, &x5, &x6);
    x7 = x3-x2;
    sx3 = sin(x3);
    cx3 = cos(x3);
    s2x3 = sin(2*x3);
    c2x3 = cos(2*x3);
    sx5 = sin(x5);
    cx5 = cos(x5);
    s2x5 = sin(2*x5);
    sx6 = sin(x6);
    sx7 = sin(x7);
    cx7 = cos(x7);
    s2x7 = sin(2*x7);
    c2x7 = cos(2*x7);
    s3x7 = sin(3*x7);
    c3x7 = cos(3*x7);
    s4x7 = sin(4*x7);
    c4x7 = cos(4*x7);
    c5x7 = cos(5*x7);

    *dml = (3.31364e-1-(1.0281e-2+4.692e-3*x1)*x1)*sx5+
           (3.228e-3-(6.4436e-2-2.075e-3*x1)*x1)*cx5-
           (3.083e-3+(2.75e-4-4.89e-4*x1)*x1)*s2x5+
           2.472e-3*sx6+1.3619e-2*sx7+1.8472e-2*s2x7+6.717e-3*s3x7+
           2.775e-3*s4x7+6.417e-3*s2x7*sx3+
           (7.275e-3-1.253e-3*x1)*sx7*sx3+
           2.439e-3*s3x7*sx3-(3.5681e-2+1.208e-3*x1)*sx7*cx3;
    *dml += -3.767e-3*c2x7*sx3-(3.3839e-2+1.125e-3*x1)*cx7*sx3-
            4.261e-3*s2x7*cx3+
            (1.161e-3*x1-6.333e-3)*cx7*cx3+
            2.178e-3*cx3-6.675e-3*c2x7*cx3-2.664e-3*c3x7*cx3-
            2.572e-3*sx7*s2x3-3.567e-3*s2x7*s2x3+2.094e-3*cx7*c2x3+
            3.342e-3*c2x7*c2x3;
    *dml = degrad(*dml);

    *ds = (3606+(130-43*x1)*x1)*sx5+(1289-580*x1)*cx5-6764*sx7*sx3-
          1110*s2x7*sx3-224*s3x7*sx3-204*sx3+(1284+116*x1)*cx7*sx3+
          188*c2x7*sx3+(1460+130*x1)*sx7*cx3+224*s2x7*cx3-817*cx3+
          6074*cx3*cx7+992*c2x7*cx3+
          508*c3x7*cx3+230*c4x7*cx3+108*c5x7*cx3;
    *ds += -(956+73*x1)*sx7*s2x3+448*s2x7*s2x3+137*s3x7*s2x3+
           (108*x1-997)*cx7*s2x3+480*c2x7*s2x3+148*c3x7*s2x3+
           (99*x1-956)*sx7*c2x3+490*s2x7*c2x3+
           158*s3x7*c2x3+179*c2x3+(1024+75*x1)*cx7*c2x3-
           437*c2x7*c2x3-132*c3x7*c2x3;
    *ds *= 1e-7;

    dp = (7.192e-3-3.147e-3*x1)*sx5-4.344e-3*sx3+
         (x1*(1.97e-4*x1-6.75e-4)-2.0428e-2)*cx5+
         3.4036e-2*cx7*sx3+(7.269e-3+6.72e-4*x1)*sx7*sx3+
         5.614e-3*c2x7*sx3+2.964e-3*c3x7*sx3+3.7761e-2*sx7*cx3+
         6.158e-3*s2x7*cx3-
         6.603e-3*cx7*cx3-5.356e-3*sx7*s2x3+2.722e-3*s2x7*s2x3+
         4.483e-3*cx7*s2x3-2.642e-3*c2x7*s2x3+4.403e-3*sx7*c2x3-
         2.536e-3*s2x7*c2x3+5.547e-3*cx7*c2x3-2.689e-3*c2x7*c2x3;

    *dm = *dml-(degrad(dp)/s);

    *da = 205*cx7-263*cx5+693*c2x7+312*c3x7+147*c4x7+299*sx7*sx3+
          181*c2x7*sx3+204*s2x7*cx3+111*s3x7*cx3-337*cx7*cx3-
          111*c2x7*cx3;
    *da *= 1e-6;
}

/* ....saturn */
static
void p_saturn (t, s, dml, ds, dm, da, dhl)
double t, s;
double *dml, *ds, *dm, *da, *dhl;
{
    double dp;
    double x1, x2, x3, x4, x5, x6, x7, x8;
    double sx3, cx3, s2x3, c2x3, s3x3, c3x3, s4x3, c4x3;
    double sx5, cx5, s2x5, c2x5;
    double sx6;
    double sx7, cx7, s2x7, c2x7, s3x7, c3x7, s4x7, c4x7, c5x7, s5x7;
    double s2x8, c2x8, s3x8, c3x8;

    aux_jsun (t, &x1, &x2, &x3, &x4, &x5, &x6);
    x7 = x3-x2;
    sx3 = sin(x3);
    cx3 = cos(x3);
    s2x3 = sin(2*x3);
    c2x3 = cos(2*x3);
    sx5 = sin(x5);
    cx5 = cos(x5);
    s2x5 = sin(2*x5);
    sx6 = sin(x6);
    sx7 = sin(x7);
    cx7 = cos(x7);
    s2x7 = sin(2*x7);
    c2x7 = cos(2*x7);
    s3x7 = sin(3*x7);
    c3x7 = cos(3*x7);
    s4x7 = sin(4*x7);
    c4x7 = cos(4*x7);
    c5x7 = cos(5*x7);

    s3x3 = sin(3*x3);
    c3x3 = cos(3*x3);
    s4x3 = sin(4*x3);
    c4x3 = cos(4*x3);
    c2x5 = cos(2*x5);
    s5x7 = sin(5*x7);
    x8 = x4-x3;
    s2x8 = sin(2*x8);
    c2x8 = cos(2*x8);
    s3x8 = sin(3*x8);
    c3x8 = cos(3*x8);

    *dml = 7.581e-3*s2x5-7.986e-3*sx6-1.48811e-1*sx7-4.0786e-2*s2x7-
           (8.14181e-1-(1.815e-2-1.6714e-2*x1)*x1)*sx5-
           (1.0497e-2-(1.60906e-1-4.1e-3*x1)*x1)*cx5-1.5208e-2*s3x7-
           6.339e-3*s4x7-6.244e-3*sx3-1.65e-2*s2x7*sx3+
           (8.931e-3+2.728e-3*x1)*sx7*sx3-5.775e-3*s3x7*sx3+
           (8.1344e-2+3.206e-3*x1)*cx7*sx3+1.5019e-2*c2x7*sx3;
    *dml += (8.5581e-2+2.494e-3*x1)*sx7*cx3+1.4394e-2*c2x7*cx3+
            (2.5328e-2-3.117e-3*x1)*cx7*cx3+
            6.319e-3*c3x7*cx3+6.369e-3*sx7*s2x3+9.156e-3*s2x7*s2x3+
            7.525e-3*s3x8*s2x3-5.236e-3*cx7*c2x3-7.736e-3*c2x7*c2x3-
            7.528e-3*c3x8*c2x3;
    *dml = degrad(*dml);

    *ds = (-7927+(2548+91*x1)*x1)*sx5+(13381+(1226-253*x1)*x1)*cx5+
          (248-121*x1)*s2x5-(305+91*x1)*c2x5+412*s2x7+12415*sx3+
          (390-617*x1)*sx7*sx3+(165-204*x1)*s2x7*sx3+26599*cx7*sx3-
          4687*c2x7*sx3-1870*c3x7*sx3-821*c4x7*sx3-
          377*c5x7*sx3+497*c2x8*sx3+(163-611*x1)*cx3;
    *ds += -12696*sx7*cx3-4200*s2x7*cx3-1503*s3x7*cx3-619*s4x7*cx3-
           268*s5x7*cx3-(282+1306*x1)*cx7*cx3+(-86+230*x1)*c2x7*cx3+
           461*s2x8*cx3-350*s2x3+(2211-286*x1)*sx7*s2x3-
           2208*s2x7*s2x3-568*s3x7*s2x3-346*s4x7*s2x3-
           (2780+222*x1)*cx7*s2x3+(2022+263*x1)*c2x7*s2x3+248*c3x7*s2x3+
           242*s3x8*s2x3+467*c3x8*s2x3-490*c2x3-(2842+279*x1)*sx7*c2x3;
    *ds += (128+226*x1)*s2x7*c2x3+224*s3x7*c2x3+
           (-1594+282*x1)*cx7*c2x3+(2162-207*x1)*c2x7*c2x3+
           561*c3x7*c2x3+343*c4x7*c2x3+469*s3x8*c2x3-242*c3x8*c2x3-
           205*sx7*s3x3+262*s3x7*s3x3+208*cx7*c3x3-271*c3x7*c3x3-
           382*c3x7*s4x3-376*s3x7*c4x3;
    *ds *= 1e-7;

    dp = (7.7108e-2+(7.186e-3-1.533e-3*x1)*x1)*sx5-7.075e-3*sx7+
         (4.5803e-2-(1.4766e-2+5.36e-4*x1)*x1)*cx5-7.2586e-2*cx3-
         7.5825e-2*sx7*sx3-2.4839e-2*s2x7*sx3-8.631e-3*s3x7*sx3-
         1.50383e-1*cx7*cx3+2.6897e-2*c2x7*cx3+1.0053e-2*c3x7*cx3-
         (1.3597e-2+1.719e-3*x1)*sx7*s2x3+1.1981e-2*s2x7*c2x3;
    dp += -(7.742e-3-1.517e-3*x1)*cx7*s2x3+
          (1.3586e-2-1.375e-3*x1)*c2x7*c2x3-
          (1.3667e-2-1.239e-3*x1)*sx7*c2x3+
          (1.4861e-2+1.136e-3*x1)*cx7*c2x3-
          (1.3064e-2+1.628e-3*x1)*c2x7*c2x3;

    *dm = *dml-(degrad(dp)/s);

    *da = 572*sx5-1590*s2x7*cx3+2933*cx5-647*s3x7*cx3+33629*cx7-
          344*s4x7*cx3-3081*c2x7+2885*cx7*cx3-1423*c3x7+
          (2172+102*x1)*c2x7*cx3-671*c4x7+296*c3x7*cx3-320*c5x7-
          267*s2x7*s2x3+1098*sx3-778*cx7*s2x3-2812*sx7*sx3;
    *da += 495*c2x7*s2x3+688*s2x7*sx3+250*c3x7*s2x3-393*s3x7*sx3-
           856*sx7*c2x3-228*s4x7*sx3+441*s2x7*c2x3+2138*cx7*sx3+
           296*c2x7*c2x3-999*c2x7*sx3+211*c3x7*c2x3-642*c3x7*sx3-
           427*sx7*s3x3-325*c4x7*sx3+398*s3x7*s3x3-890*cx3+
           344*cx7*c3x3+2206*sx7*cx3-427*c3x7*c3x3;
    *da *= 1e-6;

    *dhl = 7.47e-4*cx7*sx3+1.069e-3*cx7*cx3+2.108e-3*s2x7*s2x3+
           1.261e-3*c2x7*s2x3+1.236e-3*s2x7*c2x3-2.075e-3*c2x7*c2x3;
    *dhl = degrad(*dhl);
}

/* ....uranus */
static
void p_uranus (t, s, dl, dr, dml, ds, dm, da, dhl)
double t, s;
double *dl, *dr, *dml, *ds, *dm, *da, *dhl;
{
    double dp;
    double x1, x2, x3, x4, x5, x6;
    double x8, x9, x10, x11, x12;
    double sx4, cx4, s2x4, c2x4;
    double sx9, cx9, s2x9, c2x9;
    double sx11, cx11;

    aux_jsun (t, &x1, &x2, &x3, &x4, &x5, &x6);

    x8 = mod2PI(1.46205+3.81337*t);
    x9 = 2*x8-x4;
    sx9 = sin(x9);
    cx9 = cos(x9);
    s2x9 = sin(2*x9);
    c2x9 = cos(2*x9);

    x10 = x4-x2;
    x11 = x4-x3;
    x12 = x8-x4;

    *dml = (8.64319e-1-1.583e-3*x1)*sx9+(8.2222e-2-6.833e-3*x1)*cx9+
           3.6017e-2*s2x9-3.019e-3*c2x9+8.122e-3*sin(x6);
    *dml = degrad(*dml);

    dp = 1.20303e-1*sx9+6.197e-3*s2x9+(1.9472e-2-9.47e-4*x1)*cx9;
    *dm = *dml-(degrad(dp)/s);

    *ds = (163*x1-3349)*sx9+20981*cx9+1311*c2x9;
    *ds *= 1e-7;

    *da = -3.825e-3*cx9;

    *dl = (1.0122e-2-9.88e-4*x1)*sin(x4+x11)+
          (-3.8581e-2+(2.031e-3-1.91e-3*x1)*x1)*cos(x4+x11)+
          (3.4964e-2-(1.038e-3-8.68e-4*x1)*x1)*cos(2*x4+x11)+
          5.594e-3*sin(x4+3*x12)-1.4808e-2*sin(x10)-
          5.794e-3*sin(x11)+2.347e-3*cos(x11)+9.872e-3*sin(x12)+
          8.803e-3*sin(2*x12)-4.308e-3*sin(3*x12);

    sx11 = sin(x11);
    cx11 = cos(x11);
    sx4 = sin(x4);
    cx4 = cos(x4);
    s2x4 = sin(2*x4);
    c2x4 = cos(2*x4);
    *dhl = (4.58e-4*sx11-6.42e-4*cx11-5.17e-4*cos(4*x12))*sx4-
           (3.47e-4*sx11+8.53e-4*cx11+5.17e-4*sin(4*x11))*cx4+
           4.03e-4*(cos(2*x12)*s2x4+sin(2*x12)*c2x4);
    *dhl = degrad(*dhl);

    *dr = -25948+4985*cos(x10)-1230*cx4+3354*cos(x11)+904*cos(2*x12)+
          894*(cos(x12)-cos(3*x12))+(5795*cx4-1165*sx4+1388*c2x4)*sx11+
          (1351*cx4+5702*sx4+1388*s2x4)*cos(x11);
    *dr *= 1e-6;
}

/* ....neptune */
static
void p_neptune (t, s, dl, dr, dml, ds, dm, da, dhl)
double t, s;
double *dl, *dr, *dml, *ds, *dm, *da, *dhl;
{
    double dp;
    double x1, x2, x3, x4, x5, x6;
    double x8, x9, x10, x11, x12;
    double sx8, cx8;
    double sx9, cx9, s2x9, c2x9;
    double s2x12, c2x12;

    aux_jsun (t, &x1, &x2, &x3, &x4, &x5, &x6);

    x8 = mod2PI(1.46205+3.81337*t);
    x9 = 2*x8-x4;
    sx9 = sin(x9);
    cx9 = cos(x9);
    s2x9 = sin(2*x9);
    c2x9 = cos(2*x9);

    x10 = x8-x2;
    x11 = x8-x3;
    x12 = x8-x4;

    *dml = (1.089e-3*x1-5.89833e-1)*sx9+(4.658e-3*x1-5.6094e-2)*cx9-
           2.4286e-2*s2x9;
    *dml = degrad(*dml);

    dp = 2.4039e-2*sx9-2.5303e-2*cx9+6.206e-3*s2x9-5.992e-3*c2x9;

    *dm = *dml-(degrad(dp)/s);

    *ds = 4389*sx9+1129*s2x9+4262*cx9+1089*c2x9;
    *ds *= 1e-7;

    *da = 8189*cx9-817*sx9+781*c2x9;
    *da *= 1e-6;

    s2x12 = sin(2*x12);
    c2x12 = cos(2*x12);
    sx8 = sin(x8);
    cx8 = cos(x8);
    *dl = -9.556e-3*sin(x10)-5.178e-3*sin(x11)+2.572e-3*s2x12-
          2.972e-3*c2x12*sx8-2.833e-3*s2x12*cx8;

    *dhl = 3.36e-4*c2x12*sx8+3.64e-4*s2x12*cx8;
    *dhl = degrad(*dhl);

    *dr = -40596+4992*cos(x10)+2744*cos(x11)+2044*cos(x12)+1051*c2x12;
    *dr *= 1e-6;
}
