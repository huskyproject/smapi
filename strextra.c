/*
 *  STREXTRA.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Modified in July 1994 by John Dennis.
 *  Additional modifications by Paul Edwards and Andrew Clarke.
 *
 *  Originally distributed with the Msged mail reader source code and
 *  modified for use in SMAPI in January 1997.  Released to the public domain.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strextra.h"

#ifndef _MSC_VER

int stricmp(const char *s, const char *t)
{
    while (*s != '\0')
    {
        int rc;
        rc = tolower((unsigned char)*s) - tolower((unsigned char)*t);
        if (rc != 0)
        {
            return rc;
        }
        s++;
        t++;
    }

    if (*t != '\0')
    {
        return -tolower((unsigned char)*t);
    }

    return 0;
}

#endif

#ifndef __IBMC__

char *strdup(const char *s)
{
    char *p;
    p = malloc(strlen(s) + 1);
    if (p != NULL)
    {
        strcpy(p, s);
    }
    return p;
}

#endif
