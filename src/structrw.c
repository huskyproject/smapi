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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define MSGAPI_HANDLERS

#include <huskylib/compiler.h>

#ifdef HAS_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAS_IO_H
#  include <io.h>
#endif

#include <huskylib/huskylib.h>
/* Swith for build DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "msgapi.h"
#include "api_sq.h"
#include "old_msg.h"
#include "api_jam.h"

#define MAXHDRINCORE (1024l * 1024 * 10) /* Maximum jam hdr size for incore, 10M */

#ifdef NEED_trivial_farread
  #ifdef HAS_dos_read
/* "Text mode" not implemented !!! */
int trivial_farread(int handle, void far * buffer, unsigned len)
{
    unsigned r_len = 0;

    if(dos_read(handle, buffer, len, &r_len))
    {
        return 0;
    }

    return r_len;
}

  #else
    #error "Can't implement trivial_farread() without dos_read()"
  #endif
#endif

#ifdef NEED_trivial_farwrite
  #ifdef HAS_dos_write
/* "Text mode" not implemented !!! */
int trivial_farwrite(int handle, void far * buffer, unsigned len)
{
    unsigned r_len = 0;

    if(dos_write(handle, buffer, len, &r_len))
    {
        return 0;
    }

    return r_len;
}

  #else
    #error "Can't implement trivial_farwrite() without dos_write()"
  #endif
#endif


int read_xmsg(int handle, XMSG * pxmsg)
{
    byte buf[XMSG_SIZE], * pbuf = buf;
    word rawdate, rawtime;
    int i;

    if(farread(handle, (byte far *)buf, XMSG_SIZE) != XMSG_SIZE)
    {
        return 0;
    }

    /* 04 bytes "attr" */
    pxmsg->attr = get_dword(pbuf);
    pbuf       += 4;
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
    pxmsg->orig.zone  = get_word(pbuf);
    pbuf             += 2;
    pxmsg->orig.net   = get_word(pbuf);
    pbuf             += 2;
    pxmsg->orig.node  = get_word(pbuf);
    pbuf             += 2;
    pxmsg->orig.point = get_word(pbuf);
    pbuf             += 2;
    /* 8 bytes "dest"  */
    pxmsg->dest.zone  = get_word(pbuf);
    pbuf             += 2;
    pxmsg->dest.net   = get_word(pbuf);
    pbuf             += 2;
    pxmsg->dest.node  = get_word(pbuf);
    pbuf             += 2;
    pxmsg->dest.point = get_word(pbuf);
    pbuf             += 2;
    /* 4 bytes "date_written" */
    rawdate = get_word(pbuf);
    pbuf   += 2;
    rawtime = get_word(pbuf);
    pbuf   += 2;
    pxmsg->date_written.date.da = rawdate & 31;
    pxmsg->date_written.date.mo = (rawdate >> 5) & 15;
    pxmsg->date_written.date.yr = (rawdate >> 9) & 127;
    pxmsg->date_written.time.ss = rawtime & 31;
    pxmsg->date_written.time.mm = (rawtime >> 5) & 63;
    pxmsg->date_written.time.hh = (rawtime >> 11) & 31;
    /* 4 bytes "date_arrived" */
    rawdate = get_word(pbuf);
    pbuf   += 2;
    rawtime = get_word(pbuf);
    pbuf   += 2;
    pxmsg->date_arrived.date.da = rawdate & 31;
    pxmsg->date_arrived.date.mo = (rawdate >> 5) & 15;
    pxmsg->date_arrived.date.yr = (rawdate >> 9) & 127;
    pxmsg->date_arrived.time.ss = rawtime & 31;
    pxmsg->date_arrived.time.mm = (rawtime >> 5) & 63;
    pxmsg->date_arrived.time.hh = (rawtime >> 11) & 31;
    /* 2 byte "utc_ofs" */
    pxmsg->utc_ofs = get_word(pbuf);
    pbuf          += 2;
    /* 4 bytes "replyto" */
    pxmsg->replyto = get_dword(pbuf);
    pbuf          += 4;

    /* 10 times 4 bytes "replies" */
    for(i = 0; i < MAX_REPLY; i++)
    {
        pxmsg->replies[i] = get_dword(pbuf);
        pbuf += 4;
    }
    /* 4 bytes "umsgid" */
    pxmsg->umsgid = get_dword(pbuf);
    pbuf         += 4;
    /* 20 times FTSC date stamp */
    memmove(pxmsg->__ftsc_date, pbuf, 20);
    pbuf += 20;
    assert(pbuf - buf == XMSG_SIZE);
    return 1;
} /* read_xmsg */

int write_xmsg(int handle, XMSG * pxmsg)
{
    byte buf[XMSG_SIZE], * pbuf = buf;
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
    put_word(pbuf, pxmsg->orig.zone);
    pbuf += 2;
    put_word(pbuf, pxmsg->orig.net);
    pbuf += 2;
    put_word(pbuf, pxmsg->orig.node);
    pbuf += 2;
    put_word(pbuf, pxmsg->orig.point);
    pbuf += 2;
    /* 8 bytes "dest"  */
    put_word(pbuf, pxmsg->dest.zone);
    pbuf += 2;
    put_word(pbuf, pxmsg->dest.net);
    pbuf += 2;
    put_word(pbuf, pxmsg->dest.node);
    pbuf += 2;
    put_word(pbuf, pxmsg->dest.point);
    pbuf += 2;
    /* 4 bytes "date_written" */
    rawdate  = rawtime = 0;
    rawdate |= (((word)pxmsg->date_written.date.da) & 31);
    rawdate |= (((word)pxmsg->date_written.date.mo) & 15) << 5;
    rawdate |= (((word)pxmsg->date_written.date.yr) & 127) << 9;
    rawtime |= (((word)pxmsg->date_written.time.ss) & 31);
    rawtime |= (((word)pxmsg->date_written.time.mm) & 63) << 5;
    rawtime |= (((word)pxmsg->date_written.time.hh) & 31) << 11;
    put_word(pbuf, rawdate);
    pbuf += 2;
    put_word(pbuf, rawtime);
    pbuf += 2;
    /* 4 bytes "date_arrvied" */
    rawdate  = rawtime = 0;
    rawdate |= (((word)pxmsg->date_arrived.date.da) & 31);
    rawdate |= (((word)pxmsg->date_arrived.date.mo) & 15) << 5;
    rawdate |= (((word)pxmsg->date_arrived.date.yr) & 127) << 9;
    rawtime |= (((word)pxmsg->date_arrived.time.ss) & 31);
    rawtime |= (((word)pxmsg->date_arrived.time.mm) & 63) << 5;
    rawtime |= (((word)pxmsg->date_arrived.time.hh) & 31) << 11;
    put_word(pbuf, rawdate);
    pbuf += 2;
    put_word(pbuf, rawtime);
    pbuf += 2;
    /* 2 byte "utc_ofs" */
    put_word(pbuf, pxmsg->utc_ofs);
    pbuf += 2;
    /* 4 bytes "replyto" */
    put_dword(pbuf, pxmsg->replyto);
    pbuf += 4;

    /* 10 times 4 bytes "replies" */
    for(i = 0; i < MAX_REPLY; i++)
    {
        put_dword(pbuf, pxmsg->replies[i]);
        pbuf += 4;
    }
    /* 4 bytes "umsgid" */
    put_dword(pbuf, pxmsg->umsgid);
    pbuf += 4;
    /* 20 times FTSC date stamp */
    memmove(pbuf, pxmsg->__ftsc_date, 20);
    pbuf += 20;
    assert(pbuf - buf == XMSG_SIZE);
    return farwrite(handle, (byte far *)buf, XMSG_SIZE) == XMSG_SIZE;
} /* write_xmsg */

int read_sqhdr(int handle, SQHDR * psqhdr)
{
    byte buf[SQHDR_SIZE], * pbuf = buf;

    if(farread(handle, (byte far *)buf, SQHDR_SIZE) != SQHDR_SIZE)
    {
        return 0;
    }

    /* 4 bytes "id" */
    psqhdr->id = get_dword(pbuf);
    pbuf      += 4;
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
    pbuf        += 4;
    /* 2 bytes "frame_type" */
    psqhdr->frame_type = get_word(pbuf);
    pbuf += 2;
    /* 4 bytes "rsvd" */
    psqhdr->rsvd = get_word(pbuf);
    pbuf        += 2;
    assert(pbuf - buf == SQHDR_SIZE);
    return 1;
} /* read_sqhdr */

int write_sqhdr(int handle, SQHDR * psqhdr)
{
    byte buf[SQHDR_SIZE], * pbuf = buf;

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
    return farwrite(handle, (byte far *)buf, SQHDR_SIZE) == SQHDR_SIZE;
} /* write_sqhdr */

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
int read_sqidx(int handle, SQIDX * psqidx, dword n)
{
    byte buf[SQIDX_SIZE], * pbuf = NULL;
    byte * accel_buffer = NULL;
    dword i, maxbuf = 0, rd;

    if(n > 1)
    {
        maxbuf = n;

        if((dword)SQIDX_SIZE * (dword)n >= 32768L)
        {
            maxbuf = (dword)32768L / SQIDX_SIZE;
        }

        accel_buffer = malloc(SQIDX_SIZE * maxbuf);
    }

    for(i = 0; i < n; i++)
    {
        if(accel_buffer == NULL)
        {
            if(farread(handle, buf, SQIDX_SIZE) != SQIDX_SIZE)
            {
                return 0;
            }

            pbuf = buf;
        }
        else
        {
            if(!(i % maxbuf))
            {
                rd = (i + maxbuf > n) ? (n - i) : maxbuf;

                if(farread(handle, accel_buffer, rd * SQIDX_SIZE) != (int)(rd * SQIDX_SIZE))
                {
                    nfree(accel_buffer);
                    return 0;
                }

                pbuf = accel_buffer;
            }
        }

        /* Begin reading in a single structure */
        /* 4 bytes "ofs" */
        psqidx[i].ofs = get_dword(pbuf);
        pbuf         += 4;
        /* 4 bytes "umsgid" */
        psqidx[i].umsgid = get_dword(pbuf);
        pbuf            += 4;
        /* 4 bytes "hash" */
        psqidx[i].hash = get_dword(pbuf);
        pbuf          += 4;
        /* Stop reading in a single structure */
    }

    nfree(accel_buffer);
    return 1;
} /* read_sqidx */

int write_sqidx(int handle, SQIDX * psqidx, dword n)
{
    byte buf[SQIDX_SIZE], * pbuf = NULL;
    byte * accel_buffer = NULL;
    dword i, maxbuf = 0;
    int wr;

    if(n > 1)
    {
        maxbuf = n;

        if((dword)SQIDX_SIZE * (dword)n >= 32768L)
        {
            maxbuf = (dword)32768L / SQIDX_SIZE;
        }

        accel_buffer = malloc(SQIDX_SIZE * maxbuf);
        pbuf         = accel_buffer;
    }

    for(i = 0; i < n; i++)
    {
        if(accel_buffer == NULL)
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

        if(accel_buffer == NULL)
        {
            if(farwrite(handle, buf, SQIDX_SIZE) != SQIDX_SIZE)
            {
                return 0;
            }
        }
        else
        {
            if(i == n - 1 || (!((i + 1) % maxbuf)))
            {
                wr = (!((i + 1) % maxbuf)) ? maxbuf : (n % maxbuf);

                if(farwrite(handle, accel_buffer, wr * SQIDX_SIZE) != (wr * SQIDX_SIZE))
                {
                    nfree(accel_buffer);
                    return 0;
                }

                pbuf = accel_buffer;
            }
        }
    }

    nfree(accel_buffer);
    return 1;
} /* write_sqidx */

int read_sqbase(int handle, struct _sqbase * psqbase)
{
    byte buf[SQBASE_SIZE], * pbuf = buf;

    if(farread(handle, (byte far *)buf, SQBASE_SIZE) != SQBASE_SIZE)
    {
        return 0;
    }

    psqbase->len        = get_word(pbuf);
    pbuf               += 2;
    psqbase->rsvd1      = get_word(pbuf);
    pbuf               += 2;
    psqbase->num_msg    = get_dword(pbuf);
    pbuf               += 4;
    psqbase->high_msg   = get_dword(pbuf);
    pbuf               += 4;
    psqbase->skip_msg   = get_dword(pbuf);
    pbuf               += 4;
    psqbase->high_water = get_dword(pbuf);
    pbuf               += 4;
    psqbase->uid        = get_dword(pbuf);
    pbuf               += 4;
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
    pbuf              += 4;
    psqbase->max_msg   = get_dword(pbuf);
    pbuf              += 4;
    psqbase->keep_days = get_word(pbuf);
    pbuf              += 2;
    psqbase->sz_sqhdr  = get_word(pbuf);
    pbuf              += 2;
    memmove(psqbase->rsvd2, pbuf, 124);
    pbuf += 124;
    assert(pbuf - buf == SQBASE_SIZE);
    return 1;
} /* read_sqbase */

int write_sqbase(int handle, struct _sqbase * psqbase)
{
    byte buf[SQBASE_SIZE], * pbuf = buf;

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
    return farwrite(handle, (byte far *)buf, SQBASE_SIZE) == SQBASE_SIZE;
} /* write_sqbase */

int read_omsg(int handle, struct _omsg * pomsg)
{
    byte buf[OMSG_SIZE], * pbuf = buf;
    word rawdate, rawtime;

    if(farread(handle, (byte far *)buf, OMSG_SIZE) != OMSG_SIZE)
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
    pbuf           += 20;
    pomsg->times    = get_word(pbuf);
    pbuf           += 2;
    pomsg->dest     = get_word(pbuf);
    pbuf           += 2;
    pomsg->orig     = get_word(pbuf);
    pbuf           += 2;
    pomsg->cost     = get_word(pbuf);
    pbuf           += 2;
    pomsg->orig_net = get_word(pbuf);
    pbuf           += 2;
    pomsg->dest_net = get_word(pbuf);
    pbuf           += 2;

    /* 4 bytes "date_written" */
    rawdate = get_word(pbuf);
    pbuf   += 2;
    rawtime = get_word(pbuf);
    pbuf   += 2;
    pomsg->date_written.date.da = rawdate & 31;
    pomsg->date_written.date.mo = (rawdate >> 5) & 15;
    pomsg->date_written.date.yr = (rawdate >> 9) & 127;
    pomsg->date_written.time.ss = rawtime & 31;
    pomsg->date_written.time.mm = (rawtime >> 5) & 63;
    pomsg->date_written.time.hh = (rawtime >> 11) & 31;
    /* 4 bytes "date_arrived" */
    rawdate = get_word(pbuf);
    pbuf   += 2;
    rawtime = get_word(pbuf);
    pbuf   += 2;
    pomsg->date_arrived.date.da = rawdate & 31;
    pomsg->date_arrived.date.mo = (rawdate >> 5) & 15;
    pomsg->date_arrived.date.yr = (rawdate >> 9) & 127;
    pomsg->date_arrived.time.ss = rawtime & 31;
    pomsg->date_arrived.time.mm = (rawtime >> 5) & 63;
    pomsg->date_arrived.time.hh = (rawtime >> 11) & 31;
    pomsg->reply = get_word(pbuf);
    pbuf        += 2;
    pomsg->attr  = get_word(pbuf);
    pbuf        += 2;
    pomsg->up    = get_word(pbuf);
    pbuf        += 2;
    assert(pbuf - buf == OMSG_SIZE);
    return 1;
} /* read_omsg */

int write_omsg(int handle, struct _omsg * pomsg)
{
    byte buf[OMSG_SIZE], * pbuf = buf;
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
    rawdate  = rawtime = 0;
    rawdate |= (((word)pomsg->date_written.date.da) & 31);
    rawdate |= (((word)pomsg->date_written.date.mo) & 15) << 5;
    rawdate |= (((word)pomsg->date_written.date.yr) & 127) << 9;
    rawtime |= (((word)pomsg->date_written.time.ss) & 31);
    rawtime |= (((word)pomsg->date_written.time.mm) & 63) << 5;
    rawtime |= (((word)pomsg->date_written.time.hh) & 31) << 11;
    put_word(pbuf, rawdate);
    pbuf += 2;
    put_word(pbuf, rawtime);
    pbuf += 2;
    /* 4 bytes "date_arrvied" */
    rawdate  = rawtime = 0;
    rawdate |= (((word)pomsg->date_arrived.date.da) & 31);
    rawdate |= (((word)pomsg->date_arrived.date.mo) & 15) << 5;
    rawdate |= (((word)pomsg->date_arrived.date.yr) & 127) << 9;
    rawtime |= (((word)pomsg->date_arrived.time.ss) & 31);
    rawtime |= (((word)pomsg->date_arrived.time.mm) & 63) << 5;
    rawtime |= (((word)pomsg->date_arrived.time.hh) & 31) << 11;
    put_word(pbuf, rawdate);
    pbuf += 2;
    put_word(pbuf, rawtime);
    pbuf += 2;
    put_word(pbuf, pomsg->reply);
    pbuf += 2;
    put_word(pbuf, pomsg->attr);
    pbuf += 2;
    put_word(pbuf, pomsg->up);
    pbuf += 2;
    assert(pbuf - buf == OMSG_SIZE);
    return farwrite(handle, (byte far *)buf, OMSG_SIZE) == OMSG_SIZE;
} /* write_omsg */

int read_hdrinfo(int handle, JAMHDRINFO * HdrInfo)
{
    byte buf[HDRINFO_SIZE], * pbuf = buf;

    if(farread(handle, (byte far *)buf, HDRINFO_SIZE) != HDRINFO_SIZE)
    {
        return 0;
    } /* endif */

    /* 04 bytes Signature */
    memmove(HdrInfo->Signature, pbuf, (size_t)4);
    pbuf += 4;
    /* 04 bytes DateCreated */
    HdrInfo->DateCreated = get_dword(pbuf);
    pbuf += 4;
    /* 04 bytes ModCounter */
    HdrInfo->ModCounter = get_dword(pbuf);
    pbuf += 4;
    /* 04 bytes ActiveMsgs */
    HdrInfo->ActiveMsgs = get_dword(pbuf);
    pbuf += 4;
    /* 04 bytes PasswordCRC */
    HdrInfo->PasswordCRC = get_dword(pbuf);
    pbuf += 4;
    /* 04 bytes BaseMsgNum */
    HdrInfo->BaseMsgNum = get_dword(pbuf);
    pbuf += 4;
    /* 04 bytes highwater */
    HdrInfo->highwater = get_dword(pbuf);
    pbuf += 4;
    /* 996 bytes RSRVD */
    memmove(HdrInfo->RSRVD, pbuf, (size_t)996);
    pbuf += 996;
    assert(pbuf - buf == HDRINFO_SIZE);
    return 1;
} /* read_hdrinfo */

int read_idx(int handle, JAMIDXREC * Idx)
{
    byte buf[IDX_SIZE], * pbuf = buf;

    if(farread(handle, (byte far *)buf, IDX_SIZE) != IDX_SIZE)
    {
        return 0;
    } /* endif */

    /* 04 bytes UserCRC */
    Idx->UserCRC = get_dword(pbuf);
    pbuf        += 4;
    /* 04 bytes HdrOffset */
    Idx->HdrOffset = get_dword(pbuf);
    pbuf          += 4;
    assert(pbuf - buf == IDX_SIZE);
    return 1;
}

static void decode_hdr(byte * pbuf, JAMHDR * Hdr)
{
    /* 04 bytes Signature */
    memmove(Hdr->Signature, pbuf, (size_t)4);
    pbuf += 4;
    /* 02 bytes Revision */
    Hdr->Revision = get_word(pbuf);
    pbuf         += 2;
    /* 02 bytes ReservedWord */
    Hdr->ReservedWord = get_word(pbuf);
    pbuf += 2;
    /* 04 bytes SubfieldLen */
    Hdr->SubfieldLen = get_dword(pbuf);
    pbuf            += 4;
    /* 04 bytes TimesRead */
    Hdr->TimesRead = get_dword(pbuf);
    pbuf          += 4;
    /* 04 bytes MsgIdCRC */
    Hdr->MsgIdCRC = get_dword(pbuf);
    pbuf         += 4;
    /* 04 bytes ReplyCRC */
    Hdr->ReplyCRC = get_dword(pbuf);
    pbuf         += 4;
    /* 04 bytes ReplyTo */
    Hdr->ReplyTo = get_dword(pbuf);
    pbuf        += 4;
    /* 04 bytes Reply1st */
    Hdr->Reply1st = get_dword(pbuf);
    pbuf         += 4;
    /* 04 bytes ReplyNext */
    Hdr->ReplyNext = get_dword(pbuf);
    pbuf          += 4;
    /* 04 bytes DateWritten */
    Hdr->DateWritten = get_dword(pbuf);
    pbuf            += 4;
    /* 04 bytes DateReceived */
    Hdr->DateReceived = get_dword(pbuf);
    pbuf += 4;
    /* 04 bytes DateProcessed */
    Hdr->DateProcessed = get_dword(pbuf);
    pbuf += 4;
    /* 04 bytes MsgNum */
    Hdr->MsgNum = get_dword(pbuf);
    pbuf       += 4;
    /* 04 bytes Attribute */
    Hdr->Attribute = get_dword(pbuf);
    pbuf          += 4;
    /* 04 bytes Attribute2 */
    Hdr->Attribute2 = get_dword(pbuf);
    pbuf           += 4;
    /* 04 bytes TxtOffset */
    Hdr->TxtOffset = get_dword(pbuf);
    pbuf          += 4;
    /* 04 bytes TxtLen */
    Hdr->TxtLen = get_dword(pbuf);
    pbuf       += 4;
    /* 04 bytes PasswordCRC */
    Hdr->PasswordCRC = get_dword(pbuf);
    pbuf            += 4;
    /* 04 bytes Cost */
    Hdr->Cost = get_dword(pbuf);
} /* decode_hdr */

int read_hdr(int handle, JAMHDR * Hdr)
{
    byte buf[HDR_SIZE];

    if(farread(handle, (byte far *)buf, HDR_SIZE) != HDR_SIZE)
    {
        return 0;
    } /* endif */

    decode_hdr(buf, Hdr);
    return 1;
}

int copy_subfield(JAMSUBFIELD2LISTptr * to, JAMSUBFIELD2LISTptr from)
{
    dword i;

    *to = palloc(from->arraySize);

    if(*to == NULL)
    {
        return 1;
    }

    memcpy(*to, from, from->arraySize);

    for(i = 0; i < from->subfieldCount; i++)
    {
        to[0]->subfield[i].Buffer += ((char *)*to - (char *)from);
    }
    return 0;
}

/* Define DEBUG to catch more weirdness in databases */
static int decode_subfield(byte * buf, JAMSUBFIELD2LISTptr * subfield, dword * SubfieldLen)
{
    JAMSUBFIELD2ptr subfieldNext;
    dword datlen;
    size_t count, len;
    byte * pbuf, * limit;

    pbuf  = buf;
    limit = buf + *SubfieldLen;
    assert(limit >= buf);
    count = 0;

    while(pbuf + JAM_SF_HEADER_SIZE <= limit)
    {
        dword size;
#ifdef DEBUG
        word loID, hiID;
        loID = get_word(pbuf);
        hiID = get_word(pbuf + 2);

        if(!(loID <= JAMSFLD_ENCLINDFILE || loID == JAMSFLD_EMBINDAT ||
             loID >= JAMSFLD_FTSKLUDGE && loID <= JAMSFLD_TZUTCINFO)) /* This subfield type is
                                                                         not supported and is
                                                                         most
                                                                         probably sign of error
                                                                            in messagebase */
        {
            w_log(LL_ERROR, "SMAPI ERROR: weird subfield type! (%X)",
                  (unsigned int)loID);
            /* Keep going, these fields won't hurt unless they have improper
               size too */
        }

#endif
        size = get_dword(pbuf + 4);
#ifdef DEBUG

        if(size == 0 && loID != JAMSFLD_SUBJECT)
        {
            /* While possible, it isn't normal value */
            w_log(LL_ERROR, "SMAPI ERROR: subfield of 0 size! (%X)",
                  (unsigned int)loID);
        }

#endif

        if(pbuf + JAM_SF_HEADER_SIZE + size > limit)
        /* it means that subfield claims to be longer
           than header says. can't be. */
        {
            /* just break, ideally there shall be a setting for lax
               treatment of messagebase */
            w_log(LL_ERROR, "SMAPI ERROR: wrongly sized subfield occured!");
            break;
        }

        /* reality check: single subfield longer than 64k is not realistic */
        if(size >= 0xFFFF) 
        {
            w_log(LL_ERROR, 
                  "SMAPI ERROR: subfield is suspiciously large! (%lu bytes)",
                  (unsigned long)size);
            break;
        }

        ++count;
        pbuf += JAM_SF_HEADER_SIZE + size;
    }
    len = sizeof(JAMSUBFIELD2LIST) + count *
          (sizeof(JAMSUBFIELD2) - JAM_SF_HEADER_SIZE + 1) + *SubfieldLen;
    *subfield = palloc(len);
    if(*subfield == NULL)
    {
        return 0;
    }
    subfield[0]->arraySize     = (dword)len;
    subfield[0]->subfieldCount = 0;
    /* reserve memory for (real count + 1)*JAMSUBFIELD2 */
    subfield[0]->subfield[0].Buffer = (byte *)&(subfield[0]->subfield[count + 1]);
    subfieldNext = subfield[0]->subfield;
    pbuf         = buf;

    while((size_t)subfield[0]->subfieldCount < count && pbuf + JAM_SF_HEADER_SIZE <= limit)
    {
        /* 02 bytes LoID */
        subfieldNext->LoID = get_word(pbuf);
        pbuf += 2;
        /* 02 bytes HiID */
        subfieldNext->HiID = get_word(pbuf);
        pbuf += 2;
        /* 04 bytes DatLen */
        subfieldNext->DatLen    = 0;
        subfieldNext->Buffer[0] = '\0';
        datlen = get_dword(pbuf);
        pbuf  += 4;
        subfield[0]->subfieldCount++;

        if((size_t)*SubfieldLen - (pbuf - buf) < (size_t)datlen)
        {
            break;
        }

        /* DatLen bytes Buffer */
        subfieldNext->DatLen = datlen;
        memmove(subfieldNext->Buffer, pbuf, datlen);
        /* Set up next element */
        assert((byte *)(subfieldNext + 1) < subfield[0]->subfield[0].Buffer);
        subfieldNext[1].Buffer = subfieldNext->Buffer + subfieldNext->DatLen + 1;
        subfieldNext++;
        assert(subfieldNext->Buffer <= (byte *)*subfield + subfield[0]->arraySize);
        pbuf += datlen;
    } /* endwhile */
    *SubfieldLen = (dword)(pbuf - buf);
    return 1;
} /* decode_subfield */

int read_subfield(int handle, JAMSUBFIELD2LISTptr * subfield, dword * SubfieldLen)
{
    byte * buf;
    int enough_memory;

    buf = (byte *)palloc(*SubfieldLen);
    if(buf == NULL)
    {
        return 0;
    }

    if((dword)farread(handle, (byte far *)buf, *SubfieldLen) != *SubfieldLen)
    {
        pfree(buf);
        return 0;
    } /* endif */

    enough_memory = decode_subfield(buf, subfield, SubfieldLen);
    pfree(buf);
    if(!enough_memory)
    {
        return 0;
    }
    return 1;
}

int read_allidx(JAMBASEptr jmb)
{
    byte * buf, * pbuf, * hdrbuf = NULL;
    JAMACTMSGptr newptr;
    JAMHDR hbuf;
    int len;
    dword i, allocated, hlen;
    dword offset;

    lseek(jmb->IdxHandle, 0, SEEK_END);
    len = (int)tell(jmb->IdxHandle);
    lseek(jmb->IdxHandle, 0, SEEK_SET);
    buf  = (byte *)palloc(len);
    if(!buf)
    {
        return 0;
    }
    pbuf = buf;

    if(farread(jmb->IdxHandle, (byte far *)buf, len) != len)
    {
        pfree(buf);
        return 0;
    }

    lseek(jmb->HdrHandle, 0, SEEK_END);
    hlen = (dword)tell(jmb->HdrHandle);
    lseek(jmb->HdrHandle, 0, SEEK_SET);

    if(hlen < MAXHDRINCORE)
    {
        /* read all headers in core */
        hdrbuf = (byte *)palloc(hlen);
        if(hdrbuf == NULL)
        {
            pfree(buf);
            return 0;
        }

        if((dword)farread(jmb->HdrHandle, (byte far *)hdrbuf, hlen) != hlen)
        {
            pfree(hdrbuf);
            pfree(buf);
            return 0;
        }

        jmb->actmsg_read = 1;
    }
    else
    {
        jmb->actmsg_read = 2;
    }

    allocated = jmb->HdrInfo.ActiveMsgs;

    if(allocated > (dword)len / IDX_SIZE)
    {
        allocated = (dword)len / IDX_SIZE;
    }

    if(allocated)
    {
        jmb->actmsg = (JAMACTMSGptr)farmalloc(allocated * sizeof(JAMACTMSG));

        if(jmb->actmsg == NULL)
        {
            if(hdrbuf)
            {
                pfree(hdrbuf);
            }

            pfree(buf);
            return 0;
        }
    }

    for(i = 0; (pbuf - buf) < len; )
    {
        offset = get_dword(pbuf + 4);

        if(offset != 0xFFFFFFFFUL)
        {
            if(offset + HDR_SIZE <= hlen)
            {
                if(hdrbuf)
                {
                    decode_hdr(hdrbuf + offset, &hbuf);
                }
                else
                {
                    lseek(jmb->HdrHandle, offset, SEEK_SET);
                    read_hdr(jmb->HdrHandle, &hbuf);
                }

                if(!(hbuf.Attribute & JMSG_DELETED))
                {
                    if(i >= allocated)
                    {
                        newptr =
                            (JAMACTMSGptr)farrealloc(jmb->actmsg,
                                                     sizeof(JAMACTMSG) * (allocated += 16));

                        if(newptr == NULL)
                        {
                            pfree(jmb->actmsg);

                            if(hdrbuf)
                            {
                                pfree(hdrbuf);
                            }

                            pfree(buf);
                            return 0;
                        }

                        jmb->actmsg = newptr;
                    }

                    jmb->actmsg[i].IdxOffset = (dword)(pbuf - buf);
                    jmb->actmsg[i].TrueMsg   = offset;
                    jmb->actmsg[i].UserCRC   = get_dword(pbuf);
                    memcpy(&(jmb->actmsg[i].hdr), &hbuf, sizeof(hbuf));

                    if(hdrbuf && offset + HDR_SIZE + jmb->actmsg[i].hdr.SubfieldLen <= hlen)
                    {
                        if(!decode_subfield(hdrbuf + offset + HDR_SIZE,
                                            &(jmb->actmsg[i].subfield),
                                            &(jmb->actmsg[i].hdr.SubfieldLen)))
                        {
                            pfree(buf);
                            pfree(pbuf);
                            pfree(hdrbuf);
                            return 0;
                        }
                        i++;
                    }
                    else
                    {
                        jmb->actmsg[i++].subfield = NULL;
                    }
                } /* endif */
            } /* endif */
        } /* endif */

        pbuf += 8;
    } /* endfor */
    pfree(buf);
    pfree(hdrbuf);

    if(i != jmb->HdrInfo.ActiveMsgs)
    {
        /* warning: database corrupted! */
        jmb->HdrInfo.ActiveMsgs = i;
        jmb->modified           = 1;

        if(i == 0)
        {
            if(jmb->actmsg)
            {
                pfree(jmb->actmsg);
                jmb->actmsg = NULL;
            }
        }
        else if(i != allocated)
        {
            newptr = (JAMACTMSGptr)farrealloc(jmb->actmsg, sizeof(JAMACTMSG) * i);

            if(newptr)
            {
                jmb->actmsg = newptr;
            }
        }
    } /* endif */

    return 1;
} /* read_allidx */

int write_hdrinfo(int handle, JAMHDRINFO * HdrInfo)
{
    byte buf[HDRINFO_SIZE], * pbuf = buf;

    /* 04 bytes Signature */
    memmove(pbuf, HdrInfo->Signature, (size_t)4);
    pbuf += 4;
    /* 04 bytes DateCreated */
    put_dword(pbuf, HdrInfo->DateCreated);
    pbuf += 4;
    /* 04 bytes ModCounter */
    put_dword(pbuf, HdrInfo->ModCounter);
    pbuf += 4;
    /* 04 bytes ActiveMsgs */
    put_dword(pbuf, HdrInfo->ActiveMsgs);
    pbuf += 4;
    /* 04 bytes PasswordCRC */
    put_dword(pbuf, HdrInfo->PasswordCRC);
    pbuf += 4;
    /* 04 bytes BaseMsgNum */
    put_dword(pbuf, HdrInfo->BaseMsgNum);
    pbuf += 4;
    /* 04 bytes highwater */
    put_dword(pbuf, HdrInfo->highwater);
    pbuf += 4;
    /* 996 bytes RSRVD */
    memmove(pbuf, HdrInfo->RSRVD, (size_t)996);
    pbuf += 996;
    assert(pbuf - buf == HDRINFO_SIZE);
    return farwrite(handle, (byte far *)buf, HDRINFO_SIZE) == HDRINFO_SIZE;
} /* write_hdrinfo */

int write_idx(int handle, JAMIDXREC * Idx)
{
    byte buf[IDX_SIZE], * pbuf = buf;

    /* 04 bytes UserCRC */
    put_dword(pbuf, Idx->UserCRC);
    pbuf += 4;
    /* 04 bytes HdrOffset */
    put_dword(pbuf, Idx->HdrOffset);
    pbuf += 4;
    assert(pbuf - buf == IDX_SIZE);
    return farwrite(handle, (byte far *)buf, IDX_SIZE) == IDX_SIZE;
}

int write_hdr(int handle, JAMHDR * Hdr)
{
    byte buf[HDR_SIZE], * pbuf = buf;

    /* 04 bytes Signature */
    memmove(pbuf, Hdr->Signature, (size_t)4);
    pbuf += 4;
    /* 02 bytes Revision */
    put_word(pbuf, Hdr->Revision);
    pbuf += 2;
    /* 02 bytes ReservedWord */
    put_word(pbuf, Hdr->ReservedWord);
    pbuf += 2;
    /* 04 bytes SubfieldLen */
    put_dword(pbuf, Hdr->SubfieldLen);
    pbuf += 4;
    /* 04 bytes TimesRead */
    put_dword(pbuf, Hdr->TimesRead);
    pbuf += 4;
    /* 04 bytes MsgIdCRC */
    put_dword(pbuf, Hdr->MsgIdCRC);
    pbuf += 4;
    /* 04 bytes ReplyCRC */
    put_dword(pbuf, Hdr->ReplyCRC);
    pbuf += 4;
    /* 04 bytes ReplyTo */
    put_dword(pbuf, Hdr->ReplyTo);
    pbuf += 4;
    /* 04 bytes Reply1st */
    put_dword(pbuf, Hdr->Reply1st);
    pbuf += 4;
    /* 04 bytes ReplyNext */
    put_dword(pbuf, Hdr->ReplyNext);
    pbuf += 4;
    /* 04 bytes DateWritten */
    put_dword(pbuf, Hdr->DateWritten);
    pbuf += 4;
    /* 04 bytes DateReceived */
    put_dword(pbuf, Hdr->DateReceived);
    pbuf += 4;
    /* 04 bytes DateProcessed */
    put_dword(pbuf, Hdr->DateProcessed);
    pbuf += 4;
    /* 04 bytes MsgNum */
    put_dword(pbuf, Hdr->MsgNum);
    pbuf += 4;
    /* 04 bytes Attribute */
    put_dword(pbuf, Hdr->Attribute);
    pbuf += 4;
    /* 04 bytes Attribute2 */
    put_dword(pbuf, Hdr->Attribute2);
    pbuf += 4;
    /* 04 bytes TxtOffset */
    put_dword(pbuf, Hdr->TxtOffset);
    pbuf += 4;
    /* 04 bytes TxtLen */
    put_dword(pbuf, Hdr->TxtLen);
    pbuf += 4;
    /* 04 bytes PasswordCRC */
    put_dword(pbuf, Hdr->PasswordCRC);
    pbuf += 4;
    /* 04 bytes Cost */
    put_dword(pbuf, Hdr->Cost);
    pbuf += 4;
    assert(pbuf - buf == HDR_SIZE);
    return farwrite(handle, (byte far *)buf, HDR_SIZE) == HDR_SIZE;
} /* write_hdr */

int write_subfield(int handle, JAMSUBFIELD2LISTptr * subfield, dword SubfieldLen)
{
    unsigned char * buf, * pbuf;
    dword datlen;
    int rc;
    dword i;
    JAMSUBFIELD2ptr subfieldNext;

    buf          = (unsigned char *)palloc(SubfieldLen);
    if(!buf)
    {
        return 0;
    }
    pbuf         = buf;
    subfieldNext = &(subfield[0]->subfield[0]);

    for(i = 0; i < subfield[0]->subfieldCount; i++, subfieldNext++)
    {
        /* 02 bytes LoID */
        put_word(pbuf, subfieldNext->LoID);
        pbuf += 2;
        /* 02 bytes HiID */
        put_word(pbuf, subfieldNext->HiID);
        pbuf += 2;
        /* 04 bytes DatLen */
        put_dword(pbuf, subfieldNext->DatLen);
        datlen = subfieldNext->DatLen;
        pbuf  += 4;
        /* DatLen bytes Buffer */
        memmove(pbuf, subfieldNext->Buffer, datlen);
        pbuf += datlen;
    } /* endwhile */
    rc = ((dword)farwrite(handle, (byte far *)buf, SubfieldLen) == SubfieldLen);
    pfree(buf);
    return rc;
} /* write_subfield */
