# $Id$
#
# Legacy makefile for building on BeOS with gcc
# No support for the Husky build environment

# Uncomment this for compile info for gdb and display more compiler warnings
DEBUG	= -g -pedantic

# Uncomment this for strip all simbols from binaries to reduce file sizes
#STRIP	= -s


## programs ##

CC	= gcc
AR	= ar
RANLIB	= ranlib
RM	= rm
INSTALL	= install


## file suffixes ##

_C	= .c
_OBJ	= .o
_LIB	= .a
_EXE	= .exe
_DLL	= .dll


## program options ##

# compiler defines
CDEFS	=
# common compiler options
CFLAGS	= -Wall -O3
# on i386, you may add this
#CFLAGS	+= -fomit-frame-pointer -fstrength-reduce -fPIC -m486

# options for compile only (link is off)
OBJOPT	= -c
# options for build static library
LIBOPT	= -static
# options for build dll
DLLOPT  = -shared
# option for specify output file name
NAMEOPT	= -o
# options for $(AR) program
ARFLAGS	= r
# options for $(RM) program
RMFLAGS	= -f


## Library filename elements ##

# Prefix to construct static library name
LIBPREFIX=lib
# Prefix to construct dll name
DLLPREFIX=
# Suffix to construct library name
LIBSUFFIX=be
# Suffix to construct dll name
DLLSUFFIX=


## make directives ##

.PHONY:	distclean clean all programs default


## Default make rule ##

default: all


## include common makefiles part ##

include makefile.inc


## Additions (add into end of strings) ##

# libraries need to build binary file
LIBS	+=

# Append includes dir and debug code generate options (if defined)
CFLAGS	+= -I$(H_DIR) -I../../huskylib $(DEBUG)


## make rules ##

all: $(TARGETLIB)

%$(_EXE): $(SRC_DIR)%$(_C) $(TARGETLIB)
	$(CC) $(STRIP) $(CFLAGS) $(CDEFS) $(NAMEOPT)$@ $^ $(LIBS)

%$(_OBJ): $(SRC_DIR)%$(_C)
	$(CC) $(CFLAGS) $(CDEFS) $(OBJOPT) $<
	$(AR) $(ARFLAGS) $(TARGETLIB) $@

$(TARGETLIB): $(OBJS)

clean:
	-$(RM) $(RMOPT) *$(_OBJ)

distclean:
	-$(RM) $(RMOPT) $(TARGETDLL) $(TARGETLIB) $(PROGRAMS)
