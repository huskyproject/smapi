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

#ifndef __LOCKING_H__
#define __LOCKING_H__

#include "compiler.h"

#if defined(MSDOS)
sword far pascal shareloaded(void);
#elif defined(OS2) || defined(__NT__) || defined(UNIX)
#define shareloaded() TRUE
#else
#define shareloaded() FALSE
#endif

int lock(int handle, long ofs, long length);
int unlock(int handle, long ofs, long length);
int waitlock(int handle, long ofs, long length);
int waitlock2(int handle, long ofs, long length, long t);

#endif



