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

#ifndef __PROG_H__
#define __PROG_H__

#include <stdio.h>
#include <time.h>
#include "compiler.h"
#include "typedefs.h"
#include "stamp.h"

#ifdef __FARDATA__

#include "alc.h"

#ifndef ZREE
#define malloc(n)     farmalloc(n)
#define calloc(n,u)   farcalloc(n,u)
#define free(p)       farfree(p)
#define realloc(p,n)  farrealloc(p,n)
#endif

#endif

#if defined(_MSC_VER)

#ifndef ZREE

#define farmalloc(n)    _fmalloc(n)
#define farfree(p)      _ffree(p)
#define farrealloc(p,n) _frealloc(p,n)

void far *farcalloc(int n, int m);

#ifdef _MSC_VER
#if _MSC_VER >= 600
#define farcalloc(a,b) _fcalloc(a,b)
#endif
#endif

#endif

#define NO_STRFTIME
#define NO_MKTIME

#elif defined(__TURBOC__)

#if __TURBOC__ == 0x0295  /* TC++ includes a strftime() function */
#define NO_STRFTIME
#define NO_MKTIME
#endif

#endif

#undef farcalloc
#undef farmalloc
#undef farrealloc
#undef farfree
#undef _fmalloc

#define farcalloc  calloc
#define farmalloc  malloc
#define farrealloc realloc
#define farfree    free
#define _fmalloc   malloc

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#ifdef PATHLEN
#undef PATHLEN
#endif

/* Default separator for path specification */

#if defined(SASC) || defined(UNIX)
#define PATH_DELIM  '/'
#else
#define PATH_DELIM  '\\'
#endif

#define PATHLEN           120   /* Max. length of a path */

#define ZONE_ALL  56685u
#define NET_ALL   56685u
#define NODE_ALL  56685u
#define POINT_ALL 56685u

#define eqstr(str1,str2)      (strcmp(str1,str2)==0)
#define eqstri(str1,str2)     (stricmp(str1,str2)==0)
#define eqstrn(str1,str2,n)   (strncmp(str1,str2,n)==0)

#ifndef min
#define min(a,b)              (((a) < (b)) ? (a) : (b))
#endif

extern char _stdc months[][10];
extern char _stdc weekday[][10];

extern char _stdc months_ab[][4];
extern char _stdc weekday_ab[][4];

#include "progprot.h"

#ifndef NO_STRFTIME

/* If compiler doesn't include a strftime(), we need our own */

#include <time.h>
#include <sys/types.h>

size_t _stdc strftime(char *str, size_t maxsize, const char *fmt,
  const struct tm *tm_ptr);

#endif

#ifndef NO_MKTIME

/* If compiler doesn't include a mktime(), we need our own */

#include <time.h>
#include <sys/types.h>

time_t _stdc mktime(struct tm *tm_ptr);

#endif

#ifdef SH_DENYNO
#ifndef SH_DENYNONE
#define SH_DENYNONE SH_DENYNO
#endif
#endif

#endif
