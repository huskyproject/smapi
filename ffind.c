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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#if defined(__TURBOC__) || defined(__DJGPP__)
#include <dir.h>
#endif

#if !defined( __IBMC__) && !defined(UNIX)
#include <dos.h>
#endif

#include "ffind.h"

#ifdef OS2
#ifdef EXPENTRY
#undef EXPENTRY
#endif
#define INCL_NOPM
#define INCL_DOS
#include <os2.h>
#if defined(__386__) || defined(__FLAT__)
#undef DosQPathInfo
#define DosQPathInfo(a,b,c,d,e)  DosQueryPathInfo(a,b,c,d)
#endif
#endif

#ifdef UNIX
#include "patmat.h"
#endif

/*
 *  FindOpen;  Use like MSDOS "find first" function,  except be sure to
 *  release allocated system resources by caling FindClose() with the
 *  handle returned by this function.
 *
 *  Returns: NULL == File not found.
 */

FFIND *_fast FindOpen(char *filespec, unsigned short attribute)
{
    FFIND *ff;

    ff = malloc(sizeof(FFIND));

    if (ff != NULL)
    {
#if defined(__TURBOC__) || defined(__DJGPP__)

        if (findfirst(filespec, (struct ffblk *)ff, attribute) != 0)
        {
            free(ff);
            ff = NULL;
        }

#elif defined(OS2)

#if defined(__386__) || defined(__FLAT__)
        ULONG SearchCount = 1;
        FILEFINDBUF3 findbuf;
#else
        USHORT SearchCount = 1;
        FILEFINDBUF findbuf;
#endif

        ff->hdir = HDIR_CREATE;

        if (!DosFindFirst((PBYTE) filespec, &ff->hdir, attribute, &findbuf, sizeof(findbuf), &SearchCount, 1L))
        {
            ff->ff_attrib = (char)findbuf.attrFile;
            ff->ff_fsize = findbuf.cbFile;

            ff->ff_ftime = *((USHORT *) & findbuf.ftimeLastWrite);
            ff->ff_fdate = *((USHORT *) & findbuf.fdateLastWrite);

            strncpy(ff->ff_name, findbuf.achName, sizeof(ff->ff_name));
        }
        else
        {
            free(ff);
            ff = NULL;
        }

#elif defined(_MSC_VER) || defined(__WATCOMC__)

        if (_dos_findfirst(filespec, attribute, (struct find_t *)ff) != 0)
        {
            free(ff);
            ff = NULL;
        }

#elif defined(UNIX)

        char *p;
        int fin = 0;
        struct dirent *de;

        p = strrchr(filespec, '/');
        if (p == NULL)
        {
            strcpy(ff->firstbit, ".");
            strcpy(ff->lastbit, filespec);
        }
        else
        {
            memcpy(ff->firstbit, filespec, p - filespec);
            ff->firstbit[p - filespec] = '\0';
            strcpy(ff->lastbit, p + 1);
        }
        ff->dir = opendir(ff->firstbit);
        if (ff->dir != NULL)
        {
            while (!fin)
            {
                de = readdir(ff->dir);
                if (de == NULL)
                {
                    closedir(ff->dir);
                    free(ff);
                    ff = NULL;
                    fin = 1;
                }
                else
                {
                    if (patmat(de->d_name, ff->lastbit))
                    {
                        strncpy(ff->ff_name, de->d_name, sizeof ff->ff_name);
                        fin = 1;
                    }
                }
            }
        }
        else
        {
            free(ff);
            ff = NULL;
        }

#elif defined(SASC)

        char *temp;
        int error;

        temp = strrchr(filespec, '/');
        if (temp == NULL)
        {
            temp = strrchr(filespec, '\\');
        }
        if (temp == NULL)
        {
            temp = strrchr(filespec, ':');
        }
        if (temp == NULL)
        {
            strcpy(ff->prefix, "");
        }
        else
        {
            memcpy(ff->prefix, filespec, temp - filespec + 1);
            *(ff->prefix + (temp - filespec + 1)) = '\0';
        }
        error = dfind(&ff->info, filespec, 0);
        if (error == 0)
        {
            strcpy(ff->ff_name, ff->prefix);
            strcat(ff->ff_name, ff->info.fib_FileName);
        }
        else
        {
            free(ff);
            ff = NULL;
        }

#else
#error Unknown compiler!
#endif

    }

    return ff;
}

/*
 *  FindNext: Returns 0 if next file was found, non-zero if it was not.
 */

int _fast FindNext(FFIND * ff)
{
    int rc = -1;

    if (ff != NULL)
    {
#if defined(__TURBOC__) || defined(__DJGPP__)

        rc = findnext((struct ffblk *)ff);

#elif defined(OS2)

#if defined(__386__) || defined(__FLAT__)
        ULONG SearchCount = 1;
        FILEFINDBUF3 findbuf;
#else
        USHORT SearchCount = 1;
        FILEFINDBUF findbuf;
#endif

        if (ff->hdir && !DosFindNext(ff->hdir, &findbuf, sizeof(findbuf),
          &SearchCount))
        {
            ff->ff_attrib = (char)findbuf.attrFile;
            ff->ff_ftime = *((USHORT *) & findbuf.ftimeLastWrite);
            ff->ff_fdate = *((USHORT *) & findbuf.fdateLastWrite);
            ff->ff_fsize = findbuf.cbFile;
            strncpy(ff->ff_name, findbuf.achName, sizeof(ff->ff_name));
            rc = 0;
        }

#elif defined(_MSC_VER) || defined(__WATCOMC__)

        rc = _dos_findnext((struct find_t *)ff);

#elif defined(UNIX)

        int fin = 0;
        struct dirent *de;

        while (!fin)
        {
            de = readdir(ff->dir);
            if (de == NULL)
            {
                closedir(ff->dir);
                ff->dir = NULL;
                fin = 1;
            }
            else
            {
                if (patmat(de->d_name, ff->lastbit))
                {
                    strncpy(ff->ff_name, de->d_name, sizeof ff->ff_name);
                    fin = 1;
                    rc = 0;
                }
            }
        }

#elif defined(SASC)
        int error = 0;

        error = dnext(&ff->info);
        if (error == 0)
        {
            strcpy(ff->ff_name, ff->prefix);
            strcat(ff->ff_name, ff->info.fib_FileName);
            rc = 0;
        }

#else
#error Unable to determine compiler and target operating system!
#endif

    }

    return rc;
}

/*
 *  FindClose: End a directory search.  Failure to call this function
 *  will result in unclosed file handles under OS/2, and unreleased
 *  memory in both DOS and OS/2.
 */

void _fast FindClose(FFIND * ff)
{
    if (ff != NULL)
    {

#ifdef OS2
        if (ff->hdir)
        {
            DosFindClose(ff->hdir);
        }
#endif

#ifdef UNIX
        if (ff->dir)
        {
            closedir(ff->dir);
        }
#endif
        free(ff);
    }
}

/*
 *  FindInfo: This function was added because it is SIGNIFICANTLY faster
 *  under OS/2 to call DosQPathInfo() rather than DosFindFirst() if all
 *  you are interested in is getting a specific file's date/time/size.
 *
 *  PLF Thu  10-17-1991  18:12:37
 */

FFIND *_fast FindInfo(char *filespec)
{
#ifndef OS2
    return FindOpen(filespec, 0);
#else
    FFIND *ff;
    FILESTATUS fs;
    char *f;

    ff = malloc(sizeof *ff);
    if (ff == NULL)
    {
        return NULL;
    }

    memset(ff, 0, sizeof *ff);
    if (!DosQPathInfo((PBYTE) filespec, FIL_STANDARD, (PBYTE) &fs, sizeof fs, 0L))
    {
        ff->ff_attrib = (char)fs.attrFile;
        ff->ff_ftime = *((USHORT *) & fs.ftimeLastWrite);
        ff->ff_fdate = *((USHORT *) & fs.fdateLastWrite);
        ff->ff_fsize = fs.cbFile;

        /* isolate file name */
        f = strrchr(filespec, '\\');
        if (f == NULL)
        {
            f = filespec;
        }
        else
        {
            f++;
        }
        strncpy(ff->ff_name, f, sizeof(ff->ff_name));
    }
    else
    {
        free(ff);
        return NULL;
    }
    return ff;
#endif
}

#ifdef TEST

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* this simple function assumes the path ALWAYS has an ending backslash */

int walk(char *path)
{
    FFIND *ff;
    int done = FALSE;
    char full[127];

    strcpy(full, path);
#ifdef MSDOS
    strcat(full, "*.*");
#else
    strcat(full, "*");
#endif

    ff = FindOpen(full, MSDOS_SUBDIR);
    if (ff != NULL)
    {
        done = FALSE;
        while (done != TRUE)
        {
            if (ff->ff_attrib & MSDOS_SUBDIR && ff->ff_name[0] != '.')
            {
                strcpy(full, path);
                strcat(full, ff->ff_name);
                puts(full);
#ifndef UNIX
                strcat(full, "\\");
#else
                strcat(full, "/");
#endif
                if (!walk(full))
                {
                    return FALSE;
                }
            }
            done = FindNext(ff) != 0;
        }
        FindClose(ff);
        return TRUE;
    }
    else
    {
        puts("FindOpen() failed!");
    }
    return FALSE;
}

int main(void)
{
#ifndef UNIX
    return walk("\\") == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
#else
    return walk("/") == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
#endif
}

#endif
