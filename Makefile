# smapi/Makefile
#
# This file is part of smapi, part of the Husky fidonet software project
# Use with GNU make v.3.82 or later
# Requires: husky enviroment
#

# Version
smapi_g1:=$(GREP) -Po 'define\s+SMAPI_VER_MAJOR\s+\K\d+'
smapi_g2:=$(GREP) -Po 'define\s+SMAPI_VER_MINOR\s+\K\d+'
smapi_g3:=$(GREP) -Po 'define\s+SMAPI_VER_PATCH\s+\K\d+'
smapi_g4:=$(GREP) -Po 'char\s+cvs_date\[\]\s*=\s*"\K\d+-\d+-\d+'
smapi_VERMAJOR := $(shell $(smapi_g1) $(smapi_ROOTDIR)$(smapi_H_DIR)msgapi.h)
smapi_VERMINOR := $(shell $(smapi_g2) $(smapi_ROOTDIR)$(smapi_H_DIR)msgapi.h)
smapi_VERPATCH := $(shell $(smapi_g3) $(smapi_ROOTDIR)$(smapi_H_DIR)msgapi.h)
smapi_VERH     := $(smapi_VERMAJOR).$(smapi_VERMINOR)
smapi_cvsdate  := $(shell $(smapi_g4) $(smapi_ROOTDIR)cvsdate.h)
smapi_reldate  := $(subst -,,$(smapi_cvsdate))
smapi_VER      := $(smapi_VERH).$(smapi_reldate)

# Object files of the library
# Please sort the list to make checking it by human easy
smapi_OBJFILES = $(O)api_jam$(_OBJ) $(O)api_sdm$(_OBJ) $(O)msgapi$(_OBJ) \
	$(O)sq_area$(_OBJ) $(O)sq_hash$(_OBJ) $(O)sq_help$(_OBJ) $(O)sq_idx$(_OBJ) \
	$(O)sq_kill$(_OBJ) $(O)sq_lock$(_OBJ) $(O)sq_misc$(_OBJ) $(O)sq_msg$(_OBJ) \
	$(O)sq_read$(_OBJ) $(O)sq_uid$(_OBJ) $(O)sq_write$(_OBJ) $(O)structrw$(_OBJ)

smapi_OBJS := $(addprefix $(smapi_OBJDIR),$(smapi_OBJFILES))

smapi_DEPS := $(smapi_OBJFILES)
ifdef O
    smapi_DEPS := $(smapi_DEPS:$(O)=)
endif
ifdef _OBJ
    smapi_DEPS := $(smapi_DEPS:$(_OBJ)=$(_DEP))
else
    smapi_DEPS := $(addsuffix $(_DEP),$(smapi_DEPS))
endif
smapi_DEPS := $(addprefix $(smapi_DEPDIR),$(smapi_DEPS))

# Static and dynamic target libraries
smapi_TARGETLIB := $(L)$(LIBPREFIX)$(smapi_LIBNAME)$(LIBSUFFIX)$(_LIB)
smapi_TARGETDLL := $(B)$(DLLPREFIX)$(smapi_LIBNAME)$(DLLSUFFIX)$(_DLL)

ifeq ($(DYNLIBS), 1)
    smapi_TARGET = $(smapi_TARGETDLL).$(smapi_VER)
else
    smapi_TARGET = $(smapi_TARGETLIB)
endif

smapi_TARGET_BLD = $(smapi_BUILDDIR)$(smapi_TARGET)
smapi_TARGET_DST = $(LIBDIR_DST)$(smapi_TARGET)

smapi_CDEFS := $(CDEFS) -I$(smapi_ROOTDIR)$(smapi_H_DIR) -I$(huskylib_ROOTDIR)


.PHONY: smapi_all smapi_install smapi_install-dynlib smapi_uninstall \
	smapi_clean smapi_distclean smapi_depend smapi_rm_OBJS smapi_rm_BLD \
	smapi_rm_DEP smapi_rm_DEPS

smapi_all: $(smapi_TARGET_BLD)

ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), distclean)
ifneq ($(MAKECMDGOALS), uninstall)
    include $(smapi_DEPS)
endif
endif
endif

$(smapi_TARGET_BLD): $(smapi_OBJDIR)$(smapi_TARGET)
	$(LN) $(LNHOPT) $< $(smapi_BUILDDIR)

$(smapi_OBJDIR)$(smapi_TARGETLIB): $(smapi_OBJS) | do_not_run_make_as_root
	cd $(smapi_OBJDIR); $(AR) $(AR_R) $(smapi_TARGETLIB) $(^F)
ifdef RANLIB
	cd $(smapi_OBJDIR); $(RANLIB) $(smapi_TARGETLIB)
endif

$(smapi_OBJDIR)$(smapi_TARGETDLL).$(smapi_VER): $(smapi_OBJS) | do_not_run_make_as_root
ifeq (~$(MKSHARED)~,~ld~)
	$(LD) $(LFLAGS) -o $(smapi_OBJDIR)$(smapi_TARGETDLL).$(smapi_VER) $(smapi_OBJS)
else
	$(CC) $(LFLAGS) -shared -Wl,-soname,$(smapi_TARGETDLL).$(smapi_VER) \
	-o $(smapi_OBJDIR)$(smapi_TARGETDLL).$(smapi_VER) $(smapi_OBJS)
endif

$(smapi_OBJS): $(smapi_OBJDIR)%$(_OBJ): $(smapi_SRCDIR)%.c | $(smapi_OBJDIR)
	$(CC) $(CFLAGS) $(smapi_CDEFS) -o $(smapi_OBJDIR)$*$(_OBJ) $(smapi_SRCDIR)$*.c

$(smapi_OBJDIR): | do_not_run_make_as_root $(smapi_BUILDDIR)
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@


# Install
smapi_install: smapi_install-dynlib ;

ifneq ($(DYNLIBS), 1)
smapi_install-dynlib: ;
else
    ifneq ($(strip $(LDCONFIG)),)
        smapi_install-dynlib: \
            $(LIBDIR_DST)$(smapi_TARGETDLL).$(smapi_VER)
		-@$(LDCONFIG) >& /dev/null || true
    else
        smapi_install-dynlib: \
            $(LIBDIR_DST)$(smapi_TARGETDLL).$(smapi_VER) ;
    endif

    $(LIBDIR_DST)$(smapi_TARGETDLL).$(smapi_VER): \
        $(smapi_BUILDDIR)$(smapi_TARGETDLL).$(smapi_VER) | $(DESTDIR)$(LIBDIR)
		$(INSTALL) $(ILOPT) $(smapi_BUILDDIR)$(smapi_TARGETDLL).$(smapi_VER) \
		$(DESTDIR)$(LIBDIR); \
		cd $(DESTDIR)$(LIBDIR); \
		$(TOUCH) $(smapi_TARGETDLL).$(smapi_VER); \
		$(LN) $(LNOPT) $(smapi_TARGETDLL).$(smapi_VER) \
		$(smapi_TARGETDLL).$(smapi_VERH); \
		$(LN) $(LNOPT) $(smapi_TARGETDLL).$(smapi_VER) $(smapi_TARGETDLL)
endif


# Clean
smapi_clean: smapi_rm_OBJS
	-[ -d "$(smapi_OBJDIR)" ] && $(RMDIR) $(smapi_OBJDIR) || true

smapi_rm_OBJS:
	-$(RM) $(RMOPT) $(smapi_OBJDIR)*


# Distclean
smapi_distclean: smapi_clean smapi_rm_BLD
	-[ -d "$(smapi_BUILDDIR)" ] && $(RMDIR) $(smapi_BUILDDIR) || true

smapi_rm_BLD: smapi_rm_DEP
	-$(RM) $(RMOPT) $(smapi_BUILDDIR)$(smapi_TARGET)

smapi_rm_DEP: smapi_rm_DEPS
	-[ -d "$(smapi_DEPDIR)" ] && $(RMDIR) $(smapi_DEPDIR) || true

smapi_rm_DEPS:
	-$(RM) $(RMOPT) $(smapi_DEPDIR)*


# Uninstall
ifeq ($(DYNLIBS), 1)
    smapi_uninstall:
		-$(RM) $(RMOPT) $(DESTDIR)$(LIBDIR)$(DIRSEP)$(smapi_TARGETDLL)*
else
    smapi_uninstall: ;
endif

# Depend
smapi_depend: $(smapi_DEPS) ;

# Build dependency makefiles for every source file
$(smapi_DEPS): $(smapi_DEPDIR)%$(_DEP): $(smapi_SRCDIR)%.c | $(smapi_DEPDIR)
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(smapi_CDEFS) $< > $@.$$$$; \
	sed 's,\($*\)$(_OBJ)[ :]*,$(smapi_OBJDIR)\1$(_OBJ) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(smapi_DEPDIR): | do_not_run_depend_as_root $(smapi_BUILDDIR)
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@

$(smapi_BUILDDIR):
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@
