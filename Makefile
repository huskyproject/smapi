# Makefile for the Husky build environment
# Makefile for the Husky build environment

# include Husky-Makefile-Config
ifeq ($(DEBIAN), 1)
# Every Debian-Source-Paket has one included.
include /usr/share/husky/huskymak.cfg
else
include ../huskymak.cfg
endif

ifeq ($(OSTYPE), UNIX)
  LIBPREFIX=lib
  DLLPREFIX=lib
endif

include makefile.inc

ifeq ($(DEBUG), 1)
  CFLAGS=$(WARNFLAGS) $(DEBCFLAGS)
# CFLAGS=$(WARNFLAGS) $(DEBCFLAGS) -DNO_LOCKING
else
  CFLAGS=$(WARNFLAGS) $(OPTCFLAGS)
# CFLAGS=$(WARNFLAGS) $(OPTCFLAGS) -DNO_LOCKING
endif

CDEFS=-D$(OSTYPE) $(ADDCDEFS) -I$(INCDIR)

ifeq ($(DYNLIBS), 1)
all: $(TARGETLIB) $(TARGETDLL).$(VER)
else
all: $(TARGETLIB)
endif


.c$(_OBJ):
	$(CC) $(CFLAGS) $(CDEFS)  $<

$(TARGETLIB): $(OBJS)
	$(AR) $(AR_R) $(TARGETLIB) $?
ifdef RANLIB
	$(RANLIB) $(TARGETLIB)
endif                                                             

ifeq ($(DYNLIBS), 1)
  ifeq (~$(MKSHARED)~,~ld~)
$(TARGETDLL).$(VER): $(OBJS)
	$(LD) $(OPTLFLAGS) -o $(TARGETDLL).$(VER) $(OBJS)
  else
$(TARGETDLL).$(VER): $(OBJS)
	$(CC) -shared -Wl,-soname,$(TARGETDLL).$(VERH) \
          -o $(TARGETDLL).$(VER) $(OBJS)
  endif

instdyn: $(TARGET) $(LIBPREFIX)smapi.so.$(VER)
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(ILOPT) $(TARGETDLL).$(VER) $(LIBDIR)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGETDLL).$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGETDLL)
# Changed the symlinks from symlinks with full path to just symlinks.
# Better so :)
	cd $(LIBDIR) ;\
	$(LN) $(LNOPT) $(TARGETDLL).$(VER) $(TARGETDLL).$(VERH) ;\
	$(LN) $(LNOPT) $(TARGETDLL).$(VER) $(TARGETDLL)
ifneq (~$(LDCONFIG)~, ~~)
	$(LDCONFIG)
endif

else
instdyn: $(TARGETLIB)

endif

FORCE:

install-h-dir: FORCE
	-$(MKDIR) $(MKDIROPT) $(INCDIR)
	-$(MKDIR) $(MKDIROPT) $(INCDIR)$(DIRSEP)$(LIBNAME)

%.h: FORCE
	-$(INSTALL) $(IIOPT) $@ $(INCDIR)$(DIRSEP)$(LIBNAME)
        
install-h: install-h-dir $(HEADERS)

install: install-h instdyn
	-$(MKDIR) $(MKDIROPT) $(LIBDIR)
	$(INSTALL) $(ISLOPT) $(TARGETLIB) $(LIBDIR)

uninstall:
	-cd $(INCDIR)$(DIRSEP)$(LIBNAME)$(DIRSEP) ;\
	$(RM) $(RMOPT) $(HEADERS)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGETLIB)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGETDLL).$(VER)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGETDLL).$(VERH)
	-$(RM) $(RMOPT) $(LIBDIR)$(DIRSEP)$(TARGETDLL)

clean:
	-$(RM) $(RMOPT) *$(_OBJ)

distclean: clean
	-$(RM) $(RMOPT) $(TARGETLIB)
	-$(RM) $(RMOPT) $(TARGETDLL).$(VER)
