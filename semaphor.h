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
 
#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

#include "compiler.h"

/* At the moment cygwin (ver 1.3.12) doesn't support ipc, this will change *
 * in the future. Then NOSEMAPHORES is not longer needed for cygwin.       */
#if !defined(NOSEMAPHORES) && (defined(__NT__) || defined(__MINGW32__) || defined(__sun__) || defined(__CYGWIN__))
#define NOSEMAPHORES
#endif

/* No semaphores (unsupported or non-multitasking systems) */
#if defined(NOSEMAPHORES)

#define SEMAPHORE int

#define create_semaphore(s) 
#define delete_semaphore(s)
#define lock_semaphore(s)
#define unlock_semaphore(s)

/* BeOS */
#elif defined(__BEOS__)

#include <OS.h>

#define SEMAPHORE sem_id

/* System V semaphores */
#elif defined(UNIX)

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEMAPHORE int

/* IBM OS2 semaphores */
#elif defined(OS2)

#if 0
/*
we don't want to pollute our name space with all the fucking stuff from os.h,
so we simply KNOW that a HMTX is a ULONG.
*/
#define INCL_DOS
#include <os2.h>
#else
typedef unsigned long HMTX;
#endif

#define SEMAPHORE HMTX

#else

#error "Don't know how to setup semaphore. Use -DNOSEMAPHORES"

#endif

#ifdef HAS_SEMUN_UNDECL
union semun {
     int val;
     struct semid_ds *buf;
     ushort *array;
};
#endif

#ifndef NOSEMAPHORES
void create_semaphore(SEMAPHORE *sem);
void delete_semaphore(SEMAPHORE *sem);
void lock_semaphore(SEMAPHORE *sem);
void unlock_semaphore(SEMAPHORE *sem);
#endif

#endif
