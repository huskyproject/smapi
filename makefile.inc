# $Id$
#
# Common makefiles part (include-file)

# Use defines:
# _OBJ	object files suffix begins with point (".o", ".obj", ... )
# _LIB	library files suffix begins with point (".a", ".lib", ... )
# _DLL	dinamic linking library files suffix begins with point (".dll", ".so", ...)
# LIBPREFIX  library filename prefix ("lib", "", ...)
# LIBSUFFIX  library filename suffix (compiler ID, three characters, somewhere empty)
# DLLPREFIX  dinamic linking library prefix (usually empty)
# DLLSUFFIX  dinamic linking library suffix (compiler ID or empty)

VERH	= 2.5
VER	= $(VERH).0

OBJS1	= api_sdm$(_OBJ) sq_area$(_OBJ) sq_hash$(_OBJ) sq_help$(_OBJ)
OBJS2	= sq_idx$(_OBJ) sq_kill$(_OBJ) sq_lock$(_OBJ) sq_misc$(_OBJ)
OBJS3	= sq_msg$(_OBJ) sq_read$(_OBJ) sq_uid$(_OBJ) sq_write$(_OBJ)
OBJS4	= api_jam$(_OBJ) flush$(_OBJ) locking$(_OBJ) msgapi$(_OBJ)
OBJS5	= qksort$(_OBJ) structrw$(_OBJ)

OBJS	= $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5)

HEADERS = api_brow.h api_jam.h msgapi.h

#GMOBJS	= genmsgid$(OBJ) fexist$(OBJ) ffind$(OBJ) trail$(OBJ) patmat$(OBJ)

#GNMSGID	= gnmsgid

#PROGRAMS= $(GNMSGID)$(EXE)

# HUSKY libraries required to build binary file(s)
# Usually compose from LIBPREFIX, LIBSUFFIX and LIB.
LIBS	= 

LIBNAME	= smapi

TARGETLIB = $(LIBPREFIX)$(LIBNAME)$(LIBSUFFIX)$(_LIB)
TARGETDLL = $(DLLPREFIX)$(LIBNAME)$(DLLSUFFIX)$(_DLL)
