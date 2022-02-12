# smapi/Makefile
#
# This file is part of smapi, part of the Husky fidonet software project
# Use with GNU make v.3.82 or later
# Requires: husky enviroment
#

# The library object files
# Please sort the list to make checking it by human easy

# Static and dynamic target libraries
smapi_TARGETLIB := $(L)$(LIBPREFIX)$(smapi_LIBNAME)$(LIBSUFFIX)$(_LIB)
smapi_TARGETDLL := $(B)$(DLLPREFIX)$(smapi_LIBNAME)$(DLLSUFFIX)$(_DLL)

ifeq ($(DYNLIBS),1)
    ifeq ($(findstring Windows,$(OS)),)
        smapi_TARGET = $(smapi_TARGETDLL).$(smapi_VER)
    else
        smapi_TARGET = $(smapi_TARGETDLL)
    endif
else
    smapi_TARGET = $(smapi_TARGETLIB)
endif

smapi_TARGET_BLD = $(smapi_BUILDDIR)$(smapi_TARGET)
smapi_TARGET_DST = $(LIBDIR_DST)$(smapi_TARGET)

smapi_CDEFS := $(CDEFS) -I$(smapi_ROOTDIR)$(smapi_H_DIR) -I$(huskylib_ROOTDIR)

smapi_LIBS := $(huskylib_TARGET_BLD)

.PHONY: smapi_build smapi_install smapi_install-dynlib smapi_uninstall \
	smapi_clean smapi_distclean smapi_depend smapi_rm_OBJS smapi_rm_BLD \
	smapi_rm_DEP smapi_rm_DEPS

smapi_build: $(smapi_TARGET_BLD)

ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), distclean)
ifneq ($(MAKECMDGOALS), uninstall)
    include $(smapi_DEPS)
endif
endif
endif

$(smapi_TARGET_BLD): $(smapi_OBJDIR)$(smapi_TARGET)
	$(LN) $(LNHOPT) $< $(smapi_BUILDDIR)

$(smapi_OBJDIR)$(smapi_TARGETLIB): $(smapi_ALL_OBJS) | do_not_run_make_as_root
	cd $(smapi_OBJDIR); $(AR) $(AR_R) $(smapi_TARGETLIB) $(^F)
ifdef RANLIB
	cd $(smapi_OBJDIR); $(RANLIB) $(smapi_TARGETLIB)
endif

ifeq ($(DYNLIBS),1)
$(smapi_OBJDIR)$(smapi_TARGET): $(smapi_ALL_OBJS) $(smapi_LIBS) | do_not_run_make_as_root
    ifeq ($(filter gcc clang,$(MKSHARED)),)
		$(LD) $(LFLAGS) -o $@ $^
    else
		$(CC) $(LFLAGS) -shared -Wl,-soname,$(smapi_TARGET) -o $@ $^
    endif
endif

$(smapi_ALL_OBJS): $(smapi_OBJDIR)%$(_OBJ): $(smapi_SRCDIR)%.c | $(smapi_OBJDIR)
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
	-$(RM) $(RMOPT) $(smapi_BUILDDIR)$(smapi_TARGETLIB)
	-$(RM) $(RMOPT) $(smapi_BUILDDIR)$(smapi_TARGETDLL)*

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
