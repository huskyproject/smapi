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

#ifndef __API_SQD_H__
#define __API_SQD_H__

struct _sqdata
{
    int sfd;                    /* SquishFile handle */
    int ifd;                    /* SquishIndex handle */

    byte base[80];              /* Base name for SquishFile */

    FOFS begin_frame;           /* Offset of first frame in file */
    FOFS last_frame;            /* Offset to last frame in file */
    FOFS free_frame;            /* Offset of first FREE frame in file */
    FOFS last_free_frame;       /* Offset of LAST free frame in file */
    FOFS end_frame;             /* Pointer to end of file */

    FOFS next_frame;
    FOFS prev_frame;
    FOFS cur_frame;

    dword uid;
    dword max_msg;
    dword skip_msg;
    word keep_days;

    byte flag;
    byte rsvd1;

    word sz_sqhdr;
    byte rsvd2;

    word len;                   /* Old length of sqb structure */

    dword idxbuf_size;          /* Size of the allocated buffer */
    dword idxbuf_used;          /* # of bytes being used to hold messages */
    dword idxbuf_write;         /* # of bytes we should write to index file */
    dword idxbuf_delta;         /* Starting position from which the index
                                 * has chhg */

    struct _sqbase delta;       /* Copy of last-read sqbase, to determine
                                 * changes */

    word msgs_open;

    SQIDX far *idxbuf;
    
    HAREA  hanext;          /* Next open Area */
    HMSG   hmsgopen;        /* List of open messages */
};

#endif
