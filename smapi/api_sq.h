/***************************************************************************
 *                                                                         *
 *  Squish Developers Kit Source, Version 2.00                             *
 *  Copyright 1989-1994 by SCI Communications.  All rights reserved.       *
 *                                                                         *
 *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
 *  SQUISH DEVELOPERS KIT LICENSING AGREEMENT IN SQDEV.PRN.  IF YOU DO NOT *
 *  FIND THE TEXT OF THIS AGREEMENT IN THE AFOREMENTIONED FILE, OR IF YOU  *
 *  DO NOT HAVE THIS FILE, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT    *
 *  ONE OF THE ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO  *
 *  USE THIS FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE SQUISH          *
 *  DEVELOPERS KIT LICENSING AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE *
 *  ABLE TO REACH WITH THE AUTHOR.                                         *
 *                                                                         *
 *  You can contact the author at one of the address listed below:         *
 *                                                                         *
 *  Scott Dudley       FidoNet     1:249/106                               *
 *  777 Downing St.    Internet    sjd@f106.n249.z1.fidonet.org            *
 *  Kingston, Ont.     CompuServe  >INTERNET:sjd@f106.n249.z1.fidonet.org  *
 *  Canada  K7M 5N3    BBS         1-613-634-3058, V.32bis                 *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef __API_SQ_H_DEFINED
#define __API_SQ_H_DEFINED

#include <huskylib/compiler.h>

#include "msgapi.h"

struct _sqhdr;
struct _sqidx;
typedef struct _sqidx SQIDX;
typedef struct _sqhdr SQHDR;
typedef hUINT32       FOFS;
/* Try to access a locked Squish base up to five times */

#define SQUISH_LOCK_RETRY 5
/* Expand the Squish index by 64 records at a time */

#define SQUIQSH_IDX_EXPAND 64
/* No frame offset */

#define NULL_FRAME ((FOFS)0L)
/* Frame types for sqhdr.frame_type */

#define FRAME_NORMAL 0x00     /* Normal text frame */
#define FRAME_FREE 0x01       /* Part of the free chain */
#define FRAME_LZSS 0x02       /* Not implemented */
#define FRAME_UPDATE 0x03     /* Frame is being updated by another task */
/* BItmask for sqidx.hash to indicate that the msg was received */

#define IDXE_MSGREAD 0x80000000Lu
/* Macros for accessing the hidden parts of data structures */

#define Sqd ((SQDATA *)(ha->apidata))
#define HSqd ((SQDATA *)(((struct _msgh *)hmsg)->ha->apidata))
/* Squish frame header.  This comes before each and every message in a      *
 * Squish message base.                                                     */
struct _sqhdr
{
    #define SQHDRID 0xafae4453L

    dword id;           /* sqhdr.id must always equal SQHDRID */
    FOFS  next_frame;   /* Next frame in the linked list */
    FOFS  prev_frame;   /* Prior frame in the linked list */
    dword frame_length; /* Length of this frame */
    dword msg_length;   /* Length used in this frame by XMSG, ctrl and text */
    dword clen;         /* Length used in this frame by ctrl info only */
    word  frame_type;   /* Type of frame -- see above FRAME_XXXX */
    word  rsvd;         /* Reserved for future use */
};

/* An individual index entry in <area>.SQI */
struct _sqidx
{
    FOFS   ofs;               /* Offset of the frame relating to this msg */
    UMSGID umsgid;            /* Unique message identifier of this msg */
    dword  hash;              /* SquishHash of msg.to for this msg */
};

/* Header block at offset 0 of <area>.SQD */
typedef struct _sqbase
{
    word  len;            /* LENGTH OF THIS STRUCTURE! */           /*   0 */
    word  rsvd1;          /* reserved */                            /*   2 */
    dword num_msg;        /* Number of messages in area */          /*   4 */
    dword high_msg;       /* Highest msg in area. Same as num_msg*/ /*   8 */
    dword skip_msg;       /* Skip killing first x msgs in area */   /*  12 */
    dword high_water;     /* Msg# (not umsgid) of HWM */            /*  16 */
    dword uid;            /* Number of the next UMSGID to use */    /*  20 */
    byte  base[80];       /* Base name of SquishFile */             /*  24 */
    FOFS  begin_frame;    /* Offset of first frame in file */       /* 104 */
    FOFS  last_frame;     /* Offset to last frame in file */        /* 108 */
    FOFS  free_frame;     /* Offset of first FREE frame in file */  /* 112 */
    FOFS  last_free_frame; /* Offset of last free frame in file */   /* 116 */
    FOFS  end_frame;      /* Pointer to end of file */              /* 120 */
    dword max_msg;        /* Max # of msgs to keep in area */       /* 124 */
    word  keep_days;      /* Max age of msgs in area (SQPack) */    /* 128 */
    word  sz_sqhdr;       /* sizeof(SQHDR) */                       /* 130 */
    byte  rsvd2[124];     /* Reserved by Squish for future use*/    /* 132 */
    /* total: 256 */
} SQBASE;
typedef struct
{
    dword       dwUsed;     /* Number of entries used in this seg */
    dword       dwMax;      /* Number of entries allocated in this seg */
    SQIDX far * psqi;       /* Pointer to index entries for this segment */
} SQIDXSEG;
/* Handle to a Squish index file */
typedef struct
{
    #define ID_HIDX 0x9fee

    word       id;                    /* Must be ID_HIDX */
    HAREA      ha;                    /* Area to which this index belongs */
    long       lAllocatedRecords;     /* Space allocated in idx file */
    long       lDeltaLo;              /* Low # of changed msg */
    long       lDeltaHi;              /* High # of changed msg */
    int        fBuffer;               /* Use index buffer? */
    int        cSeg;                  /* Number of segments used */
    SQIDXSEG * pss;                   /* Segments containing messages */
} *HIDX;
/* Private data in handle passed among API functions which handle message   *
 * areas.                                                                   */
typedef struct _sqdata
{
    word   cbSqbase;      /* Length of the .SQD file header */
    word   cbSqhdr;       /* Length of a .SQD frame header */
    dword  dwMaxMsg;      /* Max number of msgs in area */
    word   wMaxDays;      /* Max age (in days) of msgs in area */
    word   wSkipMsg;      /* Number of msgs to skip before keeping wMaxMsg */
    dword  dwHighWater;   /* High water message NUMBER */
    UMSGID uidNext;       /* Next UMSGID to assign */
    FOFS   foFirst;       /* Offset of first frame in file */
    FOFS   foLast;        /* Offset to last frame in file */
    FOFS   foFree;        /* Offset of first FREE frame in file */
    FOFS   foLastFree;    /* Offset of last free frame in file */
    FOFS   foEnd;         /* Pointer to end of file */
    FOFS   foNext;        /* Next frame in the linked list */
    FOFS   foPrev;        /* Prior frame in the linked list */
    FOFS   foCur;         /* Current frame position */
    word   fHaveExclusive; /* Are we currently updating the base header? */
    word   fLocked;       /* Do we have byte 0 locked? */
    word   fLockFunc;     /* Number of times we have called Lock w/o Unlock */
    int    sfd;           /* SquishFile handle */
    int    ifd;           /* SquishIndex handle */
    SQBASE sqbDelta;      /* Last _sqbase read from .SQD file */
    /* Linked lists indicating open resources */
    HAREA haNext;         /* Next area in the list of open areas */
    HMSG  hmsgOpen;       /* List of open messages */
    HIDX  hix;            /* Index handle for current base */
} SQDATA;
/* Message handle.  This is passed back and forth between all of the        *
 * API functions that handle specific messages.                             */
struct _msgh
{
    HAREA ha;                 /* Area to which this message belongs */
    dword id;                 /* Must always equal MSGH_ID */
    dword bytes_written;      /* Bytes written to this msg so far */
    dword cur_pos;            /* Current read posn within msg */
    /* Squish-specific information starts here */
    dword dwMsg;              /* This message number */
    /* Frame offset of this message, IF we are reading a message.             *
     *                                                                        *
     * However, if we are writing a message, this may hold one of             *
     * several things, depending on the value of wMode:                       *
     *                                                                        *
     *                                                                        *
     * wMode==MOPEN_CREATE:  If we are writing a completely new message,      *
     *                       this field is zero.  Otherwise, if we are        *
     *                       creating a message on top of an existing         *
     *                       message, this will hold the frame offset of      *
     *                       the old message.                                 *
     * wMode==MOPEN_RW       This holds the offset of the message that        *
     *    or  MOPEN_WRITE    we are rewriting (same as foWrite)               */
    FOFS foRead;
    /* SQHDR used when reading.                                               *
     *                                                                        *
     * If writing, this will hold the frame header of the message that we     *
     * replaced.  (We only replaced a message if foRead != NULL_FRAME)        */
    SQHDR sqhRead;
    /* If we are writing a message, this holds the offset of the current      *
     * frame header.  If no frame header has been allocated for the message   *
     * yet, this field will be NULL_FRAME.                                    */
    FOFS foWrite;
    /* If we are writing a message, this holds the frame that we wrote.  This *
     * SQHDR is only valid when foWrite != NULL_FRAME.                        */
    SQHDR sqhWrite;
    /* If we know the UMSGID for this message, uidUs is non-zero */
    UMSGID uidUs;
    dword  dwWritePos;        /* Current write position */
    word   wMode;             /* MOPEN_READ, MOPEN_WRITE, or MOPEN_RW */
    word   fDiskErr;          /* Has a disk error occurred? */
    word   fWritten;          /* Have we already written to this message? */
    HMSG   hmsgNext;          /* Next msg in the list of open msgs */
};

#define SQHDR_SIZE 28
#define SQIDX_SIZE 12
#define SQBASE_SIZE 256


HUSKYEXT int read_xmsg(int handle, XMSG * pxmsg);
HUSKYEXT int write_xmsg(int handle, XMSG * pxmsg);
HUSKYEXT int read_sqhdr(int, SQHDR *);
HUSKYEXT int write_sqhdr(int, SQHDR *);

HUSKYEXT int read_sqidx(int, SQIDX *, dword);
HUSKYEXT int write_sqidx(int, SQIDX *, dword);
HUSKYEXT int read_sqbase(int handle, struct _sqbase * psqbase);
HUSKYEXT int write_sqbase(int handle, struct _sqbase * psqbase);

#endif /* __API_SQ_H_DEFINED */
