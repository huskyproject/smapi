/*
 * emxlock.c
 *
 * This file implements record locking for the EMX GCC compiler on OS/2.
 * It has been tested with EMX 0.9c fix 4 on OS/2.
 *
 * Written by Tobias Ernst @ 2:2476/418, released to the public domain.
 *
 */

#include "compiler.h"

#ifdef __EMX__

#include <os2.h>

int lock(int handle, long ofs, long length)
{
    int rc;
    FILELOCK urange, lrange;
    APIRET apiret;

    lrange.lOffset = ofs;
    lrange.lRange = length;
    urange.lRange = urange.lOffset = 0;

    if ((apiret = DosSetFileLocks((HFILE)handle, &urange, &lrange, 0, 0)) != 0)
    {
        return -1;
    }

    return 0;
}

int unlock(int handle, long ofs, long length)
{
    int rc;
    FILELOCK urange, lrange;
    APIRET apiret;

    urange.lOffset = ofs;
    urange.lRange = length;
    lrange.lRange = lrange.lOffset = 0;

    if ((apiret = DosSetFileLocks((HFILE)handle, &urange, &lrange, 0, 0)) != 0)
    {
        return -1;
    }
    return 0;
}

#endif  /* EMX */
