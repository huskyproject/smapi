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

/* These are compilers that have both a working stat() and (important!) the
   S_ISREG and S_ISDIR macros. The problem is that while stat() is POSIX, those
   macros are not. For compilers that do not provide these macros, we revert to
   the old "ffind" method. */
/* Moved to compiler.h
#if defined(__UNIX__) || defined(__MINGW32__) || defined(__EMX__) || defined(__RSXNT__) || defined(__DJGPP__) || defined(_MSC_VER)
#define USE_STAT_MACROS
#endif
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

#ifdef HAS_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAS_DIRECT_H
#  include <direct.h>
#endif

#ifdef HAS_DOS_H
#  include <dos.h>
#endif

#ifdef USE_STAT_MACROS
/* These are compilers that have both a working stat() and (important!) the
   S_ISREG and S_ISDIR macros. The problem is that while stat() is POSIX, those
   macros are not. For compilers that do not provide these macros, we revert to
   the old "ffind" method. */
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include "ffind.h"

#ifdef USE_STAT_MACROS

/* This is the nice code that works on UNIX and every other decent platform.
   It has been contributed by Alex S. Aganichev */

int _fast fexist(const char *filename)
{
    struct stat s;

    if (stat (filename, &s))
        return FALSE;
    return S_ISREG(s.st_mode);
}

long _fast fsize(const char *filename)
{
    struct stat s;

    if (stat (filename, &s))
        return -1L;
    return s.st_size;
}

int _fast direxist(const char *directory)
{
    struct stat s;
    int rc;

#if !defined(__WATCOMC__) && !defined(__MSVC__) && !defined(__MINGW32__)
    rc = stat (directory, &s);
#else
    char *tempstr, *p;
    size_t l;
    tempstr = strdup(directory);
    if (tempstr == NULL) {
        return FALSE;
    }

    /* Root directory of any drive always exists! */

    if ((isalpha((int)tempstr[0]) && tempstr[1] == ':' && (tempstr[2] == '\\' || tempstr[2] == '/') &&
      !tempstr[3]) || (strcmp(tempstr, "\\")==0) ) {
        free(tempstr);
        return TRUE;
    }

    l = strlen(tempstr);
    if (tempstr[l - 1] == '\\' || tempstr[l - 1] == '/')
    {
        /* remove trailing backslash */
        tempstr[l - 1] = '\0';
    }

    for (p=tempstr; *p; p++)
    {
        if (*p == '/')
          *p='\\';
    }

    rc = stat (tempstr, &s);

    free(tempstr);
#endif
    if (rc)
        return FALSE;
    return S_ISDIR(s.st_mode);
}

#else /* USE_STAT_MACROS not defined */

/* Here comes the ugly platform specific and sometimes even slow code. */

int _fast fexist(const char *filename)
{
    FFIND *ff;

    ff = FFindOpen(filename, 0);

    if (ff)
    {
        FFindClose(ff);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

long _fast fsize(const char *filename)
{
    FFIND *ff;
    FILE  *fp;
    long ret = -1L;

    ff = FFindOpen(filename, 0);

    if (ff)
    {
#ifndef __UNIX__
	ret = ff->ff_fsize;
	if (ret != -1L)
#endif
	{   fp = fopen(filename, "rb");
	    fseek(fp, 0, SEEK_END);
	    ret = ftell(fp);
	    fclose(fp);
	}
        FFindClose(ff);
    }

    return ret;
}

#if defined(__DOS__) || defined(__DPMI__)

int _fast direxist(const char *directory)
{
    FFIND *ff;
    char *tempstr;
    int ret;

    tempstr = (char *)malloc(strlen(directory) + 5);
    if (tempstr == NULL)
    {
        return FALSE;
    }

    strcpy(tempstr, directory);

    Add_Trailing(tempstr, '\\');

    /* Root directory of any drive always exists! */

    if ((isalpha(tempstr[0]) && tempstr[1] == ':' && ((tempstr[2] == '\0') || ((tempstr[2] == '\\' ||
      tempstr[2] == '/') && tempstr[3] == '\0'))) || (stricmp(tempstr, "\\")==0) )
    {
        ret = TRUE;
    }
    else
    {
        Strip_Trailing(tempstr, '\\');

        ff = FFindOpen(tempstr, MSDOS_SUBDIR | MSDOS_HIDDEN | MSDOS_READONLY);

        ret = ff != NULL && (ff->ff_attrib & MSDOS_SUBDIR);

        if (ff)
        {
            FFindClose(ff);
        }
    }

    free(tempstr);
    return ret;

}

#elif defined(__OS2__) || defined(__NT__)

#ifdef __OS2__
#define INCL_DOSFILEMGR
#include <os2.h>
#else
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMSG
#include <windows.h>
#endif

int _fast direxist(const char *directory)
{
    char *tempstr, *p;
    size_t l;
#if defined(__NT__) || defined(__CYGWIN__)
    DWORD attr;
#else
    FILESTATUS3 s;
#endif


    tempstr = strdup(directory);
    if (tempstr == NULL)
    {
        return FALSE;
    }

    /* Root directory of any drive always exists! */

    if ((isalpha((int)tempstr[0]) && tempstr[1] == ':' && (tempstr[2] == '\\' || tempstr[2] == '/') &&
      !tempstr[3]) || (strcmp(tempstr, "\\")==0) )
    {
        free(tempstr);
        return TRUE;
    }

    l = strlen(tempstr);
    if (tempstr[l - 1] == '\\' || tempstr[l - 1] == '/')
    {
        /* remove trailing backslash */
        tempstr[l - 1] = '\0';
    }

    for (p=tempstr; *p; p++)
    {
        if (*p == '/')
          *p='\\';
    }

#ifdef __OS2__
    if (DosQueryPathInfo((PSZ)tempstr, FIL_STANDARD,
                         (PVOID)&s, sizeof(s)) == 0)
    {
       free (tempstr);
       if (s.attrFile & FILE_DIRECTORY)
          return TRUE;
       else
          return FALSE;
    }
    free (tempstr);
    return FALSE;
#else
    attr = GetFileAttributes(tempstr);
    free(tempstr);
    if ((attr != 0xFFFFFFFF) && (attr & FILE_ATTRIBUTE_DIRECTORY))
       return TRUE;
    else
       return FALSE;
#endif
}

#elif defined(__UNIX__)

int _fast direxist(const char *directory)
{
    FILE *fp;

    fp = fopen(directory, "rb");
    if (fp != NULL)
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

#else

#error Unknown compiler!

#endif

#endif

int _createDirectoryTree(const char *pathName) {

   char *start, *slash;
   char limiter=PATH_DELIM;
   int i;

   start = (char *) malloc(strlen(pathName)+2);
   strcpy(start, pathName);
   i = strlen(start)-1;
   if (start[i] != limiter) {
      start[i+1] = limiter;
      start[i+2] = '\0';
   }
   slash = start;

#ifndef __UNIX__
   /*  if there is a drivename, jump over it */
   if (slash[1] == ':') slash += 2;
#endif

   /*  jump over first limiter */
   slash++;

   while ((slash = strchr(slash, limiter)) != NULL) {
      *slash = '\0';

      if (!direxist(start)) {
         if (!fexist(start)) {
            /*  this part of the path does not exist, create it */
            if (mymkdir(start) != 0) {
               free(start);
               return 1;
            }
         } else {
            free(start);
            return 1;
         }
      }

      *slash++ = limiter;
   }

   free(start);

   return 0;
}


#ifdef TEST

int main(void)
{
    printf("asdfe=%d\n", direxist("c:\\asdfe"));
    printf("blank=%d\n", direxist("c:\\blank"));
    printf("tc=%d\n", direxist("c:\\tc"));
    printf("c:\\=%d\n", direxist("c:\\"));
    printf("d:\\=%d\n", direxist("d:\\"));
    printf("e:\\=%d\n", direxist("e:\\"));
    printf("f:\\=%d\n", direxist("f:\\"));
    return 0;
}

#endif
