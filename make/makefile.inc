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

OBJS1	= api_jam$(_OBJ) api_sdm$(_OBJ) msgapi$(_OBJ) sq_area$(_OBJ)
OBJS2	= sq_hash$(_OBJ) sq_help$(_OBJ) sq_idx$(_OBJ) sq_kill$(_OBJ)
OBJS3	= sq_lock$(_OBJ) sq_misc$(_OBJ) sq_msg$(_OBJ) sq_read$(_OBJ)
OBJS4	= sq_uid$(_OBJ) sq_write$(_OBJ) structrw$(_OBJ)
OBJS	= $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)

#HEADERS = api_brow.h api_jam.h msgapi.h
HEADERS = api_jam.h msgapi.h

# Binary file(s) to build from sources
PROGRAMS=

SRC_DIR	=../src/
H_DIR	=../smapi/

# Usually compose from LIBPREFIX, LIBSUFFIX and LIB.
TARGETLIB = $(LIBPREFIX)$(LIBNAME)$(LIBSUFFIX)$(_LIB)
TARGETDLL = $(DLLPREFIX)$(LIBNAME)$(DLLSUFFIX)$(_DLL)
TARGET_H  = *.h

# List of HUSKY libraries required to build binary file(s)
LIBS	=
