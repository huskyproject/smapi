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

#include <stdlib.h>

#include "compiler.h"


#if !defined(__UNIX__) && !defined(SASC)
#include <io.h>
#endif

#if defined(__UNIX__) || defined(__DJGPP__)
#include <unistd.h>
#endif

#if !defined(__IBMC__) && !defined(__DOS__) && !defined(__UNIX__) && !defined(__MINGW32__) && !defined(__MSVC__)
#include <dos.h>
#endif

#ifdef __OS2__

#  define INCL_NOPM
#  include <os2.h>

#  if defined(__WATCOMC__)
#    ifndef DosBufReset
#      define DosBufReset DosResetBuffer
#    endif
#  elif defined(__EMX__) || defined(__FLAT__)
#    undef DosBufReset
#    define DosBufReset DosResetBuffer
#  endif
#endif

#if defined(__NT__) || defined(__MINGW32__)
#  define WIN32_LEAN_AND_MEAN
#  define NOGDI
#  define NOUSER
#  define NOMSG
#  include <windows.h>
#endif

#if defined(__DJGPP__)
/*
void pascal far flush_handle2(int fh)
{
    fsync(fh);
}
*/
#define flush_handle2(f)  fsync(f)

#elif defined(__UNIX__) || defined(SASC)
/*
void pascal far flush_handle2(int fh)
{
    unused(fh);
}
*/
#define flush_handle2(f)  unused(f)


#elif defined(__OS2__)
/*
void pascal far flush_handle2(int fh)
{
    DosBufReset((HFILE) fh);
}
*/
#define flush_handle2(f)  DosBufReset((HFILE) f)

#elif defined(__NT__) || defined(__MINGW32__)

#ifdef __RSXNT__
#  include <emx/syscalls.h>

#  ifndef F_GETOSFD
#    define F_GETOSFD 6
#  endif

#  define flush_handle2(fh)  FlushFileBuffers((HANDLE) __fcntl((fh), F_GETOSFD, 0))

#else

#  define flush_handle2(fh)  FlushFileBuffers((HANDLE) (fh))

#endif

/*
void pascal far flush_handle2(int fh)
{
#ifdef __RSXNT__
    int nt_handle = __fcntl(fh, F_GETOSFD, 0);
#else
    int nt_handle = fh;
#endif
    FlushFileBuffers((HANDLE) nt_handle);
}
*/

#elif defined(__WATCOMC__DOS__)

#include <dos.h>
/*
void pascal far flush_handle2(int fh)
{
    _dos_commit(fh);
}
*/
#  define flush_handle2(fh)  _dos_commit(fh)

#elif defined (__DOS__)

/* refer to MS-DOS flush_handle2 code in FLUSHASM.ASM */
void pascal far flush_handle2(int fd);

#else

#error unknown compiler

#endif

/*
 *  This makes sure a file gets flushed to disk.  Thanks to Ray Duncan
 *  for this tip in his _Advanced MS-DOS_ book.
 */

void _fast flush_handle(FILE * fp)
{
    fflush(fp);

#if defined(__OS2__) || defined(__DOS__) || defined(__NT__) || defined(__TURBOC__) || defined(SASC) || defined(__DJGPP__)
    flush_handle2(fileno(fp));
#else
    {
        int nfd;

        nfd = dup(fileno(fp));
        if (nfd != -1)
        {
            close(nfd);
        }
    }
#endif
}
