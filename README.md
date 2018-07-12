# SMAPI - Modified Squish MSGAPI
[![Build Status](https://travis-ci.org/huskyproject/smapi.svg?branch=master)](https://travis-ci.org/huskyproject/smapi)
[![Build status](https://ci.appveyor.com/api/projects/status/hu6cl1cvfi9erx0c/branch/master?svg=true)](https://ci.appveyor.com/project/dukelsky/smapi/branch/master)

This version of the SMAPI library is part of the HUSKY fidosoft project.


Squish MSGAPI0 is copyright 1991 by Scott J. Dudley. The terms of use are
those of the GNU LESSER GENERAL PUBLIC LICENSE (LGPL). In 2003 Scott has
agreed to change the terms of use to those of the LGPL. See LICENSE for
details.

Contained in this archive is the source code for the Squish MSGAPI which
can be used to access Squish format message bases.  It is based heavily
on Scott Dudley's original MSGAPI0 code and was modified to support
compilation on platforms in addition to those supported by MSGAPI0.

Work on Scott Dudley's original MSGAPI0 code was performed mainly by Paul
Edwards and David Begley to produce the MSGAPI3x series of archives
(MSGAPI38 being the most well-known).  Craig Morrison also provided a bug
fix.  The objective of SMAPI was to rationalise MSGAPI38 and to avoid
future confusion in version numbering standards.


## TRADEMARKS

Squish, SquishMail and Maximus are trademarks of Scott J. Dudley.


## SUPPORTED COMPILERS

The following makefiles can be used to compile this release of SMAPI:

| Makefile        | Target library |   Compiler                                     |
| :---------------| :--------------| :----------------------------------------------|
| Makefile        | libsmapi.*     | Any (huskymak.cfg required)                    |
| makefile.bcd    | smapibcd.lib   | Borland C++ 3.1 for DOS                        |
| makefile.bco    | smapibco.lib   | Borland C++ 1.0 for OS/2                       |
| makefile.bcw    | smapibco.lib   | Borland C++ 4.0 for Windows                    |
| makefile.be     | libsmapibe.a   | GNU C 2.7.x for BeOS                           |
| makefile.bsd    | libsmapibsd.a  | GNU C 2.9.x for all BSD clones, use BSD make   |
| makefile.bsd4   | libsmapibsd.a  | GNU C 2.9.x for FreeBSD, use BSD make          |
|                 |                | on FreeBSD 4.x                                 |
| makefile.cyg    | libsmapicyg.a  | GNU C 2.9.x for Cygwin                         |
| makefile.djg    | smapidjg.a     | GNU C 2.7.x-3.x for MS-DOS DPMI (DJGPP)        |
| makefile.emo    | smapiemo.lib   | GNU C 2.7.x for OS/2 (EMX 0.9) and above with  |
|                 |                | OMF-Style linkage                              |
| makefile.emx    | smapiemx.a     | GNU C 2.7.x for OS/2 (EMX 0.9) a.out-style     |
|                 |                | linkage (EMX Runtime)                          |
| makefile.hco    | smapihco.lib   | MetaWare High C 3.2 for OS/2                   |
| makefile.ibo    | smapiibo.lib   | IBM C/Set++ 2.0 for OS/2                       |
| makefile.lnx    | smapilnx.a     | GNU C 2.7.x for Linux                          |
| makefile.mgw    | libsmapimgw.a  | Mingw32 for NT and cross-compilation on unixes |
| makefile.mvc    | smapimvc.lib   | Microsoft Visual C/C++ 6.0                     |
| makefile.mvcdll | smapimvc.dll   | Microsoft Visual C/C++ 6.0                     |
|                 |                | (shared library produced)                      |
| makefile.qcd    | smapiqcd.lib   | Microsoft QuickC 2.5 for DOS (makefile         |
|                 |                | requires either the Microsoft Macro            |
|                 |                | Assembler, MASM, or the Borland Turbo          |
|                 |                | Assembler, TASM)                               |
| makefile.qnx    | libsmapiqnx.*  | GNU C 2.95.3 for QNX 6.x                       |
| makefile.rxw    | smapirxw.lib   | GNU C 2.7.x for Windows NT (RSXNT/EMX)         |
| makefile.sun    | libsmapisun.a  | GNU C 2.7.x for Solaris                        |
| makefile.unx    | libsmapiunix.a | Generic Unix Makefile                          |
| makefile.wcd    | smapiwcd.lib   | WATCOM C/C++ 10.x for 16-bit DOS               |
| makefile.wco    | smapiwco.lib   | WATCOM C/C++ 10.x for 32-bit OS/2              |
| makefile.wcw    | smapiwcw.lib   | WATCOM C/C++ 10.x for 32-bit Windows NT        |
| makefile.wcx    | smapiwcx.lib   | WATCOM C/C++ 10.x for 32-bit DOS               |

Use GNU version of make (gmake on many platforms) if don't specified other.

Note that the Linux and FreeBSD makefiles must be in UNIX text file
format (linefeeds only; no carriage returns), in addition to COMPILER.H
and MSGAPI.H.

The Linux Makefile creates a shared library and depends on GCC, while the
generic Unix Makefile (makefile.unx) should work on any Unix system with
any set of cc, ld and ranlib, and creates a static libarry.


## SUPPORT

SMAPI is no longer supported by Andrew Clarke.
Post your support requests into FIDOSOFT.HUSKY or RU.HUSKY (russian) FIDOnet
echoes or create an [issue](https://github.com/huskyproject/smapi/issues).

## PORTING

If you are currently porting, or wish to port, SMAPI to other compiler
suites or operating platforms, you may wish to contact Andrew via the
above channels.
