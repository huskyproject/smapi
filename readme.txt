SMAPI; Modified Squish MSGAPI - version 1.4

Squish MSGAPI0 is copyright 1991 by Scott J. Dudley.  All rights reserved.
Modifications released to the public domain.


Contained in this archive is the source code for the Squish MSGAPI which
can be used to access Squish format message bases.  It is based heavily
on Scott Dudley's original MSGAPI0 code and was modified to support
compilation on platforms in addition to those supported by MSGAPI0.

Work on Scott Dudley's original MSGAPI0 code was performed mainly by Paul
Edwards and David Begley to produce the MSGAPI3x series of archives
(MSGAPI38 being the most well-known).  Craig Morrison also provided a bug
fix.  The objective of SMAPI was to rationalise MSGAPI38 and to avoid
future confusion in version numbering standards.

All source code modifications are in the public domain, however most of
the source files in this archive are subject to the restrictions
contained in the MSGAPI0 licensing agreement.  The licensing agreement is
contained in this archive in the file LICENCE.TXT.  If you do not have
this file you should contact Scott Dudley at FidoNet node 1:249/106 or
Internet e-mail Scott.Dudley@f106.n249.z1.fidonet.org.  In no event
should you proceed to use any of the source files in this archive without
having accepted the terms of the MSGAPI0 licensing agreement, or such
other agreement as you are able to reach with the author.


TRADEMARKS

Squish, SquishMail and Maximus are trademarks of Scott J. Dudley.


SUPPORTED COMPILERS

The following makefiles can be used to compile this release of SMAPI:

Makefile       Target library   Compiler
------------   --------------   -------------------------------------------
MAKEFILE.BCD   SMAPIBCD.LIB     Borland C++ for DOS 3.1
MAKEFILE.BCO   SMAPIBCO.LIB     Borland C++ for OS/2 1.0
MAKEFILE.BSD   LIBSMAPIBSD.A    GNU C for FreeBSD 2.7.x
MAKEFILE.DJG   SMAPIDJG.A       GNU C for MS-DOS 2.7.x (DJGPP)
MAKEFILE.EMO   SMAPIEMO.LIB     GNU C for OS/2 2.7.x (EMX 0.9)
MAKEFILE.HCO   SMAPIHCO.LIB     MetaWare High C for OS/2 3.2
MAKEFILE.IBO   SMAPIIBO.LIB     IBM C/Set++ for OS/2 2.0
MAKEFILE.LNX   SMAPILNX.A       GNU C for Linux 2.7.x
MAKEFILE.QCD   SMAPIQCD.LIB     Microsoft QuickC for DOS 2.5 (makefile
                                requires either the Microsoft Macro
                                Assembler, MASM, or the Borland Turbo
                                Assembler, TASM)
MAKEFILE.RXW   SMAPIRXW.LIB     GNU C for Windows NT 2.7.x (RSXNT/EMX)
MAKEFILE.WCD   SMAPIWCD.LIB     WATCOM C/C++ for 16-bit DOS 10.x
MAKEFILE.WCO   SMAPIWCO.LIB     WATCOM C/C++ for 32-bit OS/2 10.x
MAKEFILE.WCW   SMAPIWCW.LIB     WATCOM C/C++ for 32-bit Windows NT 10.x
MAKEFILE.WCX   SMAPIWCX.LIB     WATCOM C/C++ for 32-bit DOS 10.x

Note that the Linux and FreeBSD makefiles must be in UNIX text file
format (linefeeds only; no carriage returns), in addition to COMPILER.H
and MSGAPI.H.


SUPPORT

SMAPI is no longer supported by Andrew Clarke.


PORTING

If you are currently porting, or wish to port, SMAPI to other compiler
suites or operating platforms, you may wish to contact Andrew via the
above channels.


OBTAINING SMAPI

The latest release of SMAPI is available for file request (FREQ) from
FidoNet node 3:635/728.  It can also be downloaded from Internet site
ftp.zws.com from the /pub/msged/ directory.  The archive name for SMAPI
release 1.4 will be named SMAPI14.ZIP, and so on.
