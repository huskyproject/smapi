#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

#include "compiler.h"

/* BeOS */
#ifdef __BEOS__

#include <OS.h>

#define SEMAPHORE sem_id

/* System V semaphores */
#elif defined(UNIX)

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEMAPHORE int

/* No semaphores (unsupported or non-multitasking systems) */
#elif defined(NOSEMAPHORES)

#define SEMAPHORE int

#define create_semaphore(s) 
#define delete_semaphore(s)
#define lock_semaphore(s)
#define unlock_semaphore(s)

#else

#error "Don't know how to setup semaphore. Use -DNOSEMAPHORES"

#endif

#ifndef NOSEMAPHORES
void create_semaphore(SEMAPHORE *sem);
void delete_semaphore(SEMAPHORE *sem);
void lock_semaphore(SEMAPHORE *sem);
void unlock_semaphore(SEMAPHORE *sem);
#endif

#endif