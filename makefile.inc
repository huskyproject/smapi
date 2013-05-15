# $Id$
#
# Common makefiles part (include-file)

VERMAJOR= 2
VERMINOR= 4
VERH =  2.4
VER  =  2.4.0

OBJS	= 1stchar$(OBJ) api_sdm$(OBJ) sq_area$(OBJ) sq_hash$(OBJ) sq_help$(OBJ) sq_idx$(OBJ)  sq_kill$(OBJ) sq_lock$(OBJ) sq_misc$(OBJ) sq_msg$(OBJ) sq_read$(OBJ) sq_uid$(OBJ) sq_write$(OBJ) api_jam$(OBJ) cvtdate$(OBJ) date2bin$(OBJ) dosdate$(OBJ) fexist$(OBJ) ffind$(OBJ) flush$(OBJ) locking$(OBJ) months$(OBJ) msgapi$(OBJ) parsenn$(OBJ) patmat$(OBJ) qksort$(OBJ) strextra$(OBJ) strftim$(OBJ) strocpy$(OBJ) structrw$(OBJ) trail$(OBJ) weekday$(OBJ) setfsize$(OBJ) tdelay$(OBJ) genmsgid$(OBJ)

HEADERS = api_brow.h api_jam.h compiler.h ffind.h msgapi.h patmat.h prog.h progprot.h stamp.h typedefs.h unused.h

GMOBJS	= genmsgid$(OBJ) fexist$(OBJ) ffind$(OBJ) trail$(OBJ) patmat$(OBJ)

GNMSGID	= gnmsgid

PROGRAMS= $(GNMSGID)$(EXE)

WARNFLAGS+= -fno-strict-aliasing
