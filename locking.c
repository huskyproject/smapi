/*
 * locking.c
 *
 * This file implements record locking for platforms where the lock(),
 * unlock() and/or sopen() functions are not available. This includes:
 *
 * EMX GCC on OS/2:
 *   lock, unlock: implemented as OS/2 API calls
 *   sopen: provided by the EMX RTL
 *
 * EMX GCC on Windows 32 bit using RSXNT:
 *   lock, unlock: implemented as Win32 API calls
 *   sopen: provided by the RSX RTL
 *
 * UNIX:
 *   lock, unlock: implemented as calls to fcntl
 *   sopen: implemented as open with subsequent shared lock
 *
 * Written by Tobias Ernst @ 2:2476/418, released to the public domain.
 *
 */

#include "compiler.h"

#ifdef __EMX__

#if defined(OS2)

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

#elif defined(__NT__)

#include <windows.h>

int lock(int handle, long ofs, long length)
{
    if (LockFile(handle, (DWORD)ofs, 0L, (DWORD)length, 0L) == FALSE)
    {
        return -1;
    }
    return 0;
}

int unlock(int handle, long ofs, long length)
{
    if (UnlockFile(handle, (DWORD)ofs, 0L, (DWORD)length, 0L) == FALSE)
    {
        return -1;
    }
    return 0;
}


#else
#error EMX compiler found, but operating system is neither OS/2 nor NT.
#endif


#elif defined(UNIX)

#include <fcntl.h>
#include <unistd.h>

static struct flock* file_lock(short type, long ofs, long length)
{
    static struct flock ret;

    ret.l_type = type;
    ret.l_start = ofs;
    ret.l_whence = SEEK_SET;
    ret.l_len = length;
    ret.l_pid = getpid();
    return &ret;
}

int lock(int handle, long ofs, long length)
{
    return fcntl(handle, F_SETLKW, file_lock(F_WRLCK, ofs, length));
}

int unlock(int handle, long ofs, long length)
{
    return fcntl(handle, F_SETLKW, file_lock(F_UNLCK, ofs, length));
}


int sopen(const char *name, int oflag, int ishared, int mode)
{
    int fd = open(name, oflag, mode);
    if (fcntl(fd, F_SETLKW,
	      file_lock((ishared == SH_DENYNONE) ? F_RDLCK : F_WRLCK, 0, 0)))

    {
	close(fd);
	return -1;
    }
    return fd;
}

#endif
    


  
  
  
