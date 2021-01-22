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

#ifndef __OLD_MSG_H__
#define __OLD_MSG_H__

#include <huskylib/compiler.h>
#include <huskylib/cvtdate.h>

#include "msgapi.h"
/* OPUS message header structure */
struct _omsg
{
    byte  from[36];
    byte  to[36];
    byte  subj[72];
    byte  date[20];              /* Obsolete/unused ASCII date information */
    word  times;                 /* FIDO<tm>: Number of times read */
    sword dest;                  /* Destination node */
    sword orig;                  /* Origination node number */
    word  cost;                  /* Unit cost charged to send the message */
    sword orig_net;              /* Origination network number */
    sword dest_net;              /* Destination network number */
    /*
     *  A timestamp is a 32-bit integer in the UNIX flavor (ie. the number
     *  of seconds since midnight, January 1, 1970).  Timestamps in messages
     *  are always Greenwich Mean Time (UTC), never local time.
     *
     *  TE 05/27/98: I doubt that the comment above is true. The Opus
     *  timestamps are in the DOS date format, not inthe UNIX stamp format.
     */
    struct _stamp date_written;  /* When user wrote the msg */
    struct _stamp date_arrived;  /* When msg arrived on-line */
    word          reply;         /* Current msg is a reply to this msg nr */
    word          attr;          /* Attribute (behavior) of the message */
    word          up;            /* Next message in the thread */
};

/* OPUS message header size (on .MSG file) */
#define OMSG_SIZE 190

#endif // ifndef __OLD_MSG_H__
