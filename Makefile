# Makefile for the Husky build environment

# include Husky-Makefile-Config
ifeq ($(DEBIAN), 1)
# Every Debian-Source-Paket has one included.
include /usr/share/husky/huskymak.cfg
else
include ../huskymak.cfg
endif

include makefile.inc

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

TARGET=	$(LIBPREFIX)smapi$(LIB)


ifeq ($(DYNLIBS), 1)
ALL: $(TARGET) $(LIBPREFIX)smapi.so.$(VER)
else
ALL: $(TARGET)
endif


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
	$(LD) $(OPTLFLAGS) \
	      -o $(LIBPREFIX)smapi.so.$(VER) $(OBJS)
  else
$(LIBPREFIX)smapi.so.$(VER): $(OBJS)
	$(CC) -shared -Wl,-soname,$(LIBPREFIX)smapi.so.$(VERH) \
          -o $(LIBPREFIX)smapi.so.$(VER) $(OBJS)
  endif

instdyn: $(TARGET) $(LIBPREFIX)smapi.so.$(VER)
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(ILOPT) $(LIBPREFIX)smapi.so.$(VER) $(LIBDIR)
	-$(RM) $(RMOPT) $(LIBDIR)/$(LIBPREFIX)smapi.so.$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)/$(LIBPREFIX)smapi.so
# Changed the symlinks from symlinks with full path to just symlinks.
# Better so :)
	cd $(LIBDIR) ;\
	$(LN) $(LNOPT) $(LIBPREFIX)smapi.so.$(VER) $(LIBPREFIX)smapi.so.$(VERH) ;\
	$(LN) $(LNOPT) $(LIBPREFIX)smapi.so.$(VER) $(LIBPREFIX)smapi.so.$(VERMAJOR)
	$(LN) $(LNOPT) $(LIBPREFIX)smapi.so.$(VER) $(LIBPREFIX)smapi.so
ifneq (~$(LDCONFIG)~, ~~)
	$(LDCONFIG)
endif

else
instdyn: $(TARGET)

endif

FORCE:

install-h-dir: FORCE
	-$(MKDIR) $(MKDIROPT) $(INCDIR)
	-$(MKDIR) $(MKDIROPT) $(INCDIR)$(DIRSEP)smapi

%.h: FORCE
	-$(INSTALL) $(IIOPT) $@ $(INCDIR)$(DIRSEP)smapi
        
install-h: install-h-dir $(HEADERS)

install: install-h instdyn
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(ISLOPT) $(TARGET) $(LIBDIR)

uninstall:
	-cd $(INCDIR)$(DIRSEP)smapi$(DIRSEP) ;\
	$(RM) $(RMOPT) $(HEADERS)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGET)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)smapi.so.$(VER)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)smapi.so.$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)smapi.so.$(VERMAJOR)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(LIBPREFIX)smapi.so

clean:
	-$(RM) $(RMOPT) *$(OBJ)
	-$(RM) $(RMOPT) *~

distclean: clean
	-$(RM) $(RMOPT) $(TARGET)
	-$(RM) $(RMOPT) $(LIBPREFIX)smapi.so.$(VER)

all: $(TARGET)
