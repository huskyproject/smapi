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
/*
   BeOS is NOT Unix, but sometime it seem's to Be ... ;)
 */
#if defined (__BEOS__)
  #ifndef UNIX
#define UNIX
  #endif
#endif

#if defined (__WATCOMC__)
#include <direct.h>
#include <io.h>
#endif

#if defined (OS2) && !defined (_MSC_VER)
#ifndef __386__
#define __386__
#endif
#ifndef __FLAT__
#define __FLAT__
#endif
#endif

#ifdef MSDOS

#define _XPENTRY pascal
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

#endif // ifdef MSDOS
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

#endif // ifdef __FLAT__

#if defined (__SMALL__) || defined (__TINY__)
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

#if defined (__LARGE__) || defined (__HUGE__)
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

#if defined (__TURBOC__) && defined (__MSDOS__)
/* Borland Turbo C/C++ for MS-DOS */
/* for BC++ 3.1 */
#define strcasecmp stricmp
#define strncasecmp strncmpi

#define _stdc cdecl
#define _intr interrupt far
#define _intcast void(_intr *)()
#define _veccast _intcast
#define _fast pascal
#define _loadds

#ifndef _XPENTRY
#define _XPENTRY
#endif


#elif defined (__WATCOMC__) && defined (MSDOS)
/* WATCOM C/C++ for MS-DOS */

#define _stdc cdecl
#define _intr interrupt far
#define _intcast void(_intr *)()
#define _veccase _intcast
#define _fast pascal

#ifdef __FLAT__
#define farread read
#define farwrite write
#endif

#define strcasecmp stricmp
#define strncasecmp strnicmp

#elif (defined (_MSC_VER) && (_MSC_VER < 1200))
/* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */

#define _stdc cdecl
#define _intr cdecl interrupt far
#define _intcast void(_intr *)()
#define _veccast _intcast

#if _MSC_VER >= 600
#define _fast _fastcall
#else
#define _fast pascal
#endif

int unlock(int handle, long ofs, long length);
int lock(int handle, long ofs, long length);

#ifdef OS2
/* just don't use 16 bit OS/2, we doubt that it still works */
#define farread read
#define farwrite write
#define _XPENTRY pascal far
#endif

#elif defined (__WATCOMC__) && (defined (__OS2__) || defined (OS2))
/* WATCOM C/C++ for OS/2 */

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast

#define farread read
#define farwrite write

#define strcasecmp stricmp
#define strncasecmp strnicmp

#define _XPENTRY _System
#define mode_t int


#elif defined (__WATCOMC__) && defined (__NT__)
/* WATCOM C/C++ for Windows NT */

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast

#define farread read
#define farwrite write

#define strcasecmp stricmp
#define strncasecmp strnicmp

#define _XPENTRY pascal
#define mode_t int

#elif defined (__HIGHC__)
/* MetaWare High C/C++ for OS/2 */

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

#define unlock(a, b, c) unused(a)
#define lock(a, b, c) 0
#error "Don't know how to implement record locking."
/* Using an executable that does no support record locking is
   discouraged in a multitasking environment. If you want to
   do it anyway, you may uncomment this line. Record lokcing is used
   to obtain a lock on the very first byte of a SQD file which
   indicates that no other program should use the message area now.
 */


#define _XPENTRY

#elif defined (__MINGW32__)
/* MINGW32 for 32-bit Windows NT on Intel and AXP */

#define _stdc
#define _intr
#define _intcast
#define _veccast
#define _fast
#define _loadds
#define cdecl
#define pascal __stdcall
#define near
#define far

#define farread _read
#define farwrite _write
#define mymkdir _mkdir
#define fdopen _fdopen
#define close _close
#define open _open
#define lseek _lseek
#define tell _tell

#define stricmp _strcmpi
#define strcmpi _strcmpi
/*extern int __mkdir (__const__ char *name);*/
int unlock(int handle, long ofs, long length);
int lock(int handle, long ofs, long length);

#define sopen _sopen
/* older mingw headers are too lazy ... */
#include <share.h>
#ifndef SH_DENYRW
#define SH_DENYRW 0x10
#endif
#ifndef SH_DENYWR
#define SH_DENYWR 0x20
#endif
#ifndef SH_DENYRD
#define SH_DENYRD 0x30
#endif
#ifndef SH_DENYNO
#define SH_DENYNO 0x40
#endif

#ifndef _XPENTRY
#define _XPENTRY
#endif

#elif defined (__TURBOC__) && defined (WINNT)
/* Borland C/C++ for Win32 */

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

#define _XPENTRY

#define strcasecmp stricmp
#define strncasecmp strncmpi


#elif defined (__EMX__)
/* EMX for 32-bit OS/2 and RSX for Windows NT */

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

int unlock(int handle, long ofs, long length);
int lock(int handle, long ofs, long length);

#define strcasecmp stricmp
#define strncasecmp strnicmp

#define _XPENTRY

#elif defined (__TURBOC__) && defined (WINNT)
/* Borland C/C++ for Win32 */

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

#define _XPENTRY

#elif defined (__TURBOC__) && defined (__OS2__)
/* Borland C/C++ for OS/2 */

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

#ifndef _XPENTRY
#define _XPENTRY __syscall
#endif

#include <io.h>
#include <dos.h>

#define strcasecmp stricmp
#define strncasecmp strncmpi

#elif defined (__IBMC__)
/* IBM C/Set++ for OS/2 */

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

#define _XPENTRY pascal far


#elif defined (UNIX)
/* Linux, FreeBSD, etc. */

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

#if defined (__NetBSD__) || defined (__FreeBSD__) || defined (_AIX) || defined (__sun__) || \
    defined (__linux__) || defined (__osf__) || defined (__hpux) || defined (__BEOS__) || \
    defined (__OpenBSD__)
#define mymkdir(a) mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#else
#define mymkdir(a) __mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif

int lock(int handle, long ofs, long length);   /* in locking.c */
int unlock(int handle, long ofs, long length);
int sopen(const char * name, int oflag, int ishared, int mode);

#ifndef __sun__
#define tell(a) lseek((a), 0, SEEK_CUR)
#endif

#ifndef stricmp
#define stricmp strcasecmp
#endif

#ifndef __BEOS__
#define O_BINARY 0
#endif

#define SH_DENYNONE 0
#define SH_DENYNO 0
#define SH_DENYALL 1

#define _XPENTRY

#elif defined (__DJGPP__)
/* DJGPP for MS-DOS */

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

#elif defined (SASC)
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

#define unlock(a, b, c) unused(a)
#define lock(a, b, c) 0

#error "Don't know how to implement record locking."
/* Using an executable that does no support record locking is
   discouraged in a multitasking environment. If you want to
   do it anyway, you may uncomment this line. Record locking is used
   to obtain a lock on the very first byte of a SQD file which
   indicates that no other program should use the message area now.
 */

#define SH_DENYNONE 0
#define sopen(a, b, c, d) open((a), (b), (d))

#define _XPENTRY

#elif defined (_MSC_VER) && (_MSC_VER >= 1200)

#define _stdc
#ifdef pascal
#undef pascal
#endif
#define pascal
#define far
#define _fast
#define near
#define _XPENTRY
#define strncasecmp strnicmp
#define farread read
#define farwrite write
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define mode_t int

#else // if defined (__TURBOC__) && defined (__MSDOS__)

#error compiler.h: Unknown compiler!

#endif // if defined (__TURBOC__) && defined (__MSDOS__)

#ifndef mymkdir
#define mymkdir mkdir
#endif

#define NO_MKTIME
#define NO_STRFTIME
/* waitlock works like lock, but blocks until the lock can be
 * performed.
 * waitlock2 works like a timed waitlock.
 */
// extern int waitlock(int, long, long);
// extern int waitlock2(int, long, long, long);

#endif // ifndef __COMPILER_H__
