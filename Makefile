# Makefile for the Husky build environment

# include Husky-Makefile-Config
include ../huskymak.cfg

ifeq ($(DEBUG), 1)
  CFLAGS=	$(WARNFLAGS) $(DEBCFLAGS)
# CFLAGS=	$(WARNFLAGS) $(DEBCFLAGS) -DNO_LOCKING
else
  CFLAGS=	$(WARNFLAGS) $(OPTCFLAGS)
# CFLAGS=	$(WARNFLAGS) $(OPTCFLAGS) -DNO_LOCKING
endif

ifneq ($(OSTYPE), UNIX)
  LIBPREFIX=
else
  LIBPREFIX=lib
endif

CDEFS=	-D$(OSTYPE) $(ADDCDEFS)
VERH =  1
VER  =  1.6.2

TARGET=	$(LIBPREFIX)smapi$(LIB)

ifeq ($(DYNLIBS), 1)
ALL: $(TARGET) $(LIBPREFIX)smapi.so.$(VER)
else
ALL: $(TARGET)
endif

OBJS=	1stchar$(OBJ)  \
	api_sdm$(OBJ)  \
	api_sq$(OBJ)   \
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
	weekday$(OBJ)

.c$(OBJ):
	$(CC) $(CFLAGS) $(CDEFS)  $<
	$(AR) $(AR_R) $(TARGET) $@

$(TARGET): $(OBJS)
ifdef RANLIB
	$(RANLIB) $(TARGET)
endif

ifeq ($(DYNLIBS), 1)
$(LIBPREFIX)smapi.so.$(VER): $(OBJS)
	gcc -shared -Wl,-soname,$(LIBPREFIX)smapi.so.$(VERH) \
          -o $(LIBPREFIX)smapi.so.$(VER) $(OBJS)

instdyn: $(LIBPREFIX)smapi.so.$(VER)
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(ILOPT) $(LIBPREFIX)smapi.so.$(VER) $(LIBDIR)
	$(LN) $(LNOPT) $(LIBDIR)/$(LIBPREFIX)smapi.so.$(VER) $(LIBDIR)/$(LIBPREFIX)smapi.so.$(VERH)
	$(LN) $(LNOPT) $(LIBDIR)/$(LIBPREFIX)smapi.so.$(VERH) $(LIBDIR)/$(LIBPREFIX)smapi.so
ifneq (~$(LDCONFIG)~, ~~)
	$(LDCONFIG)
endif

else
instdyn: $(TARGET)

endif

install: instdyn
	-$(MKDIR) $(MKDIROPT) $(INCDIR)
	-$(MKDIR) $(MKDIROPT) $(INCDIR)/smapi
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(IIOPT) alc.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) api_brow.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) compiler.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) ffind.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) msgapi.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) patmat.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) prog.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) progprot.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) stamp.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) typedefs.h $(INCDIR)/smapi
	$(INSTALL) $(IIOPT) unused.h $(INCDIR)/smapi
	$(INSTALL) $(ILOPT) $(TARGET) $(LIBDIR)
	$(INSTALL) $(ILOPT) patmat$(OBJ) $(LIBDIR)

clean:
	-$(RM) *$(OBJ)
	-$(RM) *~

distclean: clean
	-$(RM) $(TARGET)
	-$(RM) $(LIBPREFIX)smapi.so.$(VER)
