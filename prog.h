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

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define ZONE_ALL  56685u
#define NET_ALL   56685u
#define NODE_ALL  56685u
#define POINT_ALL 56685u

/* Not used
#define eqstr(str1,str2)      (strcmp(str1,str2)==0)
#define eqstri(str1,str2)     (stricmp(str1,str2)==0)
#define eqstrn(str1,str2,n)   (strncmp(str1,str2,n)==0)
*/

extern char _stdc months[][10];
extern char _stdc weekday[][10];

extern char _stdc months_ab[][4];
extern char _stdc weekday_ab[][4];

#include "progprot.h"

#endif
