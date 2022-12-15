#ifndef _EPHEM_H
#define _EPHEM_H

/* From aa_hadec.c */
void hadec_aa(double, double, double, double *, double *);

/* From altj.c */
void altj_labels();
#ifdef _CIRCUM_H
void altj_display(int, Now *);
#endif

/* From altmenus.c */
void altmenu_init();
int altmenu_setup();
void alt_nobody(int);
#ifdef _CIRCUM_H
void alt_body(int, int, Now *);
#endif
void alt_labels();
void alt_erase();
int alt_menumask();
int nxtbody(int);
void alt_plnames();

/* From anomaly.c */
void anomaly(double, double, double *, double *);

/* From cal_mjd.c */
void cal_mjd(int, double, int, double *);
void mjd_cal(double, int *, double *, int *);
void year_mjd(double y, double *);
void mjd_cal(double, int *, double *, int *);
void mjd_dow(double, int *);
void mjd_dpm(double, int *);
void mjd_year(double, double *);

/* From circum.c */
#ifdef _CIRCUM_H
int body_cir(int, double, Now *, Sky *);
int twilight_cir(Now *, double *, double *, int *);
int sun_cir(double, Now *, Sky *);
int moon_cir(double, Now *, Sky *);
int same_cir(Now *, Now *);
int same_lday(Now *, Now *);
void now_lst(Now *, double *);
#endif
void elongation(double, double, double, double *);
void rnd_second(double *);

/* From comet.c */
void comet(double, double, double, double, double, double,
  double *, double *, double *, double *, double *, double *);

/* From compiler.c */
int prog_isgood();
int compile_expr(char *, char *);
int execute_expr(double *, char *);

/* From constel.c */
#ifdef _CIRCUM_H
void constellation_msg(int, Now *);
#endif

/* From eq_ecl.c */
void eq_ecl(double, double, double, double *, double *);
void ecl_eq(double, double, double, double *, double *);

/* From flog.c */
int flog_add(int);
void flog_delete(int);
int flog_log(int, int, double, char *);
int flog_get(int, double *, char *);

/* From formats.c */
void f_on();
void f_off();
void f_blanks(int, int, int);
void f_sexad(int, int, int, int, int, double);
void f_sexag(int, int, int, double);
void f_ra(int, int, double);
void f_time(int, int, double);
void f_signtime(int, int, double);
void f_mtime(int, int, double);
void f_angle(int, int, double);
void f_gangle(int, int, double);
void f_date(int, int, double);
void f_int(int, int, char[], double);
void f_char(int, int, int);
void f_string(int, int, char *);
void f_double(int, int, char *, double);
void f_prompt(char *);
void f_eol(int, int);
void f_msg(char *);
void f_sscansex(char *, int *, int *, int *);
void f_sscandate(char *, int *, double *, int *);
void f_dec_sexsign(double, int *, int *, int *);
int decimal_year(char *);

/* From io.c */
void byetty();
void c_pos(int, int);
void c_erase();
void c_eol();
int chk_char();
char read_char();
int read_line(char *, int);

/* From listing.c */
void listing_setup();
void listing();
void listing_prstate(int);
int listing_ison();

/* From main.c */
void print_updating();
void redraw_screen(int);
void slp_sync();

/* From mainmenu.c */
void mm_borders();
void mm_labels();
#ifdef _CIRCUM_H
void mm_now(Now *, int);
void mm_twilight(Now *, int);
#endif
void mm_newcir(int);

/* From moon.c */
void moon(double, double *, double *, double *);

/* From moonnf.c */
void moonnf(double, double *, double *);

/* From nutation.c */
void nutation(double, double *, double *);

/* From obliq.c */
void obliquity (double, double *);

/* From objx.c */
void obj_setup(int);
int obj_ison(int);
void obj_setdbfilename(char *);
void obj_cir(double, int, double *, double *, double *, double *, double *, 
  double *, double *, double *);
void obj_on(int);
int obj_define(int, char *);
int obj_filelookup(int, char *);

/* From parallax.c */
void ta_par(double, double, double, double, double, double *, double *);

/* From pelement.c */
void pelement(double, double[8][9]);

/* From plans.c */
void plans(double, int, double *, double *, double *, double *, double *, 
  double *, double *, double *);

/* From plot.c */
void plot_setup();
void plot();
void plot_prstate(int);
int plot_ison();

/* From popup.c */
int popup(char **, int, int);

/* From precess.c */
void precess(double, double, double *, double *);

/* From reduce.c */
void reduce_elements(double, double, double, double, double, double *,
  double *, double *);

/* From refract.c */
void refract(double, double, double, double *);
void unrefract(double, double, double, double *);

/* From riset.c */
void riset(double, double, double, double, double *, double *, double *,
  double *, int *);

/* From riset_c.c */
#ifdef _CIRCUM_H
int riset_cir(int, Now *, int, int, double *, double *, double *, double *, 
  double *, double *, int *);
#endif

/* From sel_fld.c */
int sel_fld(int, int, char *, char *);

/* From sex_dec */
void sex_dec(int, int, int, double *);
void dec_sex(double, int *, int *, int *, int *);
void range(double *, double);

/* From srch.c */
void srch_setup();
int srch_ison();
int srch_eval(double, double *);
void srch_prstate(int);

/* From sun.c */
void sunpos(double, double *, double *);

/* From time.c */
#ifdef _CIRCUM_H
void set_t0(Now *);
void time_fromsys(Now *);
void inc_mjd(Now *, double);
#endif

/* From utc_gst.c */
void gst_utc(double, double, double *);
void utc_gst(double, double, double *);

/* From version.c */
void version();
void credits();

/* From watch.c */
#define	WATCH_DOME	0	/* nf option for watch_function() */
#define	WATCH_ALTAZ	1	/* nf option for watch_function() */
#define	WATCH_SOLAR_SYS	2	/* nf option for watch_function() */
#ifdef GLOBE_PHOON
#define	WATCH_EARTH	3	/* nf option for watch_function() */
#define	WATCH_MOON      4	/* nf option for watch_function() */
#endif
#ifdef _CIRCUM_H
void watch_menu(Now *, double, int);
int watch_function( int, Now *, double, int );
#endif

#endif
