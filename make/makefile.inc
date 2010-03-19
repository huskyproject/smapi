# $Id$
#
# Common makefiles part (include-file) for smapi

# Use defines before including this file:
# _OBJ	object files suffix begins with point (".o", ".obj", ... )
# _LIB	library files suffix begins with point (".a", ".lib", ... )
# _DLL	dinamic linking library files suffix begins with point (".dll", ".so", ...)
# LIBPREFIX  library filename prefix ("lib", "", ...)
# LIBSUFFIX  library filename suffix (compiler ID, three characters, somewhere empty)
# DLLPREFIX  dinamic linking library prefix (usually empty)
# DLLSUFFIX  dinamic linking library suffix (compiler ID or empty)

# Library name
LIBNAME	= smapi

# Version
VERMAJOR= 2
VERMINOR= 5
VERPATCH= 0
VERH	= $(VERMAJOR).$(VERMINOR)
VER	= $(VERH).$(VERPATCH)

# Sources directory
SRC_DIR	=../src/
# Headers directory
H_DIR	=../smapi/
# Object directory
O=$(OBJOUT)
# Binary directory
B=$(BINOUT)
# Library directory
L=$(LIBOUT)
# Prefix for headers. Do not use H_DIR for this.
H=$(H_PFX)

# Header files (please sort list to easy check by human)
HEADERS = $(H)api_jam.h $(H)api_jamp.h $(H)api_sdm.h $(H)api_sdmp.h \
	  $(H)api_sq.h $(H)api_sqp.h $(H)apidebug.h $(H)msgapi.h $(H)old_msg.h

# Object files: library (please sort list to easy check by human)
OBJS1	= $(O)api_jam$(_OBJ) $(O)api_sdm$(_OBJ) $(O)msgapi$(_OBJ) $(O)sq_area$(_OBJ)
OBJS2	= $(O)sq_hash$(_OBJ) $(O)sq_help$(_OBJ) $(O)sq_idx$(_OBJ) $(O)sq_kill$(_OBJ)
OBJS3	= $(O)sq_lock$(_OBJ) $(O)sq_misc$(_OBJ) $(O)sq_msg$(_OBJ) $(O)sq_read$(_OBJ)
OBJS4	= $(O)sq_uid$(_OBJ) $(O)sq_write$(_OBJ) $(O)structrw$(_OBJ)
OBJS	= $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)

# Binary file(s) to build from sources
PROGRAMS=

# Usually compose from LIBPREFIX, LIBSUFFIX and LIB.
TARGETLIB = $(L)$(LIBPREFIX)$(LIBNAME)$(LIBSUFFIX)$(_LIB)
TARGETDLL = $(B)$(DLLPREFIX)$(LIBNAME)$(DLLSUFFIX)$(_DLL)
TARGET_H  = api_jam.h msgapi.h

# List of HUSKY libraries required to build binary file(s)
LIBS	=

