/*
 *  SMAPI; Modified Squish MSGAPI
 *
 *  Squish MSGAPI0 is copyright 1991 by Scott J. Dudley.  All rights reserved.
 *  Modifications released to the public domain.
 *
 *  Use of this file is subject to the restrictions contain in the Squish
 *  MSGAPI0 licence agreement.  Please refer to licence.txt for complete
 *  details of the licencing restrictions.  If you do not find the text
 *  of this agreement in licence.txt, or if you do not have this file,
 *  you should contact Scott Dudley at FidoNet node 1:249/106 or Internet
 *  e-mail Scott.Dudley@f106.n249.z1.fidonet.org.
 *
 *  In no event should you proceed to use any of the source files in this
 *  archive without having accepted the terms of the MSGAPI0 licensing
 *  agreement, or such other agreement as you are able to reach with the
 *  author.
 */

#ifndef __COMPILER_H__
#define __COMPILER_H__

#if defined(OS2) && !defined(_MSC_VER)
#ifndef __386__
#define __386__
#endif
#ifndef __FLAT__
#define __FLAT__
#endif
#endif

#ifdef MSDOS

#define EXPENTRY pascal

/* WATCOM has both M_I86xxx and __modeltype__ macros */

#ifdef M_I86SM
#ifndef __SMALL__
#define __SMALL__
#endif
#endif

#ifdef M_I86MM
#ifndef __MEDIUM__
#define __MEDIUM__
#endif
#endif

#ifdef M_I86CM
#ifndef __COMPACT__
#define __COMPACT__
#endif
#endif

#ifdef M_I86LM
#ifndef __LARGE__
#define __LARGE__
#endif
#endif

#ifdef M_I86HM
#ifndef __HUGE__
#define __HUGE__
#endif
#endif

#endif

/* Handle 386 "flat" memory model */

#ifdef __FLAT__

/* Other macros may get defined by braindead compilers */

#ifdef __SMALL__
#undef __SMALL__
#endif

#ifdef __TINY__
#undef __TINY__
#endif

#ifdef __MEDIUM__
#undef __MEDIUM__
#endif

#ifdef __COMPACT__
#undef __COMPACT__
#endif

#ifdef __LARGE__
#undef __LARGE__
#endif

#ifdef __HUGE__
#undef __HUGE__
#endif

/*
 *  Code is really "near", but "far" in this context means that we
 *  want a 32-bit pointer (vice 16-bit).
 */

#define __FARCODE__
#define __FARDATA__

/* Everything should be "near" in the flat model */

#ifdef near
#undef near
#define near
#endif

#ifdef far
#undef far
#define far near
#endif

#ifdef huge
#undef huge
#define huge near
#endif

#endif

#if defined(__SMALL__) || defined(__TINY__)
#define __NEARCODE__
#define __NEARDATA__
#endif

#ifdef __MEDIUM__
#define __FARCODE__
#define __NEARDATA__
#endif

#ifdef __COMPACT__
#define __NEARCODE__
#define __FARDATA__
#endif

#if defined(__LARGE__) || defined(__HUGE__)
#define __FARCODE__
#define __FARDATA__
#endif

/*
 *  Compiler-specific stuff:
 *
 *  _stdc - Standard calling sequence.  This should be the type of function
 *          required for function pointers for qsort() et al.
 *
 *  _fast - Fastest calling sequence supported.  If the default calling
 *          sequence is the fastest, or if your compiler only has one,
 *          define this to nothing.
 *
 *  _intr - For defining interrupt functions.  For some idiotic reason, MSC
 *          requires that interrupt routines be declared as "cdecl interrupt",
 *          instead of just "interrupt".
 */

#if defined(__TURBOC__) && defined(__MSDOS__)

/* Borland Turbo C/C++ for MS-DOS */

#pragma option -a-

#define _stdc cdecl
#define _intr interrupt far
#define _intcast void (_intr *)()
#define _veccast _intcast
#define _fast pascal
#define _loadds

#elif defined(__WATCOMC__) && defined(MSDOS)

/* WATCOM C/C++ for MS-DOS */

#pragma pack(1)

#define _stdc cdecl
#define _intr interrupt far
#define _intcast void (_intr *)()
#define _veccase _intcast
#define _fast pascal

#ifdef __FLAT__
#define farread read
#define farwrite write
#endif

#elif defined(_MSC_VER)

/* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */

#pragma pack(1)

#define _stdc cdecl
#define _intr cdecl interrupt far
#define _intcast void (_intr *)()
#define _veccast _intcast

#if _MSC_VER >= 600
#define _fast _fastcall
#else
#define _fast pascal
#endif

#define unlock(a,b,c) unused(a)
#define lock(a,b,c) 0

#ifdef OS2
#define farread read
#define farwrite write
#define EXPENTRY pascal far
#endif

#elif defined(__WATCOMC__) && defined(__OS2__)

/* WATCOM C/C++ for OS/2 */

#pragma pack(1)

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast

#define farread read
#define farwrite write

#define EXPENTRY _System

#elif defined(__WATCOMC__) && defined(__NT__)

/* WATCOM C/C++ for Windows NT */

#pragma pack(1)

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast

#define farread read
#define farwrite write

#define EXPENTRY pascal

#elif defined(__HIGHC__)

/* MetaWare High C/C++ for OS/2 */

#pragma pack(1)

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast
#define pascal
#define near
#define far

#define farread read
#define farwrite write
#define unlock(a,b,c) unused(a)
#define lock(a,b,c) 0

#define EXPENTRY

#elif defined(__EMX__)

/* EMX for 32-bit OS/2 (MS-DOS may be supported later) */

#pragma pack(1)

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast
#define _loadds
#define cdecl
#define pascal
#define near
#define far

#define farread read
#define farwrite write
#define mymkdir __mkdir
#define unlock(a,b,c) unused(a)
#define lock(a,b,c) 0

extern int __mkdir (__const__ char *name);

#define EXPENTRY

#elif defined(__TURBOC__) && defined(__OS2__)

/* Borland C/C++ for OS/2 */

#pragma option -a1

#define _stdc cdecl
#define _intr
#define _intcast
#define _veccast
#define _fast pascal
#define _loadds
#define near
#define far

#define farread read
#define farwrite write

#define EXPENTRY _syscall

#elif defined(__IBMC__)

/* IBM C/Set++ for OS/2 */

#pragma pack(1)

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast
#define _loadds

#define cdecl
#define pascal
#define near
#define far

#define farread read
#define farwrite write
#define unlock(a,b,c) unused(a)
#define lock(a,b,c) 0

#define EXPENTRY pascal far

#elif defined(UNIX)

/* Linux, FreeBSD, etc. */

#pragma pack(1)

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast
#define _loadds

#define cdecl
#define pascal
#define near
#define far

#define farread read
#define farwrite write

#ifdef __FreeBSD__
#define mymkdir(a) mkdir((a), 0)
#else
#define mymkdir(a) __mkdir((a), 0)
#endif

#define unlock(a,b,c) unused(a)
#define lock(a,b,c) 0
#define tell(a) lseek((a),0,SEEK_CUR)
#define stricmp strcasecmp
#define O_BINARY 0
#define SH_DENYNONE 0
#define sopen(a,b,c,d) open((a),(b),(d))

#define EXPENTRY

#elif defined(__DJGPP__)

/* DJGPP for MS-DOS */

#pragma pack(1)

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast
#define _loadds

#define cdecl
#define pascal
#define near
#define far

#define farread read
#define farwrite write

#define mymkdir(a) mkdir((a), 0)

#define stricmp strcasecmp
#define SH_DENYNONE 0

#include <unistd.h>
#include <io.h>

#elif defined(SASC)

/* SAS C for AmigaDOS */

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast
#define _loadds

#define cdecl
#define pascal
#define near
#define far

#define farread read
#define farwrite write
#define mymkdir(a) mkdir((a))
#define unlock(a,b,c) unused(a)
#define lock(a,b,c) 0
#define SH_DENYNONE 0
#define sopen(a,b,c,d) open((a),(b),(d))

#define EXPENTRY

#else

#error compiler.h: Unknown compiler!

#endif

#ifndef mymkdir
#define mymkdir mkdir
#endif

#define NO_MKTIME
#define NO_STRFTIME

#endif