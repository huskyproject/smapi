 /* $Id$

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

/**************************************************************************
 * List of predefined macroses for compilers.                             *
 **************************************************************************
   Common macroses for GNU C
   -------------------------------------------------------------------
    __GNUC__          Major version number (2.95.* : 2)
    __GNUC_MINOR__    Minor version number (2.95.* : 95)
   ===================================================================
   MINGW32 for 32-bit Windows NT on Intel and AXP; (GNU C clone)
   MINGW32 cross-compiler from unixes;             (GNU C clone)
   Cygwin GCC with option -mno-cygwin.             (GNU C clone)
   -------------------------------------------------------------------
    __MINGW32__
   ===================================================================
   DJGPP (DOS DPMI)                          (GNU C clone)
   -------------------------------------------------------------------
    __DJGPP__ MSDOS
   ===================================================================
   EMX (OS/2)                                (GNU C clone)
   -------------------------------------------------------------------
    __EMX__
   ===================================================================
   GNU C on FreeBSD 4.*
   -------------------------------------------------------------------
   __unix__ = 1 
   __FreeBSD__ = 4 
   ===================================================================
   GNU C on Sun
   -------------------------------------------------------------------
   __sun__
   ===================================================================
   GNU C on BeOS
   -------------------------------------------------------------------
   __BeOS__
   ===================================================================

   ===================================================================
   Watcom C: OS/2 32bit target
   -------------------------------------------------------------------
   __WATCOMC__  __OS2__ __386__ M_I386 _M_I386
   ===================================================================
   Watcom C: OS/2 16bit target
   -------------------------------------------------------------------
   __WATCOMC__  __OS2__ __I86__ M_I86 _M_I86
   ===================================================================
   Watcom C: Windows NT and Windows 95 target (-bt=nt)
   -------------------------------------------------------------------
   __WATCOMC__  __NT__ __386__ M_I386 _M_I386
   ===================================================================
   Watcom C: Windows 16bit or 32bit target (-bt=windows), not win95/nt
   -------------------------------------------------------------------
   Win32s:  __WATCOMC__  __WINDOWS__ __WINDOWS_386__ __386__ M_I386 _M_I386
   Windows: __WATCOMC__  __WINDOWS__ _WINDOWS  __I86__ M_I86 _M_I86
   ===================================================================
   Watcom C: DOS 16 bit destination (-bt=dos)
   -------------------------------------------------------------------
   __WATCOMC__  __DOS__ _DOS  MSDOS __I86__ M_I86 _M_I86
   ===================================================================
   Memory model macroses in Watcom C DOS 16 bit destination:
   -------------------------------------------------------------------
   Model small:     __SMALL__   M_I86SM _M_I86SM
   Model medium:    __MEDIUM__  M_I86MM _M_I86MM
   Model compact:   __COMPACT__ M_I86CM _M_I86CM
   Model large:     __LARGE__   M_I86LM _M_I86LM
   Model huge:      __HUGE__    M_I86HM _M_I86HM
   ===================================================================
   Watcom C: DOS/4G destination (-bt=dos4g) - from WatcomC help
   -------------------------------------------------------------------
   __WATCOMC__  __DOS__ _DOS  MSDOS __386__ __DOS4G__ M_I386 _M_I386
   ===================================================================
   Watcom C other - from WatcomC help
   -------------------------------------------------------------------
   The __NETWARE__ and __NETWARE_386__ macros are defined when the build
   target is "NETWARE" (Novell NetWare)
   The __QNX__ macro is defined when the build target is "QNX" (16-bit
   or 32-bit QNX).
   Watcom C++ predefines the macro __cplusplus to identify the compiler
   as a C++ compiler. Also __WATCOM_CPLUSPLUS
   -------------------------------------------------------------------
   __WATCOMC__ value is compiler_version*100
   ===================================================================
   Watcom C compiler options macros - from WatcomC help
   -------------------------------------------------------------------
    Option Macro
    ====== ===================
    bm     _MT 
    br     _DLL 
    fpi    __FPI__ 
    fpi87  __FPI__ 
    j      __CHAR_SIGNED__ 
    oi      __INLINE_FUNCTIONS 
    za     NO_EXT_KEYS 
    zw     __WINDOWS__ 
    zW     __WINDOWS__ 
    zWs    __WINDOWS__ 
    ====== ===================
    Option     All        16-bit only       32-bit only 
    ====== =========== ================= ================= 
    mf     __FLAT__                      M_386FM _M_386FM 
    ms     __SMALL__   M_I86SM _M_I86SM  M_386SM _M_386SM 
    mm     __MEDIUM__  M_I86MM _M_I86MM  M_386MM _M_386MM 
    mc     __COMPACT__ M_I86CM _M_I86CM  M_386CM _M_386CM 
    ml     __LARGE__   M_I86LM _M_I86LM  M_386LM _M_386LM 
    mh     __HUGE__    M_I86HM _M_I86HM
   ===================================================================


   ===================================================================
   Borland C and Turbo C for DOS
   -------------------------------------------------------------------
   __TURBOC__ __MSDOS__
   ===================================================================
   Borland C for Win32
   -------------------------------------------------------------------
   __TURBOC__ __WIN32__
   ===================================================================
   Borland C for OS/2
   -------------------------------------------------------------------
   __TURBOC__ __OS2__
   ===================================================================

   ===================================================================
   Microsoft Visual C/C++
   -------------------------------------------------------------------
   _MSC_VER    value is greated or eq 1200
   ===================================================================
   Microsoft C or Microsoft QuickC for MS-DOS or OS/2
   -------------------------------------------------------------------
   _MSC_VER    value is less 1200
   ===================================================================

   ===================================================================
   MetaWare High C/C++ for OS/2
   -------------------------------------------------------------------
   __HIGHC__
   ===================================================================

   ===================================================================
   IBM C/Set++
   -------------------------------------------------------------------
   __IBMC__
   ===================================================================

 **************************************************************************/

#ifndef __COMPILER_H__
#define __COMPILER_H__

/* small code to demonstrate gcc 2.96 bugs:
char q[2048], *p;
int qq(void)
{ return p[p-q-1];
}
*/

#if defined(__GNUC__) 
#if (__GNUC__==2) && (__GNUC_MINOR__>95)   /* don't place in one line for prevent old compilers warnings */
#warning Latest GNU C branch 2 is 2.95.*. Your version is not GNU C and not supported. You may use it for your risk.
#warning Download and install GNU C release from www.gnu.org only, please.
#endif
#endif

/**** Compiler defines ****/

#if defined(_MSC_VER)
# if (_MSC_VER >= 1200) /* MS Visual C/C++ */
#  define __MSVC__
# endif
# if (_MSC_VER < 1200)
#  define __MSC__  /* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */
# endif
#endif

/* Watcom C */
#if defined(__WATCOMC__) 
#  if defined(MSDOS) && !defined(__WATCOMC__DOS__)
#    define __WATCOMC__DOS__
#  endif
#  if defined(__DOS4G__) && !defined(__WATCOMC__DOS4G__)
#    define __WATCOMC__DOS4G__
#  endif
#  if defined(__OS2__) && !defined(__WATCOMC__OS2__)
#    define __WATCOMC__OS2__
#  endif
#  if !defined(__WATCOMC__NT__) && defined(__NT__)
#    define __WATCOMC__NT__
#  endif
#  if !defined(__WATCOMC__QNX__) && defined(__QNX__)
#    define __WATCOMC__QNX__
#  endif
#endif

/* Turbo C/C++ & Borland C/C++ */
#if defined(__TURBOC__)
#  if defined(__MSDOS__) && !defined(__TURBOC__DOS__)
#    define __TURBOC__DOS__   /* Turbo C/C++ & Borland C/C++ for MS-DOS */
#  endif
#  if defined(__WIN32__) && !defined(__TURBOC__WIN32__)
#    define __TURBOC__WIN32__ /* Borland C/C++ for Win32 */
#  endif
#  if defined(__OS2__) && !defined(__TURBOC__OS2__)
#    define __TURBOC__OS2__   /* Borland C/C++ for OS/2 */
#  endif
#endif

/* IBM C */
#if defined(__IBMC__) && !defined(UNIX)
#  if !defined(__IBMC__OS2__)
#    define __IBMC__OS2__     /* IBM C/Set++ for OS/2 */
#  endif
#endif


/**** OS defines ****/

/*
  BeOS is NOT Unix, but sometime it seem's to Be ... ;)
*/
#if defined (__BEOS__) && !defined(UNIX)
#  define UNIX
#endif

/* defined in MINGW32 & cygwin's gcc with '-mno_cygwin' option  *
 * This is NOT needed for pure Cygwin builds, Cygwin == UNIX !! */
#if !defined(__NT__) && defined(__MINGW32__)
#  define __NT__
#endif


#if defined ( __WATCOMC__ )
#  include <direct.h>
#  include <io.h>
#endif

#if defined(OS2) && !defined(_MSC_VER)
#  ifndef __386__
#    define __386__
#  endif
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif


#if defined(__WATCOMC__) && defined(__X86__) /* Watcom C for intel x86 platform */
#  ifndef INTEL
#    define INTEL  /* using to select functions/macroses for read & write binary values */
#  endif
#endif

#ifdef __WATCOMC__DOS__

#  define _XPENTRY pascal

/* WATCOM has both M_I86xxx and __modeltype__ macros */

#  if defined(M_I86SM) && ! defined(__SMALL__)
#    define __SMALL__
#  endif

#  if defined(M_I86MM) && !defined(__MEDIUM__)
#    define __MEDIUM__
#  endif

#  if defined(M_I86CM) && !defined(__COMPACT__)
#    define __COMPACT__
#  endif

#  if defined(M_I86LM) && !defined(__LARGE__)
#    define __LARGE__
#  endif

#  if defined(M_I86HM) && !defined(__HUGE__)
#    define __HUGE__
#  endif

#endif /* ifdef __WATCOMC__DOS__ */

/* Handle 386 "flat" memory model */

#ifdef __FLAT__

/* Other macros may get defined by braindead compilers */

#  ifdef __SMALL__
#    undef __SMALL__
#  endif

#  ifdef __TINY__
#    undef __TINY__
#  endif

#  ifdef __MEDIUM__
#    undef __MEDIUM__
#  endif

#  ifdef __COMPACT__
#    undef __COMPACT__
#  endif

#  ifdef __LARGE__
#    undef __LARGE__
#  endif

#  ifdef __HUGE__
#    undef __HUGE__
#  endif

  /*
   *  Code is really "near", but "far" in this context means that we
   *  want a 32-bit pointer (vice 16-bit).
   */

#  define __FARCODE__
#  define __FARDATA__

/* Everything should be "near" in the flat model */

#  ifdef near
#    undef near
#    define near
#  endif

#  ifdef far
#    undef far
#    define far near
#  endif

#  ifdef huge
#    undef huge
#    define huge near
#  endif

#endif  /* ifdef __FLAT__ */

#if defined(__SMALL__) || defined(__TINY__)
#  define __NEARCODE__
#  define __NEARDATA__
#endif

#ifdef __MEDIUM__
#  define __FARCODE__
#  define __NEARDATA__
#endif

#ifdef __COMPACT__
#  define __NEARCODE__
#  define __FARDATA__
#endif

#if defined(__LARGE__) || defined(__HUGE__)
#  define __FARCODE__
#  define __FARDATA__
#endif

  /*
   * Compiler-specific stuff:
   *
   * _stdc - Standard calling sequence.  This should be the type of function
   *         required for function pointers for qsort() et al.
   *
   * _fast - Fastest calling sequence supported.  If the default calling
   *         sequence is the fastest, or if your compiler only has one,
   *         define this to nothing.
   *
   * _intr - For defining interrupt functions.  For some idiotic reason, MSC
   *         requires that interrupt routines be declared as "cdecl interrupt",
   *         instead of just "interrupt".
   *
   * SMAPI_EXT  - external variables & external functions call modifier
   *              (usualy 'extern')
   */


#ifdef __MSVC__
#  undef SMAPI_EXT
#  ifdef _MAKE_DLL
#    define _MAKE_DLL_MVC_
#    ifndef _SMAPI_EXT
#      define SMAPI_EXT __declspec(dllimport)
#    else
#      define SMAPI_EXT __declspec(dllexport)
#    endif /* _SMAPI_EXT */

#    define __STDC__ 1
#    define fileno       _fileno
#    define read         _read
#    define lseek        _lseek
#    define sopen        _sopen
#    define write        _write
#    define tell         _tell
#    define close        _close
#    define unlink       _unlink
#    define tzset        _tzset
#    define stricmp      _stricmp
#    define strnicmp     _strnicmp
#    define rmdir        _rmdir
#    define fstat        _fstat
#    define strdup       _strdup
#    define stat         _stat
#    define getpid       _getpid
#    define chsize       _chsize
#    define open         _open
#    define access       _access
#    define spawnvp      _spawnvp
#    define dup          _dup
#    define mktemp       _mktemp
#    define fdopen       _fdopen

#  else  /* ifdef _MAKE_DLL */
#    define SMAPI_EXT extern
#  endif /* ifdef _MAKE_DLL */

#  define _stdc
#  ifdef pascal
#    undef pascal
#  endif
#  define pascal
#  define far
#  define _fast
#  define near
#  define _XPENTRY

#  define strcasecmp  stricmp
#  define strncasecmp strnicmp

#  define sleep(x)    Sleep(1000L*(x))
#  define farread     read
#  define farwrite    write

#  ifndef S_ISDIR
#    define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  endif
#  ifndef S_ISREG
#    define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#  endif
#  define mode_t int
/* define constants for _access function */
#  ifndef F_OK
#    define F_OK 0
#  endif

#  ifndef R_OK
#    define R_OK 04
#  endif

#  ifndef W_OK
#    define W_OK 02
#  endif

#  define snprintf    _snprintf
#  define vsnprintf   _vsnprintf
#  define HAS_SNPRINTF  1
#  define HAS_VSNPRINTF 1
#  define USE_SYSTEM_COPY

#  define HAS_SPAWNVP 1

   SMAPI_EXT int unlock(int handle, long ofs, long length);
   SMAPI_EXT int lock(int handle, long ofs, long length);

#  include <direct.h>
#  define mymkdir _mkdir

/* End: MS Visual C/C++ */
/*#elif (defined(_MSC_VER) && (_MSC_VER < 1200))*/
#elif defined(__MSC__)
/* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */

#ifndef INTEL
#  define INTEL  /* using to select functions/macroses for read & write binary values */
#endif

#  define _stdc cdecl
#  define _intr cdecl interrupt far
#  define _intcast void (_intr *)()
#  define _veccast _intcast

#  if _MSC_VER >= 600
#    define _fast _fastcall
#  else
#    define _fast pascal
#  endif

  int unlock(int handle, long ofs, long length);
  int lock(int handle, long ofs, long length);

#  ifdef OS2
    /* just don't use 16 bit OS/2, we doubt that it still works */
#    define farread read
#    define farwrite write
#    define _XPENTRY pascal far
#    define mysleep(x) DosSleep(1000L*(x))
#  endif

/* End: Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */

/* Begin: Watcom C all variants (DOS, Win. OS/2 */
/*#elif defined(__WATCOMC__) && defined(MSDOS)*/
#elif defined(__WATCOMC__DOS__) || defined(__WATCOMC__DOS4G__)
/* WATCOM C/C++ for MS-DOS */

/* predefined:
   __DOS__, _DOS, MSDOS
*/

#  define _stdc cdecl
#  define _intr interrupt far
#  define _intcast void (_intr *)()
#  define _veccase _intcast
#  define _fast pascal

#  ifdef __FLAT__
#    define farread read
#    define farwrite write
#  endif

#  define strcasecmp stricmp
#  define strncasecmp strnicmp

#  define mode_t int

#  define HAS_SPAWNVP 1

/* End: WATCOM C/C++ for MS-DOS */
/*#elif defined(__WATCOMC__) && defined(__OS2__)*/
#elif defined(__WATCOMC__OS2__)
/* WATCOM C/C++ for OS/2 */
/* Predefined:
   __OS2__   -  (16-bit or 32-bit OS/2)
*/

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast

#  define farread read
#  define farwrite write

#  define strcasecmp stricmp
#  define strncasecmp strnicmp

#  define _XPENTRY _System
#  define mysleep(x) sleep(x)
#  define mode_t int

#  define HAS_SNPRINTF  1
#  define HAS_VSNPRINTF 1
#  define snprintf  _snprintf
#  define vsnprintf _vsnprintf
#  define HAS_SPAWNVP 1

/* End: WATCOM C/C++ for OS/2 */
/*#elif defined(__WATCOMC__) && defined(__NT__)*/
#elif defined(__WATCOMC__NT__)
/* WATCOM C/C++ for Windows NT */

/* Predefined:
   __NT__ 
*/

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast

#  define farread read
#  define farwrite write

#  define strcasecmp stricmp
#  define strncasecmp strnicmp

#  define _XPENTRY pascal
#  define mode_t int

#  define HAS_SPAWNVP 1

/* End: WATCOM C/C++ for Windows NT */
/* End: Watcom C all variants */
#elif defined(__HIGHC__)
/* MetaWare High C/C++ for OS/2 */

#ifndef INTEL
#  define INTEL  /* using to select functions/macroses for read & write binary values */
#endif

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define pascal
#  define near
#  define far

#  define farread read
#  define farwrite write

#  define mysleep(x) DosSlep(1000L*(x))

#  define unlock(a,b,c) unused(a)
#  define lock(a,b,c) 0
#  error "Don't know how to implement record locking."
/* Using an executable that does no support record locking is
   discouraged in a multitasking environment. If you want to
   do it anyway, you may uncomment this line. Record lokcing is used
   to obtain a lock on the very first byte of a SQD file which
   indicates that no other program should use the message area now.
*/

#  define _XPENTRY

/* End: MetaWare High C/C++ for OS/2 */
#elif defined(__MINGW32__)
/* MinGW32 */

/* Applied to:
   - MINGW32 for 32-bit Windows NT on Intel and AXP;
   - MINGW32 cross-compiler from unixes;
   - Cygwin GCC with option -mno-cygwin.
*/

/* Pre-defines:  __GNUC__, __MINGW32__
 */

#ifndef INTEL
#  define INTEL  /* using to select functions/macroses for read & write binary values */
#endif

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds
#  define cdecl
#  define pascal __stdcall
#  define near
#  define far

#  define farread _read
#  define farwrite _write
#  define mymkdir _mkdir
#  define fdopen _fdopen
#  define close _close
#  define open _open
#  define lseek _lseek
#  define tell _tell
#  define sleep(sec) _sleep((sec)*1000l)
#  define write _write
#  define read _read

/* This strange code never pass...
#  if !defined(__CYGWIN__) && !defined(__MINGW32__)
#    define stat _stat
#    define fstat _fstat
#    define chsize _chsize

#    define stricmp _strcmpi
#    define strcmpi _strcmpi
#  endif
*/

  int unlock(int handle, long ofs, long length);
  int lock(int handle, long ofs, long length);
#  define sopen _sopen

/* older mingw headers are too lazy ... */
#  include <share.h>
#  ifndef SH_DENYRW
#    define SH_DENYRW 0x10
#  endif
#  ifndef SH_DENYWR
#    define SH_DENYWR 0x20
#  endif
#  ifndef SH_DENYRD
#    define SH_DENYRD 0x30
#  endif
#  ifndef SH_DENYNO
#    define SH_DENYNO 0x40
#  endif

#  define HAS_SPAWNVP 1   /* spawnvp() present */

/* end: MinGW32 */
#elif defined(__EMX__)
/* EMX for 32-bit OS/2 and RSX for Windows NT */

/* Pre-defines:  __GNUC__, __EMX__
 */

#ifndef INTEL
#  define INTEL  /* using to select functions/macroses for read & write binary values */
#endif

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds
#  define cdecl
#  define pascal
#  define near
#  define far

#  define farread read
#  define farwrite write
#  define mymkdir(a) mkdir((a), 0)

   int unlock(int handle, long ofs, long length);
   int lock(int handle, long ofs, long length);

#  define strcasecmp stricmp
#  define strncasecmp strnicmp

#  define _XPENTRY

#  define HAS_SNPRINTF  1
#  define HAS_VSNPRINTF 1

/* End: EMX for 32-bit OS/2 and RSX for Windows NT */
#elif defined(__DJGPP__)
/* DJGPP for MS-DOS (DPMI)*/

/* Pre-defines:  MSDOS, __GNUC__, __DJGPP__
 */

#ifndef INTEL
#  define INTEL  /* using to select functions/macroses for read & write binary values */
#endif

#  ifndef __FLAT__
#    define __FLAT__  /* DOS flat memory */
#  endif

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds

#  define cdecl
#  define pascal
#  define near
#  define far

#  define farread read
#  define farwrite write

#  define mymkdir(a) mkdir((a), 0)

#  define stricmp strcasecmp
#  define SH_DENYNONE 0

#  include <unistd.h>
#  include <io.h>
#  define mysleep(x) sleep(x)

#  define HAS_SPAWNVP

/* End: DJGPP for MS-DOS (DPMI)*/
#elif defined(__TURBOC__) && defined(__MSDOS__)
#elif defined(__TURBOC__DOS__)
/* Borland Turbo C/C++ & Borland C/C++ for MS-DOS */

#ifndef INTEL
#  define INTEL  /* using to select functions/macroses for read & write binary values */
#endif

/* for BC++ 3.1 */
#  define strcasecmp stricmp
#  define strncasecmp strncmpi

#  define _stdc cdecl
#  define _intr interrupt far
#  define _intcast void (_intr *)()
#  define _veccast _intcast
#  define _fast _fastcall
#  define _loadds

   /* #include <conio.h> */
#  define mysleep(x) delay(x);

#  ifndef _XPENTRY
#    define _XPENTRY
#  endif

#  define mode_t int

/* Borland Turbo C/C++ for MS-DOS */
/*#elif defined(__TURBOC__) && defined(__WIN32__)*/
#elif defined(__TURBOC__WIN32__)
/* Borland C/C++ for Win32 */

#  define _stdc cdecl
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast _fastcall
#  define _loadds
#  define near
#  define far
   
#  define farread read
#  define farwrite write

#  define _XPENTRY

#  define strcasecmp stricmp
#  define strncasecmp strncmpi

/* End: Borland C/C++ for Win32 */
/*#elif defined(__TURBOC__) && defined(__OS2__)*/
#elif defined(__TURBOC__OS2__)
/* Borland C/C++ for OS/2 */

#  define _stdc cdecl
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast _fastcall
#  define _loadds
#  define near
#  define far

#  define farread read
#  define farwrite write

#  ifndef _XPENTRY
#    define _XPENTRY __syscall
#  endif
#  define mysleep(x) sleep(x);

#  include <io.h>
#  include <dos.h>

#  define strcasecmp stricmp
#  define strncasecmp strncmpi

/* End: Borland C/C++ for OS/2 */
#elif defined(__IBMC__) && !defined(UNIX)
#elif defined(__IBMC__OS2__)
/* IBM C/Set++ for OS/2 */

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds

#  define cdecl
#  define pascal
#  define near
#  define far

#  define farread read
#  define farwrite write
#  define mysleep(x) DosSleep(1000L*(x))

#  define _XPENTRY pascal far

/* End: IBM C/Set++ for OS/2 */
#elif defined(UNIX)
/* Unix clones: Linux, FreeBSD, SUNOS (Solaris), BeOS, MacOS etc. */

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds

#  define cdecl
#  define pascal
#  define near
#  define far

#  define farread read
#  define farwrite write

#  if (defined(__APPLE__) && defined(__MACH__)) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(_AIX) || defined(__sun__) || defined(__linux__) || defined(__osf__) || defined(__hpux) || defined(__BEOS__) || defined(__OpenBSD__) || defined(__CYGWIN__)
#    define mymkdir(a) mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#  else
#    define mymkdir(a) __mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#  endif

  int lock(int handle, long ofs, long length);   /* in locking.c */
  int unlock(int handle, long ofs, long length);
  int sopen(const char *name, int oflag, int ishared, int mode);

#  ifndef __sun__
#    define tell(a) lseek((a),0,SEEK_CUR)
#  endif

#  ifndef stricmp
#    define stricmp strcasecmp
#    define strnicmp strncasecmp
#  endif

/* Cygwin defines O_BINARY in sys/fcntl.h. Why different from other UNIXES? */
#  if !defined(__BEOS__) && !defined(__CYGWIN__)
#    define O_BINARY 0
#  endif

#  define SH_DENYNONE 0
#  define SH_DENYNO 0
#  define SH_DENYALL 1

#  define _XPENTRY

/* Other OS's may sleep with other functions */

#  ifdef __BEOS__
#    define mysleep(x) snooze(x*1000000l)
#  elif defined(__linux__) || defined(__sun__)
#    define mysleep(x) usleep(x*1000000l)
#  endif

#  define HAS_SNPRINTF  1
#  define HAS_VSNPRINTF 1

/* End: Unix clones */
#elif defined(SASC)
/* SAS C for AmigaDOS */

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds

#  define cdecl
#  define pascal
#  define near
#  define far

#  define farread read
#  define farwrite write
#  define mymkdir(a) mkdir((a))

#  define unlock(a,b,c) unused(a)
#  define lock(a,b,c) 0
#  define mysleep(x) unused(x)

#error "Don't know how to implement record locking."
/* Using an executable that does no support record locking is
   discouraged in a multitasking environment. If you want to
   do it anyway, you may uncomment this line. Record locking is used
   to obtain a lock on the very first byte of a SQD file which
   indicates that no other program should use the message area now.
*/

#  define SH_DENYNONE 0
#  define sopen(a,b,c,d) open((a),(b),(d))

#  define _XPENTRY

/* End: SAS C for AmigaDOS */
#else

#error compiler.h: Unknown compiler!

#endif   /* End compiler-specific decrarations */


#ifndef mymkdir
#  define mymkdir mkdir
#endif

#define NO_MKTIME
#define NO_STRFTIME

#ifndef mysleep
#  define mysleep(x)
#endif

/* File open and share modes */
#if !defined(O_BINARY) && defined(_O_BINARY)
#  define O_BINARY    _O_BINARY
#endif
#if !defined(O_RDWR) && defined(_O_RDWR)
#  define O_RDWR      _O_RDWR
#endif
#if !defined(S_IFMT) && defined(_S_IFMT)
#  define S_IFMT      _S_IFMT
#endif
#if !defined(S_IFDIR) && defined(_S_IFDIR)
#  define S_IFDIR     _S_IFDIR
#endif

#ifndef SMAPI_EXT
#  define SMAPI_EXT extern
#endif

#ifndef _XPENTRY
#  define _XPENTRY
#endif


 /* waitlock works like lock, but blocks until the lock can be
  * performed.
  * waitlock2 works like a timed waitlock.
  */
extern int waitlock(int, long, long);
extern int waitlock2(int, long, long, long);

#endif
