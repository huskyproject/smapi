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

#include <string.h>
#include "prog.h"

char *_fast Strip_Trailing(char *str, char strip)
{
    int x;
    if (str && *str && str[x = strlen(str) - 1] == strip)
    {
        str[x] = '\0';
    }
    return str;
}

char *_fast Add_Trailing(char *str, char add)
{
    int x;
    if (str && *str && str[x = strlen(str) - 1] != add)
    {
        str[x + 1] = add;
        str[x + 2] = '\0';
    }
    return str;
}
