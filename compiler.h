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

  /*
   * Please define this compiler-specific stuff for each new compiler:
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
   * SMAPI_EXT - external variables & external functions call modifier
   *             (usualy 'extern' for static linkage)
   *
   * _XPENTRY  - system procedures calling (conversion) modifyer
   *             ("pascal", "_system" & etc)
   *
   * HAS_SNPRINTF        - snprintf() presents
   * HAS_VSNPRINTF       - vsnprintf() presents
   * HAS_SPAWNVP         - spawnwp() presents
   * HAS_MKTIME          - mktime() presents
   * HAS_STRFTIME        - strftime() presents
   * HAS_MALLOC_H        - may be used "#include <malloc.h>" for malloc() etc.
   * HAS_DOS_H           - may be used "#include <dos.h>"
   * HAS_DIR_H           - may be used "#include <dir.h>" for findfirst() etc.
   * HAS_DIRENT_H        - may be used "#include <dirent.h>" for opendir() etc.
   * HAS_IO_H            - may be used "#include <io.h>"
   * HAS_UNISTD_H        - may be used "#include <unistd.h>"
   *
   * USE_SYSTEM_COPY     - OS have system call for files copiing (see
   *                       copy_file() and move_file() functions)
   * USE_STAT_MACROS     - may use stat() macro and non-POSIX (important!)
   *                       S_ISREG and S_ISDIR macros. (See fexist.c)
   *
   *
   ***************************************************************************
   * Functions "my*" & etc
   *
   * mysleep(x)         - wait x seconds
   * mymkdir(d)         - make directory
   * strcasecmp(s1,s2)  - case-incencitive strings comparition, declare if
   *                      present with other name or include header-file
   * stricmp(s1,s2)     - also as above
   * strncasecmp(s1,s2) - case-incencitive strings comparition not more n chars,
   *                      declare if present with other name or include header
   * strnicmp(s1,s2)    - also as above
   * farread(a,b,c)     - for flat memory models declare as read(a,b,c)
   * farwrite(a,b,c)    - for flat memory models declare as write(a,b,c)
   *
   ***************************************************************************
   * Memory and platforms
   *
   * 16bit Intel x86 memory models (compiler-predefined)
   * __TINY__    - 64K data, 64K code, stack in code or data
   * __SMALL__   - 64K data, 64K code, stack apart
   * __MEDIUM__  - 64K data, 1M (640K+HMB+UMB) code, stack apart
   * __COMPACT__ - 1M data, 64K code, stack apart
   * __LARGE__   - 1M data, 1M code, stack apart
   * __HUGE__    - similar to the __LARGE__ except for two additional features:
   *               Its segment is normalized during pointer arithmetic so that
   *               pointer comparisons are accurate. And, huge pointers can be
   *               incremented without suffering from segment wrap around.
   * __NEARCODE__ - 64K code
   * __FARCODE__  - 1M code
   * __NEARDATA__ - 64K data
   * __FARDATA__  - 1M data
   *
   * __FLAT__  - must be declared for any flat memory model, usualy all
   *             not 16 bit dos, os/2 and windows; predefined for some compilers
   *             - 64K data
   *
   ***************************************************************************
   * Platforms (binary targets)
   *
   * __NT__    - Windows NT/2000/XP target
   * __WIN32__ - Windows 95/98/Me/NT/2000/XP target
   * __OS2__   - OS/2 target (32 bit or 16 bit), 32bit is __OS2__ && __FLAT__
   * __DOS__   - MS/PC/... DOS target (32 bit or 16 bit), 32bit is __DOS__ && __FLAT__
   * __DPMI__  - DOS 32 bit (extenders: dos4g, farcall, rsx, ...)
   * __UNIX__  - All unix-like OS
   * __AMIGA__ - AmigaOS
   *
   */

/**************************************************************************
 * For informaion: list of predefined macroses for known compilers.       *
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
   Values of __TURBOC__: 0x18d = TurboC 2.0
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

#if defined(__DJGPP__) /* DJGPP for MS-DOS (DPMI)*/
#ifndef __DOS__
#  define __DOS__
#endif
#ifndef __DPMI__
#  define __DPMI__
#endif
#ifndef __FLAT__
#  define __FLAT__
#endif
#endif

#if defined(__MINGW32__) /* MinGW32 & cygwin's 'gcc -mno-cygwin' ***********/
#ifndef __NT__
#  define __NT__
#endif
#ifndef __WIN32__
#  define __WIN32__
#endif
#ifndef __FLAT__
#  define __FLAT__
#endif
#endif

#if defined(_MSC_VER)
#  if (_MSC_VER >= 1200) /* MS Visual C/C++ */
#    define __MSVC__
#    ifndef __WIN32__
#      define __WIN32__
#    endif
#  endif
#  if (_MSC_VER < 1200)
#    define __MSC__  /* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */
#    ifdef __OS2__
#      ifndef __MSC__OS2__
#        define __MSC__OS2__
#      endif
#    endif
#    ifdef __DOS__
#      ifndef __MSC__DOS__
#        define __MSC__DOS__
#      endif
#    endif
#  endif
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
#  if defined(__NT__)
#    ifndef __WIN32__
#      define __WIN32__
#    endif
#    if !defined(__WATCOMC__NT__)
#      define __WATCOMC__NT__
#    endif
#    if !defined(__WATCOMC__WIN32__)
#      define __WATCOMC__WIN32__
#    endif
#  endif
#  if defined(__QNX__)
#    if !defined(__UNIX__)
#      define __UNIX__
#    endif
#    if !defined(__WATCOMC__QNX__)
#      define __WATCOMC__QNX__
#    endif
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
#if defined(__IBMC__)
#  if !defined(UNIX)
#    if !defined(__IBMC__OS2__)
#      define __IBMC__OS2__     /* IBM C/Set++ for OS/2 */
#    endif
#  else
#    if !defined(__IBMC__UNIX__)
#      define __IBMC__UNIX__     /* IBM C/Set++ for unix */
#    endif
#    if !defined(__UNIX__)
#      define __UNIX__
#    endif
#  endif
#endif

#if defined(__sun__)
#  if !defined(__SUN__)
#    define __SUN__
#  endif
#endif

#if defined(__linux__)
#  if !defined(__LINUX__)
#    define __LINUX__
#  endif
#endif


/**** OS defines ****/

#if defined(__TURBOC__DOS__) && !defined(__DOS__)
#  define __DOS__
#endif

/* defined in MINGW32 & cygwin's gcc with '-mno_cygwin' option  *
 * This is NOT needed for pure Cygwin builds, Cygwin == UNIX !! */
#if defined(__MINGW32__) && !defined(__NT__)
#  define __NT__
#endif

#if defined(__CYGWIN__) && !defined(__UNIX__)  /* Pure cygwin */
#  define __UNIX__
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined(__SUN__) || defined(__LINUX__)
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

/*
  BeOS is NOT Unix, but sometime it seem's to Be ... ;)
*/
#if defined (__BEOS__) && !defined(__UNIX__)
#  define __UNIX__
#endif

#if defined(SASC) && !defined(__UNIX__) /* SAS C for AmigaDOS ***************/
#  define __UNIX__
#endif

/***** Platforms *************************************************************/

#if defined(__WATCOMC__) && defined(__X86__) /* Watcom C for intel x86 platform */
#  ifndef INTEL
#    define INTEL  /* using to select functions/macroses for read & write binary values */
#  endif
#endif

#if defined(__DOS__)
#  ifndef INTEL
#    define INTEL  /* using to select functions/macroses for read & write binary values */
#  endif
#endif

#if defined(SASC) && !defined(__AMIGA__) /* SAS C for AmigaDOS ***************/
#  define __AMIGA__
#endif

/***** memory models *********************************************************/

#if defined(__DOS4G__) || defined(__WIN32S__) || defined(__WIN32__) || defined(__NT__) || defined(__UNIX__)
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif

#if defined(__OS2__) && !defined(_MSC_VER)
#  ifndef __386__
#    define __386__
#  endif
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif


#ifdef __WATCOMC__DOS__

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

#if defined(__FLAT__)

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

/***** compiler-specific stuff **********************************************/

#ifdef __MSVC__  /* MS Visual C/C++ *****************************************/

#  ifdef _MAKE_DLL
#    define _MAKE_DLL_MVC_
#    ifndef _SMAPI_EXT
#      define SMAPI_EXT __declspec(dllimport)
#    else
#      define SMAPI_EXT __declspec(dllexport)
#    endif /* _SMAPI_EXT */
/*   must be included before function redefenition like '#    define fileno _fileno" */
#    include <sys/stat.h>
#    include <stdio.h>
/* system functions substitutions for DLL build */
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
#    define strupr       _strupr
#    define strlwr       _strlwr
#    define stat         _stat
#    define getpid       _getpid
#    define chsize       _chsize
#    define open         _open
#    define access       _access
#    define spawnvp      _spawnvp
#    define dup          _dup
#    define mktemp       _mktemp
#    define fdopen       _fdopen
#    define chdir        _chdir
#    define getcwd       _getcwd
#    define isatty       _isatty

#  else  /* ifdef _MAKE_DLL */
#    define SMAPI_EXT    extern
#  endif /* ifdef _MAKE_DLL */

#  define _stdc
#  ifdef pascal
#    undef pascal
#  endif
#  define pascal
#  undef  far
#  define far
#  define _fast
#  define near
#  define _XPENTRY

#  define strcasecmp  stricmp
#  define strncasecmp strnicmp

#  define sleep(x)    Sleep(1000L*(x))
#  define farread     read
#  define farwrite    write

#  define mode_t int

/* define macrofunctions for fexist.c */
#  ifndef S_ISDIR
#    define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  endif
#  ifndef S_ISREG
#    define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#  endif

/* define constants for 2nd parameter of access() function */
#  ifndef F_OK
#    define F_OK 0
#  endif

#  ifndef R_OK
#    define R_OK 04
#  endif

#  ifndef W_OK
#    define W_OK 02
#  endif

#  define mymkdir       _mkdir
#  define snprintf      _snprintf
#  define vsnprintf     _vsnprintf
#  define HAS_SNPRINTF  1     /* snprintf() presents */
#  define HAS_VSNPRINTF 1     /* vsnprintf() presents */
#  define HAS_SPAWNVP   1     /* spawnwp() presents */
#  define HAS_STRFTIME  1
#  define HAS_MKTIME    1

#  define USE_SYSTEM_COPY     /* OS have system call for files copiing */
#  define USE_STAT_MACROS

#  define HAS_MALLOC_H        /* use "#include <malloc.h>" for malloc() etc. */
#  define HAS_DIRECT_H
#  include <direct.h>

   SMAPI_EXT int unlock(int handle, long ofs, long length);
   SMAPI_EXT int lock(int handle, long ofs, long length);


/* End: MS Visual C/C++ ******************************************************/

#elif defined(__MSC__) /* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */

#  ifndef INTEL
#    define INTEL  /* using to select functions/macroses for read & write binary values */
#  endif

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

#  ifdef __OS2__
    /* just don't use 16 bit OS/2, we doubt that it still works */
#    define farread read
#    define farwrite write
#    define _XPENTRY pascal far
#    define mysleep(x) DosSleep(1000L*(x))
#  else
#    define _XPENTRY
#  endif

#  define HAS_MALLOC_H        /* use "#include <malloc.h>" for malloc() etc. */

#  define USE_STAT_MACROS

/* End: Microsoft C or Microsoft QuickC for MS-DOS or OS/2 *******************/

/* Begin: Watcom C all variants (DOS, Win, OS/2, QNX) ************************/
#elif defined(__WATCOMC__)

#  define mode_t int
#  define SMAPI_EXT extern

#  ifdef __FLAT__
#    define farread read
#    define farwrite write
#  endif

#  define strcasecmp  stricmp
#  define strncasecmp strnicmp
#  define snprintf    _snprintf
#  define vsnprintf   _vsnprintf
#  define HAS_SNPRINTF  1
#  define HAS_VSNPRINTF 1
#  define HAS_SPAWNVP   1
#  define HAS_GETPID    1
#  define HAS_STRFTIME  1  /* strftime() in time.h  */
#  define HAS_MKTIME    1  /* mktime() in time.h */

#  define HAS_MALLOC_H  1  /* may be used "#include <malloc.h>"  (see alc.h) */

#  include <direct.h>
#  define mymkdir(x)    mkdir(x) /*<direct.h>*/
#  include <io.h>

#  if defined(__WATCOMC__DOS__)
/* WATCOM C/C++ for MS-DOS or DOS4G*/

#    define _stdc      cdecl
#    define _intr      interrupt far
#    define _intcast   void (_intr *)()
#    define _veccast   _intcast
#    define _fast      pascal

#    define _XPENTRY   pascal
#    define mysleep(x) sleep(x) /* dos.h */

/* End: WATCOM C/C++ for MS-DOS */
#  elif defined(__WATCOMC__OS2__)
/* WATCOM C/C++ for OS/2 */

#    define _stdc
#    define _intr
#    define _intcast
#    define _veccast
#    define _fast

#    define _XPENTRY   _System
#    define mysleep(x) sleep(x)  /* dos.h */
/*#  define mysleep(x) DosSleep(x*1000)*/  /* os2/bsedos.h */

/* End: WATCOM C/C++ for OS/2 */
#  elif defined(__WATCOMC__NT__)
/* WATCOM C/C++ for Windows NT */

#    define _stdc
#    define _intr
#    define _intcast
#    define _veccast
#    define _fast

#    define _XPENTRY pascal

#    define mysleep(x) sleep(x)      /* dos.h */
/*#  define mysleep(x) Sleep(x*1000) */ /* winbase.h */

/* End: WATCOM C/C++ for Windows NT */
#  endif

/* End: Watcom C all variants ************************************************/

#elif defined(__HIGHC__) /* MetaWare High C/C++ for OS/2 ***********************/

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

#  define HAS_MALLOC_H 1      /* use "#include <malloc.h>" for malloc() etc. */

/* End: MetaWare High C/C++ for OS/2 */
#elif defined(__MINGW32__) /* MinGW32 & cygwin's 'gcc -mno-cygwin' ***********/

/* Applied to:
   - MINGW32 for 32-bit Windows NT on Intel and AXP;
   - MINGW32 cross-compiler from unixes;
   - Cygwin GCC with option -mno-cygwin.
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
#  undef  far
#  define far
#  define _XPENTRY
#  define SMAPI_EXT extern

#  define farread _read
#  define farwrite _write
#  define fdopen _fdopen
#  define close _close
#  define open _open
#  define lseek _lseek
#  define tell _tell
#  define write _write
#  define read _read

#  define sleep(sec) _sleep((sec)*1000l)
#  define mkdir _mkdir
#  define mysleep(sec) _sleep((sec)*1000l)
#  define mymkdir _mkdir
/*#  define strcasecmp  stricmp*/
/*#  define strncasecmp strnicmp*/

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

#  define HAS_SPAWNVP  1  /* spawnvp() present */
#  define HAS_MKTIME   1  /* time.h */
#  define HAS_STRFTIME 1  /* time.h */
#  define HAS_ACCESS   1  /* access() in io.h */

#  define HAS_MALLOC_H 1  /* may use "#include <malloc.h>" for malloc() etc. */
#  define HAS_IO_H     1  /* may use "#include <io.h> */

#  define USE_STAT_MACROS

/* end: MinGW32 **************************************************************/

#elif defined(__EMX__)/* EMX for 32-bit OS/2 and RSX for Windows NT **********/

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
#  undef  far
#  define far

#  define farread read
#  define farwrite write
#  define mymkdir(a) mkdir((a), 0)
#  define getpid() _getpid()

   int unlock(int handle, long ofs, long length);
   int lock(int handle, long ofs, long length);

#  define strcasecmp stricmp
#  define strncasecmp strnicmp

#  define _XPENTRY

#  define HAS_SNPRINTF  1
#  define HAS_VSNPRINTF 1
#  define HAS_GETPID    1  /* getpid() in process.h */
#  define HAS_SPAWNVP   1  /* spawnvp() in process.h */
#  define HAS_STRFTIME  1  /* strftime() in time.h  */
#  define HAS_MKTIME    1  /* mktime() in time.h */

#  define HAS_DIRENT_H  1  /* use "#include <dirent.h>" for opendir() etc. */
#  define HAS_IO_H      1  /* use "#include <io.h>" */
#  define HAS_UNISTD_H  1  /* use "#include <unistd.h> */

#  define USE_STAT_MACROS

/* End: EMX for 32-bit OS/2 and RSX for Windows NT ***************************/

#elif defined(__DJGPP__) /* DJGPP for MS-DOS (DPMI)***************************/

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
#  undef  far
#  define far

#  define farread read
#  define farwrite write

#  define mymkdir(a) mkdir((a), 0)

#  ifdef  SH_DENYNO
#    define SH_DENYNONE SH_DENYNO
#  else
#    define SH_DENYNONE 0
#  endif

#  include <unistd.h>
#  include <io.h>
#  define mysleep(x) sleep(x)

#  define HAS_SPAWNVP  1      /* spawnvp() in process.h */
#  define HAS_STRFTIME 1      /* strftime() in time.h  */
#  define HAS_MKTIME   1      /* mktime() in time.h */

#  define HAS_DIR_H    1      /* use "#include <dir.h>" for findfirst() etc. */
#  define HAS_IO_H     1
#  define HAS_UNISTD_H 1

#  define USE_STAT_MACROS

/* End: DJGPP for MS-DOS (DPMI) **********************************************/

#elif defined(__TURBOC__)/* Borland Turbo C/C++ & Borland C/C++ **************/

#ifndef INTEL
#  define INTEL  /* using to select functions/macroses for read & write binary values */
#endif

#  define HAS_MALLOC_H 1      /* use "#include <malloc.h>" for malloc() etc. */
#  define HAS_DIR_H    1      /* use "#include <dir.h>" for findfirst() etc. */
#  define HAS_DOS_H    1      /* use "#include <dos.h>" for delay(), intr() etc. */
#  define HAS_IO_H     1      /* access(), open(), ... */

#  if defined(__TURBOC__DOS__)/* Turbo C/C++ & Borland C/C++ for MS-DOS */

   /* for BC++ 3.1 */
#    define strcasecmp stricmp
#    define strncasecmp strncmpi

#    define _stdc cdecl
#    define _intr interrupt far
#    define _intcast void (_intr *)()
#    define _veccast _intcast
#    define _fast _fastcall
#    define _loadds

     /* #include <conio.h> */
#    define mysleep(x) delay(x);

#    ifndef _XPENTRY
#      define _XPENTRY
#    endif

#    define mode_t int

   /* Borland Turbo C/C++ for MS-DOS */
#  elif defined(__TURBOC__WIN32__)
   /* Borland C/C++ for Win32 */

#    define _stdc cdecl
#    define _intr
#    define _intcast
#    define _veccast
#    define _fast _fastcall
#    define _loadds
#    define near
#    undef  far
#    define far

#    define farread read
#    define farwrite write

#    define _XPENTRY

#    define strcasecmp stricmp
#    define strncasecmp strncmpi

   /* End: Borland C/C++ for Win32 */
#  elif defined(__TURBOC__OS2__)
   /* Borland C/C++ for OS/2 */

#    define _stdc cdecl
#    define _intr
#    define _intcast
#    define _veccast
#    define _fast _fastcall
#    define _loadds
#    define near
#    undef  far
#    define far

#    define farread read
#    define farwrite write

#    ifndef _XPENTRY
#      define _XPENTRY __syscall
#    endif
#    define mysleep(x) sleep(x);

#    include <io.h>
#    include <dos.h>

#    define strcasecmp stricmp
#    define strncasecmp strncmpi

#  endif /* End: Borland C/C++ for OS/2 **************************************/
/* End Turbo C/C++ & borland C/C++********************************************/

#elif defined(__IBMC__OS2__) /* IBM C/Set++ for OS/2**************************/

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds

#  define cdecl
#  define pascal
#  define near
#  undef  far
#  define far

#  define farread read
#  define farwrite write
#  define mysleep(x) DosSleep(1000L*(x))

#  define _XPENTRY pascal far

#  define HAS_MALLOC_H        /* use "#include <malloc.h>" for malloc() etc. */

/* End: IBM C/Set++ for OS/2 */
#elif defined(UNIX) /* Unix clones: Linux, FreeBSD, SUNOS (Solaris), BeOS, MacOS etc. */

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds

#  define cdecl
#  define pascal
#  define near
#  undef  far
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
#  endif
#  ifndef strnicmp
#    define strnicmp strncasecmp
#  endif

/* Cygwin defines O_BINARY in sys/fcntl.h. */
#  if !defined(__BEOS__) && !defined(__CYGWIN__)
#    ifndef O_BINARY
#      define O_BINARY 0
#    endif
#  else
#    include <fcntl.h>
#    ifndef O_BINARY
#      define O_BINARY 0
#    endif
#  endif

#  ifndef SH_DENYNONE
#    define SH_DENYNONE 0
#  endif
#  ifndef SH_DENYNO
#    define SH_DENYNO 0
#  endif
#  ifndef SH_DENYALL
#    define SH_DENYALL 1
#  endif

#  define _XPENTRY

/* Other OS's may sleep with other functions */

#  ifdef __BEOS__
#    define mysleep(x) snooze(x*1000000l)
#  elif defined(__linux__) || defined(__sun__)
#    define mysleep(x) usleep(x*1000000l)
#  endif

#  define HAS_SNPRINTF  1
#  define HAS_VSNPRINTF 1

#  define USE_STAT_MACROS

/* End: Unix clones **********************************************************/

#elif defined(SASC) /* SAS C for AmigaDOS ************************************/

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds

#  define cdecl
#  define pascal
#  define near
#  undef  far
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
#  error compiler.h: Unknown compiler!
#endif   /* End compiler-specific decrarations */

/* Test defines and/or set default values */

#ifdef SH_DENYNO
#ifndef SH_DENYNONE
#define SH_DENYNONE SH_DENYNO
#endif
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

/*
#ifndef mymkdir
#warning mkdir() call set to default value. Please check your compiler documentation for it and write define into compiler.h
#  define mymkdir mkdir
#endif

#ifndef mysleep
#warning sleep() call undefined. Please check your compiler documentation for it and write define into compiler.h
#  define mysleep(x)
#endif

#ifndef SMAPI_EXT
#warning Please set SMAPI_EXT to extern or proprietary token
#  define SMAPI_EXT extern
#endif

#ifndef _XPENTRY
#warning Please check your compiler to system functions call modifyer and define _XPENTRY
#  define _XPENTRY
#endif

#ifndef _stdc
#warning Please check your compiler to standard C code modifyer and define _stdc in compiler.h
#endif

#ifndef _intr
#warning Please check your compiler to interrupt handler modifyer and define _intr in compiler.h
#endif

#ifndef _intcast
#warning Please check your compiler to int. cast modifyer and define _intcast in compiler.h
#endif
#ifndef _veccast
#warning Please check your compiler to vector cast modifyer and define _veccast in compiler.h
#endif
#ifndef _fast
#warning Please check your compiler to fast functions call modifyer and define _fast in compiler.h
#endif
#ifndef _loadds
#warning Please check your compiler to "load data segment" code modifyer and define _loadds in compiler.h
#endif
#ifndef cdecl
#warning Please check your compiler to C declarations modifyer and define cdecl in compiler.h
#endif
#ifndef pascal
#warning Please check your compiler to pascal style calling conversion code modifyer and define pascal in compiler.h
#endif
#ifndef near
#warning Please check your compiler to near functions call modifyer and define near in compiler.h
#endif
#ifndef far
#warning Please check your compiler to far functions call modifyer and define far in compiler.h
#endif
#ifndef farread
#warning Please check your compiler to far calling implementation of read() function and define farread in compiler.h
#endif
#ifndef farwrite
#warning Please check your compiler to far calling implementation of write() function and define farwrite in compiler.h
#endif
*/

 /* waitlock works like lock, but blocks until the lock can be
  * performed.
  * waitlock2 works like a timed waitlock.
  */
extern int waitlock(int, long, long);
extern int waitlock2(int, long, long, long);

#if !defined(NO_MKTIME) && !defined(HAS_MKTIME)

/* If compiler doesn't include a mktime(), we need our own (see strftim.c) */

#include <time.h>
#include <sys/types.h>

time_t _stdc mktime(struct tm *tm_ptr);

#endif

/* If compiler doesn't include a strftime(), we need our own (see strftim.c) */
#if !defined(NO_STRFTIME) && !defined(HAS_STRFTIME)

#include <time.h>
#include <sys/types.h>

size_t _stdc strftime(char *str, size_t maxsize, const char *fmt,
  const struct tm *tm_ptr);

#endif

#ifndef min
#  define min(a,b)              (((a) < (b)) ? (a) : (b))
#endif

#endif
