#include <stdio.h>
#include <math.h>
#include "astro.h"
#include "ephem.h"

/* given true ha and dec, tha and tdec, the geographical latitude, phi, the
 * height above sea-level (as a fraction of the earths radius, 6378.16km),
 * ht, and the equatorial horizontal parallax, ehp, find the apparent
 * ha and dec, aha and adec allowing for parallax.
 * all angles in radians. ehp is the angle subtended at the body by the
 * earth's equator.
 */
void ta_par (tha, tdec, phi, ht, ehp, aha, adec)
double tha, tdec, phi, ht, ehp;
double *aha, *adec;
{
    static double last_phi, last_ht, rsp, rcp;
    double rp;	/* distance to object in Earth radii */
    double ctha;
    double stdec, ctdec;
    double tdtha, dtha;
    double caha;

    /* avoid calcs involving the same phi and ht */
    if (phi != last_phi || ht != last_ht)
    {
        double cphi, sphi, u;
        cphi = cos(phi);
        sphi = sin(phi);
        u = atan(9.96647e-1*sphi/cphi);
        rsp = (9.96647e-1*sin(u))+(ht*sphi);
        rcp = cos(u)+(ht*cphi);
        last_phi  =  phi;
        last_ht  =  ht;
    }

    rp = 1/sin(ehp);

    ctha = cos(tha);
    stdec = sin(tdec);
    ctdec = cos(tdec);
    tdtha = (rcp*sin(tha))/((rp*ctdec)-(rcp*ctha));
    dtha = atan(tdtha);
    *aha = tha+dtha;
    caha = cos(*aha);
    range (aha, 2*PI);
    *adec = atan(caha*(rp*stdec-rsp)/(rp*ctdec*ctha-rcp));
}

#ifdef NEEDIT
/* given the apparent ha and dec, aha and adec, the geographical latitude, phi,
 * the height above sea-level (as a fraction of the earths radius, 6378.16km),
 * ht, and the equatorial horizontal parallax, ehp, find the true ha and dec,
 * tha and tdec allowing for parallax.
 * all angles in radians. ehp is the angle subtended at the body by the
 * earth's equator.
 * uses ta_par() iteratively: find a set of true ha/dec that converts back
  *  to the given apparent ha/dec.
 */
void at_par (aha, adec, phi, ht, ehp, tha, tdec)
double aha, adec, phi, ht, ehp;
double *tha, *tdec;
{
    double nha, ndec;	/* ha/dec corres. to current true guesses */
    double eha, edec;	/* error in ha/dec */

    /* first guess for true is just the apparent */
    *tha = aha;
    *tdec = adec;

    while (1)
    {
        ta_par (*tha, *tdec, phi, ht, ehp, &nha, &ndec);
        eha = aha - nha;
        edec = adec - ndec;
        if (fabs(eha)<1e-6 && fabs(edec)<1e-6)
            break;
        *tha += eha;
        *tdec += edec;
    }
}
#endif
