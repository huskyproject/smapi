/*
 * locking.c
 *
 * This file implements record locking for platforms where the lock(),
 * unlock() and/or sopen() functions are not available. It also
 * implements the waitlock() routine, which is the same as lock(), but
 * waits until the lock can be applied.
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
 * OTHER:
 *   lock, unlock and sopen provieded by the CRTL
 *   waitlock defined as a loop calling lock and then sleep
 *
 * Written by Tobias Ernst @ 2:2476/418, released to the public domain.
 *
 */

#include "compiler.h"

#if (defined(__EMX__) || defined(__IBMC__)) && defined(OS2)

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

int waitlock(int handle, long ofs, long length)
{
    int rc;
    FILELOCK urange, lrange;
    APIRET apiret;

    lrange.lOffset = ofs;
    lrange.lRange = length;
    urange.lRange = urange.lOffset = 0;

    while ((apiret = DosSetFileLocks((HFILE)handle, &urange, &lrange, 60000, 0)) != 0);

    return 0;
}

#elif defined(__RSXNT__)  

#include <windows.h>

/* To achieve file locking with RSX, we need to access some RSX
   internal structures that are defined in rsxnt.h in order to obtain
   a file handle that can be passed on to the Win32 API. The file
   rsxnt.h can be found in \RSXNT\SOURCE\RSXNT\INCLUDE and might not
   be present on your system if you did not install the full source
   code for RSX ... Sorry, but I did not find a better way to do it. */
#include <../source/rsxnt/include/rsxnt.h>

#include <stdlib.h>

int waitlock(int handle, long ofs, long length)
{
    EMXPROCESS *p = _rsxnt_get_process_ptr();

    while (LockFile(p->file[handle].f_handle, (DWORD)ofs,
                 0L, (DWORD)length, 0L) == FALSE)
    {
        sleep(1);
    }

    return 0;
}

int lock(int handle, long ofs, long length)
{
    EMXPROCESS *p = _rsxnt_get_process_ptr();

    if (LockFile(p->file[handle].f_handle, (DWORD)ofs,
                 0L, (DWORD)length, 0L) == FALSE)
    {
        return -1;
    }
    return 0;
}

int unlock(int handle, long ofs, long length)
{
    EMXPROCESS *p = _rsxnt_get_process_ptr();

    if (UnlockFile(p->file[handle].f_handle, (DWORD)ofs,
                   0L, (DWORD)length, 0L) == FALSE)
    {
        return -1;
    }
    return 0;
}

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
    return fcntl(handle, F_SETLK, file_lock(F_WRLCK, ofs, length));
}

int waitlock(int handle, long ofs, long length)
{
    return fcntl(handle, F_SETLKW, file_lock(F_WRLCK, ofs, length));
}

int unlock(int handle, long ofs, long length)
{
    return fcntl(handle, F_SETLK, file_lock(F_UNLCK, ofs, length));
}


int sopen(const char *name, int oflag, int ishared, int mode)
{
    int fd = open(name, oflag, mode);
    if (fcntl(fd, F_SETLK,
	      file_lock((ishared == SH_DENYNONE) ? F_RDLCK : F_WRLCK, 0, 0)))

    {
	close(fd);
	return -1;
    }
    return fd;
}

#else

#ifdef OS2
#define INCL_DOSDATETIME
#include <os2.h>
#endif

int waitlock(int handle, long ofs, long length)
{
    while (lock(handle, ofs, length) == -1)
    {
        mysleep(1);
    }
    return 0;
}

#endif






