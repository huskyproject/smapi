# $Id$
#
# Common makefiles part (include-file)

VERH =  2.3
VER  =  2.3.0

OBJS	= api_sdm$(OBJ) sq_area$(OBJ) sq_hash$(OBJ) sq_help$(OBJ) sq_idx$(OBJ)  sq_kill$(OBJ) sq_lock$(OBJ) sq_misc$(OBJ) sq_msg$(OBJ) sq_read$(OBJ) sq_uid$(OBJ) sq_write$(OBJ) api_jam$(OBJ) flush$(OBJ) locking$(OBJ) msgapi$(OBJ) qksort$(OBJ) structrw$(OBJ)

HEADERS = api_brow.h api_jam.h msgapi.h compiler.h

#GMOBJS	= genmsgid$(OBJ) fexist$(OBJ) ffind$(OBJ) trail$(OBJ) patmat$(OBJ)

#GNMSGID	= gnmsgid

#PROGRAMS= $(GNMSGID)$(EXE)
