/***************************************************************************
 *                                                                         *
 *  Squish Developers Kit Source, Version 2.00                             *
 *  Copyright 1989-1994 by SCI Communications.  All rights reserved.       *
 *                                                                         *
 *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
 *  SQUISH DEVELOPERS KIT LICENSING AGREEMENT IN SQDEV.PRN.  IF YOU DO NOT *
 *  FIND THE TEXT OF THIS AGREEMENT IN THE AFOREMENTIONED FILE, OR IF YOU  *
 *  DO NOT HAVE THIS FILE, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT    *
 *  ONE OF THE ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO  *
 *  USE THIS FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE SQUISH          *
 *  DEVELOPERS KIT LICENSING AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE *
 *  ABLE TO REACH WITH THE AUTHOR.                                         *
 *                                                                         *
 *  You can contact the author at one of the address listed below:         *
 *                                                                         *
 *  Scott Dudley       FidoNet     1:249/106                               *
 *  777 Downing St.    Internet    sjd@f106.n249.z1.fidonet.org            *
 *  Kingston, Ont.     CompuServe  >INTERNET:sjd@f106.n249.z1.fidonet.org  *
 *  Canada  K7M 5N3    BBS         1-613-634-3058, V.32bis                 *
 *                                                                         *
 ***************************************************************************/

#ifdef MSDOS
#ifndef __MSDOS__
#define __MSDOS__
#endif
#endif

#if defined(__MSDOS__) || (defined(__FLAT__) && defined(__WATCOMC__))
#include <dos.h>
#endif

#if defined(__OS2__)
#define INCL_NOPM
#define INCL_DOS    /* must be before prog.h */
#endif

#include "prog.h"

#if defined(__OS2__)

#include <os2.h>

  void _fast tdelay(int msecs)
  {
      DosSleep((ULONG)msecs);
  }

#elif defined(__MSDOS__)
#include <time.h>

  void _fast tdelay(int msecs)
  {
    clock_t ctEnd;

    ctEnd = clock() + (long)msecs * (long)CLK_TCK / 1000L;

    while (clock() < ctEnd)
      ;
  }

#elif defined(__MINGW32__)

#include <stdlib.h>
  void _fast tdelay(int msecs)
  {
    _sleep((dword)msecs);
  }

#elif defined(NT) || defined(__NT__) || defined(__WIN32__)
/* win32/nt not mingw or `cygwin -mno-cygwin`  (MS VC, Borland C/win32, Watcom C)*/

# if defined(_MSC_VER) || defined(__TURBOC__)
   extern void __stdcall Sleep(dword ms);
# else
   extern void Sleep(dword ms);
# endif
   void _fast tdelay(int msecs)
   {
    Sleep((dword)msecs);
   }

#elif defined(__BEOS__)

#include <be/kernel/scheduler.h>

  void _fast tdelay(int msecs)
  {
    snooze(msecs*1000l);
  }

#elif defined(UNIX)

#include <unistd.h>

  void _fast tdelay(int msecs)
  {
    usleep(msecs*1000l);
  }

#elif defined(__WATCOMC__)
  void _fast tdelay(int msecs)
  {
    sleep(msecs);
  }
#else
#error Unknown OS
#endif


