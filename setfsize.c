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

/* name=Function to dynamically change the size of a file
*/

#ifdef __MSDOS__
#include <dos.h>
#endif
#include "prog.h"

#if defined(__MSDOS__)

  #include <stdio.h>
  #include <stdlib.h>
  #include <io.h>
  #include <fcntl.h>

  int _fast setfsize(int fd, long size)
  {
    union REGS r;
    long pos=tell(fd);

    lseek(fd, size, SEEK_SET);

  #ifdef __386__
    r.h.ah=0x40;
    r.x.ebx=fd;
    r.x.ecx=0;
    r.x.edx=0;

    int386(0x21, &r, &r);
  #else
    r.h.ah=0x40;
    r.x.bx=fd;
    r.x.cx=0;
    r.x.dx=0;

    int86(0x21, &r, &r);
  #endif

    lseek(fd, pos, SEEK_SET);

    return 0;
  }
#elif defined(OS2)

  #define INCL_DOSFILEMGR
  #include <os2.h>

  int _fast setfsize(int fd, long size)
  {
    return ((int)DosSetFileSize((HFILE)fd, (ULONG)size));
  }

#elif defined(UNIX)

  #include <unistd.h>

  int _fast setfsize(int fd, long size)
  {
    return ftruncate(fd, size);
  }
#elif defined(__NT__) || defined(NT)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <winbase.h>
  int _fast setfsize(int fd, long size)
  {
    SetFilePointer((HANDLE)fd, size, NULL, FILE_BEGIN);
    return (!SetEndOfFile((HANDLE)fd));
  }
#else
  #error Unknown OS
#endif

