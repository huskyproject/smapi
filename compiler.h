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
 *
 *  Modifications from MSGAPI are made by HUSKY: http://husky.sf.net
 */

  /*
   * Please define this compiler-specific stuff for each new compiler:
   *
   * _stdc    - Standard calling sequence.  This should be the type of function
   *            required for function pointers for qsort() et al.
   *
   * _fast    - Fastest calling sequence supported.  If the default calling
   *            sequence is the fastest, or if your compiler only has one,
   *            define this to nothing.
   *
   * _intr    - For defining interrupt functions.  For some idiotic reason,
   *            MSC requires that interrupt routines be declared
   *            as "cdecl interrupt", instead of just "interrupt".
   *
   * _intcast - interrupt cast modifyer
   *
   * _veccast - vector cast modifyer
   *
   * far      - use pointer with any segment part, actually for x86 16 bit only
   *            and in other cases must be set to empty value
   *
   * near     - (used for x86 16 bit only) use one-segment pointer
   *
   * pascal   - pascal style calling conversion code modifyer
   *
   * cdecl    - C declarations modifyer (alternate to pascal usually)
   *            Declare if compiler don't support this.
   *
   * _loadds  - (used for x86 16 bit only) 'load data segment' code modifyer
   *
   * SMAPI_EXT - external variables & external functions call modifier
   *             (usualy 'extern' for static linkage)
   *
   * _XPENTRY  - system procedures calling (conversion) modifyer
   *             ("pascal", "_system" & etc)
   *
   *
   *======================
   * HAS_* please set to 1 for usage: #if HAS_...
   *======================
   * HAS_snprintf        - snprintf() presents
   * HAS_asprintf        - asprintf() presents
   * HAS_asnprintf       - asnprintf() presents
   * HAS_vasprintf       - vasprintf() presents
   * HAS_vsnprintf       - vsnprintf() presents
   * HAS_spawnvp         - spawnwp() presents
   * HAS_getpid          - getpid() presents
   * HAS_mktime          - mktime() presents or defined here
   * HAS_strftime        - strftime() presents
   * HAS_sopen           - sopen() presents
   * HAS_sleep           - sleep() presents or defined here
   * HAS_dos_read        - dos_read() presents or defined here
   * HAS_popen_close     - popen(); pclose() ("pipe open" and "pipe close")
   * HAS_strupr		 - strupr() presents
   *
   * HAS_MALLOC_H        - may be used "#include <malloc.h>" for malloc() etc.
   * HAS_DOS_H           - may be used "#include <dos.h>"
   * HAS_DPMI_H          - may be used "#include <dpmi.h>"
   * HAS_DIR_H           - may be used "#include <dir.h>" for findfirst() etc.
   * HAS_DIRENT_H        - may be used "#include <dirent.h>" for opendir() etc.
   * HAS_IO_H            - may be used "#include <io.h>"
   * HAS_UNISTD_H        - may be used "#include <unistd.h>"
   * HAS_PROCESS_H       - may be used "#include <process.h>"
   * HAS_SHARE_H         - may be used "#include <share.h>" for sopen() etc.
   * HAS_PWD_H           - may be used "#include <pwd.h>"
   * HAS_GRP_H           - may be used "#include <grp.h>"
   * HAS_UTIME_H         - may be used "#include <utime.h>"
   * HAS_SYS_UTIME_H     - #include <sys/utime.h> in alternate to <utime.h>
   * HAS_SYS_PARAM_H     - #include <sys/params.h>
   * HAS_SYS_MOUNT_H     - #include <sys/mount.h>
   * HAS_SYS_WAIT_H      - #include <sys/wait.h>
   * HAS_SYS_STATVFS_H   - #include <sys/statvfs.h>
   * HAS_SYS_VFS_H       - #include <sys/vfs.h>
   * HAS_SYS_SYSEXITS_H  - #include <sys/sysexits.h>
   * HAS_SYSEXITS_H      - #include <sysexits.h>
   *
   * USE_SYSTEM_COPY     - OS have system call for files copiing (see
   *                       copy_file() and move_file() functions)
   * USE_SYSTEM_COPY_WIN32  - Windows 9x/NT system copy routine
   * USE_SYSTEM_COPY_OS2    - OS/2 system copy routine
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
   *
   * farread(a,b,c)     - for flat memory models declare as read(a,b,c)
   * farwrite(a,b,c)    - for flat memory models declare as write(a,b,c)
   * NEED_trivial_farread  - macro-flag: need use my own trivial_farread()
   *                         instead farread() (implemented in structrw.c)
   * NEED_trivial_farwrite - macro-flag: need use my own trivial_farwrite()
   *                         instead farwrite() (implemented in structrw.c)
   * MAXPATHLEN         - max path len value for disk i/o functions
   *
   ***************************************************************************
   * Memory and platforms
   *
   * __BIG_ENDIAN__    - big endian bytes order in memory
   * __LITTLE_ENDIAN__ - little endian bytes order in memory (like Intel x86)
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
   * Platforms & OS (binary targets)
   *
   * __NT__    - Windows NT/2000/XP target
   * __WIN32__ - Windows 95/98/Me/NT/2000/XP target
   * __OS2__   - OS/2 target (32 bit or 16 bit), 32bit is __OS2__ && __FLAT__
   * __DOS__   - MS/PC/... DOS target (32 bit or 16 bit), 32bit is __DOS__ && __FLAT__
   * __DOS16__ - MS/PC/... DOS target 16 bit
   * __DPMI__  - DOS 32 bit (extenders: dos4g, farcall, rsx, ...)
   * __MACOS__ - MacOS (Unix clone)
   * __UNIX__  - All unix-like OS
   * __BSD__   - BSD UNIX clones (BSDI, BSD/OS, FreeBSD, NetBSD, OpenBSD & etc)
   * __LINUX__ - GNU/Linux (unix clone)
   * __AMIGA__ - AmigaOS
   * __ALPHA__ - The Alpha CPU
   * __X86__   - Intel's x86 series CPU
   * __PPC__   - The PowerPC CPU
   * __MPPC__  - The PowerPC CPU on Apple Macintosh
   *
   *--------------------------------------------------------------------------
   * CPU
   *
   * __186__   - Intel 80186 CPU
   * __286__   - Intel 80286 CPU
   * __386__   - Intel 80386 CPU
   * __486__   - Intel 80486 CPU
   * __586__   - Intel Pentium CPU
   * __686__   - Intel Pentium Pro CPU
   * __786__   - Intel Pentium II CPU
   *
   */

/**************************************************************************
 * For informaion: list of predefined macroses for known compilers.       *
 **************************************************************************
   Common macroses for GNU C
   -------------------------------------------------------------------
    __GNUC__          Major version number (2.95.* : 2)
    __GNUC_MINOR__    Minor version number (2.95.* : 95)
    __VERSION__       String's compiler version representation
   ===================================================================
   MINGW32 for 32-bit Windows NT on Intel and AXP; (GNU C clone)
   MINGW32 cross-compiler from unixes;             (GNU C clone)
   Cygwin GCC with option -mno-cygwin.             (GNU C clone)
   -------------------------------------------------------------------
    __MINGW32__ __GNUC__  __STDC__
   --CygWin v1.3.16-1 with gcc 3.2:-----------------------------------
     __GNUC__=3 (0x3); __GNUC_MINOR__=2 (0x2)
     __VERSION__=3.2 20020927 (prerelease)
   --MinGW v1.1 with gcc-2.95.3---------------------------------------
   __GNUC__=2 (0x2); __GNUC_MINOR__=95 (0x5F)
   __VERSION__=2.95.3-6 (mingw special)
   ===================================================================
   DJGPP (DOS DPMI)                          (GNU C clone)
   -------------------------------------------------------------------
    __DJGPP__ MSDOS __MSDOS__  __GNUC__  __STDC__
   Some values:
   __GNUC__=3 (0x3); __GNUC_MINOR__=2 (0x2)
   __VERSION__=3.2
   ===================================================================
   EMX (OS/2)                                (GNU C clone)
   -------------------------------------------------------------------
    __EMX__
    __GNUC__ __VERSION__ __STDC__
    __CHAR_UNSIGNED__ (or __CHAR_SIGNED__)
    __32BIT__ - if build 32bit code
   ---Some values (EMX rev 61):---------------------------------------
    __GNUC__=2 (0x2); __GNUC_MINOR__=8 (0x8)
    __VERSION__=2.8.1
    __STDC__=1 (0x1); __STDC_VERSION__=199409 (0x30AF1);
   ===================================================================
   GNU C on FreeBSD
   -------------------------------------------------------------------
    __FreeBSD__  __unix__  unix  __GNUC__  __STDC__
   --- Some values (FreeBSD 4.7) -------------------------------------
   __GNUC__=2 (0x2); __GNUC_MINOR__=95 (0x5F)
   __VERSION__=2.95.4 20020320 [FreeBSD]
   __STDC__=1 (0x1); __STDC_VERSION__=199409 (0x30AF1);
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
   __WATCOMC__  __OS2__ __FLAT__ __386__ M_I386 _M_I386 __STDC__
   ===================================================================
   Watcom C: OS/2 16bit target
   -------------------------------------------------------------------
   __WATCOMC__  __OS2__ __I86__ M_I86 _M_I86
   ===================================================================
   Watcom C: Windows NT and Windows 95 target (-bt=nt)
   -------------------------------------------------------------------
   __WATCOMC__  __NT__ __FLAT__ __386__ M_I386 _M_I386
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
   __WATCOMC__ __DOS4G__  __FLAT__ __386__ __DOS4G__ M_I386 _M_I386
   ===================================================================
   Watcom C other - from WatcomC help & test program
   -------------------------------------------------------------------
   The __NETWARE__ and __NETWARE_386__ macros are defined when the build
   target is "NETWARE" (Novell NetWare)
   The __QNX__ macro is defined when the build target is "QNX" (16-bit
   or 32-bit QNX).
   Watcom C++ predefines the macro __cplusplus to identify the compiler
   as a C++ compiler. Also __WATCOM_CPLUSPLUS
   -------------------------------------------------------------------
   __WATCOMC__ value is compiler_version*100
   __WATCOMC__=1100 (0x44C) - Watcom C 11.0c
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
   Borland C and Turbo C
   -------------------------------------------------------------------
   __TURBOC__  = 397 (0x18D) for TurboC 2.0
   __TURBOC__  =1040 (0x410) for Borland C++ 3.1
   __TURBOC__  =1120 (0x460) for Borland C++ 5.02
   __TURBOC__  =1360 (0x550) for Borland C++ 5.5
   __BCPLUSPLUS__  =0x340   Borland C++ 5.0 (Defined for C++ compilation)
   __BORLANDC__    =0x500   Borland C++ 5.0 Version number
   ===================================================================
   Borland C and Turbo C for DOS
   -------------------------------------------------------------------
   __TURBOC__ __MSDOS__
   ===================================================================
   Borland C for Win32
   -------------------------------------------------------------------
   __TURBOC__ __WIN32__ __FLAT__
   ===================================================================
   Borland C for OS/2
   -------------------------------------------------------------------
   __TURBOC__ __OS2__
   ===================================================================
   Otther Borland C macros
   -------------------------------------------------------------------
   __BCOPT__       =1       Defined in any compiler that has an optimizer
   __CDECL__       =1       Defined if Calling Convention is set to C;
                            otherwise undefined
   _CHAR_UNSIGNED  1   Defined by default indicating that the default char is
   unsigned char. Use the -K option to undefine this macro.
   ===================================================================

   ===================================================================
   Microsoft Visual C/C++
   -------------------------------------------------------------------
   _MSC_VER    value is greated or eq 1200
   _MSC_VER=1200 (0x4B0) - MS Visual C++ v6.0 sp5:
   --------
   _WIN32      Defined for applications for Win32. Always defined.
   _CHAR_UNSIGNED Default char type is unsigned. Defined when /J is specified.
   __cplusplus Defined for C++ programs only.
   _CPPRTTI Defined for code compiled with /GR (Enable Run-Time Type Information).
   _CPPUNWIND Defined for code compiled with /GX (Enable Exception Handling).
   _DLL Defined when /MD or /MDd (Multithread DLL) is specified.
   _M_ALPHA Defined for DEC ALPHA platforms. It is defined as 1 by the ALPHA compiler, and it is not defined if another compiler is used.
   _M_IX86 Defined for x86 processors. See Table 1.3 for more details.
   _M_MPPC Defined for Power Macintosh platforms. Default is 601 (/QP601). See Table 1.4 for more details.
   _M_MRX000 Defined for MIPS platforms. Default is 4000 (/QMR4000). See Table 1.5 for more details.
   _M_PPC Defined for PowerPC platforms. Default is 604 (/QP604). See Table 1.6 for more details.
   _MFC_VER Defines the MFC version. Defined as 0x0421 for Microsoft Foundation Class Library 4.21. Always defined.
   _MSC_EXTENSIONS This macro is defined when compiling with the /Ze compiler option (the default).  Its value, when defined, is 1.
   _MSC_VER Defines the compiler version. Defined as 1200 for Microsoft Visual C++ 6.0. Always defined.
   _MT Defined when /MD or /MDd (Multithreaded DLL) or /MT or /MTd (Multithreaded) is specified.
   -------------------------------------------------------------------
   _M_IX86 = 300  - CPU 80486 (/G3)
   _M_IX86 = 400  - CPU 80486 (/G4)
   _M_IX86 = 500  - CPU 80486 (/G5) - default
   _M_IX86 = 600  - CPU 80486 (/G6)
   _M_MPPC = 601  - PowerPC 601 (/QP601) - Default
   _M_MPPC = 603  - PowerPC 603 (/QP603)
   _M_MPPC = 604  - PowerPC 604 (/QP604)
   _M_MPPC = 620  - PowerPC 620 (/QP620)
   _M_PPC = 601  - PowerPC 601 /QP601
   _M_PPC = 603  - PowerPC 603 /QP603
   _M_PPC = 604  - PowerPC 604 /QP604 - Default
   _M_PPC = 620  - PowerPC 620 /QP620
   _M_MRX000 = 4000  - R4000 (/QMR4000) - Default
   _M_MRX000 = 4100  - R4100 (/QMR4100)
   _M_MRX000 = 4200  - R4200 (/QMR4200)
   _M_MRX000 = 4400  - R4400 (/QMR4400)
   _M_MRX000 = 4600  - R4600 (/QMR4600)
   _M_MRX000 = 10000 - R10000 (/QMR10000)

   ===================================================================
   Microsoft C for MS-DOS or OS/2
   -------------------------------------------------------------------
   _MSC_VER    value is less 1200
   ===================================================================
   Microsoft Quick C 2.0 for MS-DOS
   -------------------------------------------------------------------
   _QC  MSDOS  __STDC__  M_I86  M_I8086
   M_I286   is defined if buld code for system based on 80286
   _QC      = 1
   __STDC__ = 0
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
#  ifndef __DPMI__
#    define __DPMI__
#  endif
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif

#if defined(__RSXNT__)
#  ifndef __NT__
#    define __NT__
#  endif
#endif

#if defined(__EMX__) && defined(__NT__)
#  ifndef __RSXNT__
#    define __RSXNT__
#  endif
#endif

#if defined(__EMX__) && defined(__32BIT__)
#  ifndef __386__
#    define __386__
#  endif
#  ifndef __X86__
#    define __X86__
#  endif
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
   /* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */
#    define __MSC__
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
#  if defined(__DOS__) && !defined(__WATCOMC__DOS__)
#    define __WATCOMC__DOS__
#  endif
#  if defined(__DOS4G__) && !defined(__WATCOMC__DOS4G__)
#    define __WATCOMC__DOS4G__
#  endif
#  if (defined(__OS2__) || defined(__OS2V2__)) && !defined(__WATCOMC__OS2__)
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
#  if defined(__MSDOS__)   /* Turbo C/C++ & Borland C/C++ for MS-DOS */
#    if !defined(__TURBOC__DOS__)
#      define __TURBOC__DOS__
#    endif
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

#if defined(__linux__) || defined(__Linux__) || defined(linux) || defined(__linux) || defined(LINUX)
#  if !defined(__LINUX__)
#    define __LINUX__
#  endif
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined( __svr4__ ) || defined( __SVR4 )
#  if !defined(__SVR4__)
#    define __SVR4__
#  endif
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined(BSD)
#  if !defined(__BSD__)
#    define __BSD__
#  endif
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif


#if defined(__DOS4G__) /* DOS4G/W dos-dpmi extender */
#ifndef __DPMI__
#  define __DPMI__
#endif
#ifndef __FLAT__
#  define __FLAT__
#endif
#endif

/**** OS defines ****/

#if defined(__TURBOC__DOS__) && !defined(__DOS__)
#  define __DOS__
#endif

#if defined(__MSDOS__) || defined(DOS) || defined(MSDOS)
#  if !defined(__DOS__)
#    define __DOS__
#  endif
#endif

#if defined(__DOS__)
#  if !defined(__FLAT__) && !defined(__DPMI__)
#    define __DOS16__
#  endif
#endif

#if defined(__OS2V2__) && !defined(__OS2__)
/*  Watcom C: wcl -bt=os2v2 */
#  define __OS2__
#endif

#if defined(_M_MPPC)
#  if !defined(__MACOS__)
#    define __MACOS__
#  endif
#endif

#if defined(__WIN32) || defined(_WIN32) || defined(WIN32)
#  if !defined(__WIN32__)
#    define __WIN32__
#  endif
#endif

#if defined(NT) || defined(WINNT)
#  if !defined(__NT__)
#    define __NT__
#  endif
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
#  if !defined(__BSD__)
#    define __BSD__
#  endif
#endif

#if defined(__SUN__) || defined(__LINUX__)
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined(_AIX)
#  if !defined(__AIX__)
#    define __AIX__
#  endif
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined(__osf__)
#  if !defined(__OSF__)
#    define __OSF__
#  endif
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined(__hpux)
#  if !defined(__HPUX__)
#    define __HPUX__
#  endif
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif


/*
  BeOS is NOT Unix, but sometime it seem's to Be ... ;)
*/
#if defined (__BEOS__) || defined(__BeOS__)
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined(SASC)  /* SAS C for AmigaDOS ***************/
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

#if defined(UNIX) || defined(_UNIX) || defined(__unix) || defined(__unix__)
#  if !defined(__UNIX__)
#    define __UNIX__
#  endif
#endif

/***** Platforms *************************************************************/

#if defined(SASC) && !defined(__AMIGA__) /* SAS C for AmigaDOS ***************/
#  define __AMIGA__
#endif

#if defined(__alpha) || defined(__alpha__) || defined(_M_ALPHA) || defined(M_ALPHA)
#  ifndef __ALPHA__
#    define __ALPHA__
#  endif
#endif

#if defined(_M_IX86)  /* MS Visual C predefined */
#  ifndef __X86__
#    define __X86__
#  endif
#  if _M_IX86 >= 300
#    ifndef __386__
#      define __386__
#    endif
#  endif
#  if _M_IX86 >= 400
#    ifndef __486__
#      define __486__
#    endif
#  endif
#  if _M_IX86 >= 500
#    ifndef __586__
#      define __586__
#    endif
#  endif
#  if _M_IX86 >= 600
#    ifndef __686__
#      define __686__
#    endif
#  endif
#endif

#if defined(__IX86__) || defined(_X86_)
#  ifndef __X86__
#    define __X86__
#  endif
#endif

#if defined(__i386__) || defined(__i386) || defined(i386)
#  ifndef __X86__
#    define __X86__
#  endif
#  ifndef __386__
#    define __386__
#  endif
#endif

#if defined(__i686__) || defined(__i686) || defined(i686)
#  ifndef __X86__
#    define __X86__
#  endif
#  ifndef __686__
#    define __686__
#  endif
#endif

#if defined(_M_MPPC) /* Power PC Macintosh */
#endif

#if defined(_M_PPC) /* Power PC */
#  ifndef __PPC__
#    define __PPC__
#  endif
#endif

#if defined(_M_MRX000) /* MIPS */
#endif

#ifdef __ALPHA__
#  ifndef __BIG_ENDIAN__
#    define __BIG_ENDIAN__
#  endif
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif

#if defined(__X86__)
#  ifndef __LITTLE_ENDIAN__
#    define __LITTLE_ENDIAN__
#  endif
#endif


#ifdef __SUN__
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif


#if defined (__CYGWIN__) || defined(__MINGW32__)
#  ifndef __X86__
#    define __X86__
#  endif
#  ifndef __386__
#    define __386__
#  endif
#  ifndef __LITTLE_ENDIAN__
#    define __LITTLE_ENDIAN__
#  endif
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif

#if defined(__DOS__) || defined(__DPMI__)
#  ifndef __LITTLE_ENDIAN__
#    define __LITTLE_ENDIAN__
#  endif
#endif

#if defined(__NT__)
#  if !defined(__WIN32__)
#    define __WIN32__
#  endif
#endif

#ifdef __386__
#  ifndef __LITTLE_ENDIAN__
#    define __LITTLE_ENDIAN__
#  endif
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#  ifndef __X86__
#    define __X86__
#  endif
#  ifndef __186__
#    define __186__
#  endif
#  ifndef __286__
#    define __286__
#  endif
#endif

/***** memory models *********************************************************/

#if defined(__DPMI__) || defined(__WIN32__) || defined(__NT__) || defined(__UNIX__)
#  ifndef __FLAT__
#    define __FLAT__
#  endif
#endif

#if defined(__OS2__) && !defined(_MSC_VER)
#if !defined(__386__) && !defined(__FLAT__)
#error Please check your compiler to target: 16 bit or 32 bit and sent report to husky developers: http:\/\/sf.net/projects/husky
#endif
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
/*   must be included before function redefenition like '#define fileno _fileno' */
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

#   include <limits.h>
#   ifndef MAXPATHLEN
#     define MAXPATHLEN _MAX_PATH
#   endif

/*   must be included before macro redefenition '# define SH_DENYNONE _SH_DENYNO' */
#   include <share.h>
#   ifndef SH_DENYNONE
#     ifdef _SH_DENYNO
#       define SH_DENYNONE _SH_DENYNO
#     else
#       pragma message("Please set SH_DENYNONE to proprietary value: used for file locking")
#     endif
#   endif
#   ifndef SH_DENYNO
#     ifdef SH_DENYNONE
#       define SH_DENYNO SH_DENYNONE
#     else
#       pragma message("Please set SH_DENYNO to proprietary value: used for file locking")
#     endif
#   endif
/*   must be included before function redefenition '#define P_WAIT _P_WAIT'  */
#   include <process.h>
#   ifndef P_WAIT
#     ifdef _P_WAIT
#       define P_WAIT		_P_WAIT   /* process.h */
#     else
#       pragma message("Please set P_WAIT to proprietary value: used for spawnvp() call")
#     endif
#   endif

#  define _stdc
#  ifndef pascal
#    define pascal
#  endif
#  ifndef far
#    define far
#  endif
#  define _fast
#  define near
#  define _XPENTRY
#  define _intr
#  define _intcast
#  define _veccast
#  define _loadds
#  define cdecl

#  define strcasecmp  stricmp
#  define strncasecmp strnicmp

#  define sleep(x)    Sleep(1000L*(x))
#  define mysleep(x)  Sleep(1000L*(x))
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
#  ifndef F_OK                  /* does file exist */
#    define F_OK 0
#  endif

#  ifndef X_OK                  /* is it executable by caller */
#    define X_OK  1
#  endif

#  ifndef R_OK                  /* is it readable by caller */
#    define R_OK 04
#  endif

#  ifndef W_OK                  /* is it writable by caller */
#    define W_OK 02
#  endif

#  define mymkdir       _mkdir
#  define snprintf      _snprintf
#  define vsnprintf     _vsnprintf
#  define HAS_snprintf       /* snprintf() presents */
#  define HAS_vsnprintf      /* vsnprintf() presents */
#  define HAS_spawnvp        /* spawnwp() presents */
#  define HAS_strftime
#  define HAS_mktime
#  define HAS_sopen
#  define HAS_sleep
#  define HAS_strupr

#  define USE_SYSTEM_COPY     /* OS have system call for files copiing */
#  define USE_SYSTEM_COPY_WIN32
#  define USE_STAT_MACROS     /* S_ISDIR, S_ISREG and stat() presents */

#  define HAS_IO_H         /* may use "#include <io.h> */
#  define HAS_SHARE_H      /* may use "#include <share.h> */
#  define HAS_MALLOC_H     /* use "#include <malloc.h>" for malloc() etc. */
#  define HAS_DIRECT_H
#  define HAS_SYS_UTIME_H  /* #include <sys/utime.h> in alternate to <utime.h> */
#  define HAS_DIRECT_H     /* #include <direct.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

   SMAPI_EXT int unlock(int handle, long ofs, long length);
   SMAPI_EXT int lock(int handle, long ofs, long length);


/* End: MS Visual C/C++ ******************************************************/

#elif defined(__MSC__) /* Microsoft C or Microsoft QuickC for MS-DOS or OS/2 */

#  ifndef __LITTLE_ENDIAN__
#    define __LITTLE_ENDIAN__
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
#    define sleep(x) DosSleep(1000L*(x))
#    define HAS_sleep     1
#  else
#    define _XPENTRY
#  endif

#  define HAS_MALLOC_H        /* use "#include <malloc.h>" for malloc() etc. */
#  define HAS_IO_H     1  /* may use "#include <io.h> */
#  define HAS_SHARE_H  1  /* may use "#include <share.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

#  define USE_STAT_MACROS

/* End: Microsoft C or Microsoft QuickC for MS-DOS or OS/2 *******************/

/* Begin: Watcom C all variants (DOS, Win, OS/2, QNX) ************************/
#elif defined(__WATCOMC__)

#  define mode_t int
#  define SMAPI_EXT extern

#  define strcasecmp  stricmp
#  define strncasecmp strnicmp
#  define snprintf    _snprintf
#  define vsnprintf   _vsnprintf
#  define HAS_snprintf
#  define HAS_vsnprintf
#  define HAS_spawnvp
#  define HAS_getpid
#  define HAS_strftime    /* strftime() in time.h  */
#  define HAS_mktime      /* mktime() in time.h */

#  define HAS_MALLOC_H     /* may be used "#include <malloc.h>"  (see alc.h) */
#  define HAS_IO_H         /* may use "#include <io.h> */
#  define HAS_SHARE_H      /* may use "#include <share.h> */
#  define HAS_DIRECT_H
#  define HAS_SYS_UTIME_H  /* #include <sys/utime.h> in alternate to <utime.h> */
#  define HAS_DOS_H
#  define HAS_SIGNAL_H  1  /* <signal.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

#  define mymkdir(x)    mkdir(x) /*<direct.h>*/
#  define HAS_mkdir
#  include <dos.h>
#  define mysleep(x)    sleep(x) /* dos.h */
#  define HAS_sleep

#  if defined(__WATCOMC__DOS4G__)
/* WATCOM C/C++ for DOS4G*/

#    define _stdc      cdecl
#    define _intr      interrupt far
#    define _intcast   void (_intr *)()
#    define _veccast   _intcast
#    define _fast      pascal

#    define farread    read
#    define farwrite   write

#    define _XPENTRY   pascal

/* End: WATCOM C/C++ for MS-DOS4G */
#  elif defined(__WATCOMC__DOS__)
/* WATCOM C/C++ for MS-DOS or DOS4G*/

#    define _stdc      cdecl
#    define _intr      interrupt far
#    define _intcast   void (_intr *)()
#    define _veccast   _intcast
#    define _fast      pascal

#    ifdef __FAR_DATA__
#      define farread    read
#      define farwrite   write
#    else
#      define farread    trivial_farread
#      define farwrite   trivial_farwrite
#      define NEED_trivial_farread   1
#      define NEED_trivial_farwrite  1
       int trivial_farread( int handle, void far *buffer, unsigned len );
       int trivial_farwrite( int handle, void far *buffer, unsigned len );
#    endif

#    define HAS_dos_read 1      /* dos_read() */
#    define dos_read _dos_read  /* _dos_read() in dos.h */

#    define _XPENTRY   pascal

/* End: WATCOM C/C++ for MS-DOS */
#  elif defined(__WATCOMC__OS2__)
/* WATCOM C/C++ for OS/2 */

#    define _stdc
#    define _intr
#    define _intcast
#    define _veccast
#    define _fast
#    define farread  read
#    define farwrite write

#    define _XPENTRY   _System

/*#  define mysleep(x) DosSleep(x*1000)*/  /* os2/bsedos.h */

/* End: WATCOM C/C++ for OS/2 */
#  elif defined(__WATCOMC__NT__)
/* WATCOM C/C++ for Windows NT */

#    define _stdc
#    define _intr
#    define _intcast
#    define _veccast
#    define _fast
#    define farread  read
#    define farwrite write

#    define _XPENTRY pascal

/*#  define mysleep(x) Sleep(x*1000) */ /* winbase.h */

#    include <limits.h>
#    ifndef MAXPATHLEN
#      define MAXPATHLEN NAME_MAX
#    endif

/* End: WATCOM C/C++ for Windows NT */
#  endif

/* End: Watcom C all variants ************************************************/

#elif defined(__HIGHC__) /* MetaWare High C/C++ for OS/2 ***********************/

#ifndef __LITTLE_ENDIAN__
#  define __LITTLE_ENDIAN__
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
#  define sleep(x)   DosSlep(1000L*(x))
#  define HAS_sleep     1

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
#  define HAS_IO_H     1  /* may use "#include <io.h> */
#  define HAS_SHARE_H  1  /* may use "#include <share.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

/* End: MetaWare High C/C++ for OS/2 */
#elif defined(__MINGW32__) /* MinGW32 & cygwin's 'gcc -mno-cygwin' ***********/

/* Applied to:
   - MINGW32 for 32-bit Windows NT on Intel and AXP;
   - MINGW32 cross-compiler from unixes;
   - Cygwin GCC with option -mno-cygwin.
*/

#ifndef __LITTLE_ENDIAN__
#  define __LITTLE_ENDIAN__
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
#  define mysleep(sec) _sleep((sec)*1000l)
#  define HAS_sleep     1

#  define mymkdir mkdir

/*#  define strcasecmp  stricmp*/
/*#  define strncasecmp strnicmp*/

  int unlock(int handle, long ofs, long length);
  int lock(int handle, long ofs, long length);

/* older mingw headers are too lazy ... */
#  include <share.h>
#  define sopen _sopen
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

#  define HAS_spawnvp    /* spawnvp() present */
#  define HAS_mktime     /* time.h */
#  define HAS_strftime   /* time.h */
#  define HAS_ACCESS    /* access() in io.h */
#  define HAS_sopen

#  define HAS_MALLOC_H   /* may use "#include <malloc.h>" for malloc() etc. */
#  define HAS_IO_H       /* may use "#include <io.h> */
#  define HAS_SHARE_H    /* may use "#include <share.h> */
#  define HAS_DIRENT_H   /* may use "#include <dirent.h> */
#  define HAS_SYS_UTIME_H  /* #include <sys/utime.h> in alternate to <utime.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

#  define USE_STAT_MACROS

/* end: MinGW32 **************************************************************/

#elif defined(__EMX__)/* EMX for 32-bit OS/2 and RSX for Windows NT **********/

#ifndef __LITTLE_ENDIAN__
#  define __LITTLE_ENDIAN__  /* using to select functions/macroses for read & write binary values */
#endif

#  define _stdc
#  define _intr
#  define _intcast
#  define _veccast
#  define _fast
#  define _loadds
#  define cdecl
#  define pascal
#  if defined(__32BIT__)
#    define near
#    undef  far
#    define far
#    define farread(a,b,c)  read(a,b,c)
#    define farwrite(a,b,c) write(a,b,c)
#  endif

#  define mymkdir(a) mkdir((a), 0)
#  define getpid() _getpid()

   int unlock(int handle, long ofs, long length);
   int lock(int handle, long ofs, long length);

#  define strcasecmp stricmp
#  define strncasecmp strnicmp

#  define _XPENTRY

#  define HAS_snprintf  1
#  define HAS_vsnprintf 1
#  define HAS_getpid    1  /* getpid() in process.h */
#  define HAS_spawnvp   1  /* spawnvp() in process.h */
#  define HAS_strftime  1  /* strftime() in time.h  */
#  define HAS_mktime    1  /* mktime() in time.h */
#  define HAS_popen_close  /* popen(); pclose() */

#  define HAS_DIRENT_H  1  /* use "#include <dirent.h>" for opendir() etc. */
#  define HAS_IO_H      1  /* use "#include <io.h>" */
#  define HAS_SHARE_H   1  /* may use "#include <share.h> */
#  define HAS_UNISTD_H  1  /* use "#include <unistd.h> */
#  define HAS_SIGNAL_H  1  /* <signal.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

#  define USE_STAT_MACROS

/* End: EMX for 32-bit OS/2 and RSX for Windows NT ***************************/

#elif defined(__DJGPP__) /* DJGPP for MS-DOS (DPMI)***************************/

#ifndef __LITTLE_ENDIAN__
#  define __LITTLE_ENDIAN__  /* using to select functions/macroses for read & write binary values */
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
/*#  undef  far*/
#  define far
#  define _XPENTRY
#  define SMAPI_EXT extern

#  define farread read
#  define farwrite write

#  define mymkdir(a) mkdir((a), 0)

#  if defined(__dj_include_fcntl_h_) && !defined( SH_DENYNONE)
#    ifdef  SH_DENYNO
#      define SH_DENYNONE SH_DENYNO
#    else
#      define SH_DENYNONE 0
#    endif
#  endif

#  include <unistd.h>
#  include <io.h>
#  define mysleep(x) sleep(x)
#  define HAS_sleep     1

#  define HAS_spawnvp   1   /* spawnvp() in process.h */
#  define HAS_strftime  1   /* strftime() in time.h  */
#  define HAS_mktime    1   /* mktime() in time.h */
#  define HAS_popen_close  /* popen(); pclose() */

#  define HAS_DIR_H     1   /* use "#include <dir.h>" for findfirst() etc. */
#  define HAS_DOS_H     1
#  define HAS_DPMI_H    1
#  define HAS_IO_H      1   /* use "#include <io.h> */
#  define HAS_SHARE_H   1   /* may use "#include <share.h> */
#  define HAS_UNISTD_H  1   /* use "#include <unistd.h> */
#  define HAS_SIGNAL_H  1  /* <signal.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

#  define USE_STAT_MACROS

/* End: DJGPP for MS-DOS (DPMI) **********************************************/

#elif defined(__TURBOC__)/* Borland Turbo C/C++ & Borland C/C++ **************/

#  define HAS_MALLOC_H 1      /* use "#include <malloc.h>" for malloc() etc. */
#  define HAS_DIR_H    1      /* use "#include <dir.h>" for findfirst() etc. */
#  define HAS_DOS_H    1      /* use "#include <dos.h>" for delay(), intr() etc. */
#  define HAS_IO_H     1      /* access(), open(), ... */
#  define HAS_SHARE_H  1  /* may use "#include <share.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

#if __TURBOC__ == 0x0295
#  define HAS_strftime
#  define HAS_mktime
#endif

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
#    define sleep(x) delay(x);
#    define HAS_sleep     1

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
#    define HAS_sleep     1

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
#  define sleep(x)   DosSleep(1000L*(x))
#  define HAS_sleep     1

#  define _XPENTRY pascal far

#  define HAS_MALLOC_H        /* use "#include <malloc.h>" for malloc() etc. */
#  define HAS_IO_H     1  /* may use "#include <io.h> */
#  define HAS_SHARE_H  1  /* may use "#include <share.h> */
#  define HAS_DIRECT_H 1  /* may use "#include <direct.h> */
#  define HAS_PROCESS_H   /* may use "#include <process.h> */

/* End: IBM C/Set++ for OS/2 */
#elif defined(__UNIX__) /* Unix clones: Linux, FreeBSD, SUNOS (Solaris), BeOS, MacOS etc. */
#  define SMAPI_EXT extern
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

#  if (defined(__APPLE__) && defined(__MACH__)) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(_AIX) || defined(__SUN__) || defined(__LINUX__) || defined(__osf__) || defined(__hpux) || defined(__BEOS__) || defined(__OpenBSD__) || defined(__CYGWIN__)
#    define mymkdir(a) mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#  else
#    define mymkdir(a) __mkdir((a), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#  endif

  int lock(int handle, long ofs, long length);   /* in locking.c */
  int unlock(int handle, long ofs, long length);
  int sopen(const char *name, int oflag, int ishared, int mode);

#  ifndef __SUN__
#    define tell(a) lseek((a),0,SEEK_CUR)
#  endif

#  ifndef stricmp
#    define stricmp strcasecmp
#  endif
#  ifndef strnicmp
#    define strnicmp strncasecmp
#  endif

#if !defined(USG)
#define HAS_SYS_PARAM_H
#endif

#if (defined(BSD) && (BSD >= 199103))
  /* now we can be sure we are on BSD 4.4 */
#define HAS_SYS_MOUNT_H
#endif
  /* we are not on any BSD-like OS */
  /* list other UNIX os'es without getfree mechanism here */
#if defined( __svr4__ ) || defined( __SVR4 ) || defined (__linux__) && defined (__GLIBC__)
#define HAS_SYS_STATVFS_H
#if !defined (__BEOS__)  /* Strange... BeOS is not SVR4, and not linux*/
#define HAS_SYS_VFS_H
#endif
#endif

#if defined (__LINUX__) && !defined(__GLIBC__)
#define HAS_SYS_VFS_H
#endif


/* Cygwin defines O_BINARY in sys/fcntl.h. */
#  if !defined(__BEOS__) && !defined(__CYGWIN__)
#    include <fcntl.h>
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
#    define sleep(x) snooze(x*1000000l)
#    define HAS_sleep     1
#    define HAS_SYS_SYSEXITS_H     1  /*  <sys/sysexits.h> */
#  elif defined(__SUN__)
#    define mysleep(x) usleep(x*1000000l)
#    define sleep(x)   usleep(x*1000000l)
#    define HAS_sleep     1
#  elif defined(__BSD__) || defined(__CYGWIN__) || defined(__LINUX__)
#    define mysleep(x) sleep(x)
#    define HAS_sleep     1
#  endif
#  ifndef __SUN__ /* SunOs 2.7 not have snprintf() and vsnprintf in libc */
                  /* If you known test for this - please report to developers */
#    define HAS_snprintf  1
#    define HAS_vsnprintf 1
#  endif

#  ifndef __BEOS__
#    define HAS_SYSEXITS_H     1  /*  <sysexits.h> */
#  endif
#  define HAS_UNISTD_H         1  /* <unistd.h> */
#  define HAS_PWD_H            1  /* <pwd.h> */
#  define HAS_GRP_H            1  /* may be used "#include <grp.h>" */
#  define HAS_SIGNAL_H         1  /* <signal.h> */
#  define HAS_SYS_WAIT_H       1  /* <sys/wait.h> */
#  define USE_STAT_MACROS

#include <fcntl.h>
#ifndef O_BINARY
# define O_BINARY 0 /* O_BINARY flag has no effect under UNIX */
#endif

#ifndef O_TEXT
# define O_TEXT   0 /* O_TEXT flag has no effect under UNIX */
#endif

#if defined(__LINUX__) || defined(__BSD__) || defined(__CYGWIN__)
#  define HAS_mktime	/* <time.h> */
#  define HAS_strftime	/* <time.h> */
#  define HAS_DIRENT_H  /* <dirent.h> */
#endif

#if defined(__CYGWIN__)
#  define HAS_strupr	/* <string.h> from libc (newlib) */
#endif

#  define HAS_popen_close  /* popen(); pclose() */

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

#include <fcntlh.>
#ifndef O_BINARY
# define O_BINARY 0 /* O_BINARY flag has no effect under UNIX */
#endif

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
#  error compiler.h: Unknown compiler! Please compile and run ../test/compiler.c (possible need RTFM of your compiler: section "Predefined macros" and update ../test/compiler.c)
#endif   /* End compiler-specific decrarations */

/**** Test defines and/or set default values *********************************/

#ifdef HAS_SHARE_H
#  include <share.h>
#  if !defined(SH_DENYNO) && defined(_SH_DENYNO)
#    define SH_DENYNO _SH_DENYNO
#  endif
#  ifdef SH_DENYNO
#    ifndef SH_DENYNONE
#      define SH_DENYNONE SH_DENYNO
#    endif
#  endif
#endif

/* File open and file modes */
#include <fcntl.h>
#if !defined(O_BINARY) && defined(_O_BINARY)
#  define O_BINARY    _O_BINARY
#endif
#if !defined(O_RDWR) && defined(_O_RDWR)
#  define O_RDWR      _O_RDWR
#endif

#include <sys/stat.h>
#if !defined(S_IFMT) && defined(_S_IFMT)
#  define S_IFMT      _S_IFMT
#endif
#if !defined(S_IFDIR) && defined(_S_IFDIR)
#  define S_IFDIR     _S_IFDIR
#endif

#ifndef mymkdir
#   ifdef __GNUC__
#       warning mymkdir() call set to default value. Please check your compiler documentation for it and write define into compiler.h
#   else
#       pragma message("mymkdir() call undefined. Please check your compiler documentation for it and write define into compiler.h")
#   endif
#   define mymkdir mkdir
#endif

#ifndef mysleep
#   ifdef __GNUC__
#	warning sleep() call undefined. Please check your compiler documentation for it and write "#define mysleep" into compiler.h
#   else
#	pragma message("sleep() call undefined. Please check your compiler documentation for it and write define into compiler.h")
#   endif
#   define mysleep(x)
#endif

#ifndef SMAPI_EXT
#   ifdef __GNUC__
#       warning Please set SMAPI_EXT to extern or proprietary token
#   else
#       pragma message("Please set SMAPI_EXT to extern or proprietary token")
#   endif
#   define SMAPI_EXT extern
#endif

#ifndef _XPENTRY
#   ifdef __GNUC__
#       warning Please check your compiler to system functions call modifyer and define _XPENTRY
#   else
#       pragma message("Please check your compiler to system functions call modifyer and define _XPENTRY")
#   endif
#   define _XPENTRY
#endif

#ifndef _stdc
#   error Please check your compiler to standard C code modifyer and define _stdc in compiler.h
#endif

#ifndef _intr
#   error Please check your compiler to interrupt handler modifyer (usually 'interrupt') and define _intr in compiler.h
#endif

#ifndef _fast
#   ifdef __GNUC__
#       warning Please check your compiler to fast functions call modifyer and define _fast in compiler.h
#   else
#       pragma message("Please check your compiler to fast functions call modifyer and define _fast in compiler.h")
#   endif
#   define _fast
#endif

#ifndef farread
#   error Please check your compiler to far calling implementation of read() function and define farread in compiler.h
#endif

#ifndef farwrite
#   error Please check your compiler to far calling implementation of write() function and define farwrite in compiler.h
#endif

 /* waitlock works like lock, but blocks until the lock can be
  * performed.
  * waitlock2 works like a timed waitlock.
  */
extern int waitlock(int, long, long);
extern int waitlock2(int, long, long, long);

#if !defined(HAS_mktime)

/* If compiler doesn't include a mktime(), we need our own */
/* But our own mktime not implemented yet...
#include <time.h>
time_t _stdc mktime(struct tm *tm_ptr);
*/
#error Our own mktime() not implemented yet... sorry.
#endif

/* If compiler doesn't include a strftime(), we need our own (see strftim.c) */
#if !defined(HAS_strftime)

#define strftime(str,max,fmt,tm) strftim(str,max,fmt,tm)
size_t _stdc strftim( char *str, size_t maxsize, const char *fmt,
                      const struct tm *tm_ptr );

#endif


#if !defined(HAS_strupr)
char *strupr(char *str);
#endif

/* Some implementations not include the min() macro or function. Usually C++ */
#ifndef min
#  define min(a,b)              (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#  define max(a,b)              (((a) > (b)) ? (a) : (b))
#endif

#ifdef HAS_PROCESS_H
#  include <process.h>
#endif
#if !defined(P_WAIT) && defined(_P_WAIT) /*for spawn* in process.h*/
#  define P_WAIT          _P_WAIT
#endif

#ifdef NEED_trivial_farread
  int trivial_farread( int handle, void far *buffer, unsigned len );
#endif
#ifdef NEED_trivial_farwrite
  int trivial_farwrite( int handle, void far *buffer, unsigned len );
#endif


#endif
