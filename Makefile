# Makefile for the Husky build environment

# include Husky-Makefile-Config
ifeq ($(DEBIAN), 1)
# Every Debian-Source-Paket has one included.
include debian/huskymak.cfg
else
include ../huskymak.cfg
endif

ifeq ($(DEBUG), 1)
  CFLAGS=	$(WARNFLAGS) $(DEBCFLAGS)
# CFLAGS=	$(WARNFLAGS) $(DEBCFLAGS) -DNO_LOCKING
else
  CFLAGS=	$(WARNFLAGS) $(OPTCFLAGS)
# CFLAGS=	$(WARNFLAGS) $(OPTCFLAGS) -DNO_LOCKING
endif

ifneq ($(OSTYPE), UNIX)
#  LIBPREFIX=
else
  LIBPREFIX=lib
endif

CDEFS=	-D$(OSTYPE) $(ADDCDEFS)
VERH =  2.2
VER  =  2.2.0

TARGET=	$(LIBPREFIX)smapi$(LIB)

ifeq ($(DYNLIBS), 1)
ALL: $(TARGET) $(LIBPREFIX)smapi.so.$(VER)
else
ALL: $(TARGET)
endif

OBJS=	1stchar$(OBJ)  \
	api_sdm$(OBJ)  \
	sq_area$(OBJ) \
	sq_hash$(OBJ) \
	sq_help$(OBJ) \
	sq_idx$(OBJ) \
	sq_kill$(OBJ) \
	sq_lock$(OBJ) \
	sq_misc$(OBJ) \
	sq_msg$(OBJ) \
	sq_read$(OBJ) \
	sq_uid$(OBJ) \
	sq_write$(OBJ) \
	api_jam$(OBJ)  \
	cvtdate$(OBJ)  \
	date2bin$(OBJ) \
	dosdate$(OBJ)  \
	fexist$(OBJ)   \
	ffind$(OBJ)    \
	flush$(OBJ)    \
	locking$(OBJ)  \
	months$(OBJ)   \
	msgapi$(OBJ)   \
	parsenn$(OBJ)  \
	patmat$(OBJ)   \
	qksort$(OBJ)   \
	strextra$(OBJ) \
	strftim$(OBJ)  \
	strocpy$(OBJ)  \
	structrw$(OBJ) \
	trail$(OBJ)    \
	weekday$(OBJ)  \
	tdelay$(OBJ)   \
	setfsize$(OBJ) \
	semaphor$(OBJ)


ifeq ($(DYNLIBS), 1)
all: $(TARGET) $(LIBPREFIX)smapi.so.$(VER)
else
all: $(TARGET)
endif

.c$(OBJ):
	$(CC) $(CFLAGS) $(CDEFS)  $<

$(TARGET): $(OBJS)
	$(AR) $(AR_R) $(TARGET) $?
ifdef RANLIB
	$(RANLIB) $(TARGET)
endif

ifeq ($(DYNLIBS), 1)
  ifeq (~$(MKSHARED)~,~ld~)
$(LIBPREFIX)smapi.so.$(VER): $(OBJS)
	$(LD) $(OPTFLAGS) \
	      -o $(LIBPREFIX)smapi.so.$(VER) $(OBJS)
  else
$(LIBPREFIX)smapi.so.$(VER): $(OBJS)
	$(CC) -shared -Wl,-soname,$(LIBPREFIX)smapi.so.$(VERH) \
          -o $(LIBPREFIX)smapi.so.$(VER) $(OBJS)
  endif

instdyn: $(LIBPREFIX)smapi.so.$(VER)
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(ILOPT) $(LIBPREFIX)smapi.so.$(VER) $(LIBDIR)
	-$(RM) $(RMOPT) $(LIBDIR)/$(LIBPREFIX)smapi.so.$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)/$(LIBPREFIX)smapi.so
# Changed the symlinks from symlinks with full path to just symlinks.
# Better so :)
	cd $(LIBDIR) ;\
	$(LN) $(LNOPT) $(LIBPREFIX)smapi.so.$(VER) $(LIBPREFIX)smapi.so.$(VERH) ;\
	$(LN) $(LNOPT) $(LIBPREFIX)smapi.so.$(VERH) $(LIBPREFIX)smapi.so
ifneq (~$(LDCONFIG)~, ~~)
	$(LDCONFIG)
endif

else
instdyn: $(TARGET)

endif

install: instdyn
	-$(MKDIR) $(MKDIROPT) $(INCDIR)
	-$(MKDIR) $(MKDIROPT) $(INCDIR)$(DIRSEP)smapi
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(IIOPT) alc.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) api_brow.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) compiler.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) ffind.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) msgapi.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) patmat.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) prog.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) progprot.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) stamp.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) typedefs.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) unused.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) semaphor.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(IIOPT) api_jam.h $(INCDIR)$(DIRSEP)smapi
	$(INSTALL) $(ILOPT) $(TARGET) $(LIBDIR)
	$(INSTALL) $(ILOPT) patmat$(OBJ) $(LIBDIR)

uninstall:
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)alc.h 
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)api_brow.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)compiler.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)ffind.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)msgapi.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)patmat.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)prog.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)progprot.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)stamp.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)typedefs.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)unused.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)semaphor.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)smapi$(DIRSEP)api_jam.h
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGET)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)patmat$(OBJ)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)smapi.so.$(VER)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)smapi.so.$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)smapi.so

clean:
	-$(RM) $(RMOPT) *$(OBJ)
	-$(RM) $(RMOPT) *~

distclean: clean
	-$(RM) $(RMOPT) $(TARGET)
	-$(RM) $(RMOPT) $(LIBPREFIX)smapi.so.$(VER)

all: $(TARGET)
