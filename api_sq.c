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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "dr.h"

#include "semaphor.h"

#if !defined(UNIX) && !defined(SASC)
#include <io.h>
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(UNIX) && !defined(SASC)
#include <share.h>
#endif

#define MSGAPI_HANDLERS

#include "prog.h"
#include "alc.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"
#include "unused.h"

static SEMAPHORE _haopen_sem;

/* The root of a simple list of open Areas */

static HAREA _haopen = NULL;

/* OG: 8/12/00
   Changed returntype 'MSG *' to HAREA 
*/

void _SquishInit()
{
  create_semaphore(&_haopen_sem);
}

void _SquishDeInit()
{
  delete_semaphore(&_haopen_sem);
}

HAREA MSGAPI SquishOpenArea(byte * name, word mode, word type)
{
    struct _sqbase sqbase;
    HAREA sq;

    unused(_junksq);

    sq = palloc(sizeof(MSG));
    if (sq == NULL)
    {
        return NULL;
    }

    memset(sq, '\0', sizeof(MSG));

    sq->id = MSGAPI_ID;

    if (type & MSGTYPE_ECHO)
    {
        sq->isecho = TRUE;
    }

    sq->api = (struct _apifuncs *)palloc(sizeof(struct _apifuncs));
    if (sq->api == NULL)
    {
        pfree(sq);
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    sq->apidata = (void *)palloc(sizeof(struct _sqdata));
    if (sq->apidata == NULL)
    {
        pfree(sq->api);
        pfree(sq);
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    memset((char *)sq->apidata, '\0', sizeof(struct _sqdata));

    Sqd->uid = 1L;
    strcpy((char *) Sqd->base, (char *) name);
    sq->len = sizeof(MSG);

    if (!_OpenSquish(sq, &mode))
    {
        pfree(sq->api);
        pfree((char *) sq->apidata);
        pfree(sq);
        return NULL;
    }

    if (mode == MSGAREA_CREATE)
    {
        sqbase.len = SQBASE_SIZE;
        sqbase.num_msg = 0L;
        sqbase.high_msg = 0L;
        sqbase.high_water = 0L;

        sqbase.begin_frame = NULL_FRAME;
        sqbase.free_frame = NULL_FRAME;
        sqbase.last_free_frame = NULL_FRAME;
        sqbase.end_frame = sqbase.len;
        sqbase.last_frame = NULL_FRAME;
        sqbase.max_msg = 0L;
        sqbase.skip_msg = 0L;
        sqbase.keep_days = 0;

        sqbase.sz_sqhdr = SQHDR_SIZE;
        sqbase.uid = 1L;
    }
    else
    {
        if (!read_sqbase(Sqd->sfd, &sqbase))
        {
            /* The base must be locked or corrupted, so return an error */

            close(Sqd->sfd);
            close(Sqd->ifd);

            pfree(sq->api);
            pfree((char *)sq->apidata);
            pfree(sq);

            msgapierr = MERR_BADF;
            return NULL;
        }
    }

    Sqd->delta = sqbase;

    strcpy((char *) sqbase.base, (char *) name);
    SqbaseToSq(&sqbase, sq);

    Sqd->next_frame = Sqd->begin_frame;
    Sqd->prev_frame = NULL_FRAME;
    Sqd->cur_frame = NULL_FRAME;

    sq->locked = 0;
    sq->cur_msg = 0;
    Sqd->idxbuf = NULL;

    if (mode == MSGAREA_CREATE)
    {
        _SquishWriteSq(sq);
    }

    sq->type = MSGTYPE_SQUISH;
    *sq->api = sq_funcs;
    sq->sz_xmsg = XMSG_SIZE;


    /* 8/12/00 
       OG: Area is opened, so it can be linked to open-area list 
    */

    lock_semaphore(&_haopen_sem);
 
    Sqd->hanext = _haopen;
    _haopen = sq;

    unlock_semaphore(&_haopen_sem);
    
    return sq;
}

int SquishDeleteBase(char *name)
{
    byte temp[PATHLEN];
    int rc = 1;
    
    sprintf((char *) temp, (char *) ss_sqd, name);
    if (unlink(temp)) rc = 0;
    sprintf((char *) temp, (char *) ss_sqi, name);
    if (unlink(temp)) rc = 0;
    sprintf((char *) temp, (char *) ss_sql, name);
    unlink(temp);
    return rc;
}   


/* Closing open messages */

static unsigned _SquishCloseAreaMsgs(HAREA sq)
{
  HMSG hm, hmNext;

  /* Close any open messages, if necessary */

  for (hm=Sqd->hmsgopen; hm; hm=hmNext)
  {
    hmNext=hm->hmsgnext;

    if (SquishCloseMsg(hm)==-1)
    {
      msgapierr=MERR_EOPEN;
      return FALSE;
    }
  }

  return TRUE;
}

/* Exitlist routine to make sure that all areas are closed */

void _SquishCloseOpenAreas(void)
{
  HAREA sq, haNext;

  /* If nothing to close, just get out. */

  if (!_haopen)
    return;

  for (sq=_haopen; sq; sq=haNext)
  {
    haNext=Sqd->hanext;

    SquishCloseArea(sq);
  }

  _haopen=NULL;
}

static sword EXPENTRY SquishCloseArea(HAREA sq)
{
    HAREA ha;
    
    if (InvalidMh(sq))
    {
        return -1;
    }

    /* If Base isn't locked, it MUST be locked, 'cause writiting the header
       is quit uncool if the base isn't locked */
    if (!sq->locked)
    {
        SquishLock(sq);
    }

    _SquishCloseAreaMsgs(sq);

    _SquishUpdateSq(sq, TRUE);

    /* All work is done. It is sure to unlock */
    if (sq->locked)
    {
      SquishUnlock(sq);
    }

    /* This is principal okay, but not a good idea.
       It is better to close all open messages, 'cause if the app 'lost'
       some open messages (yes, it's a byte-eater out there) it's impossible
       to close the area ! 
       TODO: Close all open messages */
    if (Sqd->msgs_open)
    {
        msgapierr = MERR_EOPEN;
        return -1;
    }

    close(Sqd->sfd);
    close(Sqd->ifd);


    /* OG: 8/12/00
       Remove Area from open-area list 
    */

    lock_semaphore(&_haopen_sem);

    /* Search entry */
    if (_haopen == sq)
    {
      /* Area is head of open-area list */
      _haopen = SqdSQ(_haopen)->hanext;
    }
    else
    {
      ha = _haopen;
      while (ha && SqdSQ(ha)->hanext != sq)
      {
        ha = SqdSQ(ha)->hanext;
      }
      
      if (ha)
      {
        /* Something found */
        SqdSQ(ha)->hanext = SqdSQ(sq)->hanext;    
      }
      else
      {
        /* Area not in open-area list ? - That's strange ! */
      }
    }
    unlock_semaphore(&_haopen_sem);
    
    pfree(sq->api);
    pfree((char *)sq->apidata);
    sq->id = 0L;
    pfree(sq);

    return 0;
}

static HMSG EXPENTRY SquishOpenMsg(HAREA sq, word mode, dword msgnum)
{
    struct _msgh *msgh;

    if (InvalidMh(sq))
    {
        return NULL;
    }

    if (mode == MOPEN_CREATE)
    {
        if ((sdword) msgnum < 0 || msgnum > sq->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }

        msgh = palloc(sizeof(struct _msgh));
        if (msgh == NULL)
        {
            msgapierr = MERR_NOMEM;
            return NULL;
        }

        memset(msgh, '\0', sizeof(struct _msgh));

        msgh->sq = sq;
        msgh->hdr = NULL;
        msgh->seek_frame = NULL_FRAME;
        msgh->bytes_written = 0L;
        msgh->cur_pos = 0L;
        msgh->cur_len = 0L;
        msgh->msgnum = msgnum;
    }
    else if (msgnum == 0)
    {
        msgapierr = MERR_NOENT;
        return NULL;
    }
    else
    {
        msgh = _SquishOpenMsgRead(sq, mode, msgnum);
        if (msgh == NULL)
        {
            return NULL;
        }
    }

    msgh->mode = mode;
    msgh->id = MSGH_ID;

    MsghSqd->msgs_open++;

    /* OG: 8/12/00
       Linking all open messages against to a open-messages list
    */
    
    msgh->hmsgnext = Sqd->hmsgopen;
    Sqd->hmsgopen  = msgh;

    return (HMSG) msgh;
}

/* TODO: Remember: When closing a message, remove the message from the 
   open-message list */

static sword EXPENTRY SquishCloseMsg(HMSG msgh)
{
    HAREA sq;
    HMSG  hmsg;
    
    if (InvalidMsgh(msgh))
    {
        return -1;
    }

    sq = msgh->sq;

    /* Fill the message out to the length that the app said it would be */

    if (msgh->mode == MOPEN_CREATE && msgh->bytes_written < msgh->totlen)
    {
        byte ch = '\0';

        lseek(MsghSqd->sfd, msgh->seek_frame + (dword) SQHDR_SIZE +
              (dword) XMSG_SIZE + msgh->clen +
          (dword) msgh->totlen - 1, SEEK_SET);

        farwrite(MsghSqd->sfd, &ch, 1);
    }

    MsghSqd->msgs_open--;


    /* TODO: Locking against other threads is a _MUST_ */

    /* OG: 8/12/00
       Remove message from open-message list 
    */

    /* Search entry */
    if (Sqd->hmsgopen == msgh)
    {
      /* Message is head of open-message list */
      Sqd->hmsgopen = msgh->hmsgnext;
    }
    else
    {
      /* Start at the root */
      hmsg = Sqd->hmsgopen;
      while (hmsg && hmsg->hmsgnext != msgh)
      {
        hmsg = hmsg->hmsgnext;
      }
      
      if (hmsg)
      {
        /* Something found */
        hmsg->hmsgnext = msgh->hmsgnext;    
      }
      else
      {
        /* Message not in open-message list ? - That's strange ! */
      }
    }

    if (msgh->hdr)
    {
        pfree(msgh->hdr);
    }

    msgh->id = 0L;
    pfree(msgh);

    return 0;
}

static dword EXPENTRY SquishReadMsg(MSGH * msgh, XMSG * msg, dword offset, dword bytes, byte * text, dword clen, byte * ctxt)
{
    dword bytesread = 0L;

    if (InvalidMsgh(msgh))
    {
        return -1;
    }

    if (msg)
    {
        lseek(MsghSqd->sfd, msgh->seek_frame + (dword) MsghSqd->sz_sqhdr, SEEK_SET);

        read_xmsg(MsghSqd->sfd, msg);

        msg->to[sizeof(msg->to) - 1] = '\0';
        msg->from[sizeof(msg->from) - 1] = '\0';
        msg->subj[sizeof(msg->subj) - 1] = '\0';
        msg->__ftsc_date[sizeof(msg->__ftsc_date) - 1] = '\0';

        StripNasties(msg->from);
        StripNasties(msg->to);
        StripNasties(msg->subj);

        /*
         *  If the _xmsg struct has been expanded, then seek past the junk
         *  we don't yet know about.
         */

        if (msgh->sq->sz_xmsg != XMSG_SIZE)
        {
            lseek(MsghSqd->sfd, msgh->sq->sz_xmsg - XMSG_SIZE, SEEK_CUR);
        }
    }

    if (bytes == 0L ||
        bytes > msgh->hdr->msg_length - XMSG_SIZE - offset - msgh->clen)
    {
        bytes = msgh->hdr->msg_length - XMSG_SIZE - offset - msgh->clen;
    }

    if (!(text && bytes) && clen == 0)
    {
        return 0;
    }

    switch (msgh->hdr->frame_type)
    {
    case FRAME_normal:
        msgh->cur_pos = offset;

        if (clen && ctxt)
        {
            if (lseek(MsghSqd->sfd, msgh->seek_frame + (dword) MsghSqd->sz_sqhdr +
              (dword) msgh->sq->sz_xmsg, SEEK_SET) == -1L)
            {
                msgapierr = MERR_BADF;
                return -1;
            }

            farread(MsghSqd->sfd, ctxt, (int)min(msgh->clen, clen));
            ctxt[(int)min(msgh->clen, clen)] = '\0';

            /* Skip over rest of control info */

            if (clen < msgh->clen && bytes && text)
            {
                lseek(MsghSqd->sfd, msgh->clen - clen, SEEK_CUR);
            }

            bytesread = 0;
        }
        else if (lseek(MsghSqd->sfd, msgh->seek_frame + (dword) MsghSqd->sz_sqhdr +
          (dword) msgh->sq->sz_xmsg + msgh->clen + msgh->cur_pos, SEEK_SET) == -1L)
        {
            msgapierr = MERR_BADF;
            return -1;
        }

        if (bytes && text)
        {
            bytesread = (dword) farread(MsghSqd->sfd, text, (unsigned int) bytes);
            text[(unsigned int) bytes] = '\0';
            msgh->cur_pos += (dword) bytesread;
        }

        return bytesread;

    default:
        msgapierr = MERR_BADF;
        return -1;
    }
}

static sword EXPENTRY SquishWriteMsg(MSGH * msgh, word append, XMSG * msg, byte * text, dword textlen, dword totlen, dword clen, byte * ctxt)
{
    MSG *sq;
    SQIDX idxe, here;
    SQHDR freehdr, newhdr, save, lhdr, *ohdr;
    FOFS oldofs, this_frame, last_frame, seek;
    word insert = FALSE, ctlen;
    int didlock = FALSE;

    if (InvalidMsgh(msgh))
    {
        return -1;
    }

    Init_Hdr(&freehdr);
    Init_Hdr(&lhdr);
    Init_Hdr(&newhdr);

    sq = msgh->sq;

    if (!ctxt)
    {
        clen = 0L;
    }

    if (!text)
    {
        textlen = 0L;
    }

    if (textlen == 0L)
    {
        text = NULL;
    }

    if (clen == 0L)
    {
        ctxt = NULL;
    }

    if (msgh->mode != MOPEN_CREATE)
    {
        msgh->bytes_written = 0L;
        append = TRUE;
    }

    /* Make sure that we don't write any more than we have to. */

    if (clen)
    {
        ctlen = (word) strlen((char *) ctxt);
        if ((dword) ctlen < clen)
        {
            clen = ctlen + 1;
        }
    }

    if (append)
    {
        /*
         *  The control info can only be written on the first pass, so
         *  blank it out if we're appending.
         */

        if (clen)
        {
            clen = 0L;
        }

        if (ctxt)
        {
            ctxt = NULL;
        }
    }
    else
    {
        sword res;

        if (!sq->locked)
        {
            didlock = _SquishLock(sq, 1);
        }

        msgh->totlen = totlen;

        res = _SquishFindFree(sq, &this_frame, totlen, clen, &freehdr, &last_frame, &lhdr, msgh);
        if (res != 0)
        {
            if (didlock)
            {
                _SquishUnlock(sq);
            }
            return res;
        }

        /* Update the Message Pointer chain */

        /* 0 means automatically use last message number */

        if (msgh->msgnum == 0)
        {
            msgh->msgnum = sq->num_msg + 1;
        }

        if (msgh->msgnum == sq->num_msg + 1 || (ohdr = _SquishGotoMsg(sq,
         msgh->msgnum, &oldofs, &here, FALSE)) == NULL)
        {
            insert = FALSE;     /* we're just appending */

            msgh->msgnum = sq->num_msg + 1;

            /* There is no next frame, since this is the last message... */

            newhdr.next_frame = NULL_FRAME;

            /* ...and the previous frame should be the former last message. */

            newhdr.prev_frame = Sqd->last_frame;

            /*
             *  Now update the former-last-message's frame, to say that we
             *  come after it.
             */

            _SquishUpdateHeaderNext(sq, newhdr.prev_frame, &lhdr, this_frame);
        }
        else
        {
            /* we're rewriting an 'old' message */

            insert = TRUE;      /* inserting a msg in middle of area */

            here.ofs = this_frame;

            /* Update the 'to' field, if necessary */

            if (msg)
            {
                here.hash = SquishHash(msg->to);

                if (msg->attr & MSGREAD)
                {
                    here.hash |= (dword) 0x80000000L;
                }
            }

            newhdr.next_frame = ohdr->next_frame;
            newhdr.prev_frame = ohdr->prev_frame;

            if (AddIndex(sq, &here, msgh->msgnum - 1) == -1 ||
              Add_To_Free_Chain(sq, oldofs, ohdr) == -1)
            {
                pfree(ohdr);

                if (didlock)
                {
                    _SquishUnlock(sq);
                }

                return -1;
            }

            _SquishUpdateHeaderNext(sq, newhdr.prev_frame, &save, this_frame);
            _SquishUpdateHeaderPrev(sq, newhdr.next_frame, &save, this_frame);

            if (Sqd->begin_frame == oldofs)
            {
                Sqd->begin_frame = this_frame;
            }

            if (Sqd->last_frame == oldofs)
            {
                Sqd->last_frame = this_frame;
            }

            pfree(ohdr);

            if (Sqd->cur_frame == oldofs)
            {
                Sqd->cur_frame = this_frame;
            }
        }


        /* Update the Free Pointer chain */

        /*
         *  If this was the first frame (ie. the first one pointed to by
         *  sq->free_frame, which means that the first frame found was
         *  long enough to hold the message), then set the free pointer to
         *  the start of the new free chain.
         */

        if (this_frame == Sqd->free_frame)
        {
            Sqd->free_frame = freehdr.next_frame;
        }

        if (this_frame == Sqd->last_free_frame)
        {
            Sqd->last_free_frame = freehdr.prev_frame;
        }

        /*
         *  Now update the linked list of free frames, to remove the
         *  current frame from the free-frame list, if necessary. We only
         *  need to do this if the current frame wasn't just being appended
         *  to the .SQD file, since there would be no links to update in
         *  that case.
         */

        if (this_frame != Sqd->end_frame)
        {
            _SquishUpdateHeaderNext(sq, freehdr.prev_frame, &lhdr,
               freehdr.next_frame);

            _SquishUpdateHeaderPrev(sq, freehdr.next_frame, &lhdr,
               freehdr.prev_frame);
        }

        /*
         *  If we're writing into the middle of the file, then we shouldn't
         *  update the frame length, as the frame may be longer than the
         *  actual message, and we'd lose some space that way.  However, if
         *  we're at the end of the file, then we're creating this frame, so
         *  it is by default the length of the message.
         */

        if (this_frame != Sqd->end_frame)
        {
            newhdr.frame_length = freehdr.frame_length;
        }
        else
        {
            newhdr.frame_length = totlen + clen + (dword) XMSG_SIZE;

            /*
             *  While we're at it, since we're writing at EOF, state
             *  where the new EOF is.
             */

            Sqd->end_frame = this_frame + (dword) MsghSqd->sz_sqhdr +
              (dword) XMSG_SIZE + clen + totlen;
        }

        /* Tell the system that this is now the current last message */

        if (!append && !insert)
        {
            Sqd->last_frame = this_frame;
        }

        newhdr.msg_length = (dword) XMSG_SIZE + clen + totlen;
        newhdr.clen = clen;

        newhdr.frame_type = FRAME_normal;

        _SquishWriteHeader(sq, this_frame, &newhdr);

        /*
         *  If no messages exist, point the head of the message chain to
         *  this one.
         */

        if (Sqd->begin_frame == NULL_FRAME)
        {
            Sqd->begin_frame = this_frame;
        }

        if (Sqd->next_frame == NULL_FRAME)
        {
            Sqd->next_frame = this_frame;
        }

        msgh->seek_frame = this_frame;
    }


    seek = msgh->seek_frame + MsghSqd->sz_sqhdr;

    /*
     *  Now with the pointer manipulation over with, write the message
     *  header...
     */

    if (msg)
    {
        if (tell(Sqd->sfd) != seek)
        {
            lseek(Sqd->sfd, seek, SEEK_SET);
        }

        /* make sure the __ftsc_date field is not empty */

        if (*msg->__ftsc_date)
        {
            msg->__ftsc_date[sizeof(msg->__ftsc_date) - 1] = '\0';
        }
        else
        {
            sprintf((char *) msg->__ftsc_date,
                    "%02d %s %02d  %02d:%02d:%02d",
                    msg->date_written.date.da ? msg->date_written.date.da : 1,
                    months_ab[msg->date_written.date.mo ?
                              msg->date_written.date.mo - 1 : 0],
                    (msg->date_written.date.yr + 80) % 100,
                    msg->date_written.time.hh,
                    msg->date_written.time.mm,
                    msg->date_written.time.ss << 1);
        }

        write_xmsg(Sqd->sfd, msg);
    }

    seek += XMSG_SIZE;

    if (!append)
    {
        if (ctxt && clen)
        {
            if (tell(Sqd->sfd) != seek)
            {
                lseek(Sqd->sfd, seek, SEEK_SET);
            }
            farwrite(Sqd->sfd, (byte *)ctxt, (unsigned int)clen);
        }
        else
        {
            clen = 0L;
        }

        msgh->clen = clen;
    }

    seek += msgh->clen;

    if (!append)
    {
        msgh->bytes_written = 0L;
    }

    if (text)
    {
        dword howmuch;

        if (append)
        {
            seek += msgh->bytes_written;
        }

        if (tell(Sqd->sfd) != seek)
        {
            lseek(Sqd->sfd, seek, SEEK_SET);
        }

        /*
         *  And write the message text itself!  Just don't let the app
         *  write any more than it said that it was going to.
         */

        howmuch = (dword) (msgh->totlen - msgh->bytes_written);
        howmuch = min(howmuch, (dword) textlen);

        if (howmuch)
        {
            if (farwrite(Sqd->sfd, (byte *)text, (unsigned)howmuch) != (signed) howmuch)
            {
                msgapierr = MERR_NODS;

                if (didlock)
                {
                    _SquishUnlock(sq);
                }

                return -1;
            }
        }

        msgh->bytes_written += howmuch;

        seek += textlen;
    }

    if (!append)
    {
        /*
         *  Inc number of messages, and write into right spot in index
         *  file.  (Frame location of msg #1 is stored at offset 0 *
         *  sizeof(dword) in the index, msg #2's offset is stored at
         *  offset 1 * sizeof(dword), etc.
         */

        memset(&idxe, '\0', sizeof(SQIDX));

        idxe.hash = SquishHash(msg->to);
        idxe.ofs = msgh->seek_frame;

        if (msg->attr & MSGREAD)
        {
            idxe.hash |= (dword) 0x80000000L;
        }

        idxe.umsgid = Sqd->uid++;

        /*
         *  Add the message to the .SQI file, as long as we didn't do it
         *  when we were re-writing the message links...
         */

        if (msgh->msgnum == sq->num_msg + 1 &&
          AddIndex(sq, &idxe, sq->num_msg) == -1)
        {
            if (didlock)
            {
                _SquishUnlock(sq);
            }

            return -1;
        }

        /*
         *  If we just created a new message, increment the lastread
         *  pointer
         */

        if (!insert)
        {
            sq->num_msg++;
            sq->high_msg++;
        }

        /* Update the header in the SQD file, too */

        _SquishUpdateSq(sq, FALSE);
    }

    if (didlock)
    {
        _SquishUnlock(sq);
    }

    return 0;
}

static sword EXPENTRY SquishKillMsg(MSG * sq, dword msgnum)
{
    FOFS killofs;
    SQIDX killframe;
    SQHDR killhdr, lhdr, *hd;

    if (InvalidMh(sq))
    {
        return -1;
    }

    Init_Hdr(&killhdr);
    Init_Hdr(&lhdr);

    /* Make the message number zero-based, so we can index into msgoffs[] */
    msgnum--;

    /*
     *  If the seek fails, the index isn't read correctly, or the index
     *  doesn't point to a valid message number, then report error.  Use
     *  special-case code for the first message, to make things faster.
     */

    if (msgnum == 0L)
    {
        killofs = Sqd->begin_frame;
    }
    else
    {
        if (_SquishGetIdxFrame(sq, msgnum, &killframe) == -1)
        {
            msgapierr = MERR_BADF;
            return -1;
        }

        killofs = killframe.ofs;
    }

    if (_SquishReadHeader(sq, killofs, &killhdr) == -1)
    {
        msgapierr = MERR_BADF;
        return -1;
    }

    /* If we don't have the index in memory already, grab it from disk */

    if (!sq->locked && _SquishReadIndex(sq) == -1)
    {
        return -1;
    }

    /* Update the Message Pointer chain */

    /* Fix the back/previous link */

    _SquishUpdateHeaderNext(sq, killhdr.prev_frame, &lhdr, killhdr.next_frame);

    /* Fix the forward/next link */

    _SquishUpdateHeaderPrev(sq, killhdr.next_frame, &lhdr, killhdr.prev_frame);

    /* If we delete the 1st msg in area, update the begin pointer */

    if (Sqd->begin_frame == killofs)
    {
        Sqd->begin_frame = killhdr.next_frame;
    }

    if (Sqd->last_frame == killofs)
    {
        Sqd->last_frame = killhdr.prev_frame;
    }

    if (Sqd->next_frame == killofs)
    {
        Sqd->next_frame = killhdr.next_frame;
    }

    if (Sqd->prev_frame == killofs)
    {
        Sqd->prev_frame = killhdr.prev_frame;
    }

    if (Sqd->cur_frame == killofs)
    {
        Sqd->cur_frame = killhdr.next_frame;
    }

    /* Update the Free Pointer chain */

    if (Add_To_Free_Chain(sq, killofs, &killhdr) == -1)
    {
        return -1;
    }

    /* Now, remove the message number we killed from the index */

    {
        SQIDX far *p1, *p2;

        p1 = Sqd->idxbuf + (size_t) msgnum;
        p2 = Sqd->idxbuf + (size_t) msgnum + 1;

        farmemmove(p1, p2, (size_t) (Sqd->idxbuf_used -
          (msgnum * (dword) sizeof(SQIDX))));
    }

    Sqd->idxbuf_delta = 0L;     /* Since we shifted the whole index */
    Sqd->idxbuf_used -= sizeof(SQIDX);

    /* And zero out the last index element */

    farmemset(&Sqd->idxbuf[(size_t) (Sqd->idxbuf_used /
      sizeof(SQIDX))], '\0', sizeof(SQIDX));

    /* Write it back to disk, if necessary */

    if (!sq->locked && _SquishWriteIndex(sq) == -1)
    {
        return -1;
    }

    /* Deduct one from the number of msgs */

    sq->num_msg--;
    sq->high_msg--;

    /*
     *  Adjust the current message number if we're above, since things
     *  will have shifted...
     */

    if (sq->cur_msg == msgnum + 1)
    {
        /* If we killed the msg we're on */

        /* Then jump back to the prior message, and refresh pointers */

        hd = _SquishGotoMsg(sq, --sq->cur_msg, NULL, NULL, TRUE);
        if (hd == NULL)
        {
            Sqd->prev_frame = Sqd->cur_frame = NULL_FRAME;
            Sqd->next_frame = Sqd->begin_frame;
        }
        else
        {
            pfree(hd);
        }
    }
    else if (sq->cur_msg > msgnum)
    {
        /* Decrement counter appropriately */
        sq->cur_msg--;
    }

    _SquishUpdateSq(sq, FALSE);

    return 0;
}

/*
 *  "Lock" the message base.  This means that no other applications will
 *  be able to access the message base during the duration it is locked,
 *  however, message writing will go much quicker.
 */

static sword EXPENTRY SquishLock(MSG * sq)
{
    if (InvalidMh(sq))
    {
        return -1;
    }

    /* Don't do anything if already locked */

    if (sq->locked)
    {
        return 0;
    }

    /* Read the .SQI file into memory */

    if (_SquishReadIndex(sq) == -1)
    {
        return -1;
    }

    /* And lock the file */

    if (!_SquishLock(sq, 0))
    {
        farpfree(Sqd->idxbuf);
        return -1;
    }

    /* Set the flag in the _sqdata header */

    sq->locked = TRUE;

    return 0;
}

/* Undo the above "lock" operation */

static sword EXPENTRY SquishUnlock(MSG * sq)
{
    if (InvalidMh(sq))
    {
        return -1;
    }

    if (!sq->locked)
    {
        return -1;
    }

    sq->locked = FALSE;

    if (mi.haveshare)
    {
        unlock(Sqd->sfd, 0L, 1L);
    }

    _SquishWriteIndex(sq);
    return 0;
}

sword MSGAPI SquishValidate(byte * name)
{
    byte temp[PATHLEN];

    sprintf((char *) temp, (char *) ss_sqd, name);

    if (!fexist((char *) temp))
    {
        return FALSE;
    }

    sprintf((char *) temp, (char *) ss_sqi, name);

    if (!fexist((char *) temp))
    {
        return FALSE;
    }

    return TRUE;
}

static sword EXPENTRY SquishSetCurPos(MSGH * msgh, dword pos)
{
    if (InvalidMsgh(msgh))
    {
        return -1;
    }

    msgh->cur_pos = pos;
    return 0;
}

static dword EXPENTRY SquishGetCurPos(MSGH * msgh)
{
    if (InvalidMsgh(msgh))
    {
        return -1;
    }

    return msgh->cur_pos;
}

static UMSGID EXPENTRY SquishMsgnToUid(MSG * sq, dword msgnum)
{
    SQIDX idxe;

    if (InvalidMh(sq))
    {
        return -1;
    }

    if (msgnum == 0L || _SquishGetIdxFrame(sq, msgnum - 1, &idxe) == -1)
    {
        return (UMSGID) 0L;
    }

    return idxe.umsgid;
}

static dword EXPENTRY SquishUidToMsgn(MSG * sq, UMSGID umsgid, word type)
{
    UMSGID answer;
    word getbuf;
    size_t mn, nelem, lower;
    int found = 0;

    if (InvalidMh(sq))
    {
        return 0L;
    }

    if (umsgid == 0L)
    {
        return (dword) 0L;
    }

    if (sq->locked)
    {
        getbuf = FALSE;
    }
    else
    {
        getbuf = TRUE;

        if (_SquishReadIndex(sq) == -1)
        {
            return 0L;
        }
    }

    answer = (UMSGID) 0L;

    if (sq->num_msg > 0)
    {
        nelem = (size_t) sq->num_msg;
        if (nelem > (size_t) (Sqd->idxbuf_used / sizeof(SQIDX)))
        {
            nelem = (size_t) (Sqd->idxbuf_used / sizeof(SQIDX));
        }
        lower = 0;
        mn = 0;
        while (nelem > 0)
        {
            mn = lower + nelem / 2;
            if (Sqd->idxbuf[mn].umsgid == umsgid)
            {
                found = 1;
                break;
            }
            else
            {
                if (Sqd->idxbuf[mn].umsgid > umsgid)
                {
                    nelem = mn - lower;
                }
                else
                {
                    nelem = nelem - (mn - lower) - 1;
                    lower = mn + 1;
                }
            }
        }
        if (!found)
        {
            if (type == UID_NEXT)
            {
                if (Sqd->idxbuf[mn].umsgid >= umsgid)
                {
                    found = 1;
                }
                else if ((mn + 1) < (size_t) sq->num_msg)
                {
                    mn++;
                    if (Sqd->idxbuf[mn].umsgid >= umsgid)
                    {
                        found = 1;
                    }
                }
            }
            else if (type == UID_PREV)
            {
                if (Sqd->idxbuf[mn].umsgid <= umsgid)
                {
                    found = 1;
                }
                else if (mn > 0)
                {
                    mn--;
                    if (Sqd->idxbuf[mn].umsgid <= umsgid)
                    {
                        found = 1;
                    }
                }
            }
        }
        if (found)
        {
            answer = (UMSGID) (mn + 1);
        }
    }

    if (getbuf && Sqd->idxbuf)
    {
        farpfree(Sqd->idxbuf);
        Sqd->idxbuf = NULL;
    }

    return answer;
}

static dword EXPENTRY SquishGetTextLen(MSGH * msgh)
{
    return msgh->cur_len;
}

static dword EXPENTRY SquishGetCtrlLen(MSGH * msgh)
{
    return msgh->clen;
}

static dword EXPENTRY SquishGetHighWater(MSG * sq)
{
    return SquishUidToMsgn(sq, sq->high_water, UID_PREV);
}

static sword EXPENTRY SquishSetHighWater(MSG * sq, dword hwm)
{
    sq->high_water = SquishMsgnToUid(sq, hwm);
    return 0;
}

static sword MSGAPI _OpenSquish(MSG * sq, word * mode)
{
    byte temp[PATHLEN];

    sprintf((char *) temp, (char *) ss_sqd, Sqd->base);

    if (*mode == MSGAREA_CREATE)
    {
#ifdef UNIX
	Sqd->sfd = sopen((char *) temp, fop_wpb, SH_DENYNONE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#else	    
        Sqd->sfd = sopen((char *) temp, fop_wpb, SH_DENYNONE, S_IREAD | S_IWRITE);
#endif
    }
    else
    {
#ifdef UNIX
	Sqd->sfd = sopen((char *) temp, fop_rpb, SH_DENYNONE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#else
        Sqd->sfd = sopen((char *) temp, fop_rpb, SH_DENYNONE, S_IREAD | S_IWRITE);
#endif
    }
    if (Sqd->sfd == -1)
    {
        if (*mode != MSGAREA_CRIFNEC)
        {
            msgapierr = MERR_NOENT;
            return 0;
        }

        *mode = MSGAREA_CREATE;
#ifdef UNIX
	Sqd->sfd = sopen((char *) temp, fop_wpb | O_EXCL, SH_DENYNONE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#else
        Sqd->sfd = sopen((char *) temp, fop_wpb | O_EXCL, SH_DENYNONE, S_IREAD | S_IWRITE);
#endif
        if (Sqd->sfd == -1)
        {
            msgapierr = MERR_NOENT;
            return 0;
        }
    }

    sprintf((char *) temp, (char *) ss_sqi, Sqd->base);

    if (*mode == MSGAREA_CREATE)
    {
#ifdef UNIX
	Sqd->ifd = sopen((char *) temp, fop_wpb, SH_DENYNONE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#else
        Sqd->ifd = sopen((char *) temp, fop_wpb, SH_DENYNONE, S_IREAD | S_IWRITE);
#endif
    }
    else
    {
#ifdef UNIX
	Sqd->ifd = sopen((char *) temp, fop_rpb, SH_DENYNONE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#else
        Sqd->ifd = sopen((char *) temp, fop_rpb, SH_DENYNONE, S_IREAD | S_IWRITE);
#endif
    }
    if (Sqd->ifd == -1)
    {
        if (*mode != MSGAREA_CRIFNEC)
        {
            close(Sqd->sfd);
            msgapierr = MERR_NOENT;
            return 0;
        }

        *mode = MSGAREA_CREATE;

        Sqd->ifd = sopen((char *) temp, fop_wpb | O_EXCL, SH_DENYNONE, S_IREAD | S_IWRITE);
        if (Sqd->ifd == -1)
        {
            close(Sqd->sfd);
            msgapierr = MERR_NOENT;
            return 0;
        }
    }

    return 1;
}

static SQHDR *MSGAPI _SquishGotoMsg(MSG * sq, dword msgnum, FOFS * seek_frame, SQIDX * idx, word updptrs)
{
    FOFS ofs;
    SQHDR *hdr;
    SQIDX idxe, *ip;

    /* In case the caller didn't provide a SQIDX to write to, use our own */

    if (idx)
    {
        ip = idx;
    }
    else
    {
        ip = &idxe;
    }

    /* Get the SQIDX record from the index file */

    if (_SquishGetIdxFrame(sq, msgnum - 1, ip) == -1)
    {
        return NULL;
    }

    ofs = ip->ofs;

    if (seek_frame)
    {
        *seek_frame = ofs;
    }

    /* Read the frame header from the data file */

    hdr = (SQHDR *) palloc(sizeof(SQHDR));
    if (hdr == NULL)
    {
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    if (_SquishReadHeader(sq, ofs, hdr) != 0)
    {
        pfree(hdr);
        return NULL;
    }

    if (updptrs)
    {
        sq->cur_msg = msgnum;

        /* Now update all the in-memory pointers */

        Sqd->cur_frame = ofs;
        Sqd->next_frame = hdr->next_frame;
        Sqd->prev_frame = hdr->prev_frame;
    }

    return hdr;
}

static MSGH *_SquishOpenMsgRead(MSG * sq, word mode, dword msgnum)
{
    SQHDR *hdr;
    struct _msgh *msgh;
    FOFS this_frame, seek_frame;

    unused(mode);

    this_frame = Sqd->cur_frame;

    if ((this_frame == Sqd->end_frame || this_frame == NULL_FRAME) &&
      msgnum == MSGNUM_current)
    {
        msgapierr = MERR_NOENT;
        return NULL;
    }

    /* Figure out which way to seek */

    hdr = NULL;

    if (msgnum == MSGNUM_current || msgnum == sq->cur_msg)
    {
        seek_frame = Sqd->cur_frame;
        msgnum = MSGNUM_current;
    }
    else if (msgnum == MSGNUM_next || msgnum == sq->cur_msg + 1L)
    {
        seek_frame = Sqd->next_frame;
        msgnum = MSGNUM_next;
    }
    else if (msgnum == MSGNUM_previous || msgnum == sq->cur_msg - 1L)
    {
        seek_frame = Sqd->prev_frame;
        msgnum = MSGNUM_previous;
    }
    else
    {
        hdr = _SquishGotoMsg(sq, msgnum, &seek_frame, NULL, TRUE);
        if (hdr == NULL)
        {
            return NULL;
        }
    }

    /* If we go to beginning, we're at message #0 */

    if (msgnum == MSGNUM_previous && seek_frame == NULL_FRAME)
    {
        Sqd->next_frame = Sqd->begin_frame;
        Sqd->prev_frame = NULL_FRAME;
        Sqd->cur_frame = NULL_FRAME;
        sq->cur_msg = 0;

        if (hdr)
        {
            pfree(hdr);
        }

        msgapierr = MERR_NOENT;
        return NULL;
    }
    else if (seek_frame == NULL_FRAME)
    {
        /* else if that message doesn't exist */

        if (hdr)
        {
            pfree(hdr);
        }

        msgapierr = MERR_NOENT;
        return NULL;
    }
    else if (seek_frame != this_frame)
    {
        /* If we moved to another msg, then update pointer appropriately */

        switch ((dword) msgnum)
        {
        case MSGNUM_next:
            sq->cur_msg++;
            break;

        case MSGNUM_previous:
            sq->cur_msg--;
            break;

        default:
            /* _SquishGotoMsg() will set sq->cur_msg by itself */
            break;
        }
    }
    else if (msgnum != MSGNUM_current && msgnum != sq->cur_msg)
    {
        /*
         *  If the pointer didn't move, then error, unless we specifically
         *  asked to read the same msg number */

        if (hdr)
        {
            pfree(hdr);
        }

        msgapierr = MERR_BADA;
        return NULL;
    }

    /* _SquishGotoMsg() will have already set these */

    if (msgnum == MSGNUM_current || msgnum == MSGNUM_previous ||
      msgnum == MSGNUM_next)
    {
        /* Update the current pointer */
        Sqd->cur_frame = seek_frame;

        hdr = palloc(sizeof(SQHDR));
        if (hdr == NULL)
        {
            msgapierr = MERR_NOMEM;
            return NULL;
        }

        /* Now grab the header of the frame we're trying to read */

        if (_SquishReadHeader(sq, seek_frame, hdr) != 0)
        {
            pfree(hdr);
            return NULL;
        }

        /* Copy the "next frame" pointer into memory */

        Sqd->next_frame = hdr->next_frame;
        Sqd->prev_frame = hdr->prev_frame;
    }

    msgh = palloc(sizeof(struct _msgh));
    if (msgh == NULL)
    {
        pfree(hdr);
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    msgh->hdr = hdr;
    msgh->seek_frame = seek_frame;
    msgh->sq = sq;
    msgh->cur_pos = 0L;
    msgh->clen = hdr->clen;
    msgh->cur_len = hdr->msg_length - msgh->clen - XMSG_SIZE;
    msgh->msgnum = msgnum;

    return (MSGH *) msgh;
}

static sword MSGAPI _SquishReadHeader(MSG * sq, dword ofs, SQHDR * hdr)
{
    if (ofs == NULL_FRAME)
    {
        return 0;
    }

    if (lseek(Sqd->sfd, ofs, SEEK_SET) == -1L ||
        (!read_sqhdr(Sqd->sfd, hdr)) ||
        hdr->id != SQHDRID)
    {
        msgapierr = MERR_BADF;
        return -1;
    }

    return 0;
}

static sword MSGAPI _SquishWriteHeader(MSG * sq, dword ofs, SQHDR * hdr)
{
    if (ofs == NULL_FRAME)
    {
        return 0;
    }

    hdr->id = SQHDRID;

    if (lseek(Sqd->sfd, ofs, SEEK_SET) == -1L ||
        (!write_sqhdr(Sqd->sfd, hdr)))
    {
        msgapierr = MERR_BADF;
        return -1;
    }

    return 0;
}

static sword MSGAPI _SquishUpdateHeaderNext(MSG * sq, dword ofs, SQHDR * hdr, dword newval)
{
    if (ofs == NULL_FRAME)
    {
        return 0;
    }

    if (_SquishReadHeader(sq, ofs, hdr) != 0)
    {
        return -1;
    }

    hdr->next_frame = newval;

    if (_SquishWriteHeader(sq, ofs, hdr) != 0)
    {
        return -1;
    }

    return 0;
}

static sword MSGAPI _SquishUpdateHeaderPrev(MSG * sq, dword ofs, SQHDR * hdr, dword newval)
{
    if (ofs == NULL_FRAME)
    {
        return 0;
    }

    if (_SquishReadHeader(sq, ofs, hdr) != 0)
    {
        return -1;
    }

    hdr->prev_frame = newval;

    if (_SquishWriteHeader(sq, ofs, hdr) != 0)
    {
        return -1;
    }

    return 0;
}

/*
 *  Rewrite the first "struct _sq" portion of the file (which contains
 *  the global data).
 */

static sword MSGAPI _SquishWriteSq(MSG * sq)
{
    struct _sqbase sqbase;

    SqToSqbase(sq, &sqbase);

    lseek(Sqd->sfd, 0L, SEEK_SET);

    if (!write_sqbase(Sqd->sfd, &sqbase))
    {
        return -1;
    }

    Sqd->delta = sqbase;

    /* Make sure it gets written, so other tasks see it */

#if defined(MSDOS) && !defined(__NT__)
    flush_handle2(Sqd->sfd);
#endif

    return 0;
}

static sword MSGAPI _SquishReadSq(MSG * sq, struct _sqbase *sqb)
{
    lseek(Sqd->sfd, 0L, SEEK_SET);

    if (!read_sqbase(Sqd->sfd, sqb))
    {
        return -1;
    }

    return 0;
}

static sword MSGAPI _SquishUpdateSq(MSG * sq, word force)
{
    struct _sqbase now;         /* The status of the header in the SQD */
    struct _sqbase new;         /* The new, to-be-written header */
    struct _sqbase upd;
    struct _sqbase *delta = &Sqd->delta;

    /* No need to do this if the squishfile is locked */

    if (sq->locked && !force)
    {
        return 0;
    }

    SqToSqbase(sq, &upd);

    /* Read the Squish header, as it currently is now */

    if (_SquishReadSq(sq, &now) == -1)
    {
        return -1;
    }

    /*
     *  Let the new header be equal to what it is now, but apply any
     *  changes that this task may have made.
     */

    new = now;

    /* Now make changes as necessary to the squishfile's links */

    if (!eqstri((char *) delta->base, (char *) upd.base))
    {
        strcpy((char *) new.base, (char *) upd.base);
    }

    if (delta->num_msg != upd.num_msg)
    {
        new.num_msg += (upd.num_msg - delta->num_msg);
    }

    if (delta->high_msg != upd.high_msg)
    {
        new.high_msg += (upd.high_msg - delta->high_msg);
    }

    if (delta->max_msg != upd.max_msg)
    {
        new.max_msg = upd.max_msg;
    }

    if (delta->keep_days != upd.keep_days)
    {
        new.keep_days = upd.keep_days;
    }

    if (delta->skip_msg != upd.skip_msg)
    {
        new.skip_msg = upd.skip_msg;
    }

    if (delta->high_water != upd.high_water)
    {
        new.high_water = upd.high_water;
    }

    if (delta->uid != upd.uid)
    {
        new.uid = upd.uid;
    }

    if (delta->begin_frame != upd.begin_frame)
    {
        new.begin_frame = upd.begin_frame;
    }

    if (delta->last_frame != upd.last_frame)
    {
        new.last_frame = upd.last_frame;
    }

    if (delta->free_frame != upd.free_frame)
    {
        new.free_frame = upd.free_frame;
    }

    if (delta->last_free_frame != upd.last_free_frame)
    {
        new.last_free_frame = upd.last_free_frame;
    }

    if (delta->end_frame != upd.end_frame)
    {
        new.end_frame = upd.end_frame;
    }

    SqbaseToSq(&new, sq);

    if (_SquishWriteSq(sq) == -1)
    {
        return -1;
    }

    return 0;
}

static void MSGAPI Init_Hdr(SQHDR * sh)
{
    memset(sh, '\0', sizeof(SQHDR));
    sh->id = SQHDRID;
    sh->next_frame = sh->prev_frame = 0L;
    sh->frame_length = sh->msg_length = 0L;
    sh->frame_type = FRAME_normal;
}

static void SqToSqbase(MSG * sq, struct _sqbase *sqbase)
{
    memset(sqbase, '\0', sizeof(struct _sqbase));

    sqbase->len = Sqd->len;
    sqbase->num_msg = sq->num_msg;
    sqbase->high_msg = sq->high_msg;
    sqbase->max_msg = Sqd->max_msg;
    sqbase->keep_days = Sqd->keep_days;
    sqbase->skip_msg = Sqd->skip_msg;
    sqbase->high_water = sq->high_water;
    sqbase->uid = Sqd->uid;

    strcpy((char *) sqbase->base, (char *) Sqd->base);

    sqbase->begin_frame = Sqd->begin_frame;
    sqbase->free_frame = Sqd->free_frame;
    sqbase->last_free_frame = Sqd->last_free_frame;
    sqbase->end_frame = Sqd->end_frame;
    sqbase->last_frame = Sqd->last_frame;
    sqbase->sz_sqhdr = Sqd->sz_sqhdr;
}

static void SqbaseToSq(struct _sqbase *sqbase, MSG * sq)
{
    Sqd->len = sqbase->len;
    sq->num_msg = sqbase->num_msg;
    sq->high_msg = sqbase->high_msg;
    Sqd->max_msg = sqbase->max_msg;
    Sqd->keep_days = sqbase->keep_days;
    Sqd->skip_msg = sqbase->skip_msg;
    sq->high_water = sqbase->high_water;
    Sqd->uid = sqbase->uid;

    strcpy((char *) Sqd->base, (char *) sqbase->base);

    Sqd->begin_frame = sqbase->begin_frame;
    Sqd->free_frame = sqbase->free_frame;
    Sqd->last_free_frame = sqbase->last_free_frame;
    Sqd->end_frame = sqbase->end_frame;
    Sqd->last_frame = sqbase->last_frame;
    Sqd->sz_sqhdr = sqbase->sz_sqhdr;

    if (Sqd->max_msg && Sqd->skip_msg >= Sqd->max_msg - 1)
    {
        Sqd->skip_msg = 0;
        Sqd->max_msg = 0;
    }
}

/*
 *  Write a message to the index file.  If we're in LOCKED mode, buffer
 *  it accordingly.
 */

static sword near AddIndex(MSG * sq, SQIDX * ix, dword num)
{
    dword where, old;

    if (sq->locked && Sqd->idxbuf)
    {
        where = num * sizeof(SQIDX);

        /* If we specified a number that's too big, cry and scream */

        if (where >= Sqd->idxbuf_used + 1)
        {
            return -1;
        }

        /*
         *  If this would override the size of our index buffer, make it
         *  bigger.
         */

        if (where >= Sqd->idxbuf_size)
        {
            old = Sqd->idxbuf_size;

            Sqd->idxbuf_size += (sizeof(SQIDX) * EXTRA_BUF);

            if (
#if (defined(MSDOS) && !defined(__FLAT__)) || (defined(_MSC_VER) && (_MSC_VER < 1200))
                   Sqd->idxbuf_size >= 65300L ||
#endif
                   (Sqd->idxbuf = farrealloc(Sqd->idxbuf, (word) Sqd->idxbuf_size)) == NULL)
            {
                /* Restore the old size of the index file */

                Sqd->idxbuf_size = old;

                /* Unlock and/or flush the squish index file */

                _SquishWriteIndex(sq);

                if (sq->locked)
                {
                    SquishUnlock(sq);
                }

                /* Now try to write this one to the file manually */

                if (lseek(Sqd->ifd, (long)num * SQIDX_SIZE, SEEK_SET) == -1 ||
                    (!write_sqidx(Sqd->ifd, ix, 1)))
                {
                    msgapierr = MERR_BADF;
                    return -1;
                }

                return 0;
            }
        }

        Sqd->idxbuf[(word) num] = *ix;

        if (Sqd->idxbuf_used < where + sizeof(SQIDX))
        {
            Sqd->idxbuf_used = where + sizeof(SQIDX);
        }

        if (Sqd->idxbuf_write < where + sizeof(SQIDX))
        {
            Sqd->idxbuf_write = where + sizeof(SQIDX);
        }

        if (Sqd->idxbuf_delta > where)
        {
            Sqd->idxbuf_delta = where;
        }
    }
    else
    {
        /* otherwise write the info directly to the *.SQI file */

        if (lseek(Sqd->ifd, (long)num * SQIDX_SIZE, SEEK_SET) == -1 ||
            (!write_sqidx(Sqd->ifd, ix, 1)))
        {
            msgapierr = MERR_BADF;
            return -1;
        }
    }

    return 0;
}

static sword near Add_To_Free_Chain(MSG * sq, FOFS killofs, SQHDR * killhdr)
{
    SQHDR lhdr;

    /* If there are no other free frames, then this is simple... */

    if (Sqd->free_frame == NULL_FRAME || Sqd->last_free_frame == NULL_FRAME)
    {
        Sqd->free_frame = Sqd->last_free_frame = killofs;
        killhdr->prev_frame = killhdr->next_frame = NULL_FRAME;
    }
    else
    {
        /* Insert this frame into the chain */

        killhdr->next_frame = NULL_FRAME;
        killhdr->prev_frame = Sqd->last_free_frame;

        if (_SquishUpdateHeaderNext(sq, Sqd->last_free_frame, &lhdr, killofs) == -1)
        {
            return -1;
        }

        Sqd->last_free_frame = killofs;
    }

    /* Now write the fixed header of the killed message back to the file */

    killhdr->frame_type = FRAME_free;

#ifdef DEBUG
    fprintf(stderr, "* Killed frame at %08lx (%ld bytes)\n\n", killofs, killhdr->frame_length);
#endif

    if (_SquishWriteHeader(sq, killofs, killhdr) == -1)
    {
        return -1;
    }

    /* And write to the SquishFile, to update the FREE pointer */

    _SquishUpdateSq(sq, FALSE);

    return 0;
}

static sword near _SquishReadIndex(MSG * sq)
{
    dword ofslen;

    /* Seek to end, and find length of file */

    lseek(Sqd->ifd, 0L, SEEK_END);
    ofslen = tell(Sqd->ifd);

    Sqd->idxbuf_used = (ofslen * sizeof(SQIDX)) / SQIDX_SIZE;
    Sqd->idxbuf_write = (ofslen * sizeof(SQIDX)) / SQIDX_SIZE;
    Sqd->idxbuf_delta = (ofslen * sizeof(SQIDX)) / SQIDX_SIZE;
    Sqd->idxbuf_size = (ofslen * sizeof(SQIDX)) / SQIDX_SIZE +
        (EXTRA_BUF) * (dword) sizeof(SQIDX);

#if (defined(MSDOS) && !defined(__FLAT__)) || (defined(_MSC_VER) && (_MSC_VER < 1200))
    if (Sqd->idxbuf_size >= 65300L || (Sqd->idxbuf = farpalloc((size_t) Sqd->idxbuf_size)) == NULL)
#else
    if ((Sqd->idxbuf = farpalloc((size_t) Sqd->idxbuf_size)) == NULL)
#endif
    {
        msgapierr = MERR_NOMEM;
        return -1;
    }

    /* Seek to beginning of file, and try to read everything in */

    if (ofslen &&
        ( (lseek(Sqd->ifd, 0L, SEEK_SET) == -1) ||
          (!read_sqidx(Sqd->ifd, Sqd->idxbuf, (ofslen / SQIDX_SIZE)))))
    {
        farpfree(Sqd->idxbuf);
        Sqd->idxbuf = NULL;
        msgapierr = MERR_BADF;
        return -1;
    }

    return 0;
}

static sword near _SquishWriteIndex(MSG * sq)
{
    sword ret = 0;

    if (Sqd->idxbuf == NULL)
    {
        return -1;
    }

    /* If we don't need to update it, return 0 */

    if (Sqd->idxbuf_delta == Sqd->idxbuf_write)
    {
        ret = 0;
    }
    else if (lseek(Sqd->ifd, Sqd->idxbuf_delta, SEEK_SET) == -1 ||
             !write_sqidx(Sqd->ifd,
                          Sqd->idxbuf + Sqd->idxbuf_delta / sizeof(SQIDX),
                          (Sqd->idxbuf_write - Sqd->idxbuf_delta) /
                            sizeof(SQIDX)))
    {
        ret = -1;
        msgapierr = MERR_BADF;
    }

#if defined(MSDOS) && !defined(__NT__)
    flush_handle2(Sqd->ifd);
#endif

    farpfree(Sqd->idxbuf);
    Sqd->idxbuf = NULL;

    return ret;
}

static sword near _SquishGetIdxFrame(MSG * sq, dword num, SQIDX * idx)
{
    dword ofs;

    ofs = (dword) num *(dword) sizeof(SQIDX);

    if (sq->locked && Sqd->idxbuf)
    {
        if (ofs > Sqd->idxbuf_used)
        {
            msgapierr = MERR_NOENT;
            return -1;
        }

        *idx = Sqd->idxbuf[(word) num];
    }
    else
    {
        if (lseek(Sqd->ifd, (dword)num * (dword)SQIDX_SIZE, SEEK_SET) == -1L ||
            (!read_sqidx(Sqd->ifd, idx, 1)))
        {
            msgapierr = MERR_BADF;
            return -1;
        }
    }

    if (idx->ofs == NULL_FRAME)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

/* Finds a free frame big enough to hold the specified message */

static sword near _SquishFindFree(MSG * sq, FOFS * this_frame, dword totlen, dword clen, SQHDR * freehdr, FOFS * last_frame, SQHDR * lhdr, MSGH * msgh)
{
    /*
     *  First grab the latest `free' frame from the file, so we don't
     *  overwrite ourselves, in a multi-tasking environment.
     */

    if (!sq->locked)
    {
        word type;

        /* Start off by closing & re-opening everything... */

        close(Sqd->sfd);
        close(Sqd->ifd);

        type = MSGAREA_NORMAL;

        if (!_OpenSquish(sq, &type))
        {
            sq->id = 0L;
            return -1;
        }

        /*
         *  Now read the pointer from the SquishFile, and update our info
         *  before writing back to the base.
         */

        _SquishUpdateSq(sq, FALSE);
    }

    *this_frame = Sqd->free_frame;

    /*
     *  Kill enough msgs to make room for this one, as long as there
     *  is a limit set, we're set up for dynamic renumbering, AND we're
     *  writing to the last message in the base.
     */

    if (MsghSqd->max_msg && msgh->msgnum == 0L && (MsghSqd->flag & SF_STATIC) == 0)
    {
        int didlock = FALSE;

        while (msgh->sq->num_msg + 1 > MsghSqd->max_msg)
        {
            if (!didlock && !sq->locked)
            {
                /* Lock the SQD for greater performance */
                didlock = (SquishLock(msgh->sq) == 0);
            }

            if (SquishKillMsg(msgh->sq, MsghSqd->skip_msg + 1L) == -1)
            {
                if (didlock)
                {
                    SquishUnlock(msgh->sq);
                }

                return -1;
            }
        }

        if (didlock)
        {
            SquishUnlock(msgh->sq);
        }
    }

    /*
     *  Now walk the list of free pointers, and see if we can find one
     *  which is big enough to hold everything.
     */

#ifdef DEBUG
    fprintf(stderr, "Msg #%ld; need %ld bytes:\n", msgh->sq->num_msg, totlen + clen + (dword) XMSG_SIZE);
#endif

    while (1)
    {
        /* If we're at EOF, then there's always enough room. */

        if (*this_frame == NULL_FRAME)
        {
#ifdef DEBUG
            fprintf(stderr, "!!! No go.  Allocating new frame.\n\n");
#endif
            *this_frame = Sqd->end_frame;
            freehdr->next_frame = NULL_FRAME;
            freehdr->prev_frame = NULL_FRAME;
            freehdr->frame_length = 0L;
            freehdr->msg_length = 0L;
            freehdr->frame_type = FRAME_normal;
            freehdr->clen = clen;
            break;
        }

        if (_SquishReadHeader(sq, *this_frame, freehdr) == -1)
        {
            *this_frame = NULL_FRAME;
            continue;
        }

#ifdef DEBUG
        fprintf(stderr, "  Frame at %08lx: %ld\n", *this_frame, freehdr->frame_length);
#endif

        if (freehdr->frame_length >= totlen + clen + (dword) XMSG_SIZE)
        {
#ifdef DEBUG
            fprintf(stderr, "... Got it!\n\n");
#endif
            break;
        }
        else
        {
            /*
             *  If two frames are adjacent, try to merge them to make more
             *  room for the current message.
             */

            *last_frame = NULL_FRAME;

            while (freehdr->next_frame == *this_frame + MsghSqd->sz_sqhdr +
              freehdr->frame_length && freehdr->frame_length < totlen + clen + (dword) XMSG_SIZE)
            {
                if (_SquishReadHeader(sq, freehdr->next_frame, lhdr) == -1)
                {
                    break;
                }

                freehdr->frame_length += MsghSqd->sz_sqhdr + lhdr->frame_length;

                /*
                 *  Don't bother writing the frame to disk -- if a message
                 *  is being written, then it'll get updated anyway.  If
                 *  not, then we want to keep these chunks separated, in
                 *  case we have smaller messages in the future.
                 */

#ifdef DEBUG
                fprintf(stderr, "  Compact of %08lx and %08lx (new len=%ld)\n", *this_frame,
                  freehdr->next_frame, freehdr->frame_length);
#endif

                *last_frame = freehdr->next_frame;
                freehdr->next_frame = lhdr->next_frame;
            }

            if (freehdr->frame_length >= totlen + clen + (dword) XMSG_SIZE)
            {
                /*
                 *  If one of te frames in our chain was the last free frame,
                 *  set the last free frame to the one we've merged it into,
                 *  for later a clean up effort.
                 */

                if (*last_frame == Sqd->last_free_frame)
                {
                    Sqd->last_free_frame = *this_frame;
                }

#ifdef DEBUG
                fprintf(stderr, "  Rescued frame at %08lx (%ld bytes)\n", *this_frame, freehdr->frame_length);
#endif
                break;
            }

            *this_frame = freehdr->next_frame;
        }
    }

    return 0;
}

static void far *near farmemmove(void far * destin, const void far * source, unsigned n)
{
#if defined(__FARDATA__)
    return memmove(destin, source, n);
#elif defined(__WATCOMC__)
    return _fmemmove(destin, source, n);
#else
    void far *destsave = destin;
    char far *destc;
    const char far *sourcec;

    destc = destin;
    sourcec = source;

    while (n--)
    {
        *destc++ = *sourcec++;
    }

    return destsave;
#endif
}

static void far *near farmemset(void far * s, int c, size_t length)
{
#ifdef __FARDATA__
    return (memset(s, c, length));
#else
    void far *s_save = s;
    char far *b = s;

    while (length--)
    {
        *b++ = (char)c;
    }

    return s_save;
#endif
}

/* Function to set the highest/skip message numbers for a *.SQ? base */

void EXPENTRY SquishSetMaxMsg(MSG * sq, dword max_msgs, dword skip_msgs, dword age)
{
    if (max_msgs != (dword) - 1L)
    {
        Sqd->max_msg = max_msgs;
    }

    if (skip_msgs != (dword) - 1L)
    {
        Sqd->skip_msg = skip_msgs;
    }

    if (age != (dword) - 1L)
    {
        Sqd->keep_days = (word) age;
    }
}

dword EXPENTRY SquishHash(byte * f)
{
    dword hash = 0, g;
    char *p;

    for (p = (char *) f; *p; p++)
    {
        hash = (hash << 4) + tolower(*p);

        if ((g = (hash & 0xf0000000L)) != 0L)
        {
            hash |= g >> 24;
            hash |= g;
        }
    }

    /* Strip off high bit */

    return (dword) (hash & 0x7fffffffL);
}

static int near _SquishLock(MSG * sq, int force)
{
    return !(mi.haveshare && ((force) ? waitlock(Sqd->sfd, 0L, 1L)
                                      :     lock(Sqd->sfd, 0L, 1L) == -1));
}

static void near _SquishUnlock(MSG * sq)
{
    if (mi.haveshare)
    {
        unlock(Sqd->sfd, 0L, 1L);
    }
}
