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

#if defined ( __WATCOMC__ )
#include <direct.h>
#include <io.h>
#endif

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

/* for BC++ 3.1 */
#define strcasecmp stricmp
#define strncasecmp strncmpi

#define _stdc cdecl
#define _intr interrupt far
#define _intcast void (_intr *)()
#define _veccast _intcast
#define _fast pascal
#define _loadds

#include <conio.h>
#define mysleep(x) delay(x);

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
#error "Don't know how to implement record locking."
/* Using an executable that does no support record locking is
   discouraged in a multitasking environment. If you want to
   do it anyway, you may uncomment this line. Record lokcing is used
   to obtain a lock on the very first byte of a SQD file which
   indicates that no other program should use the message area now.
*/

#ifdef OS2
#define farread read
#define farwrite write
#define EXPENTRY pascal far
#define mysleep(x) DosSleep(1000L*(x))
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
#define mysleep(x) DosSleep(1000*(x))

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

#define strcasecmp stricmp

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

#define mysleep(x) DosSlep(1000L*(x))

#define unlock(a,b,c) unused(a)
#define lock(a,b,c) 0
#error "Don't know how to implement record locking."
/* Using an executable that does no support record locking is
   discouraged in a multitasking environment. If you want to
   do it anyway, you may uncomment this line. Record lokcing is used
   to obtain a lock on the very first byte of a SQD file which
   indicates that no other program should use the message area now.
*/



#define EXPENTRY

#elif defined(__EMX__)

/* EMX for 32-bit OS/2 and RSX for Windows NT */

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

extern int __mkdir (__const__ char *name);

int unlock(int handle, long ofs, long length);
int lock(int handle, long ofs, long length);

#define EXPENTRY

#elif defined(__TURBOC__) && defined(WINNT)

/* Borland C/C++ for Win32 */

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
#define mysleep(x) _sleep(x);

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
#define mysleep(x) DosSleep(1000L*(x))

#define EXPENTRY pascal far

#elif defined(UNIX)

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

#if defined(__FreeBSD__) || defined(_AIX) || defined(__sun__)
#define mymkdir(a) mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#else
#define mymkdir(a) __mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif

int lock(int handle, long ofs, long length);   /* in locking.c */
int unlock(int handle, long ofs, long length);
int sopen(const char *name, int oflag, int ishared, int mode);

#define tell(a) lseek((a),0,SEEK_CUR)

#ifndef stricmp
#define stricmp strcasecmp
#endif

#define O_BINARY 0
#define SH_DENYNONE 0
#define SH_DENYNO 0
#define SH_DENYALL 1

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
#define mysleep(x) sleep(x)

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
#define mysleep(x) unused(x)

#error "Don't know how to implement record locking."
/* Using an executable that does no support record locking is
   discouraged in a multitasking environment. If you want to
   do it anyway, you may uncomment this line. Record lokcing is used
   to obtain a lock on the very first byte of a SQD file which
   indicates that no other program should use the message area now.
*/

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

 /* waitlock works like lock, but blocks until the lock can be
    performed. */
#ifndef mysleep
#define mysleep(x)
#endif
extern int waitlock(int, long, long);

#endif



