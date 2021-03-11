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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <huskylib/compiler.h>

#ifdef __WATCOMC__
#include <unistd.h>
#endif

#ifdef HAS_IO_H
#include <io.h>
#endif

#ifdef HAS_SHARE_H
#include <share.h>
#endif

#ifdef HAS_DOS_H
#include <dos.h>
#endif

#ifdef HAS_MALLOC_H
#include <malloc.h>
#endif

#ifdef __BEOS__
#include <be/kernel/fs_attr.h>
#include <be/support/TypeConstants.h>
#endif

#define MSGAPI_HANDLERS

#include <huskylib/huskylib.h>
/* Swith for build DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "msgapi.h"
#include "old_msg.h"
#include "api_sdm.h"
#include "api_sdmp.h"
#include "apidebug.h"

#define SDM_BLOCK 256     /* Should be exp2(x) */
#define Mhd ((struct _sdmdata *)(mh->apidata))
#define MsghMhd ((struct _sdmdata *)(((struct _msgh *)msgh)->sq->apidata))

static byte * hwm_from = (byte *)"-=|smapi internal|=-";
MSGA * MSGAPI SdmOpenArea(byte * name, word mode, word type)
{
    MSGA * mh;

    if(!name || !*name)
    {
        msgapierr = MERR_BADNAME;
        return NULL;
    }

    mh = palloc(sizeof(MSGA));

    if(mh == NULL)
    {
        msgapierr = MERR_NOMEM;
        goto ErrOpen;
    }

    memset(mh, '\0', sizeof(MSGA));
    mh->id = MSGAPI_ID;

    if(type & MSGTYPE_ECHO)
    {
        mh->isecho = TRUE;
    }

    mh->api = (struct _apifuncs *)palloc(sizeof(struct _apifuncs));

    if(mh->api == NULL)
    {
        msgapierr = MERR_NOMEM;
        goto ErrOpen;
    }

    memset(mh->api, '\0', sizeof(struct _apifuncs));
    mh->apidata = (void *)palloc(sizeof(struct _sdmdata));

    if(mh->apidata == NULL)
    {
        msgapierr = MERR_NOMEM;
        goto ErrOpen;
    }

    memset((byte *)mh->apidata, '\0', sizeof(struct _sdmdata));
    strcpy((char *)Mhd->base, (char *)name);
    Add_Trailing((char *)Mhd->base, PATH_DELIM);
    Mhd->hwm       = (dword) - 1L;
    mh->len        = sizeof(MSGA);
    mh->num_msg    = 0;
    mh->high_msg   = 0;
    mh->high_water = (dword) - 1L;

    if(!direxist((char *)name) &&
       (mode == MSGAREA_NORMAL || _createDirectoryTree((char *)name) != 0))
    {
        msgapierr = MERR_NOENT;
        goto ErrOpen;
    }

    if(!_SdmRescanArea(mh))
    {
        goto ErrOpen;
    }

    mh->type   &= ~MSGTYPE_ECHO;
    *mh->api    = sdm_funcs;
    mh->sz_xmsg = sizeof(XMSG);
    msgapierr   = 0;
    return mh;

ErrOpen:

    if(mh)
    {
        if(mh->api)
        {
            if(mh->apidata)
            {
                pfree(mh->apidata);
            }

            pfree(mh->api);
        }

        pfree(mh);
    }

    return NULL;
} /* SdmOpenArea */

int SdmDeleteBase(char * name)
{
    FFIND * ff;
    char * temp;

    if(!name || !*name)
    {
        msgapierr = MERR_BADNAME;
        return FALSE;
    }

    temp = (char *)malloc(strlen(name) + 6);

    if(temp == NULL)
    {
        return FALSE;
    }

    sprintf(temp, "%s*.msg", name);
    ff = FFindOpen(temp, 0);
    nfree(temp);

    if(ff != 0)
    {
        do
        {
            temp = (char *)malloc(strlen(name) + strlen(ff->ff_name) + 1);

            if(temp == NULL)
            {
                FFindClose(ff);
                return FALSE;
            }

            sprintf(temp, "%s%s", name, ff->ff_name);
            unlink(temp);
            nfree(temp);
        }
        while(FFindNext(ff) == 0);
        FFindClose(ff);
    }

    temp = (char*)malloc(strlen(name) + 10);
    if (temp == NULL)
    {
        return FALSE;
    }
    sprintf(temp, "%slastread", name);
    unlink(temp);
    nfree(temp);
    rmdir(name);
    return TRUE; /* rmdir error is ok */
} /* SdmDeleteBase */

static sword _XPENTRY SdmCloseArea(MSGA * mh)
{
    XMSG msg;
    MSGH * msgh;
    static byte * msgbody =
        (byte *)"NOECHO\r\rPlease ignore.  This message is only used by the SquishMail "
                "system to store\rthe high water mark for each conference area.\r\r\r\r"
                "\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r(Elvis was here!)\r\r\r";

    if(InvalidMh(mh))
    {
        return -1;
    }

    if(Mhd->hwm_chgd)
    {
        msgh = SdmOpenMsg(mh, MOPEN_CREATE, 1L);

        if(msgh != NULL)
        {
            dword temp;
            Init_Xmsg(&msg);
            Get_Dos_Date((union stamp_combo *)&msg.date_arrived);
            Get_Dos_Date((union stamp_combo *)&msg.date_written);
            /*
             *  Use high-bit chars in the to/from field, so that (l)users
             *  can't log on as this userid and delete the HWM.
             */
            strcpy((char *)msg.from, (char *)hwm_from);
            strcpy((char *)msg.to, (char *)msg.from);
            strcpy((char *)msg.subj, "High wadda' mark");
            /* To prevent "INTL 0:0/0 0:0/0" kludges */
            msg.orig.zone = msg.dest.zone = mi.def_zone;
            msg.replyto   = mh->high_water;
            msg.attr      = MSGPRIVATE | MSGREAD | MSGLOCAL | MSGSENT;
            temp = (dword)(strlen((char*)msgbody) + 1);
            SdmWriteMsg(msgh, FALSE, &msg, msgbody, temp, temp, 0L, NULL);
            SdmCloseMsg(msgh);
        }
    }

    if(Mhd->msgs_open)
    {
        msgapierr = MERR_EOPEN;
        return -1;
    }

    if(Mhd->msgnum)
    {
        pfree(Mhd->msgnum);
    }

    pfree(mh->apidata);
    pfree(mh->api);
    mh->id = 0L;
    pfree(mh);
    msgapierr = MERR_NONE;
    return 0;
} /* SdmCloseArea */

static MSGH * _XPENTRY SdmOpenMsg(MSGA * mh, word mode, dword msgnum)
{
    byte msgname[PATHLEN];
    int handle, filemode;
    int owrite   = FALSE;
    dword msguid = 0;
    MSGH * msgh;

    if(InvalidMh(mh))
    {
        return NULL;
    }

    if(msgnum == MSGNUM_CUR)
    {
        msgnum = mh->cur_msg;
    }
    else if(msgnum == MSGNUM_PREV)
    {
        msgnum = mh->cur_msg - 1;

        if(msgnum == 0)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
    }
    else if(msgnum == MSGNUM_NEXT)
    {
        msgnum = mh->num_msg + 1;

        if(msgnum > mh->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
    }
    else if(mode != MOPEN_CREATE)
    {
        /*
         *  If we're not creating, make sure that the specified msg# can
         *  be found.
         */
        if(msgnum == 0 || msgnum > mh->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
    }

    if(msgnum <= mh->num_msg && msgnum > 0)
    {
        msguid = SdmMsgnToUid(mh, msgnum);
    }

    if(mode == MOPEN_CREATE)
    {
        /* If we're creating a new message... */
        if(msgnum == 0L)
        {
            /*
             *  If the base isn't locked, make sure that we avoid
             *  conflicts...
             */
            if(!mh->locked)
            {
                /* Check to see if the msg we're writing already exists */
                sprintf((char *)msgname,
                        (char *)sd_msg,
                        Mhd->base,
                        (sdm_msgnum_type)mh->high_msg + 1);

                if(fexist((char *)msgname))
                {
                    /* If so, rescan the base, to find out which msg# it is. */
                    if(Mhd->msgnum && Mhd->msgnum_len)
                    {
                        pfree(Mhd->msgnum);
                    }

                    if(!_SdmRescanArea(mh))
                    {
                        return NULL;
                    }
                }
            }

            msgnum = ++mh->num_msg;
            msguid = ++mh->high_msg;

            /*
             *  Make sure that we don't overwrite the high-water mark,
             *  unless we call with msgnum != 0L (a specific number).
             */
            if(mh->isecho && msgnum == 1)
            {
                msgnum = mh->high_msg = 2;
            }
        }
        else
        {
            /* otherwise, we're overwriting an existing message */
            owrite = TRUE;
        }

        filemode = O_CREAT | O_TRUNC | O_RDWR;
    }
    else if(mode == MOPEN_READ)
    {
        filemode = O_RDONLY;
    }
    else if(mode == MOPEN_WRITE)
    {
        filemode = O_WRONLY;
    }
    else
    {
        filemode = O_RDWR;
    }

    sprintf((char *)msgname, (char *)sd_msg, Mhd->base, (int)msguid);
    handle = sopen((char *)msgname, filemode | O_BINARY, SH_DENYNONE, FILEMODE(mh->isecho));

    if(handle == -1)
    {
        if(filemode & O_CREAT)
        {
            msgapierr = MERR_BADF;
        }
        else
        {
            msgapierr = MERR_NOENT;
        }

        return NULL;
    }

    mh->cur_msg = msgnum;
    msgh        = palloc(sizeof(MSGH));

    if(msgh == NULL)
    {
        close(handle);
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    memset(msgh, '\0', sizeof(MSGH));
    msgh->fd = handle;

    if(mode == MOPEN_CREATE)
    {
        if(mh->num_msg == (dword)(1 << (sizeof(mh->num_msg) * 8 - 1))) /* Messagebase
                                                                          implementaion (size)
                                                                          limit (messages
                                                                          counter is full)*/
        {
            pfree(msgh);
            close(handle);
            msgapierr = MERR_LIMIT;
            return NULL;
        }

        if((mh->num_msg + 1) >= Mhd->msgnum_len)
        {
            unsigned int * temp;
            word msgnum_len_new = Mhd->msgnum_len + (word)SDM_BLOCK;
            temp = realloc(Mhd->msgnum, msgnum_len_new * sizeof(unsigned));
            if(!temp)
            {
                pfree(Mhd->msgnum);
                pfree(msgh);
                close(handle);
                msgapierr = MERR_NOMEM;
                return NULL;
            }
            Mhd->msgnum = temp;

            Mhd->msgnum_len = msgnum_len_new;
        }

        /*
         *  If we're writing a new msg, this is easy -- just add to
         *  end of list.
         */
        if(!owrite)
        {
            Mhd->msgnum[(size_t)(mh->num_msg)] = (word)msguid;
            mh->num_msg++;
        }
        else
        {
            /*
             *  If this message is already in the list then do nothing --
             *  simply overwrite it, keeping the same message number, so
             *  no action is required.  Otherwise, we have to shift
             *  everything up by one since we're adding this new message
             *  in between two others.
             */
            if((dword)Mhd->msgnum[msgnum - 1] != msguid)
            {
                memmove(Mhd->msgnum + msgnum, Mhd->msgnum + msgnum - 1,
                        ((size_t)mh->num_msg - msgnum) * sizeof(Mhd->msgnum[0]));
                Mhd->msgnum[msgnum - 1] = (word)msguid;
                mh->num_msg++;
            }
        }
    }

    msgh->cur_pos = 0L;

    if(mode == MOPEN_CREATE)
    {
        msgh->msg_len = 0;
    }
    else
    {
        msgh->msg_len = (dword) - 1;
    }

    msgh->sq    = mh;
    msgh->id    = MSGH_ID;
    msgh->ctrl  = NULL;
    msgh->clen  = -1;
    msgh->zplen = 0;
    msgapierr   = MERR_NONE;

    /* Keep track of how many messages were opened for this area */
    MsghMhd->msgs_open++;
    return msgh;
} /* SdmOpenMsg */

static sword _XPENTRY SdmCloseMsg(MSGH * msgh)
{
    if(InvalidMsgh(msgh))
    {
        return -1;
    }

    MsghMhd->msgs_open--;

    if(msgh->ctrl)
    {
        pfree(msgh->ctrl);
    }

    close(msgh->fd);
    msgh->id = 0L;
    pfree(msgh);
    msgapierr = MERR_NONE;
    return 0;
}

static dword _XPENTRY SdmReadMsg(MSGH * msgh,
                                 XMSG * msg,
                                 dword offset,
                                 dword bytes,
                                 byte * text,
                                 dword clen,
                                 byte * ctxt)
{
    unsigned len;
    dword realbytes, got;
    struct _omsg fmsg;
    word need_ctrl;
    byte * fake_msgbuf = NULL, * newtext;

    if(InvalidMsgh(msgh))
    {
        return (dword) - 1L;
    }

    if(!(clen && ctxt))
    {
        clen = 0L;
        ctxt = NULL;
    }

    if(!(text && bytes))
    {
        bytes = 0L;
        text  = NULL;
    }

    if(msg)
    {
        lseek(msgh->fd, 0L, SEEK_SET);

        if(!read_omsg(msgh->fd, &fmsg))
        {
            msgapierr = MERR_BADF;
            return (dword) - 1L;
        }

        fmsg.to[sizeof(fmsg.to) - 1]     = '\0';
        fmsg.from[sizeof(fmsg.from) - 1] = '\0';
        fmsg.subj[sizeof(fmsg.subj) - 1] = '\0';
        fmsg.date[sizeof(fmsg.date) - 1] = '\0';
        Convert_Fmsg_To_Xmsg(&fmsg, msg, mi.def_zone);
        StripNasties(msg->from);
        StripNasties(msg->to);
        StripNasties(msg->subj);
    }

    /*
     *  If we weren't instructed to read some message text (ie. only the
     *  header, read a block anyway.  We need to scan for kludge lines,
     *  to pick out the appropriate zone/point info.)
     */
    if(msgh->ctrl == NULL
#if 0
       /* the following is always true */
       && ((msg || ctxt || text) || (msg || ctxt || text) == 0)
#endif
      )
    {
        need_ctrl = TRUE;
    }
    else
    {
        need_ctrl = FALSE;
    }

    realbytes = bytes;
    unused(realbytes);

    /*
     *  If we need to read the control information, and the user hasn't
     *  requested a read operation, we'll need to do one anyway.
     */
    if(need_ctrl && text == NULL)
    {
        struct stat st;
        fstat(msgh->fd, &st);
        text = fake_msgbuf = (byte *)palloc((size_t)st.st_size - OMSG_SIZE + 1);

        if(text == NULL)
        {
            msgapierr = MERR_NOMEM;
            return (dword) - 1L;
        }

        text[st.st_size - OMSG_SIZE] = '\0';
        bytes = st.st_size - OMSG_SIZE;
    }

    /* If we need to read in some text... */
    if(text)
    {
        /* Seek is superfluous if we just read msg header */
        if(!msg || msgh->msgtxt_start != 0)
        {
            lseek(msgh->fd, (dword)OMSG_SIZE + msgh->msgtxt_start + offset, SEEK_SET);
            msgh->cur_pos = offset;
        }

        got = (dword)farread(msgh->fd, text, (unsigned int)bytes);
        text[(unsigned int)got] = '\0';

        /*
         *  Update counter only if we got some text, and only if we're
         *  doing a read requested by the user (as opposed to reading
         *  ahead to find kludge lines).
         */
        if(got > 0 && !fake_msgbuf)
        {
            msgh->cur_pos += got;
        }
    }
    else
    {
        got = 0;
    }

    /* Convert the kludges into 'ctxt' format */
    if(need_ctrl && got && offset == 0L)
    {
        len        = got;
        msgh->ctrl = CopyToControlBuf(text, &newtext, &len);

        if(msgh->ctrl != NULL)
        {
            msgh->clen         = (dword)strlen((char *)msgh->ctrl) + 1;
            msgh->msgtxt_start = (dword)(newtext - text);
            /* Shift back the text buffer to counter absence of ^a strings */
            memmove(text, newtext, (size_t)((ptrdiff_t)bytes - (newtext - text)));
            got -= (dword)(msgh->clen - 1);
        }
    }

    /* Scan the ctxt ourselves to find zone/point info */
    if(msg && msgh->ctrl)
    {
        ConvertControlInfo(msgh->ctrl, &msg->orig, &msg->dest);
    }

    /* And if the app requested ctrlinfo, put it in its place. */
    if(ctxt && msgh->ctrl)
    {
        size_t slen;
        slen = strlen((char *)msgh->ctrl) + 1;
        memmove(ctxt, msgh->ctrl, min(slen, (size_t)clen));
        ctxt[min(slen, (size_t)clen)] = '\0';
    }

    if(fake_msgbuf)
    {
        pfree(fake_msgbuf);
        got = 0;
    }

    msgapierr = MERR_NONE;
    return got;
} /* SdmReadMsg */

static sword _XPENTRY SdmWriteMsg(MSGH * msgh,
                                  word append,
                                  XMSG * msg,
                                  byte * text,
                                  dword textlen,
                                  dword totlen,
                                  dword clen,
                                  byte * ctxt)
{
    struct _omsg fmsg;
    byte * s;

    unused(totlen);

    if(clen == 0L || ctxt == NULL)
    {
        ctxt = NULL;
        clen = 0L;
    }

    if(InvalidMsgh(msgh))
    {
        return -1;
    }

    lseek(msgh->fd, 0L, SEEK_SET);

    if(msg)
    {
        Convert_Xmsg_To_Fmsg(msg, &fmsg);

        if(!write_omsg(msgh->fd, &fmsg))
        {
            msgapierr = MERR_NODS;
            return -1;
        }

        if(!append && msgh->clen <= 0 && msgh->zplen == 0 && !msgh->sq->isecho)
        {
            statfd      = msgh->fd;
            msgh->zplen = (word)WriteZPInfo(msg, WriteToFd, ctxt);
        }

        /* Use Attributes und BeOS */
#ifdef __BEOS__
        {
            struct tm tmdate;
            time_t ttime;
            fs_write_attr(msgh->fd, "BEOS:TYPE", B_MIME_TYPE, 0l, "message/fmsg", 13);
            fs_write_attr(msgh->fd, "XMSG:FROM", B_STRING_TYPE, 0l, msg->from,
                          strlen((char *)msg->from));
            fs_write_attr(msgh->fd, "XMSG:TO", B_STRING_TYPE, 0l, msg->to,
                          strlen((char *)msg->to));
            fs_write_attr(msgh->fd, "XMSG:SUBJ", B_STRING_TYPE, 0l, msg->subj,
                          strlen((char *)msg->subj));
            ttime = mktime(DosDate_to_TmDate((union stamp_combo *)&msg->date_written, &tmdate));
            fs_write_attr(msgh->fd, "XMSG:DATE", B_TIME_TYPE, 0l, &ttime, 4l);
            ttime = mktime(DosDate_to_TmDate((union stamp_combo *)&msg->date_arrived, &tmdate));
            fs_write_attr(msgh->fd, "XMSG:DTAR", B_TIME_TYPE, 0l, &ttime, 4l);
            /* ... and so on ... not fully implemented ! (Yet) */
        }
#endif
    }
    else if(!append || ctxt)
    {
        /* Skip over old message header */
        lseek(msgh->fd, (dword)OMSG_SIZE + (dword)msgh->zplen, SEEK_SET);
    }

    /* Now write the control info / kludges */
    if(clen && ctxt)
    {
        if(!msg)
        {
            lseek(msgh->fd, (dword)OMSG_SIZE + (dword)msgh->zplen, SEEK_SET);
        }

        s = CvtCtrlToKludge(ctxt);

        if(s)
        {
            unsigned sl_s = (unsigned)strlen((char *)s);
            int ret;
            ret = farwrite(msgh->fd, s, sl_s);
            pfree(s);

            if(ret != (int)sl_s)
            {
                msgapierr = MERR_NODS;
                return -1;
            }
        }
    }

    if(append)
    {
        lseek(msgh->fd, 0L, SEEK_END);
    }

    if(text)
    {
        if(farwrite(msgh->fd, text, (unsigned)textlen) != (signed)textlen)
        {
            msgapierr = MERR_NODS;
            return -1;
        }

        if(text[textlen])
        {
            if(farwrite(msgh->fd, "", 1) != 1)
            {
                msgapierr = MERR_NODS;
                return -1;
            }
        }
    }

    msgapierr = MERR_NONE;
    return 0;
} /* SdmWriteMsg */

static sword _XPENTRY SdmKillMsg(MSGA * mh, dword msgnum)
{
    dword hwm;
    byte temp[PATHLEN];
    UMSGID msguid;

    if(InvalidMh(mh))
    {
        return -1;
    }

    if(msgnum > mh->num_msg || msgnum == 0)
    {
        msgapierr = MERR_NOENT;
        return -1;
    }

    msguid = SdmMsgnToUid(mh, msgnum);

    /* Remove the message number from our private index */
    memmove(Mhd->msgnum + msgnum - 1, Mhd->msgnum + msgnum,
            (size_t)(mh->num_msg - msgnum) * sizeof(Mhd->msgnum[0]));
    /* If we couldn't find it, return an error message */
    sprintf((char *)temp, (char *)sd_msg, Mhd->base, (unsigned int)msguid);

    if(unlink((char *)temp) == -1)
    {
        msgapierr = MERR_NOENT;
        return -1;
    }

    mh->num_msg--;

    /* Adjust the high message number */
    if(msguid == mh->high_msg)
    {
        if(mh->num_msg)
        {
            mh->high_msg = SdmMsgnToUid(mh, mh->num_msg);
        }
        else
        {
            mh->high_msg = 0;
        }
    }

    /* Now adjust the high-water mark, if necessary */
    hwm = SdmGetHighWater(mh);

    if(hwm != (dword) - 1 && hwm > 0 && hwm >= msgnum)
    {
        SdmSetHighWater(mh, msgnum - 1);
    }

    if(mh->cur_msg >= msgnum)
    {
        mh->cur_msg--;
    }

    msgapierr = MERR_NONE;
    return 0;
} /* SdmKillMsg */

/* This function is never used */
static sword _XPENTRY SdmLock(MSGA * mh)
{
    if(InvalidMh(mh))
    {
        return -1;
    }

    msgapierr = MERR_NONE;
    return 0;
}

/* This function is never used */
static sword _XPENTRY SdmUnlock(MSGA * mh)
{
    if(InvalidMh(mh))
    {
        return -1;
    }

    msgapierr = MERR_NONE;
    return 0;
}

sword MSGAPI SdmValidate(byte * name)
{
    msgapierr = MERR_NONE;
    return (sword)(direxist((char *)name) != FALSE);
}

static sword _XPENTRY SdmSetCurPos(MSGH * msgh, dword pos)
{
    if(InvalidMsgh(msgh))
    {
        return 0;
    }

    lseek(msgh->fd, msgh->cur_pos = pos, SEEK_SET);
    msgapierr = MERR_NONE;
    return 0;
}

static dword _XPENTRY SdmGetCurPos(MSGH * msgh)
{
    if(InvalidMsgh(msgh))
    {
        return (dword) - 1L;
    }

    msgapierr = MERR_NONE;
    return msgh->cur_pos;
}

static UMSGID _XPENTRY SdmMsgnToUid(MSGA * mh, dword msgnum)
{
    if(InvalidMh(mh))
    {
        return (UMSGID)-1;
    }

    msgapierr = MERR_NONE;

    if(msgnum > mh->num_msg)
    {
        return (UMSGID)-1;
    }

    if(msgnum == 0)
    {
        return 0;
    }

    return (UMSGID)Mhd->msgnum[msgnum - 1];
}

static dword _XPENTRY SdmUidToMsgn(MSGA * mh, UMSGID umsgid, word type)
{
    dword left, right, nnew;
    UMSGID umsg;

    if(InvalidMh(mh))
    {
        return (dword) - 1L;
    }

    if(umsgid == 0)
    {
        return 0;
    }

    left  = 1;
    right = mh->num_msg;

    while(left <= right)
    {
        nnew = (right + left) / 2;
        umsg = SdmMsgnToUid(mh, nnew);

        if(umsg == (UMSGID)-1)
        {
            return 0;
        }

        if(umsg < umsgid)
        {
            left = nnew + 1;
        }
        else if(umsg > umsgid)
        {
            if(nnew > 0)
            {
                right = nnew - 1;
            }
            else
            {
                right = 0;
            }
        }
        else
        {
            return nnew;
        }
    }

    if(type == UID_EXACT)
    {
        return 0;
    }

    if(type == UID_PREV)
    {
        return right;
    }

    return (left > mh->num_msg) ? mh->num_msg : left;
} /* SdmUidToMsgn */

static dword _XPENTRY SdmGetHighWater(MSGA * mh)
{
    MSGH * msgh;
    XMSG msg;

    if(InvalidMh(mh))
    {
        return (dword) - 1L;
    }

    /* If we've already fetched the highwater mark... */
    if(mh->high_water != (dword) - 1L)
    {
        return SdmUidToMsgn(mh, mh->high_water, UID_PREV);
    }

    msgh = SdmOpenMsg(mh, MOPEN_READ, 1L);

    if(msgh == NULL)
    {
        return 0L;
    }

    if(SdmReadMsg(msgh, &msg, 0L, 0L, NULL, 0L,
                  NULL) == (dword) - 1 || (strcmp((char *)msg.from, (char *)hwm_from) != 0))
    {
        mh->high_water = 0L;
    }
    else
    {
        mh->high_water = (dword)msg.replyto;
    }

    SdmCloseMsg(msgh);
    return SdmUidToMsgn(mh, mh->high_water, UID_PREV);
} /* SdmGetHighWater */

static sword _XPENTRY SdmSetHighWater(MSGA * mh, dword hwm)
{
    if(InvalidMh(mh))
    {
        return -1;
    }

    /*
     *  Only write it to memory for now.  We'll do a complete update of
     *  the real HWM in 1.MSGA only when doing a MsgCloseArea(), to save
     *  time.
     */
    if(hwm != mh->high_water)
    {
        Mhd->hwm_chgd = TRUE;
    }

    mh->high_water = hwm;
    return 0;
}

static dword _XPENTRY SdmGetTextLen(MSGH * msgh)
{
    dword pos, end;

    if(InvalidMsgh(msgh))
    {
        return (dword) - 1L;
    }

    /* Figure out the physical length of the message */
    if(msgh->msg_len == -1)
    {
        pos = (dword)tell(msgh->fd);
        end = (dword)lseek(msgh->fd, 0L, SEEK_END);

        if(end < OMSG_SIZE)
        {
            msgh->msg_len = 0L;
        }
        else
        {
            msgh->msg_len = end - (dword)OMSG_SIZE;
        }

        lseek(msgh->fd, pos, SEEK_SET);
    }

    /* If we've already figured out the length of the control info */
    if((msgh->clen == -1) && (_Grab_Clen(msgh) == -1))
    {
        return 0;
    }
    else
    {
        return (dword)(msgh->msg_len - msgh->msgtxt_start);
    }
} /* SdmGetTextLen */

static dword _XPENTRY SdmGetCtrlLen(MSGH * msgh)
{
    if(InvalidMsgh(msgh))
    {
        return (dword) - 1L;
    }

    /* If we've already figured out the length of the control info */
    if((msgh->clen == -1) && (_Grab_Clen(msgh) == -1))
    {
        return 0;
    }
    else
    {
        return (dword)msgh->clen;
    }
}

static sword near _Grab_Clen(MSGH * msgh)
{
    if((sdword)SdmReadMsg(msgh, NULL, 0L, 0L, NULL, 0L, NULL) < (sdword)0)
    {
        return (sword) - 1;
    }
    else
    {
        return (sword)0;
    }
}

static sword near _SdmRescanArea(MSGA * mh)
{
    FFIND * ff;
    char * temp;
    word mn, thismsg;

    if(InvalidMh(mh))
    {
        return -1;
    }

    mh->num_msg = 0;
    Mhd->msgnum = palloc(SDM_BLOCK * sizeof(unsigned));

    if(Mhd->msgnum == NULL)
    {
        msgapierr = MERR_NOMEM;
        return FALSE;
    }

    Mhd->msgnum_len = SDM_BLOCK;
    temp            = malloc(strlen((char *)Mhd->base) + 6);

    if(temp == NULL)
    {
        return -1;
    }

    sprintf((char *)temp, "%s*.msg", Mhd->base);
    ff = FFindOpen((char *)temp, 0);
    nfree(temp);

    if(ff != 0)
    {
        mn = 0;

        do
        {
            /* Don't count zero-length or invalid messages */

#ifndef __UNIX__

            if(ff->ff_fsize < OMSG_SIZE)
            {
                continue;
            }

#endif

            if(mh->num_msg == (dword)(1 << (sizeof(mh->num_msg) * 8 - 1))) /* Messagebase
                                                                              implementaion
                                                                              (size) limit
                                                                              (messages counter
                                                                              is full)*/
            {
                msgapierr = MERR_LIMIT;
                return FALSE;
            }

            if(mn >= Mhd->msgnum_len)
            {
                unsigned int * tmp;
                word msgnum_len_new = Mhd->msgnum_len + (word)SDM_BLOCK;
                tmp = (unsigned int *)realloc(Mhd->msgnum,
                                               msgnum_len_new * sizeof(unsigned));
                if(!tmp)
                {
                    pfree(Mhd->msgnum);
                    msgapierr = MERR_NOMEM;
                    return FALSE;
                }
                Mhd->msgnum = tmp;

                Mhd->msgnum_len = msgnum_len_new;
            }

            thismsg = (word)atoi(ff->ff_name);

            if(thismsg != 0)
            {
                Mhd->msgnum[mn++] = thismsg;

                if((dword)thismsg > mh->high_msg)
                {
                    mh->high_msg = (dword)thismsg;
                }

                mh->num_msg = (dword)mn;
            }

#ifdef __OS2__

            if((mn % 128) == 127)
            {
                tdelay(1L); /* give up cpu */
            }

#endif
        }
        while(FFindNext(ff) == 0);
        FFindClose(ff);
        /* Now sort the list of messages */
        qksort((int *)Mhd->msgnum, (word)mh->num_msg);
    }

    return TRUE;
} /* _SdmRescanArea */

static void MSGAPI Init_Xmsg(XMSG * msg)
{
    if(InvalidMsg(msg))
    {
        return;
    }

    memset(msg, '\0', sizeof(XMSG));
}

static void MSGAPI Convert_Fmsg_To_Xmsg(struct _omsg * fmsg, XMSG * msg, word def_zone)
{
    NETADDR * orig, * dest;

    if(!fmsg)
    {
        msgapierr = MERR_BADH;
        return;
    }

    if(InvalidMsg(msg))
    {
        return;
    }

    Init_Xmsg(msg);
    orig = &msg->orig;
    dest = &msg->dest;
    fmsg->to[sizeof(fmsg->to) - 1]     = '\0';
    fmsg->from[sizeof(fmsg->from) - 1] = '\0';
    fmsg->subj[sizeof(fmsg->subj) - 1] = '\0';
    fmsg->date[sizeof(fmsg->date) - 1] = '\0';
    strcpy((char *)msg->from, (char *)fmsg->from);
    strcpy((char *)msg->to, (char *)fmsg->to);
    strcpy((char *)msg->subj, (char *)fmsg->subj);
    orig->zone  = dest->zone = def_zone;
    orig->point = dest->point = 0;
    orig->net   = fmsg->orig_net;
    orig->node  = fmsg->orig;
    dest->net   = fmsg->dest_net;
    dest->node  = fmsg->dest;
    Get_Binary_Date(&msg->date_written, &fmsg->date_written, fmsg->date);
    Get_Binary_Date(&msg->date_arrived, &fmsg->date_arrived, fmsg->date);
    strcpy((char *)msg->__ftsc_date, (char *)fmsg->date);
    msg->utc_ofs     = 0;
    msg->replyto     = fmsg->reply;
    msg->replies[0]  = fmsg->up;
    msg->attr        = (dword)fmsg->attr;
    msg->xmtimesread = fmsg->times;
    msg->xmcost      = fmsg->cost;
} /* Convert_Fmsg_To_Xmsg */

static void MSGAPI Convert_Xmsg_To_Fmsg(XMSG * msg, struct _omsg * fmsg)
{
    NETADDR * orig, * dest;

    if(!fmsg)
    {
        msgapierr = MERR_BADH;
        return;
    }

    if(InvalidMsg(msg))
    {
        return;
    }

    memset(fmsg, '\0', sizeof(struct _omsg));
    orig = &msg->orig;
    dest = &msg->dest;
    strncpy((char *)fmsg->from, (char *)msg->from, sizeof(fmsg->from));
    strncpy((char *)fmsg->to, (char *)msg->to, sizeof(fmsg->to));
    strncpy((char *)fmsg->subj, (char *)msg->subj, sizeof(fmsg->subj));
    fmsg->from[sizeof(fmsg->from) - 1] = '\0';
    fmsg->to[sizeof(fmsg->to) - 1]     = '\0';
    fmsg->subj[sizeof(fmsg->subj) - 1] = '\0';
    fmsg->orig_net = orig->net;
    fmsg->orig     = orig->node;
    fmsg->dest_net = dest->net;
    fmsg->dest     = dest->node;

    if(*msg->__ftsc_date)
    {
        strncpy((char *)fmsg->date, (char *)msg->__ftsc_date, sizeof(fmsg->date));
        fmsg->date[sizeof(fmsg->date) - 1] = '\0';
    }
    else
    {
        sprintf((char *)fmsg->date,
                "%02d %s %02d  %02d:%02d:%02d",
                msg->date_written.date.da ? msg->date_written.date.da : 1,
                months_ab[msg->date_written.date.mo ? msg->date_written.date.mo - 1 : 0],
                (msg->date_written.date.yr + 80) % 100,
                msg->date_written.time.hh,
                msg->date_written.time.mm,
                msg->date_written.time.ss << 1);
    }

    fmsg->date_written = msg->date_written;
    fmsg->date_arrived = msg->date_arrived;
    fmsg->reply        = (word)msg->replyto;
    fmsg->up           = (word)msg->replies[0];
    fmsg->attr         = (word)(msg->attr & 0xffffL);
    fmsg->times        = (word)msg->xmtimesread;
    fmsg->cost         = (word)msg->xmcost;
} /* Convert_Xmsg_To_Fmsg */

int _XPENTRY WriteZPInfo(XMSG * msg, void(_stdc * wfunc)(byte * str), byte * kludges)
{
    byte temp[PATHLEN], * null = (byte *)"";
    int bytes = 0;

    if(!wfunc)
    {
        msgapierr = MERR_BADH;
        return -1;
    }

    if(InvalidMsg(msg))
    {
        return -1;
    }

    if(!kludges)
    {
        kludges = null;
    }

    if((msg->dest.zone != mi.def_zone || msg->orig.zone != mi.def_zone) &&
       !strstr((char *)kludges, "\001INTL"))
    {
        sprintf((char *)temp,
                "\001INTL %hu:%hu/%hu %hu:%hu/%hu\r",
                msg->dest.zone,
                msg->dest.net,
                msg->dest.node,
                msg->orig.zone,
                msg->orig.net,
                msg->orig.node);
        (*wfunc)(temp);
        bytes += (int)strlen((char *)temp);
    }

    if(msg->orig.point && !strstr((char *)kludges, "\001" "FMPT"))
    {
        sprintf((char *)temp, "\001" "FMPT %hu\r", msg->orig.point);
        (*wfunc)(temp);
        bytes += (int)strlen((char *)temp);
    }

    if(msg->dest.point && !strstr((char *)kludges, "\001" "TOPT"))
    {
        sprintf((char *)temp, "\001" "TOPT %hu\r", msg->dest.point);
        (*wfunc)(temp);
        bytes += (int)strlen((char *)temp);
    }

    return bytes;
} /* WriteZPInfo */

static void _stdc WriteToFd(byte * str)
{
    if(str && *str)
    {
        if(0 > farwrite(statfd, str, (unsigned int)strlen((char *)str)))
        {
            msgapierr = MERR_BADF;
        }
    }
    else
    {
        msgapierr = MERR_BADH;
    }
}

static void near Get_Binary_Date(struct _stamp * todate, struct _stamp * fromdate,
                                 byte * asciidate)
{
    if(!todate || !fromdate || !asciidate)
    {
        msgapierr = MERR_BADH;
        return;
    }

    if(fromdate->date.da == 0 || fromdate->date.da > 31 || fromdate->date.yr > 50 ||
       fromdate->time.hh > 23 || fromdate->time.mm > 59 || fromdate->time.ss > 59 ||
       ((union stamp_combo *)(void *)&fromdate)->ldate == 0)
    {
        ASCII_Date_To_Binary((char *)asciidate, (union stamp_combo *)todate);
    }
    else
    {
        *todate = *fromdate;
    }
}

static dword _XPENTRY SdmGetHash(HAREA mh, dword msgnum)
{
    XMSG xmsg;
    HMSG msgh;
    dword rc = 0l;

    msgh = SdmOpenMsg(mh, MOPEN_READ, msgnum);

    if(msgh == NULL)
    {
        return (dword)0l;
    }

    if(SdmReadMsg(msgh, &xmsg, 0L, 0L, NULL, 0L, NULL) != (dword) - 1)
    {
        rc = SquishHash(xmsg.to) | (xmsg.attr & MSGREAD) ? 0x80000000l : 0;
    }

    SdmCloseMsg(msgh);
    msgapierr = MERR_NONE;
    return rc;
}

static UMSGID _XPENTRY SdmGetNextUid(HAREA ha)
{
    if(InvalidMh(ha))
    {
        return 0L;
    }

    if(!ha->locked)
    {
        msgapierr = MERR_NOLOCK;
        return 0L;
    }

    msgapierr = MERR_NONE;
    return ha->high_msg + 1;
}
