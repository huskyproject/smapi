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

/*  STRUCTRW.C written 1998 by Tobias Ernst
 *
 *  This file contains routines read and write packed structures like the
 *  struct XMSG in an platform-independent manner.
 *
 *  Background information: You should never ever read or write any
 *  structure directly from disk with something like
 *  fread(&structure, sizeof(structure), 1, f)
 *  This will fail on some compliers that can't be told to pack structures
 *  in exactly the way that they are supposed to be. It will also fail
 *  when trying to read a file that is supposed to be used by a little-
 *  endian machine (like Intel) on a big endian machine (like PPC).
 *
 *  So the conclusion: Never use fread, fwrite, farread, farwrite on
 *  structures - use the routines from this module instead, or if you are
 *  introducing a new structure, add a routine for it to this file.
 *
 *  If you have any questions on this topic, feel free to contact me
 *  at 2:2476/418 or tobi@bland.fido.de.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if !defined(UNIX) && !defined(SASC)
#include <io.h>
#endif
#ifdef UNIX
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MSGAPI_HANDLERS
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "old_msg.h"


/*
 *  get_dword
 *
 *  Reads in a 4 byte word that is stored in little endian (Intel) notation
 *  and converts it to the local representation n an architecture-
 *  independent manner
 */

#define get_dword(ptr)            \
   ((dword)((ptr)[0]) |           \
    (((dword)((ptr)[1])) << 8)  | \
    (((dword)((ptr)[2])) << 16) | \
    (((dword)((ptr)[3])) << 24))  \

/*
 *  get_word
 *
 *  Reads in a 2 byte word that is stored in little endian (Intel) notation
 *  and converts it to the local representation in an architecture-
 *  independent manner
 */

#define get_word(ptr)         \
    ((word)(ptr)[0] |         \
     (((word)(ptr)[1]) << 8 ))


/*
 *  put_dword
 *
 *  Writes a 4 byte word in little endian notation, independent of the local
 *  system architecture.
 */

static void put_dword(byte *ptr, dword value)
{
    ptr[0] = (value & 0xFF);
    ptr[1] = (value >> 8) & 0xFF;
    ptr[2] = (value >> 16) & 0xFF;
    ptr[3] = (value >> 24) & 0xFF;
}

/*
 *  put_word
 *
 *  Writes a 4 byte word in little endian notation, independent of the local
 *  system architecture.
 */

static void put_word(byte *ptr, word value)
{
    ptr[0] = (value & 0xFF);
    ptr[1] = (value >> 8) & 0xFF;
}

int read_xmsg(sword handle, XMSG *pxmsg)
{
    byte buf[XMSG_SIZE], *pbuf = buf;
    word rawdate, rawtime;
    int i;

    if (farread(handle, (byte far *)buf, XMSG_SIZE) != XMSG_SIZE)
    {
        return 0;
    }

                                /* 04 bytes "attr" */
    pxmsg->attr = get_dword(pbuf);
    pbuf += 4;

                                /* 36 bytes "from" */
    memmove(pxmsg->from, pbuf, XMSG_FROM_SIZE);
    pbuf += XMSG_FROM_SIZE;

                                /* 36 bytes "to"   */
    memmove(pxmsg->to, pbuf, XMSG_TO_SIZE);
    pbuf += XMSG_TO_SIZE;

                                /* 72 bytes "subj" */
    memmove(pxmsg->subj, pbuf, XMSG_SUBJ_SIZE);
    pbuf += XMSG_SUBJ_SIZE;

                                /* 8 bytes "orig"  */
    pxmsg->orig.zone = get_word(pbuf); pbuf += 2;
    pxmsg->orig.net  = get_word(pbuf); pbuf += 2;
    pxmsg->orig.node = get_word(pbuf); pbuf += 2;
    pxmsg->orig.point= get_word(pbuf); pbuf += 2;

                                /* 8 bytes "dest"  */
    pxmsg->dest.zone = get_word(pbuf); pbuf += 2;
    pxmsg->dest.net  = get_word(pbuf); pbuf += 2;
    pxmsg->dest.node = get_word(pbuf); pbuf += 2;
    pxmsg->dest.point= get_word(pbuf); pbuf += 2;

                                /* 4 bytes "date_written" */
    rawdate = get_word(pbuf); pbuf += 2;
    rawtime = get_word(pbuf); pbuf += 2;
    pxmsg->date_written.date.da = rawdate & 31;
    pxmsg->date_written.date.mo = (rawdate >> 5) & 15;
    pxmsg->date_written.date.yr = (rawdate >> 9) & 127;
    pxmsg->date_written.time.ss = rawtime & 31;
    pxmsg->date_written.time.mm = (rawtime >> 5) & 63;
    pxmsg->date_written.time.hh = (rawtime >> 11) & 31;

                                /* 4 bytes "date_arrived" */
    rawdate = get_word(pbuf); pbuf += 2;
    rawtime = get_word(pbuf); pbuf += 2;
    pxmsg->date_arrived.date.da = rawdate & 31;
    pxmsg->date_arrived.date.mo = (rawdate >> 5) & 15;
    pxmsg->date_arrived.date.yr = (rawdate >> 9) & 127;
    pxmsg->date_arrived.time.ss = rawtime & 31;
    pxmsg->date_arrived.time.mm = (rawtime >> 5) & 63;
    pxmsg->date_arrived.time.hh = (rawtime >> 11) & 31;

                                /* 2 byte "utc_ofs" */
    pxmsg->utc_ofs = get_word(pbuf);
    pbuf += 2;

                                /* 4 bytes "replyto" */
    pxmsg->replyto = get_dword(pbuf);
    pbuf += 4;

                                /* 10 times 4 bytes "replies" */
    for (i = 0; i < MAX_REPLY; i++)
    {
        pxmsg->replies[i] = get_dword(pbuf);
        pbuf += 4;
    }

                                /* 20 times FTSC date stamp */
    memmove(pxmsg->__ftsc_date, pbuf, 20);
    pbuf += 20;

    assert(pbuf - buf == XMSG_SIZE);
    return 1;
}

int write_xmsg(sword handle, XMSG *pxmsg)
{
    byte buf[XMSG_SIZE], *pbuf = buf;
    word rawdate, rawtime;
    int i;

                                /* 04 bytes "attr" */
    put_dword(pbuf, pxmsg->attr);
    pbuf += 4;

                                /* 36 bytes "from" */
    memmove(pbuf, pxmsg->from, XMSG_FROM_SIZE);
    pbuf += XMSG_FROM_SIZE;

                                /* 36 bytes "to"   */
    memmove(pbuf, pxmsg->to, XMSG_TO_SIZE);
    pbuf += XMSG_TO_SIZE;

                                /* 72 bytes "subj" */
    memmove(pbuf, pxmsg->subj, XMSG_SUBJ_SIZE);
    pbuf += XMSG_SUBJ_SIZE;

                                /* 8 bytes "orig"  */
    put_word(pbuf, pxmsg->orig.zone);  pbuf += 2;
    put_word(pbuf, pxmsg->orig.net);   pbuf += 2;
    put_word(pbuf, pxmsg->orig.node);  pbuf += 2;
    put_word(pbuf, pxmsg->orig.point); pbuf += 2;

                                    /* 8 bytes "dest"  */
    put_word(pbuf, pxmsg->dest.zone);  pbuf += 2;
    put_word(pbuf, pxmsg->dest.net);   pbuf += 2;
    put_word(pbuf, pxmsg->dest.node);  pbuf += 2;
    put_word(pbuf, pxmsg->dest.point); pbuf += 2;


                                /* 4 bytes "date_written" */
    rawdate = rawtime = 0;

    rawdate |= (((word)pxmsg->date_written.date.da) & 31);
    rawdate |= (((word)pxmsg->date_written.date.mo) & 15) << 5;
    rawdate |= (((word)pxmsg->date_written.date.yr) & 127) << 9;

    rawtime |= (((word)pxmsg->date_written.time.ss) & 31);
    rawtime |= (((word)pxmsg->date_written.time.mm) & 63) << 5;
    rawtime |= (((word)pxmsg->date_written.time.hh) & 31) << 11;

    put_word(pbuf, rawdate); pbuf += 2;
    put_word(pbuf, rawtime); pbuf += 2;


                                /* 4 bytes "date_arrvied" */
    rawdate = rawtime = 0;

    rawdate |= (((word)pxmsg->date_arrived.date.da) & 31);
    rawdate |= (((word)pxmsg->date_arrived.date.mo) & 15) << 5;
    rawdate |= (((word)pxmsg->date_arrived.date.yr) & 127) << 9;

    rawtime |= (((word)pxmsg->date_arrived.time.ss) & 31);
    rawtime |= (((word)pxmsg->date_arrived.time.mm) & 63) << 5;
    rawtime |= (((word)pxmsg->date_arrived.time.hh) & 31) << 11;

    put_word(pbuf, rawdate); pbuf += 2;
    put_word(pbuf, rawtime); pbuf += 2;


                                /* 2 byte "utc_ofs" */
    put_word(pbuf, pxmsg->utc_ofs);
    pbuf += 2;

                                /* 4 bytes "replyto" */
    put_dword(pbuf, pxmsg->replyto);
    pbuf += 4;

                                /* 10 times 4 bytes "replies" */
    for (i = 0; i < MAX_REPLY; i++)
    {
        put_dword(pbuf, pxmsg->replies[i]);
        pbuf += 4;
    }

                                /* 20 times FTSC date stamp */
    memmove(pbuf, pxmsg->__ftsc_date, 20);
    pbuf += 20;

    assert(pbuf - buf == XMSG_SIZE);
    return (farwrite(handle, (byte far *)buf, XMSG_SIZE) == XMSG_SIZE);
}

int read_sqhdr(sword handle, SQHDR *psqhdr)
{
    byte buf[SQHDR_SIZE], *pbuf = buf;

    if (farread(handle, (byte far *)buf, SQHDR_SIZE) != SQHDR_SIZE)
    {
        return 0;
    }
                                /* 4 bytes "id" */
    psqhdr->id = get_dword(pbuf);
    pbuf += 4;

                                /* 4 bytes "next_frame" */
    psqhdr->next_frame = get_dword(pbuf);
    pbuf += 4;

                                /* 4 bytes "prev_frame" */
    psqhdr->prev_frame = get_dword(pbuf);
    pbuf += 4;

                                /* 4 bytes "frame_length" */
    psqhdr->frame_length = get_dword(pbuf);
    pbuf += 4;

                                /* 4 bytes "msg_length" */
    psqhdr->msg_length = get_dword(pbuf);
    pbuf += 4;
                                /* 4 bytes "clen" */
    psqhdr->clen = get_dword(pbuf);
    pbuf += 4;

                                /* 2 bytes "frame_type" */
    psqhdr->frame_type = get_word(pbuf);
    pbuf += 2;
                                /* 4 bytes "rsvd" */
    psqhdr->rsvd = get_word(pbuf);
    pbuf += 2;

    assert(pbuf - buf == SQHDR_SIZE);

    return 1;
}

int write_sqhdr(sword handle, SQHDR *psqhdr)
{
    byte buf[SQHDR_SIZE], *pbuf = buf;

                                /* 4 bytes "id" */
    put_dword(pbuf, psqhdr->id);
    pbuf += 4;

                                /* 4 bytes "next_frame" */
    put_dword(pbuf, psqhdr->next_frame);
    pbuf += 4;

                                /* 4 bytes "prev_frame" */
    put_dword(pbuf, psqhdr->prev_frame);
    pbuf += 4;

                                /* 4 bytes "frame_length" */
    put_dword(pbuf, psqhdr->frame_length);
    pbuf += 4;

                                /* 4 bytes "msg_length" */
    put_dword(pbuf, psqhdr->msg_length);
    pbuf += 4;
                                /* 4 bytes "clen" */
    put_dword(pbuf, psqhdr->clen);
    pbuf += 4;

                                /* 2 bytes "frame_type" */
    put_word(pbuf, psqhdr->frame_type);
    pbuf += 2;
                                /* 4 bytes "rsvd" */
    put_word(pbuf, psqhdr->rsvd);
    pbuf += 2;

    assert(pbuf - buf == SQHDR_SIZE);

    return (farwrite(handle, (byte far *)buf, SQHDR_SIZE) == SQHDR_SIZE);
}

/*
 * read_sqidx
 *
 * This function needs a little explanation. Just like the other functions,
 * it reads in a special structure, the SQIDX structure. The problem is
 * that this is done very often: It is not uncommon that 5000 SQIDXs are
 * being read in seqeuence. Therefore, I had to do a little performance
 * tuning here. I try to read in as much SQIDX structures as possible
 * at once, while at the same time being able to cope with the fact that
 * the OS might not be able to provide me with enough temporrary storage.
 *
 * Normally, you will not have to care about the buffering thing. Only
 * the code between "begin reading in a single structre" and "end reading
 * in a single structure" must be change if the structure layout changes.
 */

int read_sqidx(sword handle, SQIDX *psqidx, dword n)
{
    byte buf[SQIDX_SIZE], *pbuf = NULL;
    byte *accel_buffer = NULL;
    dword i, maxbuf = 0, rd;

    if (n > 1)
    {
        maxbuf = n;
        if ((dword)SQIDX_SIZE * (dword)n >= 32768L)
        {
            maxbuf = (dword)32768L / SQIDX_SIZE;
        }
        accel_buffer = malloc(SQIDX_SIZE * maxbuf);
    }

    for (i = 0; i < n; i++)
    {
        if (accel_buffer == NULL)
        {
            if (read(handle, buf, SQIDX_SIZE) != SQIDX_SIZE)
            {
                return 0;
            }
            pbuf = buf;
        }
        else
        {
            if (!(i % maxbuf))
            {
                rd = (i + maxbuf > n) ? (n - i) : maxbuf;
                if (read(handle, accel_buffer, rd * SQIDX_SIZE) !=
                    (int)(rd * SQIDX_SIZE))
                {
                    free(accel_buffer);
                    return 0;
                }
                pbuf = accel_buffer;
            }
        }

        /* Begin reading in a single structure */

                                /* 4 bytes "ofs" */
        psqidx[i].ofs = get_dword(pbuf);
        pbuf += 4;

                                /* 4 bytes "umsgid" */
        psqidx[i].umsgid = get_dword(pbuf);
        pbuf += 4;

                                /* 4 bytes "hash" */
        psqidx[i].hash = get_dword(pbuf);
        pbuf += 4;

        /* Stop reading in a single structure */
    }

    if (accel_buffer != NULL)
    {
        free(accel_buffer);
    }

    return 1;
}


int write_sqidx(sword handle, SQIDX *psqidx, dword n)
{
    byte buf[SQIDX_SIZE], *pbuf = NULL;
    byte *accel_buffer = NULL;
    dword i, maxbuf = 0, wr;

    if (n > 1)
    {
        maxbuf = n;
        if ((dword)SQIDX_SIZE * (dword)n >= 32768L)
        {
            maxbuf = (dword)32768L / SQIDX_SIZE;
        }
        accel_buffer = malloc(SQIDX_SIZE * maxbuf);
        pbuf = accel_buffer;
    }

    for (i = 0; i < n; i++)
    {
        if (accel_buffer == NULL)
        {
            pbuf = buf;
        }
 
                                /* 4 bytes "ofs" */
        put_dword(pbuf, psqidx[i].ofs);
        pbuf += 4;

                                /* 4 bytes "umsgid" */
        put_dword(pbuf, psqidx[i].umsgid);
        pbuf += 4;

                                /* 4 bytes "hash" */
        put_dword(pbuf, psqidx[i].hash);
        pbuf += 4;

        if (accel_buffer == NULL)
        {
            if (write(handle, buf, SQIDX_SIZE) != SQIDX_SIZE)
            {
                return 0;
            }
        }
        else
        {
            if (i == n - 1 || (!((i + 1) % maxbuf)))
            {
                wr = (!((i + 1) % maxbuf)) ? maxbuf : (n % maxbuf);
                
                if (write(handle, accel_buffer, wr * SQIDX_SIZE) !=
                    (wr * SQIDX_SIZE))
                {
                    free(accel_buffer);
                    return 0;
                }
                pbuf = accel_buffer;
            }
        }
    }

    return 1;
}

int read_sqbase(sword handle, struct _sqbase *psqbase)
{
    byte buf[SQBASE_SIZE], *pbuf = buf;

    if (farread(handle, (byte far *)buf, SQBASE_SIZE) != SQBASE_SIZE)
    {
        return 0;
    }

    psqbase->len = get_word(pbuf);
    pbuf += 2;

    psqbase->rsvd1 = get_word(pbuf);
    pbuf += 2;

    psqbase->num_msg = get_dword(pbuf);
    pbuf += 4;

    psqbase->high_msg = get_dword(pbuf);
    pbuf += 4;

    psqbase->skip_msg = get_dword(pbuf);
    pbuf += 4;

    psqbase->high_water = get_dword(pbuf);
    pbuf += 4;

    psqbase->uid = get_dword(pbuf);
    pbuf += 4;

    memmove(psqbase->base, pbuf, 80);
    pbuf += 80;

    psqbase->begin_frame = get_dword(pbuf);
    pbuf += 4;

    psqbase->last_frame = get_dword(pbuf);
    pbuf += 4;

    psqbase->free_frame = get_dword(pbuf);
    pbuf += 4;

    psqbase->last_free_frame = get_dword(pbuf);
    pbuf += 4;

    psqbase->end_frame = get_dword(pbuf);
    pbuf += 4;

    psqbase->max_msg = get_dword(pbuf);
    pbuf += 4;

    psqbase->keep_days = get_word(pbuf);
    pbuf += 2;

    psqbase->sz_sqhdr = get_word(pbuf);
    pbuf += 2;

    memmove(psqbase->rsvd2, pbuf, 124);
    pbuf += 124;

    assert(pbuf-buf == SQBASE_SIZE);

    return 1;
}

int write_sqbase(sword handle, struct _sqbase *psqbase)
{
    byte buf[SQBASE_SIZE], *pbuf = buf;

    put_word(pbuf, psqbase->len);
    pbuf += 2;

    put_word(pbuf, psqbase->rsvd1);
    pbuf += 2;

    put_dword(pbuf, psqbase->num_msg);
    pbuf += 4;

    put_dword(pbuf, psqbase->high_msg);
    pbuf += 4;

    put_dword(pbuf, psqbase->skip_msg);
    pbuf += 4;

    put_dword(pbuf, psqbase->high_water);
    pbuf += 4;

    put_dword(pbuf, psqbase->uid);
    pbuf += 4;

    memmove(pbuf, psqbase->base, 80);
    pbuf += 80;

    put_dword(pbuf, psqbase->begin_frame);
    pbuf += 4;

    put_dword(pbuf, psqbase->last_frame);
    pbuf += 4;

    put_dword(pbuf, psqbase->free_frame);
    pbuf += 4;

    put_dword(pbuf, psqbase->last_free_frame);
    pbuf += 4;

    put_dword(pbuf, psqbase->end_frame);
    pbuf += 4;

    put_dword(pbuf, psqbase->max_msg);
    pbuf += 4;

    put_word(pbuf, psqbase->keep_days);
    pbuf += 2;

    put_word(pbuf, psqbase->sz_sqhdr);
    pbuf += 2;

    memmove(pbuf, psqbase->rsvd2, 124);
    pbuf += 124;

    assert(pbuf - buf == SQBASE_SIZE);

    return (farwrite(handle, (byte far *)buf, SQBASE_SIZE) == SQBASE_SIZE);
}

int read_omsg(sword handle, struct _omsg *pomsg)
{
    byte buf[OMSG_SIZE], *pbuf = buf;
    word rawdate, rawtime;

    if (farread(handle, (byte far *)buf, OMSG_SIZE) != OMSG_SIZE)
    {
        return 0;
    }

    memmove(pomsg->from, pbuf, 36);
    pbuf += 36;

    memmove(pomsg->to, pbuf, 36);
    pbuf += 36;

    memmove(pomsg->subj, pbuf, 72);
    pbuf += 72;

    memmove(pomsg->date, pbuf, 20);
    pbuf += 20;

    pomsg->times = get_word(pbuf);
    pbuf += 2;

    pomsg->dest = get_word(pbuf);
    pbuf += 2;

    pomsg->orig = get_word(pbuf);
    pbuf += 2;

    pomsg->cost = get_word(pbuf);
    pbuf += 2;

    pomsg->orig_net = get_word(pbuf);
    pbuf += 2;

    pomsg->dest_net = get_word(pbuf);
    pbuf += 2;

                                /* 4 bytes "date_written" */
    rawdate = get_word(pbuf); pbuf += 2;
    rawtime = get_word(pbuf); pbuf += 2;
    pomsg->date_written.date.da = rawdate & 31;
    pomsg->date_written.date.mo = (rawdate >> 5) & 15;
    pomsg->date_written.date.yr = (rawdate >> 9) & 127;
    pomsg->date_written.time.ss = rawtime & 31;
    pomsg->date_written.time.mm = (rawtime >> 5) & 63;
    pomsg->date_written.time.hh = (rawtime >> 11) & 31;

                                /* 4 bytes "date_arrived" */
    rawdate = get_word(pbuf); pbuf += 2;
    rawtime = get_word(pbuf); pbuf += 2;
    pomsg->date_arrived.date.da = rawdate & 31;
    pomsg->date_arrived.date.mo = (rawdate >> 5) & 15;
    pomsg->date_arrived.date.yr = (rawdate >> 9) & 127;
    pomsg->date_arrived.time.ss = rawtime & 31;
    pomsg->date_arrived.time.mm = (rawtime >> 5) & 63;
    pomsg->date_arrived.time.hh = (rawtime >> 11) & 31;

    pomsg->reply = get_word(pbuf);
    pbuf += 2;

    pomsg->attr = get_word(pbuf);
    pbuf += 2;

    pomsg->up = get_word(pbuf);
    pbuf += 2;

    assert(pbuf - buf == OMSG_SIZE);

    return 1;
}

int write_omsg(sword handle, struct _omsg *pomsg)
{
    byte buf[OMSG_SIZE], *pbuf = buf;
    word rawdate, rawtime;

    memmove(pbuf, pomsg->from, 36);
    pbuf += 36;

    memmove(pbuf, pomsg->to, 36);
    pbuf += 36;

    memmove(pbuf, pomsg->subj, 72);
    pbuf += 72;

    memmove(pbuf, pomsg->date, 20);
    pbuf += 20;

    put_word(pbuf, pomsg->times);
    pbuf += 2;

    put_word(pbuf, pomsg->dest);
    pbuf += 2;

    put_word(pbuf, pomsg->orig);
    pbuf += 2;

    put_word(pbuf, pomsg->cost);
    pbuf += 2;

    put_word(pbuf, pomsg->orig_net);
    pbuf += 2;

    put_word(pbuf, pomsg->dest_net);
    pbuf += 2;

                                /* 4 bytes "date_written" */
    rawdate = rawtime = 0;

    rawdate |= (((word)pomsg->date_written.date.da) & 31);
    rawdate |= (((word)pomsg->date_written.date.mo) & 15) << 5;
    rawdate |= (((word)pomsg->date_written.date.yr) & 127) << 9;

    rawtime |= (((word)pomsg->date_written.time.ss) & 31);
    rawtime |= (((word)pomsg->date_written.time.mm) & 63) << 5;
    rawtime |= (((word)pomsg->date_written.time.hh) & 31) << 11;

    put_word(pbuf, rawdate); pbuf += 2;
    put_word(pbuf, rawtime); pbuf += 2;


                                /* 4 bytes "date_arrvied" */
    rawdate = rawtime = 0;

    rawdate |= (((word)pomsg->date_arrived.date.da) & 31);
    rawdate |= (((word)pomsg->date_arrived.date.mo) & 15) << 5;
    rawdate |= (((word)pomsg->date_arrived.date.yr) & 127) << 9;

    rawtime |= (((word)pomsg->date_arrived.time.ss) & 31);
    rawtime |= (((word)pomsg->date_arrived.time.mm) & 63) << 5;
    rawtime |= (((word)pomsg->date_arrived.time.hh) & 31) << 11;

    put_word(pbuf, rawdate); pbuf += 2;
    put_word(pbuf, rawtime); pbuf += 2;

    put_word(pbuf, pomsg->reply);
    pbuf += 2;

    put_word(pbuf, pomsg->attr);
    pbuf += 2;

    put_word(pbuf, pomsg->up);
    pbuf += 2;

    assert(pbuf - buf == OMSG_SIZE);

    return (farwrite(handle, (byte far *)buf, OMSG_SIZE) == OMSG_SIZE);
}

