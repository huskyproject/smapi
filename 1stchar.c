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
#include <string.h>
#include "compiler.h"

char *_fast firstchar(char *strng, char *delim, int findword)
{
    int isw=0, sl_s, wordno = 0;
    register int sl_d, x;
    register char *string=strng;
    char *oldstring=strng;

    /* We can't do *anything* if the string or delim is NULL or blank... */

    if ( !string || !*strng || !delim || !*delim )
    {
        return NULL;
    }

    sl_d = strlen(delim);

    for (string = strng; *string; string++)
    {
        for (x = 0, isw = 0; x <= sl_d; x++)
        {
            if (*string == delim[x])
            {
                isw=1;
            }
        }

        if (isw == 0)
        {
            oldstring = string;
            break;
        }
    }

    sl_s = strlen(string);

    for (wordno = 0; string - oldstring < sl_s; string++)
    {
        for (x = 0, isw = 0; x <= sl_d; x++)
        {
            if (*string == delim[x])
            {
                isw = 1;
                break;
            }
        }

        if (!isw && string == oldstring)
        {
            wordno++;
        }

        if (isw && string != oldstring)
        {
            for (x = 0, isw = 0; x <= sl_d; x++)
            {
                if (*(string + 1) == delim[x])
                {
                    isw = 1;
                    break;
                }
            }

            if (isw == 0)
            {
                wordno++;
            }
        }

        if (wordno == findword)
        {
            if (string == oldstring || string == oldstring + sl_s)
            {
                return string;
            }
            else
            {
                return string + 1;
            }
        }
    }

    return NULL;
}

#ifdef TEST

int main(void)
{
    char *test = "  NORMAL   ";
    char *test2 = "NORMAL\n";
    char *test3 = "Sysop";

    printf("1:`%s'\n", firstchar(test," \t\n", 2));
    printf("2:`%s'\n", firstchar(test2," \t\n", 2));
    printf("3:`%s'\n", firstchar(test3," \t\n", 2));

    return 0;
}

#endif
