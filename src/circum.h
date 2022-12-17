#ifndef _CIRCUM_H
#define _CIRCUM_H

#define	SPD	(24.0*3600.0)	/* seconds per day */

#define	EOD	(-9786)		/* special epoch flag: use epoch of date */
#define	RTC	(-1324)		/* special tminc flag: use rt clock */
#define	NOMJD	(-58631.)	/* an unlikely mjd for initing static mjd's */
#define	NOHELIO	(-2314)		/* special s_hlong flag: means it and s_hlat are
				 * undefined
				 */

#define	STDHZN		0	/* rise/set times based on nominal conditions */
#define	ADPHZN		1	/* rise/set times based on exact current " */
#define	TWILIGHT	2	/* rise/set times for sun 18 degs below hor */

/* info about our local observing circumstances */
typedef struct
{
    double n_mjd;	/* modified Julian date, ie, days since
			 * Jan 0.5 1900 (== 12 noon, Dec 30, 1899), utc.
			 * enough precision to get well better than 1 second.
			 * N.B. if not first member, must move NOMJD inits.
			 */
    double n_lat;	/* latitude, >0 north, rads */
    double n_lng;	/* longitude, >0 east, rads */
    double n_tz;	/* time zone, hrs behind UTC */
    double n_temp;	/* atmospheric temp, degrees C */
    double n_pressure; /* atmospheric pressure, mBar */
    double n_height;	/* height above sea level, earth radii */
    double n_epoch;	/* desired precession display epoch as an mjd, or EOD */
    char n_tznm[4];	/* time zone name; 3 chars or less, always 0 at end */
} Now;
extern double	mjd_day(), mjd_hr();

/* info about where and how we see something in the sky */
typedef struct
{
    double s_ra;	/* ra, rads (precessed to n_epoch) */
    double s_dec;	/* dec, rads (precessed to n_epoch) */
    double s_az;	/* azimuth, >0 e of n, rads */
    double s_alt;	/* altitude above topocentric horizon, rads */
    double s_sdist;	/* dist from object to sun, au */
    double s_edist;	/* dist from object to earth, au */
    double s_elong;	/* angular sep between object and sun, >0 if east */
    double s_hlong;	/* heliocentric longitude, rads */
    double s_hlat;	/* heliocentric latitude, rads */
    double s_size;	/* angular size, arc secs */
    double s_phase;	/* phase, % */
    double s_mag;	/* visual magnitude */
} Sky;

/* flags for riset_cir() status */
#define	RS_NORISE	0x001	/* object does not rise as such today */
#define	RS_2RISES	0x002	/* object rises more than once today */
#define	RS_NOSET	0x004	/* object does not set as such today */
#define	RS_2SETS	0x008	/* object sets more than once today */
#define	RS_CIRCUMPOLAR	0x010	/* object stays up all day today */
#define	RS_2TRANS	0x020	/* transits twice in one day */
#define	RS_NEVERUP	0x040	/* object never rises today */
#define	RS_NOTRANS	0x080	/* doesn't transit today */
#define	RS_ERROR	0x100	/* can't figure out times... */

/* shorthands for fields a Now pointer, np */
#define mjd	np->n_mjd
#define lat	np->n_lat
#define lng	np->n_lng
#define tz	np->n_tz
#define temp	np->n_temp
#define pressure np->n_pressure
#define height	np->n_height
#define epoch	np->n_epoch
#define tznm	np->n_tznm

#endif
