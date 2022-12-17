PROJECT = ephem
CFLAGS = -Wall -O2 -DGLOBE_PHOON 
LIBS = -lm -lcurses
PREFIX = /usr/local
INSTALL = install
RM = rm -f

OBJS =  src/aa_hadec.o \
	src/altj.o \
	src/altmenus.o \
	src/anomaly.o \
	src/cal_mjd.o \
	src/circum.o \
	src/comet.o \
	src/compiler.o \
	src/constel.o \
	src/eq_ecl.o \
	src/flog.o \
	src/formats.o \
	src/globe.o \
	src/io.o \
	src/listing.o \
	src/mainmenu.o \
	src/moon.o \
	src/moonnf.o \
	src/nutation.o \
	src/objx.o \
	src/obliq.o \
	src/parallax.o \
	src/pelement.o \
	src/phoon.o \
	src/phoonastro.o \
	src/plans.o \
	src/plot.o \
	src/popup.o \
	src/precess.o \
	src/reduce.o \
	src/refract.o \
	src/riset.o \
	src/riset_c.o \
	src/sel_fld.o \
	src/sex_dec.o \
	src/srch.o \
	src/sun.o \
	src/time.o \
	src/utc_gst.o \
	src/version.o \
	src/watch.o \
	src/main.o

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@

%.c: %.o
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

install: $(PROJECT)
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) -m 0755 $(PROJECT) $(PREFIX)/bin

clean:
	$(RM) $(PROJECT) $(OBJS)
