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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "ffind.h"
#include "prog.h"

#if !defined(__IBMC__) && !defined(MSDOS) && !defined(UNIX)
#include <dos.h>
#endif

int _fast fexist(char *filename)
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

long _fast fsize(char *filename)
{
    FFIND *ff;
    long ret = -1L;

    ff = FFindOpen(filename, 0);

    if (ff)
    {
        ret = ff->ff_fsize;
        FFindClose(ff);
    }

    return ret;
}

#if defined(MSDOS)

int _fast direxist(char *directory)
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
      tempstr[2] == '/') && tempstr[3] == '\0'))) || eqstri(tempstr, "\\"))
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

#elif defined(OS2) || defined(__NT__)

#include <io.h>

int _fast direxist(char *directory)
{
    int ret;
    char *tempstr;
    size_t l;

    tempstr = strdup(directory);
    if (tempstr == NULL)
    {
        return FALSE;
    }

    /* Root directory of any drive always exists! */

    if ((isalpha(tempstr[0]) && tempstr[1] == ':' && (tempstr[2] == '\\' || tempstr[2] == '/') &&
      !tempstr[3]) || eqstr(tempstr, "\\"))
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

    ret = !access(tempstr, 06);

    free(tempstr);
    return ret;
}

#elif defined(UNIX) || defined(SASC)

#include <stdio.h>

int _fast direxist(char *directory)
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
