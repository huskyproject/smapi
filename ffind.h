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

#ifndef __FFIND_H__
#define __FFIND_H__

#include "compiler.h"

#ifdef UNIX
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#endif

#ifdef SASC
#include <stdio.h>
#include <dos.h>
#endif

#if defined(__RSXNT__) || defined(__MINGW32__)
#define NOUSER
#include <windows.h>
#endif


#define FFIND struct ffind

struct ffind
{
#ifndef OS2
    char reserved[21];
#endif

    char ff_attrib;
    unsigned short ff_ftime;
    unsigned short ff_fdate;
    long ff_fsize;
#if defined (__WATCOMC__) && ( defined(__OS2__) || defined(__NT__) )
    char ff_name[256];
#else
    char ff_name[13];  /* urks! */
#endif

#ifdef OS2
#if defined(__386__) || defined(__FLAT__)
    unsigned long hdir;   /* directory handle from DosFindFirst */
#else
    unsigned short hdir;  /* directory handle from DosFindFirst */
#endif
#endif

#ifdef UNIX
    DIR *dir;
    char firstbit[FILENAME_MAX];
    char lastbit[FILENAME_MAX];
#endif

#ifdef SASC
    struct FileInfoBlock info;
    char newfile[FILENAME_MAX];
    char prefix[FILENAME_MAX];
#endif

#if defined(__RSXNT__) || defined(__MINGW32__)
    WIN32_FIND_DATA InfoBuf;
    HANDLE hDirA;
    char attrib_srch;
#endif
};

/*
 * I prefixed the functions below with an additional F in order to
 * prevent name clashes with the Win32 API
 */

FFIND *_fast FFindOpen(char *filespec, unsigned short attribute);
FFIND *_fast FFindInfo(char *filespec);
int _fast FFindNext(FFIND * ff);
void _fast FFindClose(FFIND * ff);

#define MSDOS_READONLY  0x01
#define MSDOS_HIDDEN    0x02
#define MSDOS_SYSTEM    0x04
#define MSDOS_VOLUME    0x08
#define MSDOS_SUBDIR    0x10
#define MSDOS_ARCHIVE   0x20
#define MSDOS_RSVD1     0x40
#define MSDOS_RSVD2     0x80

#endif
