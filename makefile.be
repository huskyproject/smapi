# Legacy makefile for building on BeOS with gcc
# No support for the Husky build environment

CC=	gcc
CFLAGS=	-Wall
CDEFS=	-DUNIX
COPT=	-O3

# on i386, you may add this
# -fomit-frame-pointer -fstrength-reduce -fPIC -m486

LIB=	ar

TARGET=	libsmapibe.a

all: $(TARGET)

OBJS=	1stchar.o  \
	api_sdm.o  \
	api_sq.o   \
	api_jam.o  \
	cvtdate.o  \
	date2bin.o \
	dosdate.o  \
	fexist.o   \
	ffind.o    \
	flush.o    \
	locking.o  \
	months.o   \
	msgapi.o   \
	parsenn.o  \
	patmat.o   \
	qksort.o   \
	strextra.o \
	strftim.o  \
	strocpy.o  \
	structrw.o \
	trail.o    \
	weekday.o

# Thunderbirds are go!

.c.o:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<
	$(LIB) r $(TARGET) $@

$(TARGET): $(OBJS)

clean:
	-rm *.o

distclean: clean
	-rm $(TARGET)
