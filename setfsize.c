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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "compiler.h"

#ifdef HAS_DOS_H
#include <dos.h>
#endif

#ifdef HAS_IO_H
#include <io.h>
#endif

#ifdef HAS_UNISTD_H
  #include <unistd.h>
#endif


#ifdef __DOS__

  /* Call DOS Fn 40H: Write to File via Handle
   * AH    0x40
   * BX    file handle
   * CX    number of bytes to write (Note: 0 means truncate the file)
   * DS:DX address of a buffer containing the data to write
   * Returns: AX    error code if CF is set to CY
   *                number of bytes actually written ออออ use for error test
   *
   * DOS 3.0+ If CX is 0000H on entry, the file is truncated at the
   * current file position -- or the file is padded to that position.
   */
  int _fast setfsize(int fd, long size)
  {
    union REGS r;
    long pos=tell(fd);

    lseek(fd, size, SEEK_SET);
    memset(&r,0,sizeof(r));

    r.h.ah=0x40;

    #if defined(__DOS16__) || defined(__DJGPP__)
    r.x.bx=fd;

    int86(0x21, &r, &r);

    #elif defined(__DPMI__)
    r.x.ebx=fd;

    int386(0x21, &r, &r);
    #endif

    lseek(fd, pos, SEEK_SET);

    return 0;
  }
#elif defined(__OS2__)

  #define INCL_DOSFILEMGR
  #include <os2.h>

  int _fast setfsize(int fd, long size)
  {
    return ((int)DosSetFileSize((HFILE)fd, (ULONG)size));
  }

#elif defined(__UNIX__)

  int _fast setfsize(int fd, long size)
  {
    return ftruncate(fd, size);
  }
#elif defined(__WIN32__)

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <winbase.h>

  int _fast setfsize(int fd, long size)
  {
#if defined(__MSVC__) || defined(__MINGW32__)
    return chsize(fd, size);
#else
    SetFilePointer((HANDLE)fd, size, NULL, FILE_BEGIN);
    return (!SetEndOfFile((HANDLE)fd));
#endif
  }
#else
  #error Unknown OS
#endif

