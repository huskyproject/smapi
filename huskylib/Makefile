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
VER  =  1.0.0

TARGET=	$(LIBPREFIX)husky$(LIB)

ifeq ($(DYNLIBS), 1)
ALL: $(TARGET) $(LIBPREFIX)husky.so.$(VER)
else
ALL: $(TARGET)
endif

OBJS=locking$(OBJ)


ifeq ($(DYNLIBS), 1)
all: $(TARGET) $(LIBPREFIX)husky.so.$(VER)
else
all: $(TARGET)
endif

.c$(OBJ):
	$(CC) $(CFLAGS) $(CDEFS)  $<
	$(AR) $(AR_R) $(TARGET) $@

$(TARGET): $(OBJS)
ifdef RANLIB
	$(RANLIB) $(TARGET)
endif

ifeq ($(DYNLIBS), 1)
  ifeq (~$(MKSHARED)~,~ld~)
$(LIBPREFIX)husky.so.$(VER): $(OBJS)
	$(LD) -s -shared \
	      -o $(LIBPREFIX)husky.so.$(VER) $(OBJS)
  else
$(LIBPREFIX)husky.so.$(VER): $(OBJS)
	$(CC) -shared -Wl,-soname,$(LIBPREFIX)husky.so.$(VERH) \
          -o $(LIBPREFIX)husky.so.$(VER) $(OBJS)
  endif

instdyn: $(LIBPREFIX)husky.so.$(VER)
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(ILOPT) $(LIBPREFIX)husky.so.$(VER) $(LIBDIR)
	-$(RM) $(RMOPT) $(LIBDIR)/$(LIBPREFIX)husky.so.$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)/$(LIBPREFIX)husky.so
	$(LN) $(LNOPT) $(LIBDIR)/$(LIBPREFIX)husky.so.$(VER) $(LIBDIR)/$(LIBPREFIX)husky.so.$(VERH)
	$(LN) $(LNOPT) $(LIBDIR)/$(LIBPREFIX)husky.so.$(VERH) $(LIBDIR)/$(LIBPREFIX)husky.so
ifneq (~$(LDCONFIG)~, ~~)
	$(LDCONFIG)
endif

else
instdyn: $(TARGET)

endif

install: instdyn
	-$(MKDIR) $(MKDIROPT) $(INCDIR)
	-$(MKDIR) $(MKDIROPT) $(INCDIR)/husky
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(IIOPT) compiler.h $(INCDIR)/husky
	$(INSTALL) $(IIOPT) locking.h $(INCDIR)/husky

uninstall:
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)alc.h 
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)api_brow.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)compiler.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)ffind.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)msgapi.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)patmat.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)prog.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)progprot.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)stamp.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)typedefs.h
	-$(RM) $(RMOPT) $(INCDIR)$(DIRSEP)husky$(DIRSEP)unused.h
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGET)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)patmat$(OBJ)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)husky.so.$(VER)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)husky.so.$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)husky.so

clean:
	-$(RM) $(RMOPT) *$(OBJ)
	-$(RM) $(RMOPT) *~

distclean: clean
	-$(RM) $(RMOPT) $(TARGET)
	-$(RM) $(RMOPT) $(LIBPREFIX)husky.so.$(VER)

all: $(TARGET)

