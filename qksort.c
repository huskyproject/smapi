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
#include "progprot.h"


#define NUM sizeof(array)/sizeof(array[0])
#define SWAP(a,b,s) s=a; a=b; b=s;

static void _fast iqksort(int *p_lo, int *p_hi)
{
    int *p_mid, *p_i, *p_lastlo, tmp;

    p_mid = p_lo + (((int)(p_hi - p_lo)) / 2);

    SWAP(*p_lo, *p_mid, tmp);

    p_lastlo = p_lo;

    for (p_i = p_lo + 1; p_i <= p_hi; ++p_i)
    {
        if (*p_lo > *p_i)
        {
            ++p_lastlo;
            SWAP(*p_lastlo, *p_i, tmp);
        }
    }

    SWAP(*p_lo, *p_lastlo, tmp);

    if (p_lo < p_lastlo && p_lo < p_lastlo - 1)
    {
        iqksort(p_lo, p_lastlo - 1);
    }

    if (p_lastlo + 1 < p_hi)
    {
        iqksort(p_lastlo + 1, p_hi);
    }
}

void _fast qksort(int a[], size_t n)
{
    if (n > 1)
    {
        iqksort(a, &a[n - 1]);
    }
}
