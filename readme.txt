SMAPI; Modified Squish MSGAPI - version 2.3-current

This version of the SMAPI library is part of the HUSKY fidosoft project.
This version are development (unstable). Use carefully!


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
Makefile       libsmapi.*       Any (huskymak.cfg required)
makefile.bcd   smapibcd.lib     Borland C++ for DOS 3.1
makefile.bco   smapibco.lib     Borland C++ for OS/2 1.0
makefile.bcw   smapibco.lib     Borland C++ for Windows 4.0
makefile.be    libsmapibe.a     GNU C for BeOS 2.7.x
makefile.bsd   libsmapibsd.a    GNU C for FreeBSD 2.7.x
makefile.bsd4  libsmapibsd.a    GNU C for FreeBSD 2.7.x, use make (FreeBSD 4.*)
makefile.cyg   libsmapicyg.a    GNU C for Cygwin 2.7.x
makefile.djg   smapidjg.a       GNU C for MS-DOS 2.7.x (DJGPP)
makefile.emo   smapiemo.lib     GNU C for OS/2 2.7.x (EMX 0.9) with OMF-Style
                                linkage
makefile.emx   smapiemx.a       GNU C for OS/2 2.7.x (EMX 0.9) a.out-style
                                linkage (EMX Runtime)
makefile.hco   smapihco.lib     MetaWare High C for OS/2 3.2
makefile.ibo   smapiibo.lib     IBM C/Set++ for OS/2 2.0
makefile.lnx   smapilnx.a       GNU C for Linux 2.7.x
makefile.mgw   libsmapimgw.a    Mingw32 for NT
makefile.mvc   smapimvc.lib     Microsoft Visual C/C++ 6.0
makefile.mvcdll smapimvc.dll    Microsoft Visual C/C++ 6.0
                                (shared library produced)
makefile.qcd   smapiqcd.lib     Microsoft QuickC for DOS 2.5 (makefile
                                requires either the Microsoft Macro
                                Assembler, MASM, or the Borland Turbo
                                Assembler, TASM)
makefile.rxw   smapirxw.lib     GNU C for Windows NT 2.7.x (RSXNT/EMX)
makefile.sun   libsmapisun.a    GNU C for Solaris 2.7.x
makefile.unx   libsmapiunix.a   Generic Unix Makefile
makefile.wcd   smapiwcd.lib     WATCOM C/C++ for 16-bit DOS 10.x
makefile.wco   smapiwco.lib     WATCOM C/C++ for 32-bit OS/2 10.x
makefile.wcw   smapiwcw.lib     WATCOM C/C++ for 32-bit Windows NT 10.x
makefile.wcx   smapiwcx.lib     WATCOM C/C++ for 32-bit DOS 10.x

Use GNU version of make (gmake on many platforms) if don't specified other.

Note that the Linux and FreeBSD makefiles must be in UNIX text file
format (linefeeds only; no carriage returns), in addition to COMPILER.H
and MSGAPI.H.

The Linux Makefile creates a shared library and depends on GCC, while the
generic Unix Makefile (makefile.unx) should work on any Unix system with
any set of cc, ld and ranlib, and creates a static libarry.


SUPPORT

SMAPI is no longer supported by Andrew Clarke.


PORTING

If you are currently porting, or wish to port, SMAPI to other compiler
suites or operating platforms, you may wish to contact Andrew via the
above channels.


OBTAINING SMAPI

Latest version of the Husky Smapi available at the Husky homepage:
http://husky.physcip.uni-stuttgart.de.

The latest release of original SMAPI is available for file request (FREQ) from
FidoNet node 3:635/728.  It can also be downloaded from Internet site
ftp.zws.com from the /pub/msged/ directory.  The archive name for SMAPI
release 1.4 will be named SMAPI14.ZIP, and so on.
