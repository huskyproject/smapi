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

#if defined(UNIX) || defined(__EMX__)
#include <unistd.h>
#endif
#include <stdlib.h> /* NULL */

#include "semaphor.h"

#if defined(NOSEMAPHORES)

/* No code for Non-Multitasking Systems */

#elif defined(__BEOS__)

void create_semaphore(SEMAPHORE *sem)
{
  *sem = create_sem(0,0);
  release_sem(*sem);
}

void delete_semaphore(SEMAPHORE *sem)
{
  delete_sem(*sem);
}

void lock_semaphore(SEMAPHORE *sem)
{
  if (acquire_sem(*sem) != B_NO_ERROR)
    snooze(10);
}

void unlock_semaphore(SEMAPHORE *sem)
{
  release_sem(*sem);
}

#elif defined(UNIX)

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
   /* union semun is defined by including <sys/sem.h> */
#else
   /* according to X/OPEN we have to define it ourselves */
   union semun 
   {
     int val;                    /* value for SETVAL */
     struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
     unsigned short int *array;  /* array for GETALL, SETALL */
     struct seminfo *__buf;      /* buffer for IPC_INFO */
   };
#endif   

void create_semaphore(SEMAPHORE *sem)
{
  union semun fl;
  fl.val = 1;

  *sem = semget(IPC_PRIVATE, 1, 0600|IPC_CREAT);

  semctl(*sem, 0, SETVAL, fl);
}

void delete_semaphore(SEMAPHORE *sem)
{
  union semun fl;
  fl.val = 0;

  semctl(*sem, 0, IPC_RMID, fl);
}

void lock_semaphore(SEMAPHORE *sem)
{
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op  = -1;
  sops.sem_flg = 0;

  while (semop(*sem, &sops, 1))
    usleep(10);
}

void unlock_semaphore(SEMAPHORE *sem)
{
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op  = 1;
  sops.sem_flg = 0;

  semop(*sem, &sops, 1);
}

#elif defined(OS2)

#define INCL_DOS
#include <os2.h>


void create_semaphore(SEMAPHORE *s)
{
  DosCreateMutexSem(NULL, s, DC_SEM_SHARED, 0);
}

void lock_semaphore(SEMAPHORE *s)
{
  DosRequestMutexSem(*s, SEM_INDEFINITE_WAIT);
}

void unlock_semaphore(SEMAPHORE *s)
{
  DosReleaseMutexSem(*s);
}

void delete_semaphore(SEMAPHORE *s)
{
  DosCloseMutexSem(*s);
}

#endif
