#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

/* given the mjd, find the geocentric ecliptic longitude, lam, and latitude,
 * bet, and horizontal parallax, hp for the moon.
 * N.B. series for long and lat are good to about 10 and 3 arcseconds. however,
 *   math errors cause up to 100 and 30 arcseconds error, even if use double.
 *   why?? suspect highly sensitive nature of difference used to get m1..6.
 * N.B. still need to correct for nutation. then for topocentric location
 *   further correct for parallax and refraction.
 */
void moon (mjd, lam, bet, hp)
double mjd;
double *lam, *bet, *hp;
{
    double t, t2;
    double ld;
    double ms;
    double md;
    double de;
    double f;
    double n;
    double a, sa, sn, b, sb, c, sc, e, e2, l, g, w1, w2;
    double m1, m2, m3, m4, m5, m6;

    t = mjd/36525.;
    t2 = t*t;

    m1 = mjd/27.32158213;
    m1 = 360.0*(m1-(long)m1);
    m2 = mjd/365.2596407;
    m2 = 360.0*(m2-(long)m2);
    m3 = mjd/27.55455094;
    m3 = 360.0*(m3-(long)m3);
    m4 = mjd/29.53058868;
    m4 = 360.0*(m4-(long)m4);
    m5 = mjd/27.21222039;
    m5 = 360.0*(m5-(long)m5);
    m6 = mjd/6798.363307;
    m6 = 360.0*(m6-(long)m6);

    ld = 270.434164+m1-(.001133-.0000019*t)*t2;
    ms = 358.475833+m2-(.00015+.0000033*t)*t2;
    md = 296.104608+m3+(.009192+.0000144*t)*t2;
    de = 350.737486+m4-(.001436-.0000019*t)*t2;
    f = 11.250889+m5-(.003211+.0000003*t)*t2;
    n = 259.183275-m6+(.002078+.000022*t)*t2;

    a = degrad(51.2+20.2*t);
    sa = sin(a);
    sn = sin(degrad(n));
    b = 346.56+(132.87-.0091731*t)*t;
    sb = .003964*sin(degrad(b));
    c = degrad(n+275.05-2.3*t);
    sc = sin(c);
    ld = ld+.000233*sa+sb+.001964*sn;
    ms = ms-.001778*sa;
    md = md+.000817*sa+sb+.002541*sn;
    f = f+sb-.024691*sn-.004328*sc;
    de = de+.002011*sa+sb+.001964*sn;
    e = 1-(.002495+7.52e-06*t)*t;
    e2 = e*e;

    ld = degrad(ld);
    ms = degrad(ms);
    n = degrad(n);
    de = degrad(de);
    f = degrad(f);
    md = degrad(md);

    l = 6.28875*sin(md)+1.27402*sin(2*de-md)+.658309*sin(2*de)+
        .213616*sin(2*md)-e*.185596*sin(ms)-.114336*sin(2*f)+
        .058793*sin(2*(de-md))+.057212*e*sin(2*de-ms-md)+
        .05332*sin(2*de+md)+.045874*e*sin(2*de-ms)+.041024*e*sin(md-ms);
    l = l-.034718*sin(de)-e*.030465*sin(ms+md)+.015326*sin(2*(de-f))-
        .012528*sin(2*f+md)-.01098*sin(2*f-md)+.010674*sin(4*de-md)+
        .010034*sin(3*md)+.008548*sin(4*de-2*md)-e*.00791*sin(ms-md+2*de)-
        e*.006783*sin(2*de+ms);
    l = l+.005162*sin(md-de)+e*.005*sin(ms+de)+.003862*sin(4*de)+
        e*.004049*sin(md-ms+2*de)+.003996*sin(2*(md+de))+
        .003665*sin(2*de-3*md)+e*.002695*sin(2*md-ms)+
        .002602*sin(md-2*(f+de))+e*.002396*sin(2*(de-md)-ms)-
        .002349*sin(md+de);
    l = l+e2*.002249*sin(2*(de-ms))-e*.002125*sin(2*md+ms)-
        e2*.002079*sin(2*ms)+e2*.002059*sin(2*(de-ms)-md)-
        .001773*sin(md+2*(de-f))-.001595*sin(2*(f+de))+
        e*.00122*sin(4*de-ms-md)-.00111*sin(2*(md+f))+.000892*sin(md-3*de);
    l = l-e*.000811*sin(ms+md+2*de)+e*.000761*sin(4*de-ms-2*md)+
        e2*.000704*sin(md-2*(ms+de))+e*.000693*sin(ms-2*(md-de))+
        e*.000598*sin(2*(de-f)-ms)+.00055*sin(md+4*de)+.000538*sin(4*md)+
        e*.000521*sin(4*de-ms)+.000486*sin(2*md-de);
    l = l+e2*.000717*sin(md-2*ms);
    *lam = ld+degrad(l);
    range (lam, 2*PI);

    g = 5.12819*sin(f)+.280606*sin(md+f)+.277693*sin(md-f)+
        .173238*sin(2*de-f)+.055413*sin(2*de+f-md)+.046272*sin(2*de-f-md)+
        .032573*sin(2*de+f)+.017198*sin(2*md+f)+.009267*sin(2*de+md-f)+
        .008823*sin(2*md-f)+e*.008247*sin(2*de-ms-f);
    g = g+.004323*sin(2*(de-md)-f)+.0042*sin(2*de+f+md)+
        e*.003372*sin(f-ms-2*de)+e*.002472*sin(2*de+f-ms-md)+
        e*.002222*sin(2*de+f-ms)+e*.002072*sin(2*de-f-ms-md)+
        e*.001877*sin(f-ms+md)+.001828*sin(4*de-f-md)-e*.001803*sin(f+ms)-
        .00175*sin(3*f);
    g = g+e*.00157*sin(md-ms-f)-.001487*sin(f+de)-e*.001481*sin(f+ms+md)+
        e*.001417*sin(f-ms-md)+e*.00135*sin(f-ms)+.00133*sin(f-de)+
        .001106*sin(f+3*md)+.00102*sin(4*de-f)+.000833*sin(f+4*de-md)+
        .000781*sin(md-3*f)+.00067*sin(f+4*de-2*md);
    g = g+.000606*sin(2*de-3*f)+.000597*sin(2*(de+md)-f)+
        e*.000492*sin(2*de+md-ms-f)+.00045*sin(2*(md-de)-f)+
        .000439*sin(3*md-f)+.000423*sin(f+2*(de+md))+
        .000422*sin(2*de-f-3*md)-e*.000367*sin(ms+f+2*de-md)-
        e*.000353*sin(ms+f+2*de)+.000331*sin(f+4*de);
    g = g+e*.000317*sin(2*de+f-ms+md)+e2*.000306*sin(2*(de-ms)-f)-
        .000283*sin(md+3*f);
    w1 = .0004664*cos(n);
    w2 = .0000754*cos(c);
    *bet = degrad(g)*(1-w1-w2);

    *hp = .950724+.051818*cos(md)+.009531*cos(2*de-md)+.007843*cos(2*de)+
          .002824*cos(2*md)+.000857*cos(2*de+md)+e*.000533*cos(2*de-ms)+
          e*.000401*cos(2*de-md-ms)+e*.00032*cos(md-ms)-.000271*cos(de)-
          e*.000264*cos(ms+md)-.000198*cos(2*f-md);
    *hp = *hp+.000173*cos(3*md)+.000167*cos(4*de-md)-e*.000111*cos(ms)+
          .000103*cos(4*de-2*md)-.000084*cos(2*md-2*de)-
          e*.000083*cos(2*de+ms)+.000079*cos(2*de+2*md)+.000072*cos(4*de)+
          e*.000064*cos(2*de-ms+md)-e*.000063*cos(2*de+ms-md)+
          e*.000041*cos(ms+de);
    *hp = *hp+e*.000035*cos(2*md-ms)-.000033*cos(3*md-2*de)-
          .00003*cos(md+de)-.000029*cos(2*(f-de))-e*.000029*cos(2*md+ms)+
          e2*.000026*cos(2*(de-ms))-.000023*cos(2*(f-de)+md)+
          e*.000019*cos(4*de-ms-md);
    *hp = degrad(*hp);
}
