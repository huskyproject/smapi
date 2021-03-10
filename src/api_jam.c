/*
** Adapted for MSGAPI by Fedor Lizunkov 2:5020/960@FidoNet
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

#include <huskylib/compiler.h>

#ifdef HAS_IO_H
#include <io.h>
#endif

#ifdef HAS_SHARE_H
#include <share.h>
#endif

#ifdef HAS_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAS_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAS_STRINGS_H
#include <strings.h>
#endif

#define MSGAPI_HANDLERS

#include <huskylib/huskylib.h>
/* Swith for build DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>
#include <huskylib/strext.h>

#include "msgapi.h"
#include "api_jam.h"
#include "api_jamp.h"
#include "apidebug.h"

#define Jmd ((JAMBASE *)(jm->apidata))
#define MsghJm ((JAMBASE *)(((struct _msgh *)msgh)->sq->apidata))

#ifdef __TURBOC__
#pragma warn -pia*
#pragma warn -ucp
#pragma warn -sig
#endif

#define NOTH 3

static JAMBASE * jbOpen = NULL;
/* Free's up a SubField-Chain */
static void freejamsubfield(JAMSUBFIELD2LIST * subfield)
{
    if(subfield)
    {
        pfree(subfield);
    }
}

MSGA * MSGAPI JamOpenArea(byte * name, word mode, word type)
{
    MSGA * jm = NULL;
    size_t len;

    if(!name || !*name)
    {
        msgapierr = MERR_BADNAME;
        return NULL;
    }

    jm = palloc(sizeof(MSGA));

    if(jm == NULL)
    {
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    memset(jm, '\0', sizeof(MSGA));
    jm->id = MSGAPI_ID;

    if(type & MSGTYPE_ECHO)
    {
        jm->isecho = TRUE;
    }

    if(type & MSGTYPE_NOTH)
    {
        jm->isecho = NOTH;
    }

    jm->api = (struct _apifuncs *)palloc(sizeof(struct _apifuncs));

    if(jm->api == NULL)
    {
        msgapierr = MERR_NOMEM;
        pfree(jm);
        return NULL;
    }

    memset(jm->api, '\0', sizeof(struct _apifuncs));
    jm->apidata = (void *)palloc(sizeof(JAMBASE));

    if(jm->apidata == NULL)
    {
        msgapierr = MERR_NOMEM;
        pfree(jm->api);
        pfree(jm);
        return NULL;
    }

    memset((byte *)jm->apidata, '\0', sizeof(JAMBASE));
    jm->len        = sizeof(MSGA);
    jm->num_msg    = 0;
    jm->high_msg   = 0;
    jm->high_water = (dword) - 1;

    if(!Jam_OpenBase(jm, &mode, name))
    {
        pfree(jm->api);
        pfree(jm->apidata);
        pfree(jm);
        msgapierr = MERR_BADF;
        return NULL;
    }

    /* fix for corrupted areas */
    lseek(Jmd->IdxHandle, 0, SEEK_END);
    len = tell(Jmd->IdxHandle);

    if(Jmd->HdrInfo.ActiveMsgs > len / IDX_SIZE)
    {
        Jmd->HdrInfo.ActiveMsgs = (dword)len / IDX_SIZE;
        Jmd->modified           = 1;
    }

    lseek(Jmd->IdxHandle, 0, SEEK_SET);

    /* If ActiveMsgs is not correct, it will be updated on first OpenMessage  */
    /* Jam_ActiveMsgs() is not needed for toss, just append new msgs          */
    /* But if ActiveMsgs==0 read returns immediately without Jam_ActiveMsgs() */
    /* For prevent losing messages in this case do Jam_ActiveMsgs here        */
    if(Jmd->HdrInfo.ActiveMsgs == 0 && len > 0)
    {
        Jam_ActiveMsgs(jm);
    }

    jm->high_water = Jmd->HdrInfo.highwater;
    /* jm->high_msg = Jam_HighMsg(Jmd); */
    jm->high_msg = jm->num_msg = Jmd->HdrInfo.ActiveMsgs;
    jm->type     = MSGTYPE_JAM;
    jm->sz_xmsg  = sizeof(XMSG);
    *jm->api     = jm_funcs;
    msgapierr    = 0;
    return jm;
} /* JamOpenArea */

static sword _XPENTRY JamCloseArea(MSGA * jm)
{
    dword i;
    JAMBASE * jbptr = jbOpen;

    if(InvalidMh(jm))
    {
        return -1;
    }

    if(Jmd->msgs_open)
    {
        msgapierr = MERR_EOPEN;
        return -1;
    }

    if(jbptr != Jmd)
    {
        while((jbptr) && ((JAMBASE *)jbptr->jbNext) && ((JAMBASE *)jbptr->jbNext != Jmd))
        {
            jbptr = (JAMBASE *)jbptr->jbNext;
        }
    }

    if(Jmd->modified || Jmd->HdrInfo.highwater != jm->high_water)
    {
        Jmd->HdrInfo.highwater = jm->high_water;
        Jmd->HdrInfo.ModCounter++;
        Jam_WriteHdrInfo(Jmd);
    }

    if(jm->locked)
    {
        JamUnlock(jm);
    }

    Jam_CloseFile(Jmd);
    pfree(Jmd->BaseName);

    if(Jmd->actmsg)
    {
        for(i = 0; i < Jmd->HdrInfo.ActiveMsgs; i++)
        {
            freejamsubfield(Jmd->actmsg[i].subfield);
        }
        pfree(Jmd->actmsg);
    }

    if(jbOpen != Jmd)
    {
        jbptr->jbNext = Jmd->jbNext;
    }
    else
    {
        jbOpen = jbOpen->jbNext;
    }

    pfree(jm->api);
    pfree(jm->apidata);
    jm->id = 0L;
    memset(jm, 0, sizeof(MSGA));
    pfree(jm);
    return 0;
} /* JamCloseArea */

int JamCloseOpenAreas()
{
    void * broken, ** current = &broken; /* JAMBASE* and JAMBASE** */

    *current = NULL;

    while(jbOpen)
    {
        if(-1 == JamCloseArea(jbOpen->jm)) /* avoid infinite loop possibility in case of error
                                              in JamCloseArea */
                                           /* not closed bases are stored in the broken list */
        {
            printf("SMAPI ERROR: can't close '%s' properly!\n", jbOpen->BaseName); /* at least
                                                                                      we must
                                                                                      say
                                                                                      something
                                                                                      */
            *current = jbOpen;
            current  = &jbOpen->jbNext;
            jbOpen   = jbOpen->jbNext;
            *current = NULL;
        }
    }
    /* TODO: implement 'forced close' mode to do as much cleanup work as possible */
    jbOpen = (JAMBASE *)broken;

/*
    do something tricky with it

    JamForceCloseArea();
 */
    if(jbOpen) /* If we still have unclosed areas then return -1
                  in hope that somebody (most likely somebody in heaven)
                  will do something with it */
    {
        return -1;
    }

    if(broken) /* If we had problems with closing areas but have closed
                  them anyway then return -2. Really, does anybody expect we will
                  just suffer here in silence? NO, WE WON'T! =) */
    {
        return -2;
    }

    return 0;
} /* JamCloseOpenAreas */

static MSGH * _XPENTRY JamOpenMsg(MSGA * jm, word mode, dword msgnum)
{
    struct _msgh * msgh;

    if(InvalidMh(jm))
    {
        return NULL;
    }

    if(mode == MOPEN_CREATE)
    {
        if((sdword)msgnum < 0 || msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }

        if(msgnum != 0)
        {
            msgh = Jam_OpenMsg(jm, mode, msgnum);

            if(msgh == NULL)
            {
                msgapierr = MERR_NOENT;
                return NULL;
            }
        }
        else
        {
            msgh = palloc(sizeof(struct _msgh));

            if(msgh == NULL)
            {
                msgapierr = MERR_NOMEM;
                return NULL;
            }

            memset(msgh, '\0', sizeof(struct _msgh));
            msgh->sq            = jm;
            msgh->bytes_written = 0L;
            msgh->cur_pos       = 0L;
            msgh->msgnum        = msgnum;
            msgh->Hdr.TxtLen    = 0;
        } /* endif */
    }
    else if(msgnum == 0)
    {
        msgapierr = MERR_NOENT;
        return NULL;
    }
    else
    {
        msgh = Jam_OpenMsg(jm, mode, msgnum);

        if(msgh == NULL)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
    }

    msgh->mode = mode;
    msgh->id   = MSGH_ID;
    MsghJm->msgs_open++;
    return (MSGH *)msgh;
} /* JamOpenMsg */

static sword _XPENTRY JamCloseMsg(MSGH * msgh)
{
    if(InvalidMsgh(msgh))
    {
        return -1;
    }

    MsghJm->msgs_open--;
    /* Fill the message out to the length that the app said it would be */
    msgh->id = 0L;
    pfree(msgh->ctrl);
    pfree(msgh->lctrl);
    pfree(msgh->SubFieldPtr);
    pfree(msgh);
    return 0;
}

static int openfilejm(char * name, word mode, mode_t permissions)
{
    return sopen(name, mode, SH_DENYNONE, permissions);
}

static int opencreatefilejm(char * name, word mode, mode_t permissions)
{
    int hF = sopen(name, mode, SH_DENYNONE, permissions);

    if((hF == -1) && (mode & O_CREAT) && (errno == ENOENT))
    {
        char * slash = strrchr(name, PATH_DELIM);

        if(slash)
        {
            *slash = '\0';
            _createDirectoryTree(name);
            *slash = PATH_DELIM;
        }

        hF = sopen(name, mode, SH_DENYNONE, permissions);
    }

    return hF;
}

static int Jam_OpenTxtFile(JAMBASE * jambase)
{
    char * txt;

    if(!jambase || !jambase->BaseName)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    txt = (char *)palloc(strlen((char *)(jambase->BaseName)) + 5);

    if(!txt)
    {
        msgapierr = MERR_NOMEM;
        return 0;
    }

    strcpy(txt, (char *)(jambase->BaseName));
    strcat(txt, EXT_TXTFILE);

    if(jambase->mode == MSGAREA_CREATE)
    {
        jambase->TxtHandle = openfilejm(txt, fop_wpb, jambase->permissions);
    }
    else
    {
        jambase->TxtHandle = openfilejm(txt, fop_rpb, jambase->permissions);
    }

    if((jambase->TxtHandle == -1) && (jambase->mode == MSGAREA_CRIFNEC))
    {
        jambase->mode      = MSGAREA_CREATE;
        jambase->TxtHandle = opencreatefilejm(txt, fop_cpb, jambase->permissions);
    }

    pfree(txt);

    if(jambase->TxtHandle == -1)
    {
        Jam_CloseFile(jambase);
        msgapierr = MERR_NOENT;
        return 0;
    }

    return 1;
} /* Jam_OpenTxtFile */

static dword _XPENTRY JamReadMsg(MSGH * msgh,
                                 XMSG * msg,
                                 dword offset,
                                 dword bytes,
                                 byte * text,
                                 dword clen,
                                 byte * ctxt)
{
    JAMSUBFIELD2ptr SubField;
    dword SubPos, bytesread;
    struct tm * s_time;
    SCOMBO * scombo;
    /* unsigned char * ftsdate; */

    if(InvalidMsgh(msgh))
    {
        return (dword) - 1L;
    }

    if(msgh->Hdr.Attribute & JMSG_DELETED)
    {
        return (dword) - 1L;
    }

    if(msg)
    {
        /* zero all fields */
        memset(msg, 0, sizeof(*msg));

        /* make msg */
        msg->attr = Jam_JamAttrToMsg(msgh);

        /* get "from name" line */
        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_SENDERNAME);

        if(SubField != NULL)
        {
            strncpy((char *)(msg->from), (char *)(SubField->Buffer),
                    min(SubField->DatLen, sizeof(msg->from) - 1));
        }

        /* get "to name" line */
        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_RECVRNAME);

        if(SubField != NULL)
        {
            strncpy((char *)(msg->to), (char *)(SubField->Buffer),
                    min(SubField->DatLen, sizeof(msg->to) - 1));
        }

        /* get "subj" line */
        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_SUBJECT);

        if(SubField != NULL)
        {
            strncpy((char *)(msg->subj), (char *)(SubField->Buffer),
                    min(SubField->DatLen, sizeof(msg->subj) - 1));
        }

        /* try to fetch orig/dest addresses even for echomail */
        /* get "orig address" line */
        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_OADDRESS);

        if(SubField != NULL)
        {
            parseFtnAddrS((char *)SubField->Buffer, &(msg->orig), SubField->DatLen);
        }

        /* get "dest address" line */
        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_DADDRESS);

        if(SubField != NULL)
        {
            parseFtnAddrS((char *)SubField->Buffer, &(msg->dest), SubField->DatLen);
        }

        /* get "orig address" from MSGID */
        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_MSGID);

        if(SubField != NULL)
        {
            if(!(msg->orig.zone || msg->orig.net || msg->orig.node))
            {
                parseFtnAddrS((char *)SubField->Buffer, &(msg->dest), SubField->DatLen);
            }
        }

        {
            const time_t c_time = msgh->Hdr.DateWritten;
            s_time = gmtime(&c_time);
        }
        scombo = (SCOMBO *)(&(msg->date_written));
        scombo = TmDate_to_DosDate(s_time, scombo);
        /* ftsdate = (unsigned char *)sc_time(scombo, (char *)(msg->__ftsc_date)); */

        if(msgh->Hdr.DateProcessed)
        {
            const time_t c_time = msgh->Hdr.DateProcessed;
            s_time = gmtime(&c_time);
            scombo = (SCOMBO *)(&(msg->date_arrived));
            scombo = TmDate_to_DosDate(s_time, scombo);
        }
        else
        {
            ((SCOMBO *)(&(msg->date_arrived)))->ldate = 0;
        }

        msg->replyto     = msgh->Hdr.ReplyTo;
        msg->xmreply1st  = msgh->Hdr.Reply1st;
        msg->replies[1]  = 0;
        msg->xmreplynext = msgh->Hdr.ReplyNext;
        msg->xmtimesread = msgh->Hdr.TimesRead;
        msg->xmcost      = msgh->Hdr.Cost;
    } /* endif */

    bytesread = 0;

    if(bytes > 0 && text)
    {
        /* read text message */
        if(offset > (msgh->Hdr.TxtLen + msgh->lclen))
        {
            offset = msgh->Hdr.TxtLen + msgh->lclen;
        }

        msgh->cur_pos = offset;

        if(offset >= msgh->Hdr.TxtLen)
        {
            offset -= msgh->Hdr.TxtLen;

            if(bytes > msgh->lclen)
            {
                bytes = msgh->lclen;
            }

            if(offset < msgh->lclen)
            {
                if(offset < bytes)
                {
                    bytesread = bytes - offset;
                }

                strncpy((char *)text, (char *)(msgh->lctrl + offset), (size_t)bytesread);
            }
            else
            {} /* endif */

            msgh->cur_pos += bytesread;
        }
        else
        {
            if(MsghJm->TxtHandle == 0)
            {
                Jam_OpenTxtFile(MsghJm);
            }

            lseek(MsghJm->TxtHandle, (size_t)msgh->Hdr.TxtOffset + offset, SEEK_SET);

            if(bytes > msgh->Hdr.TxtLen - offset)
            {
                bytesread = farread(MsghJm->TxtHandle, text, msgh->Hdr.TxtLen - offset);
                bytes    -= (msgh->Hdr.TxtLen - offset);

                if(offset < bytes)
                {
                    bytes -= offset;
                }
                else
                {
                    bytes = 0;
                }

                if(bytes > msgh->lclen)
                {
                    bytes = msgh->lclen;
                }

                strncpy((char *)(text + bytesread), (char *)(msgh->lctrl), (size_t)bytes);
                bytesread += bytes;
            }
            else
            {
                bytesread = farread(MsghJm->TxtHandle, text, bytes);
            } /* endif */

            msgh->cur_pos += bytesread;
        } /* endif */

        text[bytesread] = '\0';
    }

    if(clen && ctxt)
    {
        /* read first kludges */
        if(clen > msgh->clen)
        {
            clen = msgh->clen;
        }

        strncpy((char *)(ctxt), (char *)(msgh->ctrl), (size_t)clen);
        ctxt[clen] = '\0';
    }

    msgapierr = MERR_NONE;
    return bytesread;
} /* JamReadMsg */

static sword _XPENTRY JamWriteMsg(MSGH * msgh,
                                  word append,
                                  XMSG * msg,
                                  byte * text,
                                  dword textlen,
                                  dword totlen,
                                  dword clen,
                                  byte * ctxt)
{
    /* not supported append if JAM !!!
       the reason is that we need to know ALL of the text before we can set up
       the JAM headers.
     */
    JAMHDR jamhdrNew;
    JAMIDXREC jamidxNew;
    JAMSUBFIELD2LISTptr subfieldNew;
    XMSG msg_old;
    MSGA * jm;
    dword x = 0;
    char ch = 0;
    unsigned char * onlytext = NULL;
    int didlock = FALSE;
    sword rc    = 0;

    assert(append == 0);

    if(InvalidMsgh(msgh))
    {
        return -1L;
    }

    if(msgh->mode != MOPEN_CREATE && msgh->mode != MOPEN_WRITE && msgh->mode != MOPEN_RW)
    {
        return -1L;
    }

    jm            = msgh->sq;
    Jmd->modified = 1;
    memset(&jamidxNew, '\0', sizeof(JAMIDXREC));
    memset(&jamhdrNew, '\0', sizeof(JAMHDR));

    if(!ctxt)
    {
        clen = 0L;
    }

    if(!text)
    {
        textlen = 0L;
    }

    if(textlen == 0L)
    {
        text = NULL;
    }

    if(clen == 0L)
    {
        ctxt = NULL;
    }

    if(msgh->mode != MOPEN_CREATE)
    {
        if(clen)
        {
            clen = 0;
        }

        if(ctxt)
        {
            ctxt = NULL;
        }
    }

    if(clen && ctxt)
    {
        x = (dword)strlen((char *)ctxt);

        if(clen < x)
        {
            clen = x + 1;
        }
    }

    subfieldNew = NULL;

    if(msg)
    {
        ConvertXmsgToJamHdr(msgh, msg, &jamhdrNew, &subfieldNew);
    }
    else
    {
        if(msgh->mode != MOPEN_CREATE)
        {
            JamReadMsg(msgh, &msg_old, 0, 0, NULL, 0, NULL);
            ConvertXmsgToJamHdr(msgh, &msg_old, &jamhdrNew, &subfieldNew);
        }
    }

    if(!jm->locked)
    {
        didlock = Jam_Lock(jm, 1);

        if(didlock == 0)
        {
            freejamsubfield(subfieldNew);
            msgapierr = MERR_SHARE;
            return -1;
        }
    }

    jamhdrNew.ReplyCRC = jamhdrNew.MsgIdCRC = 0xFFFFFFFFUL;

    if(clen && ctxt)
    {
        ConvertCtrlToSubf(&jamhdrNew, &subfieldNew, clen, ctxt);
    }

    if(textlen && text)
    {
        onlytext = DelimText(&jamhdrNew, &subfieldNew, text, (size_t)textlen);
    }
    else
    {
        if(msgh->mode != MOPEN_CREATE)
        {
            DelimText(&jamhdrNew, &subfieldNew, msgh->lctrl, (size_t)(msgh->lclen));
            jamhdrNew.TxtOffset = msgh->Hdr.TxtOffset;
            jamhdrNew.TxtLen    = msgh->Hdr.TxtLen;
        }
    }

    if(onlytext == NULL)
    {
        onlytext = calloc(1, 1);

        if(!onlytext)
        {
            freejamsubfield(subfieldNew);
            msgapierr = MERR_NOMEM;
            return -1;
        }
    }

    if(msgh->mode == MOPEN_CREATE)
    {
        /* no logic if msg not present */
        if(msg)
        {
            if(Jmd->TxtHandle == 0)
            {
                Jam_OpenTxtFile(Jmd);
            }

            if(msgh->msgnum == 0)
            {
                /* new message in end of position */
                lseek(Jmd->IdxHandle, 0, SEEK_END);
                lseek(Jmd->HdrHandle, 0, SEEK_END);
                lseek(Jmd->TxtHandle, 0, SEEK_END);
                jamhdrNew.MsgNum    = Jmd->HdrInfo.BaseMsgNum + ((dword)tell(Jmd->IdxHandle) / IDX_SIZE);
                msgh->seek_idx      = (dword)tell(Jmd->IdxHandle);
                msgh->seek_hdr      = jamidxNew.HdrOffset = (dword)tell(Jmd->HdrHandle);
                jamidxNew.UserCRC   = Jam_Crc32(msg->to, (dword)strlen((char *)(msg->to)));
                jamhdrNew.TxtOffset = (dword)tell(Jmd->TxtHandle);
                jamhdrNew.TxtLen    = (dword)strlen((char *)onlytext);
                msgh->bytes_written = (dword)farwrite(Jmd->TxtHandle, onlytext, jamhdrNew.TxtLen);

                if(msgh->bytes_written != jamhdrNew.TxtLen)
                {
                    setfsize(Jmd->TxtHandle, jamhdrNew.TxtOffset);
                    freejamsubfield(subfieldNew);
                    pfree(onlytext);
                    msgapierr = MERR_NODS;
                    return -1;
                }

                msgh->cur_pos = (dword)tell(Jmd->TxtHandle);

                if(!write_hdr(Jmd->HdrHandle, &jamhdrNew))
                {
                    setfsize(Jmd->HdrHandle, msgh->seek_hdr);
                    setfsize(Jmd->TxtHandle, jamhdrNew.TxtOffset);
                    freejamsubfield(subfieldNew);
                    pfree(onlytext);
                    msgapierr = MERR_NODS;
                    return -1;
                }

                if(!write_subfield(Jmd->HdrHandle, &subfieldNew, jamhdrNew.SubfieldLen))
                {
                    setfsize(Jmd->HdrHandle, msgh->seek_hdr);
                    setfsize(Jmd->TxtHandle, jamhdrNew.TxtOffset);
                    freejamsubfield(subfieldNew);
                    free(onlytext);
                    msgapierr = MERR_NODS;
                    return -1;
                }

                if(!write_idx(Jmd->IdxHandle, &jamidxNew))
                {
                    setfsize(Jmd->IdxHandle, msgh->seek_idx);
                    setfsize(Jmd->HdrHandle, msgh->seek_hdr);
                    setfsize(Jmd->TxtHandle, jamhdrNew.TxtOffset);
                    freejamsubfield(subfieldNew);
                    free(onlytext);
                    msgapierr = MERR_NODS;
                    return -1;
                }

                Jmd->HdrInfo.ActiveMsgs++;
#ifdef HARD_WRITE_HDR
                Jmd->HdrInfo.ModCounter++;

                if(Jam_WriteHdrInfo(Jmd))
                {
                    setfsize(Jmd->HdrHandle, msgh->seek_hdr);
                    setfsize(Jmd->IdxHandle, msgh->seek_idx);
                    setfsize(Jmd->TxtHandle, jamhdrNew.TxtOffset);
                    freejamsubfield(subfieldNew);
                    free(onlytext);
                    msgapierr = MERR_NODS;
                    return -1;
                }

#endif
                jm->high_msg++;

                if(Jmd->actmsg_read)
                {
                    JAMACTMSGptr tmp;
                    tmp = (JAMACTMSGptr)farrealloc(Jmd->actmsg,
                              sizeof(JAMACTMSG) * ((size_t)jm->num_msg + 1));
                    if(!tmp)
                    {
                        freejamsubfield(subfieldNew);
                        free(onlytext);
                        pfree(Jmd->actmsg);
                        msgapierr = MERR_NOMEM;
                        return -1;
                    }
                    Jmd->actmsg = tmp;

                    Jmd->actmsg[jm->num_msg].IdxOffset = msgh->seek_idx;
                    Jmd->actmsg[jm->num_msg].TrueMsg   = msgh->seek_hdr;
                    Jmd->actmsg[jm->num_msg].UserCRC   = jamidxNew.UserCRC;
                    memcpy(&(Jmd->actmsg[jm->num_msg].hdr), &jamhdrNew, sizeof(jamhdrNew));

                    if(Jmd->actmsg_read == 1)
                    {
                        Jmd->actmsg[jm->num_msg].subfield = subfieldNew;
                    }
                    else /* not incore subfields */
                    {
                        Jmd->actmsg[jm->num_msg].subfield = NULL;
                        freejamsubfield(subfieldNew);
                    }
                }
                else
                {
                    freejamsubfield(subfieldNew);
                }

                jm->num_msg++;
            }
            else
            {
                /* new message instead of old message position */
                msgh->Hdr.TxtLen     = 0;
                msgh->Hdr.Attribute |= JMSG_DELETED;
                lseek(Jmd->HdrHandle, 0, SEEK_END);
                jamidxNew.HdrOffset = (dword)tell(Jmd->HdrHandle);
                jamhdrNew.MsgNum    = msgh->Hdr.MsgNum;
                jamidxNew.UserCRC   = Jam_Crc32(msg->to, (dword)strlen((char *)(msg->to)));
                lseek(Jmd->TxtHandle, 0, SEEK_END);
                jamhdrNew.TxtOffset = (dword)tell(Jmd->TxtHandle);
                jamhdrNew.TxtLen    = (dword)strlen((char *)onlytext);
                msgh->bytes_written = (dword)farwrite(Jmd->TxtHandle, onlytext, jamhdrNew.TxtLen);
                msgh->cur_pos       = (dword)tell(Jmd->TxtHandle);
                lseek(Jmd->TxtHandle, (size_t)jamhdrNew.TxtOffset + (size_t)(totlen - 1), SEEK_SET);
                farwrite(Jmd->TxtHandle, &ch, 1);
                write_hdr(Jmd->HdrHandle, &jamhdrNew);
                write_subfield(Jmd->HdrHandle, &subfieldNew, jamhdrNew.SubfieldLen);
                lseek(Jmd->IdxHandle, msgh->seek_idx, SEEK_SET);
                write_idx(Jmd->IdxHandle, &jamidxNew);
                lseek(Jmd->HdrHandle, msgh->seek_hdr, SEEK_SET);
                write_hdr(Jmd->HdrHandle, &(msgh->Hdr));
                msgh->seek_hdr = jamidxNew.HdrOffset;
#ifdef HARD_WRITE_HDR
                Jmd->HdrInfo.ModCounter++;
                Jam_WriteHdrInfo(Jmd);
#endif

                /* info from new message to msgh srtuct */
                if(Jmd->actmsg_read)
                {
                    Jmd->actmsg[msgh->msgnum - 1].TrueMsg = msgh->seek_hdr;
                    Jmd->actmsg[msgh->msgnum - 1].UserCRC = jamidxNew.UserCRC;
                    memcpy(&(Jmd->actmsg[msgh->msgnum - 1].hdr), &jamhdrNew, sizeof(jamhdrNew));

                    if(Jmd->actmsg_read == 1)
                    {
                        Jmd->actmsg[msgh->msgnum - 1].subfield = subfieldNew;
                    }
                    else /* subfields not incore */
                    {
                        Jmd->actmsg[msgh->msgnum - 1].subfield = NULL;
                        freejamsubfield(subfieldNew);
                    }
                }
                else
                {
                    freejamsubfield(subfieldNew);
                }
            } /* endif */
        } /* endif */
    }
    else
    {
        /* change text and SEEN_BY, PATH, VIA kludges posible only (message != create)*/
        ConvertCtrlToSubf(&jamhdrNew, &subfieldNew, msgh->clen, msgh->ctrl);

        if(msg)
        {
            jamidxNew.UserCRC = Jam_Crc32(msg->to, (dword)strlen((char *)(msg->to)));
        }
        else
        {
            jamidxNew.UserCRC = msgh->Idx.UserCRC;
        }

        if(text && textlen)
        {
            if(Jmd->TxtHandle == 0)
            {
                Jam_OpenTxtFile(Jmd);
            }

            lseek(Jmd->TxtHandle, 0, SEEK_END);
            jamhdrNew.TxtOffset = (dword)tell(Jmd->TxtHandle);
            jamhdrNew.TxtLen    = (dword)strlen((char *)onlytext);
            farwrite(Jmd->TxtHandle, onlytext, jamhdrNew.TxtLen);
        } /* endif */

        if(jamhdrNew.SubfieldLen > msgh->Hdr.SubfieldLen)
        {
            msgh->Hdr.TxtLen     = 0;
            msgh->Hdr.Attribute |= JMSG_DELETED;
            lseek(Jmd->HdrHandle, msgh->seek_hdr, SEEK_SET);
            write_hdr(Jmd->HdrHandle, &(msgh->Hdr));
            lseek(Jmd->HdrHandle, 0, SEEK_END);
            msgh->seek_hdr = (dword)tell(Jmd->HdrHandle);
        }

        jamhdrNew.MsgNum    = msgh->Hdr.MsgNum;
        jamidxNew.HdrOffset = msgh->seek_hdr;
        lseek(Jmd->HdrHandle, msgh->seek_hdr, SEEK_SET);
        write_hdr(Jmd->HdrHandle, &jamhdrNew);
        write_subfield(Jmd->HdrHandle, &subfieldNew, jamhdrNew.SubfieldLen);

        if(Jmd->actmsg_read && Jmd->actmsg[msgh->msgnum - 1].TrueMsg == msgh->seek_hdr &&
           Jmd->actmsg[msgh->msgnum - 1].UserCRC == jamidxNew.UserCRC)
        {
            /* no index update needed */
        }
        else
        {
            lseek(Jmd->IdxHandle, msgh->seek_idx, SEEK_SET);
            write_idx(Jmd->IdxHandle, &jamidxNew);
        }

#ifdef HARD_WRITE_HDR
        Jmd->HdrInfo.ModCounter++;
        Jam_WriteHdrInfo(Jmd);
#endif
        memmove(&(msgh->Idx), &(jamidxNew), sizeof(JAMIDXREC));
        memmove(&(msgh->Hdr), &(jamhdrNew), sizeof(JAMHDR));
        freejamsubfield(msgh->SubFieldPtr);
        msgh->SubFieldPtr = subfieldNew;
        pfree(msgh->ctrl);
        pfree(msgh->lctrl);
        DecodeSubf(msgh);

        if(Jmd->actmsg_read)
        {
            Jmd->actmsg[msgh->msgnum - 1].TrueMsg = msgh->seek_hdr;
            Jmd->actmsg[msgh->msgnum - 1].UserCRC = jamidxNew.UserCRC;
            memcpy(&(Jmd->actmsg[msgh->msgnum - 1].hdr), &jamhdrNew, sizeof(jamhdrNew));
            freejamsubfield(Jmd->actmsg[msgh->msgnum - 1].subfield);

            if(Jmd->actmsg_read == 1)
            {
                copy_subfield(&(Jmd->actmsg[msgh->msgnum - 1].subfield), subfieldNew);
            }
            else
            {
                Jmd->actmsg[msgh->msgnum - 1].subfield = NULL;
            }
        }
    } /* endif */

    if(didlock)
    {
        Jam_Unlock(jm);
    } /* endif */

    pfree(onlytext);
    return rc;
} /* JamWriteMsg */

static sword _XPENTRY JamKillMsg(MSGA * jm, dword msgnum)
{
    JAMIDXREC jamidx;
    JAMHDR jamhdr;

    if(InvalidMh(jm))
    {
        return -1;
    }

    if(jm->locked)
    {
        return -1L;
    }

    if(msgnum == 0 || msgnum > jm->num_msg)
    {
        msgapierr = MERR_NOENT;
        return -1L;
    } /* endif */

    if(!Jam_PosHdrMsg(jm, msgnum, &jamidx, &jamhdr))
    {
        msgapierr = MERR_BADF;
        return -1L;
    } /* endif */

    if(JamLock(jm) == -1)
    {
        return -1L;
    }

    lseek(Jmd->HdrHandle, jamidx.HdrOffset, SEEK_SET);
    /*lseek(Jmd->IdxHandle, (IDX_SIZE)*(msgnum - 1), SEEK_SET);*/
    lseek(Jmd->IdxHandle, Jmd->actmsg[msgnum - 1].IdxOffset, SEEK_SET); /* can be buggy! */
    Jmd->modified = 1;
    Jmd->HdrInfo.ActiveMsgs--;
    jamhdr.TxtLen     = 0;
    jamhdr.Attribute |= JMSG_DELETED;
    jamidx.UserCRC    = 0xFFFFFFFFL;
    jamidx.HdrOffset  = 0xFFFFFFFFL;
/*
   lseek(Jmd->HdrHandle, -(HDR_SIZE), SEEK_CUR);
   lseek(Jmd->IdxHandle, -(IDX_SIZE), SEEK_CUR);
 */
    write_idx(Jmd->IdxHandle, &jamidx);
    write_hdr(Jmd->HdrHandle, &jamhdr);
#ifdef HARD_WRITE_HDR
    Jmd->HdrInfo.ModCounter++;
    Jam_WriteHdrInfo(Jmd);
#endif

    if(Jmd->actmsg_read)
    {
        dword i;

        for(i = 0; i < Jmd->HdrInfo.ActiveMsgs; i++)
        {
            freejamsubfield(Jmd->actmsg[i].subfield);
        }
        pfree(Jmd->actmsg);
        Jmd->actmsg_read = 0;
        Jmd->actmsg      = NULL;
    }

    Jam_ActiveMsgs(jm);
    jm->num_msg = Jmd->HdrInfo.ActiveMsgs;
    JamUnlock(jm);
    return 0;
} /* JamKillMsg */

static sword _XPENTRY JamLock(MSGA * jm)
{
    if(InvalidMh(jm))
    {
        return -1;
    }

    /* Don't do anything if already locked */
    if(jm->locked)
    {
        return 0;
    }

    if(!Jam_Lock(jm, 0))
    {
        return -1;
    }

    jm->locked = TRUE;
    return 0;
}

static sword _XPENTRY JamUnlock(MSGA * jm)
{
    if(InvalidMh(jm))
    {
        return -1;
    }

    if(!jm->locked)
    {
        return -1;
    }

    jm->locked = FALSE;

    if(mi.haveshare)
    {
        unlock(Jmd->HdrHandle, 0L, 1L);
    }

    return 0;
}

static sword _XPENTRY JamSetCurPos(MSGH * msgh, dword pos)
{
    if(InvalidMsgh(msgh))
    {
        return -1;
    }

    msgh->cur_pos = pos;
    return 0;
}

static dword _XPENTRY JamGetCurPos(MSGH * msgh)
{
    if(InvalidMsgh(msgh))
    {
        return (dword) - 1;
    }

    return msgh->cur_pos;
}

static UMSGID _XPENTRY JamMsgnToUid(MSGA * jm, dword msgnum)
{
    if(InvalidMh(jm))
    {
        return (UMSGID)-1;
    }

    msgapierr = MERR_NONE;

    if(msgnum > jm->num_msg)
    {
        return (UMSGID)-1;
    }

    if(msgnum == 0)
    {
        return (UMSGID)0;
    }

    if(!Jmd->actmsg_read)
    {
        Jam_ActiveMsgs(jm);

        if(msgnum > jm->num_msg)
        {
            return (UMSGID)-1;
        }
    }

    return (UMSGID)(Jmd->actmsg[msgnum - 1].IdxOffset / 8 + Jmd->HdrInfo.BaseMsgNum);
} /* JamMsgnToUid */

static dword _XPENTRY JamUidToMsgn(MSGA * jm, UMSGID umsgid, word type)
{
    dword msgnum, left, right, current;
    UMSGID umsg;

    if(InvalidMh(jm))
    {
        return (dword) - 1L;
    }

    msgnum = umsgid - Jmd->HdrInfo.BaseMsgNum + 1;

    if(msgnum == 0)
    {
        return 0;
    }

    if(!Jmd->actmsg_read)
    {
        Jam_ActiveMsgs(jm);
    }

    left  = 1;
    right = jm->num_msg;

    while(left <= right)
    {
        current = (right + left) / 2;
        umsg    = JamMsgnToUid(jm, current);

        if(umsg == (UMSGID)-1)
        {
            return 0;
        }

        if(umsg < msgnum)
        {
            left = current + 1;
        }
        else if(umsg > msgnum)
        {
            if(current > 0)
            {
                right = current - 1;
            }
            else
            {
                right = 0;
            }
        }
        else
        {
            return current;
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

    return (left > jm->num_msg) ? jm->num_msg : left;
} /* JamUidToMsgn */

static dword _XPENTRY JamGetHighWater(MSGA * jm)
{
    if(InvalidMh(jm))
    {
        return (dword) - 1L;
    }

    return JamUidToMsgn(jm, jm->high_water, UID_PREV);
}

static sword _XPENTRY JamSetHighWater(MSGA * jm, dword hwm)
{
    if(InvalidMh(jm))
    {
        return -1L;
    }

    hwm = JamMsgnToUid(jm, hwm);

    if(hwm > jm->high_msg + Jmd->HdrInfo.BaseMsgNum)
    {
        return -1L;
    }

    jm->high_water = hwm;
    return 0;
}

static dword _XPENTRY JamGetTextLen(MSGH * msgh)
{
    if(InvalidMsgh(msgh))
    {
        return (dword)0L;
    }

    return msgh->Hdr.TxtLen + msgh->lclen;
}

static dword _XPENTRY JamGetCtrlLen(MSGH * msgh)
{
    if(InvalidMsgh(msgh))
    {
        return (dword)0L;
    }

    return msgh->clen;
}

sword MSGAPI JamValidate(byte * name)
{
    byte temp[PATHLEN];

    if(!name || !*name)
    {
        return FALSE;
    }

    sprintf((char *)temp, "%s%s", name, EXT_HDRFILE);

    if(!fexist((char *)temp))
    {
        return FALSE;
    }

    sprintf((char *)temp, "%s%s", name, EXT_TXTFILE);

    if(!fexist((char *)temp))
    {
        return FALSE;
    }

    sprintf((char *)temp, "%s%s", name, EXT_IDXFILE);

    if(!fexist((char *)temp))
    {
        return FALSE;
    }

    return TRUE;
} /* JamValidate */

void Jam_CloseFile(JAMBASE * jambase)
{
    if(!jambase)
    {
        errno     = EINVAL;
        msgapierr = MERR_BADA;
        return;
    }

    if(jambase->HdrHandle != 0 && jambase->HdrHandle != -1)
    {
        close(jambase->HdrHandle);
        jambase->HdrHandle = 0;
    } /* endif */

    if(jambase->TxtHandle != 0 && jambase->TxtHandle != -1)
    {
        close(jambase->TxtHandle);
        jambase->TxtHandle = 0;
    } /* endif */

    if(jambase->IdxHandle != 0 && jambase->IdxHandle != -1)
    {
        close(jambase->IdxHandle);
        jambase->IdxHandle = 0;
    } /* endif */

    if(jambase->LrdHandle != 0 && jambase->LrdHandle != -1)
    {
        close(jambase->LrdHandle);
        jambase->LrdHandle = 0;
    } /* endif */
} /* Jam_CloseFile */

/*  Return values:
 *  0 = error
 *  1 = success
 */
int JamDeleteBase(char * name)
{
    char * fn = NULL;
    int rc    = 1;

    if(!name || !*name)
    {
        errno     = EINVAL;
        msgapierr = MERR_BADNAME;
        return 0;
    }

    fn = palloc(strlen(name) + 5);

    if(!fn)
    {
        errno     = ENOMEM;
        msgapierr = MERR_NOMEM;
        return 0;
    }

    sprintf(fn, "%s%s", name, EXT_HDRFILE);

    if(unlink(fn))
    {
        rc = 0;          /* error */
    }

    sprintf(fn, "%s%s", name, EXT_TXTFILE);

    if(unlink(fn))
    {
        rc = 0;          /* error */
    }

    sprintf(fn, "%s%s", name, EXT_IDXFILE);

    if(unlink(fn))
    {
        rc = 0;          /* error */
    }

    sprintf(fn, "%s%s", name, EXT_LRDFILE);

    if(unlink(fn) && errno != ENOENT)
    {
        rc = 0;                           /* error */
    }

    pfree(fn);
    return rc;
} /* JamDeleteBase */

/*  Return values:
 *  0 = error
 *  1 = success
 */
int Jam_OpenFile(JAMBASE * jambase, word * mode, mode_t permissions)
{
    char * hdr = NULL, * idx = NULL, * txt = NULL, * lrd = NULL;
    int x;

    if(!jambase || !mode)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    x   = (dword)strlen((char *)(jambase->BaseName)) + 5;
    hdr = (char *)palloc((size_t)x);
    idx = (char *)palloc((size_t)x);
    txt = (char *)palloc((size_t)x);

/*   lrd = (char*) palloc(x);*/
    if(!(hdr && idx && txt /*&& lrd*/))
    {
        pfree(hdr);
        pfree(idx);
        pfree(txt);
        pfree(lrd);
        errno     = ENOMEM;
        msgapierr = MERR_NOMEM;
        return 0;
    }

    sprintf(hdr, "%s%s", jambase->BaseName, EXT_HDRFILE);
    sprintf(txt, "%s%s", jambase->BaseName, EXT_TXTFILE);
    sprintf(idx, "%s%s", jambase->BaseName, EXT_IDXFILE);

    if(*mode == MSGAREA_CREATE)
    {
        jambase->HdrHandle = opencreatefilejm(hdr, fop_wpb, permissions);
        jambase->TxtHandle = openfilejm(txt, fop_wpb, permissions);
        jambase->IdxHandle = openfilejm(idx, fop_wpb, permissions);
        jambase->LrdHandle = 0;
        memset(&(jambase->HdrInfo), '\0', sizeof(JAMHDRINFO));
        strcpy((char *)(jambase->HdrInfo.Signature), HEADERSIGNATURE);
        jambase->HdrInfo.DateCreated = (dword)(time(NULL) + gettz());
        jambase->HdrInfo.ModCounter  = 1;
        jambase->HdrInfo.PasswordCRC = 0xffffffffUL;
        jambase->HdrInfo.BaseMsgNum  = 1;
        write_hdrinfo(jambase->HdrHandle, &(jambase->HdrInfo));
    }
    else
    {
        jambase->HdrHandle = openfilejm(hdr, fop_rpb, permissions);
        jambase->TxtHandle = 0;
        jambase->IdxHandle = openfilejm(idx, fop_rpb, permissions);
        jambase->LrdHandle = 0;
    } /* endif */

    if(jambase->HdrHandle == -1 || jambase->TxtHandle == -1 || jambase->IdxHandle == -1)
    {
        if(*mode != MSGAREA_CRIFNEC)
        {
            Jam_CloseFile(jambase);
            pfree(hdr);
            pfree(txt);
            pfree(idx);
            pfree(lrd);
            msgapierr = MERR_NOENT;
            return 0;
        }

        *mode = MSGAREA_CREATE;
        jambase->HdrHandle = opencreatefilejm(hdr, fop_cpb, permissions);
        jambase->TxtHandle = openfilejm(txt, fop_cpb, permissions);
        jambase->IdxHandle = openfilejm(idx, fop_cpb, permissions);

        if(jambase->HdrHandle == -1 || jambase->TxtHandle == -1 || jambase->IdxHandle == -1)
        {
            Jam_CloseFile(jambase);
            pfree(hdr);
            pfree(txt);
            pfree(idx);
            pfree(lrd);
            msgapierr = MERR_NOENT;
            return 0;
        } /* endif */

        memset(&(jambase->HdrInfo), '\0', sizeof(JAMHDRINFO));
        strcpy((char *)(jambase->HdrInfo.Signature), HEADERSIGNATURE);
        jambase->HdrInfo.DateCreated = (dword)(time(NULL) + gettz());
        jambase->HdrInfo.ModCounter  = 1;
        jambase->HdrInfo.PasswordCRC = 0xffffffffUL;
        jambase->HdrInfo.BaseMsgNum  = 1;
        write_hdrinfo(jambase->HdrHandle, &(jambase->HdrInfo));
    } /* endif */

    pfree(hdr);
    pfree(txt);
    pfree(idx);
    pfree(lrd);
    jambase->mode        = *mode;
    jambase->permissions = permissions;
    jambase->modified    = 0;
    return 1;
} /* Jam_OpenFile */

/*  Return values:
 *  0 = error
 *  1 = success
 */
static sword MSGAPI Jam_OpenBase(MSGA * jm, word * mode, unsigned char * basename)
{
    if(InvalidMh(jm) || !mode)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    if(!basename || !*basename)
    {
        msgapierr = MERR_BADNAME;
        return 0;
    }

    Jmd->BaseName = (unsigned char *)palloc(strlen((char *)basename) + 1);

    if(!Jmd->BaseName)
    {
        errno     = ENOMEM;
        msgapierr = MERR_NOMEM;
        return 0;
    }

    strcpy((char *)(Jmd->BaseName), (char *)basename);

    if(!Jam_OpenFile(Jmd, mode, FILEMODE(jm->isecho)))
    {
        /* Jam_OpenFile() set MSGAPIERR */
        pfree(Jmd->BaseName);
        return 0;
    } /* endif */

    Jmd->jm     = jm;
    Jmd->jbNext = jbOpen;
    jbOpen      = Jmd;
    lseek(Jmd->HdrHandle, 0, SEEK_SET);
    read_hdrinfo(Jmd->HdrHandle, &(Jmd->HdrInfo));
    return 1;
} /* Jam_OpenBase */

/*  Return values:
 *  0 = error
 *  1 = success
 */
static MSGH * Jam_OpenMsg(MSGA * jm, word mode, dword msgnum)
{
    struct _msgh * msgh;

/*   JAMIDXREC    idx; */
/*   JAMHDR       hdr; */
    unused(mode);

    if(InvalidMh(jm))
    {
        return NULL;
    }

    if(msgnum == MSGNUM_CUR)
    {
        msgnum = jm->cur_msg;
    }
    else if(msgnum == MSGNUM_NEXT)
    {
        msgnum = jm->cur_msg + 1;

        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */

        jm->cur_msg = msgnum;
    }
    else if(msgnum == MSGNUM_PREV)
    {
        msgnum = jm->cur_msg - 1;

        if(msgnum == 0)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */

        jm->cur_msg = msgnum;
    }
    else
    {
        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */
    }

    msgh = palloc(sizeof(struct _msgh));

    if(msgh == NULL)
    {
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    memset(msgh, '\0', sizeof(struct _msgh));
    msgh->sq     = jm;
    msgh->msgnum = msgnum;
    msgh->mode   = mode;
    msgh->id     = MSGH_ID;

/*   msgh->Idx.HdrOffset = 0xffffffff; */
/*   msgh->Idx.UserCRC   = 0xffffffff; */
    if(!Jmd->actmsg_read)
    {
        Jam_ActiveMsgs(jm);

        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            pfree(msgh);
            return NULL;
        }
    }

    if(Jmd->actmsg)
    {
        msgh->seek_idx      = Jmd->actmsg[msgnum - 1].IdxOffset;
        msgh->Idx.HdrOffset = Jmd->actmsg[msgnum - 1].TrueMsg;
        msgh->Idx.UserCRC   = Jmd->actmsg[msgnum - 1].UserCRC;

        if(msgh->Idx.HdrOffset != 0xffffffffUL)
        {
            msgh->seek_hdr = msgh->Idx.HdrOffset;
            memcpy(&(msgh->Hdr), &(Jmd->actmsg[msgnum - 1].hdr), sizeof(msgh->Hdr));

            if(sstricmp((char *)&msgh->Hdr, "JAM") != 0)
            {
                pfree(msgh);
                return NULL;
            }
            else
            {} /* endif */

            if(mode == MOPEN_CREATE)
            {
                return (MSGH *)msgh;
            }

            msgh->SubFieldPtr = 0;

            if(Jmd->actmsg[msgnum - 1].subfield)
            {
                copy_subfield(&(msgh->SubFieldPtr), Jmd->actmsg[msgnum - 1].subfield);
            }
            else
            {
                lseek(Jmd->HdrHandle, (size_t)(Jmd->actmsg[msgnum - 1].TrueMsg) + HDR_SIZE, SEEK_SET);
                read_subfield(Jmd->HdrHandle, &(msgh->SubFieldPtr),
                              &(Jmd->actmsg[msgnum - 1].hdr.SubfieldLen));
            }

            DecodeSubf(msgh);
            return (MSGH *)msgh;
        }
    }

    pfree(msgh);
    return NULL;
} /* Jam_OpenMsg */

JAMSUBFIELD2ptr Jam_GetSubField(MSGH * msgh, dword * SubPos, word what)
{
    JAMSUBFIELD2ptr SubField;
    JAMSUBFIELD2LISTptr SubFieldList;
    dword i;

    if(InvalidMsgh(msgh))
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    SubFieldList = msgh->SubFieldPtr;
    SubField     = msgh->SubFieldPtr->subfield;

    for(i = *SubPos; i < SubFieldList->subfieldCount; i++, SubField++)
    {
        if(SubField->LoID == what)
        {
            *SubPos = i;
            return SubField;
        }
    }
    return NULL;
}

char * Jam_GetKludge(MSGA * jm, dword msgnum, word what)
{
    JAMSUBFIELD2LISTptr subf;
    JAMSUBFIELD2ptr subfptr;
    dword i;
    char * res;

    if(InvalidMh(jm))
    {
        return NULL;
    }

    if(msgnum == MSGNUM_CUR)
    {
        msgnum = jm->cur_msg;
    }
    else if(msgnum == MSGNUM_NEXT)
    {
        msgnum = jm->cur_msg + 1;

        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */

        jm->cur_msg = msgnum;
    }
    else if(msgnum == MSGNUM_PREV)
    {
        msgnum = jm->cur_msg - 1;

        if(msgnum == 0)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */

        jm->cur_msg = msgnum;
    }
    else
    {
        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */
    }

    if(!Jmd->actmsg_read)
    {
        Jam_ActiveMsgs(jm);

        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
    }

    if(!Jmd->actmsg)
    {
        return NULL;
    }

    subf = Jmd->actmsg[msgnum - 1].subfield;

    if(subf == NULL)
    {
        lseek(Jmd->HdrHandle, (size_t)(Jmd->actmsg[msgnum - 1].TrueMsg) + HDR_SIZE, SEEK_SET);
        read_subfield(Jmd->HdrHandle, &subf, &(Jmd->actmsg[msgnum - 1].hdr.SubfieldLen));
    }

    for(i = 0, subfptr = subf->subfield; i < subf->subfieldCount; i++, subfptr++)
    {
        if(subfptr->LoID == what)
        {
            res = palloc((size_t)(subfptr->DatLen) + 1);

            if(res == NULL)
            {
                if(Jmd->actmsg[msgnum - 1].subfield == NULL)
                {
                    pfree(subf);
                }

                msgapierr = MERR_NOMEM;
                return NULL;
            }

            memcpy(res, subfptr->Buffer, subfptr->DatLen);
            res[subfptr->DatLen] = '\0';

            if(Jmd->actmsg[msgnum - 1].subfield == NULL)
            {
                pfree(subf);
            }

            return res;
        }
    }

    if(Jmd->actmsg[msgnum - 1].subfield == NULL)
    {
        pfree(subf);
    }

    return NULL;
} /* Jam_GetKludge */

JAMHDR * Jam_GetHdr(MSGA * jm, dword msgnum)
{
    if(InvalidMh(jm))
    {
        return NULL;
    }

    msgapierr = MERR_NONE;

    if(msgnum == MSGNUM_CUR)
    {
        msgnum = jm->cur_msg;
    }
    else if(msgnum == MSGNUM_NEXT)
    {
        msgnum = jm->cur_msg + 1;

        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */

        jm->cur_msg = msgnum;
    }
    else if(msgnum == MSGNUM_PREV)
    {
        msgnum = jm->cur_msg - 1;

        if(msgnum == 0)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */

        jm->cur_msg = msgnum;
    }
    else
    {
        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        } /* endif */
    }

    if(!Jmd->actmsg_read)
    {
        Jam_ActiveMsgs(jm);

        if(msgnum > jm->num_msg)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
    }

    if(!Jmd->actmsg)
    {
        return NULL;
    }

    return &(Jmd->actmsg[msgnum - 1].hdr);
} /* Jam_GetHdr */

void Jam_WriteHdr(MSGA * jm, JAMHDR * jamhdr, dword msgnum)
{
    if(InvalidMh(jm) || !jamhdr)
    {
        msgapierr = MERR_BADA;
        return;
    }

    msgapierr = MERR_NONE;

    if(!Jmd->actmsg_read)
    {
        Jam_ActiveMsgs(jm);
    }

    if(!Jmd->actmsg)
    {
        return;
    }

    memcpy(&(Jmd->actmsg[msgnum - 1].hdr), jamhdr, sizeof(JAMHDR));
    lseek(Jmd->HdrHandle, Jmd->actmsg[msgnum - 1].TrueMsg, SEEK_SET);
    write_hdr(Jmd->HdrHandle, jamhdr);
}

dword Jam_HighMsg(JAMBASEptr jambase)
{
    dword highmsg;

    if(!jambase)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    msgapierr = MERR_NONE;
    lseek(jambase->IdxHandle, 0, SEEK_END);
    highmsg = (dword)tell(jambase->IdxHandle);
    return highmsg / IDX_SIZE;
}

void Jam_ActiveMsgs(MSGA * jm)
{
    if(InvalidMh(jm))
    {
        msgapierr = MERR_BADA;
        return;
    }

    msgapierr = MERR_NONE;
    read_allidx(Jmd);
    jm->num_msg = Jmd->HdrInfo.ActiveMsgs;
}

dword Jam_PosHdrMsg(MSGA * jm, dword msgnum, JAMIDXREC * jamidx, JAMHDR * jamhdr)
{
    if(InvalidMh(jm))
    {
        return 0;
    }

    if(!jamidx)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    msgapierr = MERR_NONE;

    if(!Jmd->actmsg_read)
    {
        Jam_ActiveMsgs(jm);
    }

    jamidx->HdrOffset = Jmd->actmsg[msgnum - 1].TrueMsg;

    if(jamidx->HdrOffset == 0xffffffffUL)
    {
        return 0;
    }

    if(lseek(Jmd->HdrHandle, jamidx->HdrOffset, SEEK_SET) == -1)
    {
        return 0;
    }

    if(read_hdr(Jmd->HdrHandle, jamhdr) == 0)
    {
        return 0;
    }

    if(jamhdr->Attribute & JMSG_DELETED)
    {
        return 0;
    }

    return 1;
} /* Jam_PosHdrMsg */

static int near Jam_Lock(MSGA * jm, int force)
{
    return !(mi.haveshare &&
             ((force) ? waitlock(Jmd->HdrHandle, 0L, 1L) : lock(Jmd->HdrHandle, 0L, 1L) == -1));
}

static void near Jam_Unlock(MSGA * jm)
{
    if(mi.haveshare)
    {
        unlock(Jmd->HdrHandle, 0L, 1L);
    }
}

sword Jam_WriteHdrInfo(JAMBASEptr jambase)
{
    if(!jambase)
    {
        msgapierr = MERR_BADA;
        return -1;
    }

    msgapierr = MERR_NONE;

    if(lseek(jambase->HdrHandle, 0, SEEK_SET) == -1)
    {
        return -1;
    }

    if(write_hdrinfo(jambase->HdrHandle, &(jambase->HdrInfo)) == 0)
    {
        return -1;
    }

    jambase->modified = 0;
    return 0;
}

static dword Jam_JamAttrToMsg(MSGH * msgh)
{
    dword attr = 0;

    if(InvalidMsgh(msgh))
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    if(msgh->Hdr.Attribute & JMSG_LOCAL)
    {
        attr |= MSGLOCAL;
    }

    if(msgh->Hdr.Attribute & JMSG_PRIVATE)
    {
        attr |= MSGPRIVATE;
    }

    if(msgh->Hdr.Attribute & JMSG_READ)
    {
        attr |= MSGREAD;
    }

    if(msgh->Hdr.Attribute & JMSG_SENT)
    {
        attr |= MSGSENT;
    }

    if(msgh->Hdr.Attribute & JMSG_KILLSENT)
    {
        attr |= MSGKILL;
    }

    if(msgh->Hdr.Attribute & JMSG_HOLD)
    {
        attr |= MSGHOLD;
    }

    if(msgh->Hdr.Attribute & JMSG_CRASH)
    {
        attr |= MSGCRASH;
    }

    if(msgh->Hdr.Attribute & JMSG_FILEREQUEST)
    {
        attr |= MSGFRQ;
    }

    if(msgh->Hdr.Attribute & JMSG_FILEATTACH)
    {
        attr |= MSGFILE;
    }

    if(msgh->Hdr.Attribute & JMSG_INTRANSIT)
    {
        attr |= MSGFWD;
    }

    if(msgh->Hdr.Attribute & JMSG_RECEIPTREQ)
    {
        attr |= MSGRRQ;
    }

    if(msgh->Hdr.Attribute & JMSG_ORPHAN)
    {
        attr |= MSGORPHAN;
    }

    if(msgh->Hdr.Attribute & JMSG_CONFIRMREQ)
    {
        attr |= MSGCPT;
    }

    if(msgh->Hdr.Attribute & JMSG_LOCKED)
    {
        attr |= MSGLOCKED;
    }

    if(msgh->Hdr.Attribute & JMSG_DIRECT)
    {
        attr |= MSGXX2;
    }

    if(msgh->Hdr.Attribute & JMSG_IMMEDIATE)
    {
        attr |= MSGIMM;
    }

    if(msgh->Hdr.Attribute2 & 0x00000800L)
    {
        attr |= MSGREADTMR;
    }

    return attr;
} /* Jam_JamAttrToMsg */

static dword Jam_MsgAttrToJam(XMSG * msg)
{
    dword attr = 0;

    if(InvalidMsg(msg))
    {
        return 0;
    }

    if(msg->attr & MSGLOCAL)
    {
        attr |= JMSG_LOCAL;
    }

    if(msg->attr & MSGPRIVATE)
    {
        attr |= JMSG_PRIVATE;
    }

    if(msg->attr & MSGREAD)
    {
        attr |= JMSG_READ;
    }

    if(msg->attr & MSGSENT)
    {
        attr |= JMSG_SENT;
    }

    if(msg->attr & MSGKILL)
    {
        attr |= JMSG_KILLSENT;
    }

    if(msg->attr & MSGHOLD)
    {
        attr |= JMSG_HOLD;
    }

    if(msg->attr & MSGCRASH)
    {
        attr |= JMSG_CRASH;
    }

    if(msg->attr & MSGFRQ)
    {
        attr |= JMSG_FILEREQUEST;
    }

    if(msg->attr & MSGFILE)
    {
        attr |= JMSG_FILEATTACH;
    }

    if(msg->attr & MSGFWD)
    {
        attr |= JMSG_INTRANSIT;
    }

    if(msg->attr & MSGRRQ)
    {
        attr |= JMSG_RECEIPTREQ;
    }

    if(msg->attr & MSGORPHAN)
    {
        attr |= JMSG_ORPHAN;
    }

    if(msg->attr & MSGCPT)
    {
        attr |= JMSG_CONFIRMREQ;
    }

    if(msg->attr & MSGLOCKED)
    {
        attr |= JMSG_LOCKED;
    }

    if(msg->attr & MSGXX2)
    {
        attr |= JMSG_DIRECT;
    }

    if(msg->attr & MSGIMM)
    {
        attr |= JMSG_IMMEDIATE;
    }

    return attr;
} /* Jam_MsgAttrToJam */

static dword Jam_MsgAttr2ToJam(XMSG * msg)
{
    if(InvalidMsg(msg))
    {
        return 0;
    }

    if(msg->attr & MSGREADTMR)
    {
        return 0x00000800L;
    }

    return 0L;
}

static int StrToSubfield(const unsigned char * str, dword lstr, dword * len, JAMSUBFIELD2ptr subf)
{
    /* warning: str is read only and NOT nul-terminated! */
    const unsigned char * kludge;
    word subtypes;

    if(!str || !subf)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    msgapierr = MERR_NONE;

    while(lstr > 0)
    {
        if(str[lstr - 1] == '\r')
        {
            lstr--;
        }
        else
        {
            break;
        }
    }
    kludge   = str;
    subtypes = JAMSFLD_FTSKLUDGE;

    switch(*str)
    {
        case 'F':

            if(lstr > 5 && strncmp((char *)str, "FMPT ", 5) == 0)
            {
                return 0;
            }
            else if(lstr > 6 && strncmp((char *)str, "FLAGS ", 6) == 0)
            {
                kludge   = str + 6;
                subtypes = JAMSFLD_FLAGS;
            }

            break;

        case 'I':

            if(lstr > 4 && strncmp((char *)str, "INTL ", 4) == 0)
            {
                return 0;
            }

            break;

        case 'M':

            if(lstr > 7 && strncmp((char *)str, "MSGID: ", 7) == 0)
            {
                kludge   = str + 7;
                subtypes = JAMSFLD_MSGID;
            }

            break;

        case 'P':

            if(lstr > 6 && strncmp((char *)str, "PATH: ", 6) == 0)
            {
                kludge   = str + 6;
                subtypes = JAMSFLD_PATH2D;
            }
            else if(lstr > 5 && strncmp((char *)str, "PID: ", 5) == 0)
            {
                kludge   = str + 5;
                subtypes = JAMSFLD_PID;
            }

            break;

        case 'R':

            if(lstr > 7 && strncmp((char *)str, "REPLY: ", 7) == 0)
            {
                kludge   = str + 7;
                subtypes = JAMSFLD_REPLYID;
            }

            break;

        case 'S':

            if(lstr > 9 && strncmp((char *)str, "SEEN-BY: ", 9) == 0)
            {
                kludge   = str + 9;
                subtypes = JAMSFLD_SEENBY2D;
            }

            break;

        case 'T':

            if(lstr > 5 && strncmp((char *)str, "TOPT ", 5) == 0)
            {
                return 0;
            }
            else if(lstr > 7 && strncmp((char *)str, "TZUTC: ", 7) == 0)
            {
                kludge   = str + 7;
                subtypes = JAMSFLD_TZUTCINFO;
            }

            break;

        case 'V':

            if(lstr > 4 && strncmp((char *)str, "Via ", 4) == 0)
            {
                kludge   = str + 4;
                subtypes = JAMSFLD_TRACE;
            }

            break;

        default:
            break;
    } /* switch */
    subf->LoID   = subtypes;
    subf->HiID   = 0;
    subf->DatLen = lstr - (dword)(kludge - str);
    memcpy(subf->Buffer, kludge, subf->DatLen);
    subf->Buffer[subf->DatLen] = '\0';

    if(len)
    {
        *len = sizeof(JAMBINSUBFIELD) + subf->DatLen;
    }

    return 1;
} /* StrToSubfield */

static int NETADDRtoSubf(NETADDR addr, dword * len, word opt, JAMSUBFIELD2ptr subf)
{
    /* opt = 0 origaddr */
    /* opt = 1 destaddr */
    if(!subf)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    if(addr.zone == 0 && addr.net == 0 && addr.node == 0 && addr.point == 0)
    {
        return 0;
    }

    if(addr.point)
    {
        sprintf((char *)(subf->Buffer), "%d:%d/%d.%d", addr.zone, addr.net, addr.node, addr.point);
    }
    else
    {
        sprintf((char *)(subf->Buffer), "%d:%d/%d", addr.zone, addr.net, addr.node);
    } /* endif */

    subf->DatLen = (dword)strlen((char *)(subf->Buffer));

    if(len)
    {
        *len = subf->DatLen + sizeof(JAMBINSUBFIELD);
    }

    subf->LoID = opt ? JAMSFLD_DADDRESS : JAMSFLD_OADDRESS;
    subf->HiID = 0;
    msgapierr  = MERR_NONE;
    return 1;
} /* NETADDRtoSubf */

static int FromToSubjTOSubf(word jamsfld, unsigned char * txt, dword * len, JAMSUBFIELD2ptr subf)
{
    if(!subf)
    {
        msgapierr = MERR_BADA;
        return 0;
    }

    subf->LoID = jamsfld;
    subf->HiID = 0;

    if(txt)
    {
        memmove(subf->Buffer, txt, subf->DatLen = (dword)strlen((char *)txt));
    }
    else
    {
        subf->Buffer = NULL;
        subf->DatLen = 0;
    }

    if(len)
    {
        *len = subf->DatLen + sizeof(JAMBINSUBFIELD);
    }

    return 1;
} /* FromToSubjTOSubf */

static void MSGAPI ConvertXmsgToJamHdr(MSGH * msgh,
                                       XMSG * msg,
                                       JAMHDRptr jamhdr,
                                       JAMSUBFIELD2LISTptr * subfield)
{
    JAMSUBFIELD2ptr SubFieldCur;
    struct tm stm, * ptm;
    dword clen, sublen;

    memset(jamhdr, '\0', sizeof(JAMHDR));

    if(InvalidMsgh(msgh) || InvalidMsg(msg))
    {
        return;
    }

    clen   = msgh->sq->isecho ? 3 : 5;
    sublen = sizeof(JAMSUBFIELD2LIST) + sizeof(JAMSUBFIELD2) * clen + 37 + 37 + 73 +
        (msgh->sq->isecho ? 0 : 30 * 2);
    *subfield = palloc(sublen);

    if(*subfield == NULL)
    {
        msgapierr = MERR_NOMEM;
        return;
    }

    subfield[0]->arraySize          = sublen;
    subfield[0]->subfieldCount      = 0;
    subfield[0]->subfield[0].Buffer = (unsigned char *)&(subfield[0]->subfield[clen + 1]);
    jamhdr->Attribute  = Jam_MsgAttrToJam(msg);
    jamhdr->Attribute2 = Jam_MsgAttr2ToJam(msg);

    if(msgh->sq->isecho != NOTH)
    {
        if(msgh->sq->isecho)
        {
            jamhdr->Attribute |= JMSG_TYPEECHO;
        }
        else
        {
            jamhdr->Attribute |= JMSG_TYPENET;
        } /* endif */
    }

    strcpy((char *)(jamhdr->Signature), HEADERSIGNATURE);
    jamhdr->Revision = CURRENTREVLEV;

    if(((SCOMBO *)&(msg->date_arrived))->ldate)
    {
        /* save arrived date for sqpack */
        ptm = &stm;
        ptm = DosDate_to_TmDate((SCOMBO *)(&(msg->date_arrived)), ptm);
        jamhdr->DateProcessed = (dword)(mktime(ptm) + gettz());
    }
    else
    {
        jamhdr->DateProcessed = (dword)(time(NULL) + gettz());
    }

    ptm = &stm;
    ptm = DosDate_to_TmDate((SCOMBO *)(&(msg->date_written)), ptm);
    jamhdr->DateWritten = (dword)(mktime(ptm) + gettz());
    sublen = 0;

    /* From Name */
    SubFieldCur = subfield[0]->subfield;

    if(FromToSubjTOSubf(JAMSFLD_SENDERNAME, msg->from, &clen, SubFieldCur))
    {
        SubFieldCur[1].Buffer = SubFieldCur->Buffer + SubFieldCur->DatLen + 1;
        subfield[0]->subfieldCount++;
        SubFieldCur++;
        sublen += clen;
    } /* endif */

    /* To Name */
    if(FromToSubjTOSubf(JAMSFLD_RECVRNAME, msg->to, &clen, SubFieldCur))
    {
        SubFieldCur[1].Buffer = SubFieldCur->Buffer + SubFieldCur->DatLen + 1;
        subfield[0]->subfieldCount++;
        SubFieldCur++;
        sublen += clen;
    } /* endif */

    /* Subject */
    if(FromToSubjTOSubf(JAMSFLD_SUBJECT, msg->subj, &clen, SubFieldCur))
    {
        SubFieldCur[1].Buffer = SubFieldCur->Buffer + SubFieldCur->DatLen + 1;
        subfield[0]->subfieldCount++;
        SubFieldCur++;
        sublen += clen;
    } /* endif */

    if(!msgh->sq->isecho)
    {
        /* Orig Address */
        if(NETADDRtoSubf(msg->orig, &clen, 0, SubFieldCur))
        {
            SubFieldCur[1].Buffer = SubFieldCur->Buffer + SubFieldCur->DatLen + 1;
            subfield[0]->subfieldCount++;
            SubFieldCur++;
            sublen += clen;
        } /* endif */

        /* Dest Address */
        if(NETADDRtoSubf(msg->dest, &clen, 1, SubFieldCur))
        {
            SubFieldCur[1].Buffer = SubFieldCur->Buffer + SubFieldCur->DatLen + 1;
            subfield[0]->subfieldCount++;
            SubFieldCur++;
            sublen += clen;
        } /* endif */
    }

    assert(SubFieldCur->Buffer <= (byte *)*subfield + subfield[0]->arraySize);
    assert((byte *)(SubFieldCur + 1) <= subfield[0]->subfield[0].Buffer);
    jamhdr->SubfieldLen = sublen;
    jamhdr->PasswordCRC = 0xFFFFFFFFUL;
    jamhdr->ReplyTo     = msg->replyto;
    jamhdr->Reply1st    = msg->xmreply1st;
    jamhdr->ReplyNext   = msg->xmreplynext;
    jamhdr->TimesRead   = msg->xmtimesread;
    jamhdr->Cost        = msg->xmcost;
    msgapierr           = MERR_NONE;
} /* ConvertXmsgToJamHdr */

static void resize_subfields(JAMSUBFIELD2LISTptr * sflist, dword newcount, dword len)
{
    #define LASTFIELD(x) ((x)->subfield[(x)->subfieldCount - 1])
    dword i;
    JAMSUBFIELD2LISTptr new_list, old_list;
    byte * new_buffer, * old_buffer;
    size_t new_buffer_size, old_buffer_size;

    if(!sflist || !*sflist || len == 0)
    {
        msgapierr = MERR_BADA;
        return;
    }

    old_list = *sflist;

    if(old_list->subfieldCount == 0)
    {
        old_buffer      = (byte *)&(old_list->subfield[0]);
        old_buffer_size = 0;
    }
    else
    {
        old_buffer = old_list->subfield[0].Buffer;
        /* really used size */
        old_buffer_size = LASTFIELD(old_list).Buffer + LASTFIELD(old_list).DatLen - old_buffer;
    }

    /* Check consistency of source structure */
    assert(old_buffer + old_buffer_size <= (byte *)old_list + old_list->arraySize);
    assert(newcount > old_list->subfieldCount); /* Code below relies on this */
    new_list = palloc(len);

    if(!new_list)
    {
        msgapierr = MERR_NOMEM;
        return;
    }

    new_list->arraySize     = len;
    new_list->subfieldCount = old_list->subfieldCount;
    new_buffer      = (byte *)&(new_list->subfield[newcount]);
    new_buffer_size = (size_t)new_list->arraySize - (size_t)(new_buffer - (byte *)new_list);
    if(new_buffer_size < old_buffer_size)
    {
        msgapierr = MERR_INTERN;
        return;
    }

    if(old_list->subfieldCount > 0)
    {
        /* copy array of JAMSUBFIELD2 */
        memcpy(new_list->subfield, old_list->subfield,
               new_list->subfieldCount * sizeof(JAMSUBFIELD2));

        /* fix pointers. note brackets: that's right, subtracting pointers
         * referring to two different objects gives UB */
        for(i = 0; i < old_list->subfieldCount; ++i)
        {
            new_list->subfield[i].Buffer = new_buffer +
                                           (old_list->subfield[i].Buffer - old_buffer);
        }
        /* copy data */
        memcpy(new_buffer, old_buffer, old_buffer_size);
    }

    /* Set up next subfield's buffer */
    new_list->subfield[new_list->subfieldCount].Buffer = new_buffer + old_buffer_size;
    freejamsubfield(old_list);
    *sflist   = new_list;
    msgapierr = MERR_NONE;
#undef LASTFIELD
} /* resize_subfields */

static void MSGAPI ConvertCtrlToSubf(JAMHDRptr jamhdr,
                                     JAMSUBFIELD2LISTptr * subfield,
                                     dword clen,
                                     unsigned char * ctxt)
{
    JAMSUBFIELD2ptr SubField;
    dword len, i;
    unsigned char * ptr;

    if(!subfield || !ctxt || !jamhdr)
    {
        msgapierr = MERR_BADA;
        return;
    }

    /* count new subfields */
    i = *ctxt ? 2 : 1;

    for(ptr = ctxt, len = 0; len < clen; len++)
    {
        if(*ptr++ == '\001')
        {
            i++;
        }
    }
    resize_subfields(subfield,
                     subfield[0]->subfieldCount + i,
                     subfield[0]->arraySize + clen + i + i * sizeof(JAMSUBFIELD2));
    SubField = &(subfield[0]->subfield[subfield[0]->subfieldCount]);

    for(ptr = ctxt, len = 0; len <= clen; len++, ptr++)
    {
        if(*ptr == '\0' || len == clen || *ptr == '\001')
        {
            if(*ctxt && *ctxt != '\001' && StrToSubfield(ctxt, (dword)(ptr - ctxt), &i, SubField))
            {
                SubField[1].Buffer   = SubField->Buffer + SubField->DatLen + 1;
                jamhdr->SubfieldLen += i;
                subfield[0]->subfieldCount++;

                if(SubField->LoID == JAMSFLD_MSGID)
                {
                    jamhdr->MsgIdCRC = Jam_Crc32(SubField->Buffer, SubField->DatLen);
                }
                else if(SubField->LoID == JAMSFLD_REPLYID)
                {
                    jamhdr->ReplyCRC = Jam_Crc32(SubField->Buffer, SubField->DatLen);
                }

                SubField++;
            }

            if(*ptr == '\0' || len == clen)
            {
                break;
            }

            ctxt = ptr + 1;
        }
    }
    assert(SubField->Buffer <= (byte *)*subfield + subfield[0]->arraySize);
    assert((byte *)(SubField + 1) <= subfield[0]->subfield[0].Buffer);
    msgapierr = MERR_NONE;
} /* ConvertCtrlToSubf */

unsigned char * DelimText(JAMHDRptr jamhdr,
                          JAMSUBFIELD2LISTptr * subfield,
                          unsigned char * text,
                          size_t textlen)
{
    JAMSUBFIELD2ptr SubField;
    dword clen, firstlen, i, len;
    unsigned char * onlytext, * first, * ptr, * curtext;

    if(!subfield || !jamhdr)
    {
        msgapierr = MERR_BADA;
        return NULL;
    }

    if(text && textlen)
    {
        if(text[textlen - 1] != '\r')
        {
            textlen++;
        }

        onlytext = curtext = (unsigned char *)palloc(textlen + 1);

        if(!onlytext)
        {
            msgapierr = MERR_NOMEM;
            return NULL;
        }

        *onlytext = '\0';
        /* count subfields */
        i   = 1;
        len = 0;

        for(first = text; first < text + textlen; first = ptr + 1)
        {
            ptr = (unsigned char *)strchr((char *)first, '\r');

            if(ptr == NULL)
            {
                ptr = text + textlen;
            }

            if(*first == '\001' || strncmp((char *)first, "SEEN-BY: ", 9) == 0)
            {
                if(*first == '\001')
                {
                    first++;
                }

                i++;
                len += (dword)(ptr - first);
            }
        }
        resize_subfields(subfield,
                         subfield[0]->subfieldCount + i,
                         subfield[0]->arraySize + len + i + i * sizeof(JAMSUBFIELD2));
        SubField = &(subfield[0]->subfield[subfield[0]->subfieldCount]);
        first    = text;

        while(*first)
        {
            ptr = (unsigned char *)strchr((char *)first, '\r');

            if(ptr)
            {
                *ptr = '\0';
            }

            firstlen = ptr ? (dword)(ptr - first) : (dword)strlen((char *)first);

            if((firstlen > 9 && strncmp((char *)first, "SEEN-BY: ", 9) == 0) || *first == '\001')
            {
                if(*first == '\001')
                {
                    first++;
                    firstlen--;
                }

                if(StrToSubfield(first, firstlen, &clen, SubField))
                {
                    SubField[1].Buffer   = SubField->Buffer + SubField->DatLen + 1;
                    jamhdr->SubfieldLen += clen;
                    subfield[0]->subfieldCount++;

                    if(SubField->LoID == JAMSFLD_MSGID)
                    {
                        jamhdr->MsgIdCRC = Jam_Crc32(SubField->Buffer, SubField->DatLen);
                    }
                    else if(SubField->LoID == JAMSFLD_REPLYID)
                    {
                        jamhdr->ReplyCRC = Jam_Crc32(SubField->Buffer, SubField->DatLen);
                    }

                    SubField++;
                }
            }
            else
            {
                assert((size_t)firstlen + 1 <= textlen);
                strcpy((char *)curtext, (char *)first);
                curtext   += firstlen;
                *curtext++ = '\r';
                *curtext   = '\0';
            } /* endif */

            if(ptr)
            {
                *ptr  = '\r';
                first = ptr + 1;
            }
            else
            {
                first += firstlen;
            } /* endif */
        } /* endwhile */
        assert(SubField->Buffer <= (byte *)*subfield + subfield[0]->arraySize);
        assert((byte *)(SubField + 1) <= subfield[0]->subfield[0].Buffer);
    }
    else
    {
        onlytext = NULL;
    }

    msgapierr = MERR_NONE;
    return onlytext;
} /* DelimText */

static void addkludge(char ** line, char * kludge, char * ent, char * lf, dword len)
{
    if(!line || !*line || !kludge || !ent || !lf)
    {
        msgapierr = MERR_BADA;
        return;
    }

    strcpy(*line, kludge);
    *line += strlen(kludge);
    strncpy(*line, ent, len);
    (*line)[len] = '\0';
    *line       += strlen(*line);
    strcpy(*line, lf);
    *line += strlen(*line);
}

/* WARNING: rewrites clen, ctrl, lclen, lctrl */
void DecodeSubf(MSGH * msgh)
{
    dword SubPos;
    JAMSUBFIELD2ptr SubField;
    JAMSUBFIELD2LISTptr sfl;
    char * ptr, * pctrl, * plctrl, * fmpt, * topt;
    char orig[101], dest[101]; /* by JAM's spec DADDRESS and OADDRESS
                                * subfield's DATLEN must not exceed 100 bytes.
                                * 101 for last \0 */
    dword i;

    if(InvalidMsgh(msgh))
    {
        return;
    }

    msgh->ctrl  = (byte *)palloc((size_t)(msgh->SubFieldPtr->arraySize) + 65);
    msgh->lctrl = (byte *)palloc((size_t)(msgh->SubFieldPtr->arraySize) + 65);

    if(!(msgh->ctrl && msgh->lctrl))
    {
        pfree(msgh->ctrl);
        pfree(msgh->lctrl);
        msgapierr = MERR_NOMEM;
        return;
    }

    *(msgh->ctrl) = *(msgh->lctrl) = '\0';
    pctrl         = (char *)(msgh->ctrl);
    plctrl        = (char *)(msgh->lctrl);
    orig[0]       = dest[0] = '\0';

    if(!msgh->sq->isecho)
    {
        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_OADDRESS);

        if(SubField != NULL)
        {
            memcpy(orig, SubField->Buffer, min(SubField->DatLen, sizeof(orig) - 1));
            orig[min(SubField->DatLen, sizeof(orig) - 1)] = '\0';
        }

        SubPos   = 0;
        SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_DADDRESS);

        if(SubField != NULL)
        {
            memcpy(dest, SubField->Buffer, min(SubField->DatLen, sizeof(dest) - 1));
            dest[min(SubField->DatLen, sizeof(dest) - 1)] = '\0';
        }

        fmpt = topt = NULL;

        if(orig[0])
        {
            ptr = strchr(orig, '@');

            if(ptr)
            {
                *ptr = '\0';
            }

            ptr = strchr(orig, '.');

            if(ptr)
            {
                *ptr++ = '\0';

                if(atoi(ptr) != 0)
                {
                    fmpt = ptr;
                }
            }
        }

        if(dest[0])
        {
            ptr = strchr(dest, '@');

            if(ptr)
            {
                *ptr = '\0';
            }

            ptr = strchr(dest, '.');

            if(ptr)
            {
                *ptr++ = '\0';

                if(atoi(ptr) != 0)
                {
                    topt = ptr;
                }
            }
        }

        if(orig[0] && dest[0])
        {
            strcpy(pctrl, "\001" "INTL ");
            pctrl += strlen(pctrl);
            strcpy(pctrl, dest);
            pctrl += strlen(pctrl);
            strcpy(pctrl, " ");
            pctrl++;
            strcpy(pctrl, orig);
            pctrl += strlen(pctrl);
        }

        if(fmpt)
        {
            addkludge(&pctrl, "\001" "FMPT ", "", fmpt, 0);
        }

        if(topt)
        {
            addkludge(&pctrl, "\001" "TOPT ", "", topt, 0);
        }

        orig[0] = dest[0] = '\0';
    }

    SubPos   = 0;
    sfl      = msgh->SubFieldPtr;
    SubField = &(sfl->subfield[0]);

    for(i = 0; i < sfl->subfieldCount; i++, SubField++)
    {
        if(SubField->LoID == JAMSFLD_MSGID)
        {
            addkludge(&pctrl, "\001MSGID: ", (char *)(SubField->Buffer), "", SubField->DatLen);
        }
        else if(SubField->LoID == JAMSFLD_REPLYID)
        {
            addkludge(&pctrl, "\001REPLY: ", (char *)(SubField->Buffer), "", SubField->DatLen);
        }
        else if(SubField->LoID == JAMSFLD_PID)
        {
            addkludge(&pctrl, "\001PID: ", (char *)(SubField->Buffer), "", SubField->DatLen);
        }
        else if(SubField->LoID == JAMSFLD_TRACE)
        {
            addkludge(&plctrl, "\001Via ", (char *)(SubField->Buffer), "\r", SubField->DatLen);
        }
        else if(SubField->LoID == JAMSFLD_FTSKLUDGE)
        {
            if(strncasecmp((char *)(SubField->Buffer), "Via",  3) == 0 ||
               strncasecmp((char *)(SubField->Buffer), "Recd", 4) == 0)
            {
                addkludge(&plctrl, "\001", (char *)(SubField->Buffer), "\r", SubField->DatLen);
            }
            else
            {
                addkludge(&pctrl, "\001", (char *)(SubField->Buffer), "", SubField->DatLen);
            }
        }
        else if(SubField->LoID == JAMSFLD_FLAGS)
        {
            addkludge(&pctrl, "\001" "FLAGS ", (char *)(SubField->Buffer), "", SubField->DatLen);
        }
        else if(SubField->LoID == JAMSFLD_PATH2D)
        {
            addkludge(&plctrl, "\001PATH: ", (char *)(SubField->Buffer), "\r", SubField->DatLen);
        }
        else if(SubField->LoID == JAMSFLD_SEENBY2D)
        {
            addkludge(&plctrl, "SEEN-BY: ", (char *)(SubField->Buffer), "\r", SubField->DatLen);
        }
        else if(SubField->LoID == JAMSFLD_TZUTCINFO)
        {
            addkludge(&pctrl, "\001TZUTC: ", (char *)(SubField->Buffer), "", SubField->DatLen);
        }
    } /* endwhile */

    msgh->clen  = (dword)(pctrl - (char *)msgh->ctrl);
    msgh->lclen = (dword)(plctrl - (char *)msgh->lctrl);
    assert(msgh->clen < msgh->SubFieldPtr->arraySize + 65);
    assert(msgh->lclen < msgh->SubFieldPtr->arraySize + 65);
    {
        byte * tmp1;
        tmp1 = (byte *)realloc(msgh->ctrl, (size_t)(msgh->clen) + 1);
        if(!tmp1)
        {
            pfree(msgh->ctrl);
            pfree(msgh->lctrl);
            msgapierr = MERR_NOMEM;
            return;
        }
        msgh->ctrl = tmp1;
        tmp1 = (byte *)realloc(msgh->lctrl, (size_t)(msgh->lclen) + 1);
        if(!tmp1)
        {
            pfree(msgh->ctrl);
            pfree(msgh->lctrl);
            msgapierr = MERR_NOMEM;
            return;
        }
        msgh->lctrl = tmp1;
    }

    msgapierr = MERR_NONE;
} /* DecodeSubf */

/***********************************************************************
**
**  Crc32 lookup table
**
***********************************************************************/
static hINT32 crc32tab[256] =
{
    0L,           1996959894L,           -301047508L,              -1727442502L,
    124634137L,
    1886057615L,
    -379345611L,  -1637575261L,
    249268274L,   2044508324L,           -522852066L,              -1747789432L,
    162941995L,
    2125561021L,
    -407360249L,
    -1866523247L, 498536548L,            1789927666L,              -205950648L,
    -2067906082L,
    450548861L,
    1843258603L,
    -187386543L,  -2083289657L,          325883990L,               1684777152L,
    -43845254L,
    -1973040660L, 335633487L,
    1661365465L,  -99664541L,            -1928851979L,             997073096L,
    1281953886L,
    -715111964L,
    -1570279054L,
    1006888145L,  1258607687L,           -770865667L,              -1526024853L,
    901097722L,
    1119000684L,
    -608450090L,
    -1396901568L, 853044451L,            1172266101L,              -589951537L,
    -1412350631L,
    651767980L,
    1373503546L,
    -925412992L,  -1076862698L,          565507253L,               1454621731L,
    -809855591L,
    -1195530993L, 671266974L,
    1594198024L,  -972236366L,           -1324619484L,             795835527L,
    1483230225L,
    -1050600021L,
    -1234817731L,
    1994146192L,  31158534L,             -1731059524L,             -271249366L,
    1907459465L,
    112637215L,
    -1614814043L,
    -390540237L,  2013776290L,           251722036L,               -1777751922L,
    -519137256L,
    2137656763L,  141376813L,
    -1855689577L, -429695999L,           1802195444L,              476864866L,
    -2056965928L,
    -228458418L,
    1812370925L,
    453092731L,   -2113342271L,          -183516073L,              1706088902L,
    314042704L,
    -1950435094L, -54949764L,
    1658658271L,  366619977L,            -1932296973L,             -69972891L,
    1303535960L,
    984961486L,
    -1547960204L,
    -725929758L,  1256170817L,           1037604311L,              -1529756563L,
    -740887301L,
    1131014506L,  879679996L,
    -1385723834L, -631195440L,           1141124467L,              855842277L,
    -1442165665L,
    -586318647L,
    1342533948L,
    654459306L,   -1106571248L,          -921952122L,              1466479909L,
    544179635L,
    -1184443383L,
    -832445281L,
    1591671054L,  702138776L,            -1328506846L,             -942167884L,
    1504918807L,
    783551873L,
    -1212326853L,
    -1061524307L, -306674912L,           -1698712650L,             62317068L,
    1957810842L,
    -355121351L,
    -1647151185L,
    81470997L,    1943803523L,           -480048366L,              -1805370492L,
    225274430L,
    2053790376L,
    -468791541L,
    -1828061283L, 167816743L,            2097651377L,              -267414716L,
    -2029476910L,
    503444072L,
    1762050814L,
    -144550051L,  -2140837941L,          426522225L,               1852507879L,
    -19653770L,
    -1982649376L, 282753626L,
    1742555852L,  -105259153L,           -1900089351L,             397917763L,
    1622183637L,
    -690576408L,
    -1580100738L,
    953729732L,   1340076626L,           -776247311L,              -1497606297L,
    1068828381L,
    1219638859L,
    -670225446L,
    -1358292148L, 906185462L,            1090812512L,              -547295293L,
    -1469587627L,
    829329135L,
    1181335161L,
    -882789492L,  -1134132454L,          628085408L,               1382605366L,
    -871598187L,
    -1156888829L, 570562233L,
    1426400815L,  -977650754L,           -1296233688L,             733239954L,
    1555261956L,
    -1026031705L,
    -1244606671L,
    752459403L,   1541320221L,           -1687895376L,             -328994266L,
    1969922972L,
    40735498L,
    -1677130071L,
    -351390145L,  1913087877L,           83908371L,                -1782625662L,
    -491226604L,
    2075208622L,  213261112L,
    -1831694693L, -438977011L,           2094854071L,              198958881L,
    -2032938284L,
    -237706686L,
    1759359992L,
    534414190L,   -2118248755L,          -155638181L,              1873836001L,
    414664567L,
    -2012718362L, -15766928L,
    1711684554L,  285281116L,            -1889165569L,             -127750551L,
    1634467795L,
    376229701L,
    -1609899400L,
    -686959890L,  1308918612L,           956543938L,               -1486412191L,
    -799009033L,
    1231636301L,
    1047427035L,
    -1362007478L, -640263460L,           1088359270L,              936918000L,
    -1447252397L,
    -558129467L,
    1202900863L,
    817233897L,   -1111625188L,          -893730166L,              1404277552L,
    615818150L,
    -1160759803L,
    -841546093L,
    1423857449L,  601450431L,            -1285129682L,             -1000256840L,
    1567103746L,
    711928724L,
    -1274298825L,
    -1022587231L, 1510334235L,           755167117L
};
/***********************************************************************
**
**  JAM_Crc32 - Calculate CRC32 on a data block
**
**  Note: This function returns data, NOT a status code!
**
***********************************************************************/
dword Jam_Crc32(unsigned char * buff, dword len)
{
    dword crc           = 0xffffffffUL;
    unsigned char * ptr = buff;

    if(buff)
    {
        for( ; len; len--, ptr++)
        {
            crc = (crc >> 8) ^ crc32tab[(int)((crc ^ tolower(*ptr)) & 0xffUL)];
        }
    }

    return crc;
}

static dword _XPENTRY JamGetHash(HAREA mh, dword msgnum)
{
    XMSG xmsg;
    HMSG msgh;
    dword rc = 0l;

    msgh = JamOpenMsg(mh, MOPEN_READ, msgnum);

    if(msgh == NULL)
    {
        return (dword)0l;
    }

    if(JamReadMsg(msgh, &xmsg, 0L, 0L, NULL, 0L, NULL) != (dword) - 1)
    {
        rc = SquishHash(xmsg.to) | ((xmsg.attr & MSGREAD) ? 0x80000000l : 0);
    }

    JamCloseMsg(msgh);
    msgapierr = MERR_NONE;
    return rc;
}

static UMSGID _XPENTRY JamGetNextUid(HAREA ha)
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
