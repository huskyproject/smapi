/*
** Adapted for MSGAPI by Fedor Lizunkov 2:5020/960@FidoNet
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dr.h"

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
#include "stamp.h"
#include "alc.h"
#include "msgapi.h"
#include "api_jam.h"
#include "api_jamp.h"
#include "apidebug.h"
#include "unused.h"

#define Jmd ((JAMBASE *)(jm->apidata))
#define MsghJm ((JAMBASE *)(((struct _msgh *)msgh)->sq->apidata))

#ifdef __TURBOC__
#pragma warn -pia*
#pragma warn -ucp
#pragma warn -sig
#endif

MSG *MSGAPI JamOpenArea(byte * name, word mode, word type)
{
   MSG *jm;

   jm = palloc(sizeof(MSG));
   if (jm == NULL)
   {
      msgapierr = MERR_NOMEM;
      return NULL;
   }

   memset(jm, '\0', sizeof(MSG));

   jm->id = MSGAPI_ID;

   if (type & MSGTYPE_ECHO)
   {
      jm->isecho = TRUE;
   }

   jm->api = (struct _apifuncs *)palloc(sizeof(struct _apifuncs));
   if (jm->api == NULL) {
      msgapierr = MERR_NOMEM;
      pfree(jm);
      return NULL;
   }

   memset(jm->api, '\0', sizeof(struct _apifuncs));

   jm->apidata = (void*)palloc(sizeof(JAMBASE));

   if (jm->apidata == NULL) {
      msgapierr = MERR_NOMEM;
      pfree(jm->api);
      pfree(jm);
      return NULL;
   }

   memset((byte *) jm->apidata, '\0', sizeof(JAMBASE));

   jm->len = sizeof(MSG);

   jm->num_msg = 0;
   jm->high_msg = 0;
   jm->high_water = (dword) -1;

   if (!Jam_OpenBase(jm, &mode, name)) {
      pfree(jm->api);
      pfree((char*)jm->apidata);
      pfree(jm);
      msgapierr = MERR_BADF;
      return NULL;
   }

   Jam_ActiveMsgs(Jmd);

   jm->high_water = Jmd->HdrInfo.highwater;
   jm->high_msg = Jam_HighMsg(Jmd);
   jm->num_msg = Jmd->HdrInfo.ActiveMsgs;

   jm->type = MSGTYPE_JAM;

   jm->sz_xmsg = sizeof(XMSG);
   *jm->api = jm_funcs;

   msgapierr = 0;
   return jm;
}

static sword EXPENTRY JamCloseArea(MSG * jm)
{
   if (InvalidMh(jm))
   {
      return -1;
   }

   if (Jmd->msgs_open) {
      msgapierr = MERR_EOPEN;
      return -1;
   }

   Jmd->HdrInfo.highwater = jm->high_water;
   Jmd->HdrInfo.ModCounter++;
   Jam_WriteHdrInfo(Jmd);


   if (jm->locked) JamUnlock(jm);

   Jam_CloseFile(Jmd);

   pfree(Jmd->BaseName);
   pfree(Jmd->actmsg);
   pfree(jm->api);
   pfree((char *)jm->apidata);
   jm->id = 0L;
   pfree(jm);

   return 0;
}

static MSGH *EXPENTRY JamOpenMsg(MSG * jm, word mode, dword msgnum)
{
   struct _msgh *msgh;

   if (InvalidMh(jm)) {
      return NULL;
   }

   if (mode == MOPEN_CREATE) {
      if ((sdword) msgnum < 0 || msgnum > jm->num_msg) {
          msgapierr = MERR_NOENT;
          return NULL;
      }

      if (msgnum != 0) {
         msgh = Jam_OpenMsg(jm, mode, msgnum);

         if (msgh == NULL) {
            msgapierr = MERR_NOENT;
            return NULL;
         }
      } else {
         msgh = palloc(sizeof(struct _msgh));
         if (msgh == NULL) {
            msgapierr = MERR_NOMEM;
            return NULL;
         }

         memset(msgh, '\0', sizeof(struct _msgh));

         msgh->sq = jm;
         msgh->bytes_written = 0L;
         msgh->cur_pos = 0L;
         msgh->msgnum = msgnum;
         msgh->Hdr.TxtLen = 0;

      } /* endif */

   }
   else if (msgnum == 0) {
      msgapierr = MERR_NOENT;
      return NULL;
   } else {
      msgh = Jam_OpenMsg(jm, mode, msgnum);
      if (msgh == NULL) {
         msgapierr = MERR_NOENT;
         return NULL;
      }
   }

   msgh->mode = mode;
   msgh->id = MSGH_ID;

   MsghJm->msgs_open++;

   return (MSGH *) msgh;
}

static sword EXPENTRY JamCloseMsg(MSGH * msgh)
{
   if (InvalidMsgh(msgh))
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

static dword EXPENTRY JamReadMsg(MSGH * msgh, XMSG * msg, dword offset, dword bytes, byte * text, dword clen, byte * ctxt)
{
   JAMSUBFIELDptr SubField;
   dword SubPos, bytesread;
   struct tm *s_time;
   SCOMBO *scombo;
   unsigned char *ftsdate;

   if (InvalidMsgh(msgh))
   {
      return -1L;
   }

   if (msgh->Hdr.Attribute & JMSG_DELETED) {
      return -1L;
   } /* endif */

   if (msg) {
      /* make msg */

      msg->attr = Jam_JamAttrToMsg(msgh);

      memset(msg->from, '\0', XMSG_FROM_SIZE);
      memset(msg->to, '\0', XMSG_TO_SIZE);
      memset(msg->subj, '\0', XMSG_SUBJ_SIZE);

      /* get "from name" line */
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_SENDERNAME))) {
         strncpy(msg->from, SubField->Buffer, SubField->DatLen);
      } /* endif */

      /* get "to name" line */
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_RECVRNAME))) {
         strncpy(msg->to, SubField->Buffer, SubField->DatLen);
      } /* endif */

      /* get "subj" line */
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_SUBJECT))) {
         strncpy(msg->subj, SubField->Buffer, SubField->DatLen);
      } /* endif */

      /* get "orig address" line */
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_OADDRESS))) {
         parseAddr(&(msg->orig), SubField->Buffer, SubField->DatLen);
      } /* endif */

      /* get "dest address" line */
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_DADDRESS))) {
         parseAddr(&(msg->dest), SubField->Buffer, SubField->DatLen);
      } /* endif */


      s_time = localtime((time_t *)(&(msgh->Hdr.DateWritten)));
      scombo = (SCOMBO*)(&(msg->date_written));
      scombo = TmDate_to_DosDate(s_time, scombo);
      /* ftsdate = msg->__ftsc_date; */
      ftsdate = (unsigned char *)sc_time(scombo, (char *)(msg->__ftsc_date));

   } /* endif */

   bytesread = 0;

   if (bytes > 0 && text) {

      /* read text message */

      if (offset > (msgh->Hdr.TxtLen+msgh->lclen)) offset = msgh->Hdr.TxtLen+msgh->lclen;

      msgh->cur_pos = offset;

      if (offset >= msgh->Hdr.TxtLen) {
         offset -= msgh->Hdr.TxtLen;
         if (bytes > msgh->lclen) bytes = msgh->lclen;
         if (offset < msgh->lclen) {
            strncpy(text, msgh->lctrl+offset, bytes-offset);
            bytesread = bytes-offset;
         } else {
         } /* endif */
         msgh->cur_pos += bytesread;
      } else {
         lseek(MsghJm->TxtHandle, msgh->Hdr.TxtOffset+offset, SEEK_SET);
         if (bytes > msgh->Hdr.TxtLen-offset) {
            bytesread =  farread(MsghJm->TxtHandle, text, msgh->Hdr.TxtLen-offset);
            bytes -= (msgh->Hdr.TxtLen-offset);
            bytes -= offset;
            if (bytes > msgh->lclen) bytes = msgh->lclen;
            strncat(text, msgh->lctrl, bytes);
            bytesread += bytes;
         } else {
            bytesread = farread(MsghJm->TxtHandle, text, bytes);
         } /* endif */
         msgh->cur_pos += bytesread;
      } /* endif */

   }

   if (clen && ctxt) {
      /* read first kludges */
      if (clen > msgh->clen) clen = msgh->clen;
      strncpy(ctxt, msgh->ctrl, clen);
      ctxt[clen] = '\0';
   }


   msgapierr = MERR_NONE;
   return bytesread;
}

static sword EXPENTRY JamWriteMsg(MSGH * msgh, word append, XMSG * msg, byte * text, dword textlen, dword totlen, dword clen, byte * ctxt)
{
   /* not supported append if JAM !!! */

   JAMHDR         jamhdrNew;
   JAMIDXREC      jamidxNew;
   JAMSUBFIELDptr subfieldNew;
   XMSG           msg_old;
   MSG            *jm;

   char           ch = 0;
   unsigned char *onlytext = NULL;
   int            didlock = FALSE;

   assert(append == 0);


   if (InvalidMsgh(msgh)) {
      return -1L;
   }

   if (msgh->mode != MOPEN_CREATE && msgh->mode != MOPEN_WRITE && msgh->mode != MOPEN_RW) {
      return -1L;
   } /* endif */

   jm = msgh->sq;

   memset(&jamidxNew, '\0', sizeof(JAMIDXREC));
   memset(&jamhdrNew, '\0', sizeof(JAMHDR));

   if (!ctxt) clen = 0L;

   if (!text) textlen = 0L;

   if (textlen == 0L) text = NULL;

   if (clen == 0L) ctxt = NULL;

/*   if (msgh->mode != MOPEN_CREATE) {
      msgh->bytes_written = 0L;
      append = TRUE;
   }*/

   if (msgh->mode != MOPEN_CREATE) {
      if (clen) clen = 0;
      if (ctxt) ctxt = NULL;
   }


   if (clen && ctxt) if (clen < strlen((char*)ctxt)) clen = strlen((char*)ctxt)+1;

   subfieldNew = NULL;

   if (msg) ConvertXmsgToJamHdr(msgh, msg, &jamhdrNew, &subfieldNew);
   else if (msgh->mode != MOPEN_CREATE) {
      JamReadMsg(msgh, &msg_old, 0, 0, NULL, 0, NULL);
      ConvertXmsgToJamHdr(msgh, &msg_old, &jamhdrNew, &subfieldNew);
   }

   if (!jm->locked) {
      didlock = Jam_Lock(jm, 1);
   } else {
   } /* endif */

   if (clen && ctxt) ConvertCtrlToSubf(&jamhdrNew, &subfieldNew, clen, ctxt);

   if (textlen && text) onlytext = DelimText(&jamhdrNew, &subfieldNew, text);
   else if (msgh->mode != MOPEN_CREATE) {
      DelimText(&jamhdrNew, &subfieldNew, msgh->lctrl);
      jamhdrNew.TxtOffset = msgh->Hdr.TxtOffset;
      jamhdrNew.TxtLen = msgh->Hdr.TxtLen;
   }

   if (msgh->mode == MOPEN_CREATE) {
      /* no logic if msg not present */
      if (msg) {
         if (msgh->msgnum == 0) {
            /* new message in end of position */
            lseek(Jmd->IdxHandle, 0, SEEK_END);
            lseek(Jmd->HdrHandle, 0, SEEK_END);
            lseek(Jmd->TxtHandle, 0, SEEK_END);
            jamhdrNew.MsgNum = Jmd->HdrInfo.BaseMsgNum+(tell(Jmd->IdxHandle)/sizeof(JAMIDXREC));
            msgh->seek_idx = tell(Jmd->IdxHandle);
            msgh->seek_hdr = jamidxNew.HdrOffset = tell(Jmd->HdrHandle);
            jamidxNew.UserCRC = Jam_Crc32(msg->to, strlen(msg->to));
            jamidxNew.HdrOffset = msgh->seek_hdr;
            write_idx(Jmd->IdxHandle, &jamidxNew);
            jamhdrNew.TxtOffset = tell(Jmd->TxtHandle);
            jamhdrNew.TxtLen = strlen(oE  (ëÞ@ 2ÖáÃ&‚aÁŸ~è	aþvÖš1Âí€nmd->TxtHandle, onlytext, strlen(onlytext));
            msgh->cur_pos = tell(Jmd->TxtHandle);
            msgh->bytes_written = strlen(onlytext);
            jamhdrNew.ReplyCRC = 0xFFFFFFFF;
            write_hdr(Jmd->HdrHandle, &jamhdrNew);
            write_subfield(Jmd->HdrHandle, &subfieldNew, jamhdrNew.SubfieldLen);
            Jmd->HdrInfo.ActiveMsgs++;
            Jmd->HdrInfo.ModCounter++;
            Jam_WriteHdrInfo(Jmd);
            jm->high_msg++;
         } else {
            /* new messgae instead of old message position */
            msgh->Hdr.TxtLen = 0;
            msgh->Hdr.Attribute |= JMSG_DELETED;
            lseek(Jmd->HdrHandle, msgh->seek_hdr, SEEK_SET);
            write_hdr(Jmd->HdrHandle, &(msgh->Hdr));
            lseek(Jmd->HdrHandle, 0, SEEK_END);
            msgh->seek_hdr = tell(Jmd->HdrHandle);
            jamhdrNew.MsgNum = msgh->Hdr.MsgNum;
            jamidxNew.UserCRC = Jam_Crc32(msg->to, strlen(msg->to));
            jamidxNew.HdrOffset = msgh->seek_hdr;
            lseek(Jmd->IdxHandle, msgh->seek_idx, SEEK_SET);
            write_idx(Jmd->IdxHandle, &jamidxNew);
            lseek(Jmd->TxtHandle, 0, SEEK_END);
            jamhdrNew.TxtOffset = tell(Jmd->TxtHandle);
            jamhdrNew.TxtLen = strlen(onlytext);
            farwrite(Jmd->TxtHandle, onlytext, strlen(onlytext));
            msgh->cur_pos = tell(Jmd->TxtHandle);
            msgh->bytes_written = strlen(onlytext);
            lseek(Jmd->TxtHandle, jamhdrNew.TxtOffset+totlen-1, SEEK_SET);
            farwrite(Jmd->TxtHandle, &ch, 1);
            jamhdrNew.ReplyCRC = 0xFFFFFFFF;
            write_hdr(Jmd->HdrHandle, &jamhdrNew);
            write_subfield(Jmd->HdrHandle, &subfieldNew, jamhdrNew.SubfieldLen);
            Jmd->HdrInfo.ModCounter++;
            Jam_WriteHdrInfo(Jmd);
/*	    memmove(&(msgh->Idx), &(jamidxNew), sizeof(JAMIDXREC));
	    memmove(&(msgh->Hdr), &(jamhdrNew), sizeof(JAMHDR));
	    msgh->SubFieldPtr = subfieldNew; */
	    /* info from new message to msgh srtuct */
/*	    DecodeSubf(msgh); */
         } /* endif */
         pfree(subfieldNew);
      } /* endif */
   } else {
      /* change text and SEEN_BY, PATH, VIA kludges posible only (message != create)*/
      ConvertCtrlToSubf(&jamhdrNew, &subfieldNew, msgh->clen, msgh->ctrl);

      if (msg) {
         jamidxNew.UserCRC = Jam_Crc32(msg->to, strlen(msg->to));
      } else {
         jamidxNew.UserCRC = msgh->Idx.UserCRC;
      } /* endif */

      if (jamhdrNew.SubfieldLen > msgh->Hdr.SubfieldLen) {
         msgh->Hdr.TxtLen = 0;
         msgh->Hdr.Attribute |= JMSG_DELETED;
         lseek(Jmd->HdrHandle, msgh->seek_hdr, SEEK_SET);
         write_hdr(Jmd->HdrHandle, &(msgh->Hdr));
         lseek(Jmd->HdrHandle, 0, SEEK_END);
         msgh->seek_hdr = tell(Jmd->HdrHandle);
      } else {
      } /* endif */

      jamhdrNew.MsgNum = msgh->Hdr.MsgNum;
      jamidxNew.HdrOffset = msgh->seek_hdr;
      jamhdrNew.ReplyCRC = 0xFFFFFFFF;


      lseek(Jmd->IdxHandle, msgh->seek_idx, SEEK_SET);
      write_idx(Jmd->IdxHandle, &jamidxNew);

      if (text && textlen) {
         lseek(Jmd->TxtHandle, 0, SEEK_END);
         jamhdrNew.TxtOffset = tell(Jmd->TxtHandle);
         jamhdrNew.TxtLen = strlen(onlytext);
         farwrite(Jmd->TxtHandle, onlytext, strlen(onlytext));
      } /* endif */

      lseek(Jmd->HdrHandle, msgh->seek_hdr, SEEK_SET);
      write_hdr(Jmd->HdrHandle, &jamhdrNew);
      write_subfield(Jmd->HdrHandle, &subfieldNew, jamhdrNew.SubfieldLen);
      Jmd->HdrInfo.ModCounter++;
      Jam_WriteHdrInfo(Jmd);
      memmove(&(msgh->Idx), &(jamidxNew), sizeof(JAMIDXREC));
      memmove(&(msgh->Hdr), &(jamhdrNew), sizeof(JAMHDR));
      pfree(msgh->SubFieldPtr);
      msgh->SubFieldPtr = subfieldNew;
      DecodeSubf(msgh);

   } /* endif */

   if (didlock) {
      Jam_Unlock(jm);
   } else {
   } /* endif */

   pfree(onlytext);

   Jam_ActiveMsgs(Jmd);
   jm->num_msg = Jmd->HdrInfo.ActiveMsgs;

   return 0;
}


static sword EXPENTRY JamKillMsg(MSG * jm, dword msgnum)
{
   JAMIDXREC jamidx;
   JAMHDR    jamhdr;
   if (InvalidMh(jm)) {
      return -1;
   }

   if (jm->locked) return -1L;

   if (msgnum == 0 || msgnum > jm->num_msg) {
      msgapierr = MERR_NOENT;
      return -1L;
   } /* endif */

   if (!Jam_PosHdrMsg(jm, msgnum-1, &jamidx, &jamhdr)) {
      msgapierr = MERR_BADF;
      return -1L;
   } /* endif */

   if (JamLock(jm) == -1) return -1L;


   Jmd->HdrInfo.ActiveMsgs--;
   Jmd->HdrInfo.ModCounter++;
   jamhdr.TxtLen = 0;
   jamhdr.Attribute |= JMSG_DELETED;
   jamidx.UserCRC = 0xFFFFFFFFL;
   jamidx.HdrOffset = 0xFFFFFFFFL;
   lseek(Jmd->HdrHandle, -(sizeof(JAMHDR)), SEEK_CUR);
   lseek(Jmd->IdxHandle, -(sizeof(JAMIDXREC)), SEEK_CUR);
   write_idx(Jmd->IdxHandle, &jamidx);
   write_hdr(Jmd->HdrHandle, &jamhdr);
   Jam_WriteHdrInfo(Jmd);

   Jam_ActiveMsgs(Jmd);
   jm->num_msg = Jmd->HdrInfo.ActiveMsgs;

   JamUnlock(jm);
   return 0;
}

static sword EXPENTRY JamLock(MSG * jm)
{
   if (InvalidMh(jm)) {
      return -1;
   }

   /* Don't do anything if already locked */

   if (jm->locked) {
      return 0;
   }

   if (!Jam_Lock(jm, 0)) {

      return -1;
   }

   jm->locked = TRUE;

   return 0;
}

static sword EXPENTRY JamUnlock(MSG * jm)
{
   if (InvalidMh(jm)) {
      return -1;
   }

   if (!jm->locked) {
      return -1;
   }

   jm->locked = FALSE;

   if (mi.haveshare) {
      unlock(Jmd->HdrHandle, 0L, 1L);
   }

   return 0;
}

static sword EXPENTRY JamSetCurPos(MSGH * msgh, dword pos)
{
    if (InvalidMsgh(msgh))
    {
        return -1;
    }

    msgh->cur_pos = pos;
    return 0;
}

static dword EXPENTRY JamGetCurPos(MSGH * msgh)
{
    if (InvalidMsgh(msgh))
    {
        return -1;
    }

    return msgh->cur_pos;
}

static UMSGID EXPENTRY JamMsgnToUid(MSG * jm, dword msgnum)
{
    if (InvalidMh(jm))
    {
        return (UMSGID) - 1;
    }

    msgapierr = MERR_NONE;
    return (UMSGID) (msgnum + Jmd->HdrInfo.BaseMsgNum - 1);
}

static dword EXPENTRY JamUidToMsgn(MSG * jm, UMSGID umsgid, word type)
{
   JAMIDXREC idxmsg;
   JAMHDR hdrmsg;
   dword  msgnum;

   if (InvalidMh(jm)) {
      return -1L;
   }

   msgnum = umsgid - Jmd->HdrInfo.BaseMsgNum + 1;

   while (1) {
      if (!Jam_PosHdrMsg(jm, msgnum-1, &idxmsg, &hdrmsg)) {
         if (type == UID_EXACT) return 0;
         else if (msgnum != 0 && type == UID_PREV) msgnum--;
         else if (msgnum <= jm->high_msg && type == UID_NEXT) msgnum++;
         if ((msgnum == 0 && type == UID_PREV) ||
            (msgnum > jm->high_msg && type == UID_NEXT)) return 0;
      } else {
         return msgnum;
      } /* endif */
   } /* endwhile */
}

static dword EXPENTRY JamGetHighWater(MSG * jm)
{
   if (InvalidMh(jm))
   {
      return -1L;
   }

   return jm->high_water;
}

static sword EXPENTRY JamSetHighWater(MSG * jm, dword hwm)
{
   if (InvalidMh(jm))
   {
      return -1L;
   }

   if (hwm > jm->high_msg) return -1L;

   jm->high_water = hwm;

   return 0;
}

static dword EXPENTRY JamGetTextLen(MSGH * msgh)
{
   return (msgh->Hdr.TxtLen+msgh->lclen);
}

static dword EXPENTRY JamGetCtrlLen(MSGH * msgh)
{
   return (msgh->clen);
}


sword MSGAPI JamValidate(byte * name)
{
    byte temp[PATHLEN];

    sprintf((char *) temp, "%s%s", name, EXT_HDRFILE);

    if (!fexist((char *) temp))
    {
        return FALSE;
    }

    sprintf((char *) temp, "%s%s", name, EXT_TXTFILE);

    if (!fexist((char *) temp))
    {
        return FALSE;
    }

    sprintf((char *) temp, "%s%s", name, EXT_IDXFILE);

    if (!fexist((char *) temp))
    {
        return FALSE;
    }

    return TRUE;
}

void Jam_CloseFile(JAMBASE *jambase)
{
   if (jambase->HdrHandle != 0 || jambase->HdrHandle != -1) {
      close(jambase->HdrHandle);
   } /* endif */

   if (jambase->TxtHandle != 0 || jambase->TxtHandle != -1) {
      close(jambase->TxtHandle);
   } /* endif */

   if (jambase->IdxHandle != 0 || jambase->IdxHandle != -1) {
      close(jambase->IdxHandle);
   } /* endif */

   if (jambase->LrdHandle != 0 || jambase->LrdHandle != -1) {
      close(jambase->LrdHandle);
   } /* endif */
}

int openfilejm(char *name, word mode)
{
   int handle;

#ifdef UNIX
      handle = sopen(name, mode, SH_DENYNONE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#else
      handle = sopen(name, mode, SH_DENYNONE, S_IREAD | S_IWRITE);
#endif

   return handle;
}

int Jam_OpenFile(JAMBASE *jambase, word *mode)
{
   char *hdr, *idx, *txt, *lrd;

   hdr = (char*)palloc(strlen(jambase->BaseName)+5);
   idx = (char*)palloc(strlen(jambase->BaseName)+5);
   txt = (char*)palloc(strlen(jambase->BaseName)+5);
   lrd = (char*)palloc(strlen(jambase->BaseName)+5);

   sprintf(hdr, "%s%s", jambase->BaseName, EXT_HDRFILE);
   sprintf(txt, "%s%s", jambase->BaseName, EXT_TXTFILE);
   sprintf(idx, "%s%s", jambase->BaseName, EXT_IDXFILE);
   sprintf(lrd, "%s%s", jambase->BaseName, EXT_LRDFILE);

   if (*mode == MSGAREA_CREATE) {
      jambase->HdrHandle = openfilejm(hdr, fop_wpb);
      jambase->TxtHandle = openfilejm(txt, fop_wpb);
      jambase->IdxHandle = openfilejm(idx, fop_wpb);
      jambase->LrdHandle = openfilejm(lrd, fop_wpb);

      memset(&(jambase->HdrInfo), '\0', sizeof(JAMHDRINFO));
      strcpy(jambase->HdrInfo.Signature, HEADERSIGNATURE);

      jambase->HdrInfo.DateCreated = time(NULL);
      jambase->HdrInfo.ModCounter  = 1;
      jambase->HdrInfo.PasswordCRC = 0xffffffff;
      jambase->HdrInfo.BaseMsgNum  = 1;

      write_hdrinfo(jambase->HdrHandle, &(jambase->HdrInfo));

   } else {
      jambase->HdrHandle = openfilejm(hdr, fop_rpb);
      jambase->TxtHandle = openfilejm(txt, fop_rpb);
      jambase->IdxHandle = openfilejm(idx, fop_rpb);
      jambase->LrdHandle = openfilejm(lrd, fop_rpb);
   } /* endif */

   if (jambase->HdrHandle == -1 || jambase->TxtHandle == -1 || jambase->IdxHandle == -1) {
      if (*mode != MSGAREA_CRIFNEC) {
         Jam_CloseFile(jambase);
         pfree(hdr);
         pfree(txt);
         pfree(idx);
         pfree(lrd);
         msgapierr = MERR_NOENT;
         return 0;
      }
      *mode = MSGAREA_CREATE;
      jambase->HdrHandle = openfilejm(hdr, fop_wpb|O_EXCL);
      jambase->TxtHandle = openfilejm(txt, fop_wpb|O_EXCL);
      jambase->IdxHandle = openfilejm(idx, fop_wpb|O_EXCL);
      jambase->LrdHandle = openfilejm(lrd, fop_wpb|O_EXCL);

      if (jambase->HdrHandle == -1 || jambase->TxtHandle == -1 || jambase->IdxHandle == -1) {
         Jam_CloseFile(jambase);
         pfree(hdr);
         pfree(txt);
         pfree(idx);
         pfree(lrd);
         msgapierr = MERR_NOENT;
         return 0;
      } /* endif */


      memset(&(jambase->HdrInfo), '\0', sizeof(JAMHDRINFO));
      strcpy(jambase->HdrInfo.Signature, HEADERSIGNATURE);
      jambase->HdrInfo.DateCreated = time(NULL);
      jambase->HdrInfo.ModCounter  = 1;
      jambase->HdrInfo.PasswordCRC = 0xffffffff;
      jambase->HdrInfo.BaseMsgNum  = 1;

      write_hdrinfo(jambase->HdrHandle, &(jambase->HdrInfo));

   } /* endif */

   pfree(hdr);
   pfree(txt);
   pfree(idx);
   pfree(lrd);

   return 1;
}

static sword MSGAPI Jam_OpenBase(MSG *jm, word *mode, unsigned char *basename)
{
   Jmd->BaseName = (unsigned char*)palloc(strlen(basename)+1);
   strcpy(Jmd->BaseName, basename);

   if (!Jam_OpenFile(Jmd, mode)) {
      pfree(Jmd->BaseName);
      return 0;
   } /* endif */

   lseek(Jmd->HdrHandle, 0, SEEK_SET);
   read_hdrinfo(Jmd->HdrHandle, &(Jmd->HdrInfo));

   return 1;
}

static MSGH *Jam_OpenMsg(MSG * jm, word mode, dword msgnum)
{
   struct _msgh *msgh;
/*   JAMIDXREC    idx; */
/*   JAMHDR       hdr; */

   unused(mode);

   if (msgnum == MSGNUM_CUR) {
      msgnum = jm->cur_msg;
   } else if (msgnum == MSGNUM_NEXT) {
      msgnum = jm->cur_msg+1;
      if (msgnum > jm->num_msg) {
         msgapierr = MERR_NOENT;
         return NULL;
      } /* endif */
      jm->cur_msg = msgnum;
   } else if (msgnum == MSGNUM_PREV) {
      msgnum = jm->cur_msg-1;
      if (msgnum == 0) {
        msgapierr = MERR_NOENT;
        return NULL;
      } /* endif */
      jm->cur_msg = msgnum;
   }

   msgh = palloc(sizeof(struct _msgh));
   if (msgh == NULL) {
      msgapierr = MERR_NOMEM;
      return NULL;
   }

   memset(msgh, '\0', sizeof(struct _msgh));

   msgh->sq = jm;
   msgh->msgnum = msgnum;

   msgh->mode = mode;
   msgh->id = MSGH_ID;

/*   msgh->Idx.HdrOffset = 0xffffffff; */
/*   msgh->Idx.UserCRC   = 0xffffffff; */


   if (Jmd->actmsg) {
      if (lseek(Jmd->IdxHandle, Jmd->actmsg[msgnum-1].IdxOffset, SEEK_SET) != -1) {
         msgh->seek_idx = tell(Jmd->IdxHandle);
         if (read_idx(Jmd->IdxHandle, &(msgh->Idx))) {
            if (msgh->Idx.HdrOffset != 0xffffffff) {
               msgh->seek_hdr = msgh->Idx.HdrOffset;
               lseek(Jmd->HdrHandle, msgh->Idx.HdrOffset, SEEK_SET);
               read_hdr(Jmd->HdrHandle, &(msgh->Hdr));
               if (stricmp((char*)&msgh->Hdr, "JAM") != 0) {
                  pfree(msgh);
                  return NULL;
               } else {
               } /* endif */
               if(mode == MOPEN_CREATE) return (MSGH *)msgh;
               msgh->SubFieldPtr = (JAMSUBFIELDptr)palloc(msgh->Hdr.SubfieldLen);
               read_subfield(Jmd->HdrHandle, &(msgh->SubFieldPtr), msgh->Hdr.SubfieldLen);
               DecodeSubf(msgh);
               return (MSGH *) msgh;
            }
         }
      }
   }

   pfree(msgh);
   return NULL;
}

JAMSUBFIELDptr Jam_GetSubField(struct _msgh *msgh, dword *SubPos, dword what)
{
   JAMSUBFIELDptr SubField;

   while (*SubPos < msgh->Hdr.SubfieldLen) {
      SubField = (JAMSUBFIELDptr)((char*)(msgh->SubFieldPtr)+(*SubPos));
      *SubPos += (SubField->DatLen+sizeof(JAMBINSUBFIELD));
      if (SubField->LoID == what) {
         return SubField;
      } /* endif */
   } /* endwhile */

   return NULL;
}

dword Jam_HighMsg(JAMBASEptr jambase)
{
   dword highmsg;
   lseek(jambase->IdxHandle, 0, SEEK_END);
   highmsg = tell(jambase->IdxHandle);
   return (highmsg / sizeof(JAMIDXREC));
}

void Jam_ActiveMsgs(JAMBASEptr jambase)
{
   read_allidx(jambase);
}

dword Jam_PosHdrMsg(MSG * jm, dword msgnum, JAMIDXREC *jamidx, JAMHDR *jamhdr)
{
   if (lseek(Jmd->IdxHandle, Jmd->actmsg[msgnum].IdxOffset, SEEK_SET) == -1) return 0;

   if (read_idx(Jmd->IdxHandle, jamidx) == 0) return 0;

   if (jamidx->HdrOffset == 0xffffffff) return 0;

   if (lseek(Jmd->HdrHandle, jamidx->HdrOffset, SEEK_SET) == -1) return 0;

   if (read_hdr(Jmd->HdrHandle, jamhdr) == 0) return 0;

   if (jamhdr->Attribute & JMSG_DELETED) return 0;

   return 1;
}


static int near Jam_Lock(MSG *jm, int force)
{
    return !(mi.haveshare && ((force) ? waitlock(Jmd->HdrHandle, 0L, 1L)
                                      :     lock(Jmd->HdrHandle, 0L, 1L) == -1));
}

static void near Jam_Unlock(MSG * jm)
{
    if (mi.haveshare)
    {
        unlock(Jmd->HdrHandle, 0L, 1L);
    }
}

sword Jam_WriteHdrInfo(JAMBASEptr jambase)
{
   if (lseek(jambase->HdrHandle, 0, SEEK_SET) == -1) return -1;
   if (write_hdrinfo(jambase->HdrHandle, &(jambase->HdrInfo)) == 0) return -1;
   return 0;
}

static dword Jam_JamAttrToMsg(MSGH *msgh)
{
   dword attr = 0;

   if (msgh->Hdr.Attribute & JMSG_LOCAL)       attr |= MSGLOCAL;
   if (msgh->Hdr.Attribute & JMSG_PRIVATE)     attr |= MSGPRIVATE;
   if (msgh->Hdr.Attribute & JMSG_READ)        attr |= MSGREAD;
   if (msgh->Hdr.Attribute & JMSG_SENT)        attr |= MSGSENT;
   if (msgh->Hdr.Attribute & JMSG_KILLSENT)    attr |= MSGKILL;
   if (msgh->Hdr.Attribute & JMSG_HOLD)        attr |= MSGHOLD;
   if (msgh->Hdr.Attribute & JMSG_CRASH)       attr |= MSGCRASH;
   if (msgh->Hdr.Attribute & JMSG_FILEREQUEST) attr |= MSGFRQ;
   if (msgh->Hdr.Attribute & JMSG_FILEATTACH)  attr |= MSGFILE;
   if (msgh->Hdr.Attribute & JMSG_INTRANSIT)   attr |= MSGFWD;
   if (msgh->Hdr.Attribute & JMSG_RECEIPTREQ)  attr |= MSGRRQ;
   if (msgh->Hdr.Attribute & JMSG_ORPHAN)      attr |= MSGORPHAN;
   if (msgh->Hdr.Attribute & JMSG_CONFIRMREQ)  attr |= MSGCPT;
   if (msgh->Hdr.Attribute & JMSG_LOCKED)      attr |= MSGLOCKED;

   return attr;
}

static dword Jam_MsgAttrToJam(XMSG *msg)
{
   dword attr = 0;

   if (msg->attr & MSGLOCAL)   attr |= JMSG_LOCAL;
   if (msg->attr & MSGPRIVATE) attr |= JMSG_PRIVATE;
   if (msg->attr & MSGREAD)    attr |= JMSG_READ;
   if (msg->attr & MSGSENT)    attr |= JMSG_SENT;
   if (msg->attr & MSGKILL)    attr |= JMSG_KILLSENT;
   if (msg->attr & MSGHOLD)    attr |= JMSG_HOLD;
   if (msg->attr & MSGCRASH)   attr |= JMSG_CRASH;
   if (msg->attr & MSGFRQ)     attr |= JMSG_FILEREQUEST;
   if (msg->attr & MSGFILE)    attr |= JMSG_FILEATTACH;
   if (msg->attr & MSGFWD)     attr |= JMSG_INTRANSIT;
   if (msg->attr & MSGRRQ)     attr |= JMSG_RECEIPTREQ;
   if (msg->attr & MSGORPHAN)  attr |= JMSG_ORPHAN;
   if (msg->attr & MSGCPT)     attr |= JMSG_CONFIRMREQ;
   if (msg->attr & MSGLOCKED)  attr |= JMSG_LOCKED;

   return attr;
}

static JAMSUBFIELDptr StrToSubfield(unsigned char *str, dword *len)
{
   JAMSUBFIELDptr subf;
   unsigned char *kludge;
   dword subtypes;

   for (subtypes = strlen(str); subtypes > 0; subtypes--) {
      if (str[subtypes-1] == '\r') str[subtypes-1] = '\0';
      else break;
   } /* endfor */

   if (strstr(str, "INTL ")) return NULL;
   else if (strstr(str, "FMPT ")) return NULL;
   else if (strstr(str, "TOPT ")) return NULL;
   else if (strstr(str, "SEEN-BY: ")) {
      kludge = str+9;
      subtypes = JAMSFLD_SEENBY2D;
   }
   else if (strstr(str, "PATH: ")) {
      kludge = str+6;
      subtypes = JAMSFLD_PATH2D;
   }
   else if (strstr(str, "MSGID: ")) {
      kludge = str+7;
      subtypes = JAMSFLD_MSGID;
   }
   else if (strstr(str, "REPLY: ")) {
      kludge = str+7;
      subtypes = JAMSFLD_REPLYID;
   }
   else if (strstr(str, "PID: ")) {
      kludge = str+5;
      subtypes = JAMSFLD_PID;
   }
   else {
      kludge = str;
      subtypes = JAMSFLD_FTSKLUDGE;
   }

   *len = sizeof(JAMBINSUBFIELD)+strlen(kludge);
   subf = (JAMSUBFIELDptr)palloc(*len);
   if (!subf) return NULL;
   memset(subf, '\0', *len);

   subf->LoID = subtypes;
   subf->DatLen = strlen(kludge);
   memmove(subf->Buffer, kludge, strlen(kludge));


   return subf;
}

JAMSUBFIELDptr NETADDRtoSubf(NETADDR addr, dword *len, word opt)
{
   /* opt = 0 origaddr */
   /* opt = 1 destaddr */

   JAMSUBFIELDptr subf;
   char buf[30];


   if (addr.point) {
      sprintf(buf, "%d:%d/%d.%d", addr.zone, addr.net, addr.node, addr.point);
   } else {
      sprintf(buf, "%d:%d/%d", addr.zone, addr.net, addr.node);
   } /* endif */

   *len = sizeof(JAMBINSUBFIELD)+strlen(buf);
   subf = (JAMSUBFIELDptr)palloc(*len);
   if (!subf) return NULL;
   memset(subf, '\0', *len);
   if (opt) {
      subf->LoID = JAMSFLD_DADDRESS;
   } else {
      subf->LoID = JAMSFLD_OADDRESS;
   } /* endif */
   subf->DatLen = strlen(buf);
   memmove(subf->Buffer, buf, strlen(buf));

   return subf;
}

static JAMSUBFIELDptr FromToSubjTOSubf(dword jamsfld, unsigned char *txt, dword *len)
{
   JAMSUBFIELDptr subf;

   *len = sizeof(JAMBINSUBFIELD)+strlen(txt);
   subf = (JAMSUBFIELDptr)palloc(*len);
   if (!subf) return NULL;
   memset(subf, '\0', *len);
   subf->LoID = jamsfld;
   subf->DatLen = strlen(txt);
   memmove(subf->Buffer, txt, strlen(txt));

   return subf;
}

static void MSGAPI ConvertXmsgToJamHdr(MSGH *msgh, XMSG *msg, JAMHDRptr jamhdr, JAMSUBFIELDptr *subfield)
{
   JAMSUBFIELDptr SubFieldCur, SubField;
   struct tm stm, *ptm;
   dword clen, sublen;

   SubField = *subfield;

   memset(jamhdr, '\0', sizeof(JAMHDR));

   jamhdr->Attribute = Jam_MsgAttrToJam(msg);
   if (msgh->sq->isecho) {
      jamhdr->Attribute |= JMSG_TYPEECHO;
   } else {
      jamhdr->Attribute |= JMSG_TYPENET;
   } /* endif */

   strcpy(jamhdr->Signature, HEADERSIGNATURE);
   jamhdr->Revision = CURRENTREVLEV;
   jamhdr->DateProcessed = time(NULL);
   ptm = &stm;
   ptm = DosDate_to_TmDate((SCOMBO*)(&(msg->date_written)), ptm);
   jamhdr->DateWritten = mktime(ptm);

   sublen = 0;

   /* From Name */

   SubFieldCur = FromToSubjTOSubf(JAMSFLD_SENDERNAME, msg->from, &clen);
   if (SubFieldCur) {
      SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+clen);
      memmove((char*)SubField+sublen, SubFieldCur, clen);
      free(SubFieldCur);
      sublen += clen;
   } /* endif */

   /* To Name */

   SubFieldCur = FromToSubjTOSubf(JAMSFLD_RECVRNAME, msg->to, &clen);
   if (SubFieldCur) {
      SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+clen);
      memmove((char*)SubField+sublen, SubFieldCur, clen);
      free(SubFieldCur);
      sublen += clen;
   } /* endif */

   /* Subject */

   SubFieldCur = FromToSubjTOSubf(JAMSFLD_SUBJECT, msg->subj, &clen);
   if (SubFieldCur) {
      SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+clen);
      memmove((char*)SubField+sublen, SubFieldCur, clen);
      free(SubFieldCur);
      sublen += clen;
   } /* endif */

   /* Orig Address */

   if ((SubFieldCur = NETADDRtoSubf(msg->orig, &clen, 0))) {
      SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+clen);
      memmove((char*)SubField+sublen, SubFieldCur, clen);
      free(SubFieldCur);
      sublen += clen;
   }

   /* Dest Address */

   if ((SubFieldCur = NETADDRtoSubf(msg->dest, &clen, 1))) {
      SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+clen);
      memmove((char*)SubField+sublen, SubFieldCur, clen);
      free(SubFieldCur);
      sublen += clen;
   }

   jamhdr->SubfieldLen = sublen;
   jamhdr->PasswordCRC = 0xFFFFFFFF;

   *subfield = SubField;
}

static void MSGAPI ConvertCtrlToSubf(JAMHDRptr jamhdr, JAMSUBFIELDptr
                                   *subfield, dword clen, unsigned char *ctxt)
{
   JAMSUBFIELDptr SubFieldCur, SubField;
   dword len, sublen;
   unsigned char *ctrl, *ctrlp, *ptr;

   sublen = jamhdr->SubfieldLen;
   SubField = *subfield;

   ctrl = (unsigned char*)palloc(clen+1);
   strcpy((char *)ctrl, ctxt);
   ctrl[clen] = '\0';

   ctrlp = ctrl;

   ptr = (unsigned char *)strchr((char *)ctrlp, '\1');

   while (ptr) {
      *ptr = '\0';
      if (*ctrlp) {
         if ((SubFieldCur = StrToSubfield(ctrlp, &len))) {
            SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+len);
            memmove((char*)SubField+sublen, SubFieldCur, len);
            free(SubFieldCur);
            sublen += len;
         }
      }
      ptr++;
      ctrlp = ptr;
      ptr = (unsigned char *)strchr(ctrlp, '\1');
   }

   if (*ctrlp && (SubFieldCur = StrToSubfield(ctrlp, &len))) {
      SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+len);
      memmove((char*)SubField+sublen, SubFieldCur, len);
      free(SubFieldCur);
      sublen += len;
   }

   pfree(ctrl);

   ctrl = (unsigned char*)GetCtrlToken(ctxt, (unsigned char *)"MSGID");
   if (ctrl) {
      jamhdr->MsgIdCRC = Jam_Crc32(ctrl+7, strlen(ctrl)-7);
      pfree(ctrl);
   }
   ctrl = (unsigned char*)GetCtrlToken(ctxt, (unsigned char *)"REPLY");
   if (ctrl) {
      jamhdr->ReplyCRC = Jam_Crc32(ctrl+7, strlen(ctrl)-7);
      pfree(ctrl);
   }

   jamhdr->SubfieldLen = sublen;
   *subfield = SubField;
}

unsigned char *DelimText(JAMHDRptr jamhdr, JAMSUBFIELDptr *subfield, unsigned char *text)
{
   JAMSUBFIELDptr SubField, SubFieldCur;
   dword sublen, clen;
   unsigned char *onlytext, *first, *ptr;

   SubField = *subfield;

   sublen = jamhdr->SubfieldLen;

   onlytext = (unsigned char*)palloc(1);
   *onlytext = '\0';

   first = text;
   while (*first) {
      ptr = (unsigned char *)strchr(first, '\r');
      if (ptr) *ptr = 0;
      if (strstr(first, "SEEN-BY: ") == (char*)first  || *first == '\1') {

         if (*first == '\1') first++;

         if ((SubFieldCur = StrToSubfield(first, &clen))) {
            SubField = (JAMSUBFIELDptr)farrealloc(SubField, sublen+clen);
            memmove((char*)SubField+sublen, SubFieldCur, clen);
            free(SubFieldCur);
            sublen += clen;
         } else {;}

      } else {
         onlytext = (unsigned char*)farrealloc(onlytext, strlen(onlytext)+strlen(first)+2);
         sprintf(onlytext+strlen(onlytext), "%s\r", first);
      } /* endif */
      if (ptr) {
         *ptr = '\r';
         first = ptr+1;
      } else {
         first += strlen(first);
      } /* endif */
   } /* endwhile */

   jamhdr->SubfieldLen = sublen;
   *subfield = SubField;

   return onlytext;
}

int makeKludge(char **buff, char *sstr, unsigned char *str, char *ent, int len)
{
   if (!*buff) *buff = (char*)calloc(1, sizeof(char));

   if (!*buff) return 0;

   *buff = (char*)realloc(*buff, strlen(*buff)+strlen(sstr)+strlen(ent)+len+1);

   if (!*buff) return 0;

   strcat(*buff, sstr);
   strncat(*buff, str, len);
   strcat(*buff, ent);

   return 1;
}

void parseAddr(NETADDR *netAddr, unsigned char *str, dword len)
{
   char *strAddr, *ptr, *tmp, ch[10];

   strAddr = (char*)calloc(len+1, sizeof(char*));
   if (!strAddr) return;

   memset(netAddr, 0, sizeof(NETADDR));


   strncpy(strAddr, str, len);

   ptr = strchr(strAddr, '@');

   if (ptr)  *ptr = 0;

   ptr = strchr(strAddr, ':');
   if (ptr) {
      memset(ch, 0, sizeof(ch));
      strncpy(ch, strAddr, ptr-strAddr);
      netAddr->zone = atoi(ch);
      tmp = ++ptr;
   } else {
      tmp = strAddr;
      netAddr->zone = 0;
   } /* endif */

   ptr = strchr(tmp, '/');
   if (ptr) {
      memset(ch, 0, sizeof(ch));
      strncpy(ch, tmp, ptr-tmp);
      netAddr->net = atoi(ch);
      tmp = ++ptr;
   } else {
      netAddr->net = 0;
   } /* endif */

   ptr = strchr(tmp, '.');
   if (ptr) {
      memset(ch, 0, sizeof(ch));
      strncpy(ch, tmp, ptr-tmp);
      netAddr->node = atoi(ch);
      ptr++;
      netAddr->point = atoi(ptr);
   } else {
      netAddr->node = atoi(tmp);
      netAddr->point = 0;
   } /* endif */
}

void DecodeSubf(MSGH *msgh)
{
   dword  SubPos;
   JAMSUBFIELDptr SubField;
   char *msgid, *reply, *via, *pid, *seenby, *path, *flags, *kludges;
   char *orig, *dest, *intl, *ptr, *fmpt, *topt;

   msgid=reply=via=pid=seenby=path=flags=kludges = NULL;
   orig=dest=intl=fmpt=topt = NULL;

   if (!msgh->sq->isecho) {
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_OADDRESS))) {
         makeKludge(&orig, "", SubField->Buffer, "", SubField->DatLen);
      }
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_DADDRESS))) {
         makeKludge(&dest, "", SubField->Buffer, "", SubField->DatLen);
      }
      if (orig) {
         ptr = strchr(orig, '@');
         if (ptr) *ptr = '\0';
         ptr = strchr(orig, '.');
         if (ptr) {
            *(ptr++) = '\0';
            if (atoi(ptr) != 0) fmpt = ptr;
         }
      }
      if (dest) {
         ptr = strchr(dest, '@');
         if (ptr) *ptr = '\0';
         ptr = strchr(dest, '.');
         if (ptr) {
            *(ptr++) = '\0';
            if (atoi(ptr) != 0) topt = ptr;
         }
      }
      if (orig && dest) {
         intl = (char*)palloc(strlen(orig)+strlen(dest)+8);
         sprintf(intl, "%cINTL %s %s", '\x01', dest, orig);
      }
   } else {
   } /* endif */
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_MSGID))) {
         makeKludge(&msgid, "\x01MSGID: ", SubField->Buffer, "", SubField->DatLen);
      }
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_REPLYID))) {
         makeKludge(&reply, "\x01REPLY: ", SubField->Buffer, "", SubField->DatLen);
      }
      SubPos = 0;
      if ((SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_PID))) {
         makeKludge(&pid, "\x01PID: ", SubField->Buffer, "", SubField->DatLen);
      }
      for (SubPos = 0; (SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_FTSKLUDGE));) {
#ifdef UNIX
         if (strncasecmp(SubField->Buffer, "VIA", 3) == 0) {
#else
         if (strnicmp(SubField->Buffer, "VIA", 3) == 0) {
#endif
            makeKludge(&via, "\x01", SubField->Buffer, "\r", SubField->DatLen);
         } else {
            makeKludge(&kludges, "\x01", SubField->Buffer, "", SubField->DatLen);
         }
      }
      for (SubPos = 0; (SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_FLAGS));) {
         makeKludge(&flags, "\x01", SubField->Buffer, "", SubField->DatLen);
      }
      for (SubPos = 0; (SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_SEENBY2D));) {
         makeKludge(&seenby, "SEEN-BY: ", SubField->Buffer, "\r", SubField->DatLen);
      }
      for (SubPos = 0; (SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_PATH2D));) {
         makeKludge(&seenby, "\x01PATH: ", SubField->Buffer, "\r", SubField->DatLen);
      }
      for (SubPos = 0; (SubField = Jam_GetSubField(msgh, &SubPos, JAMSFLD_FLAGS));) {
         makeKludge(&path, "\x01", SubField->Buffer, "\r", SubField->DatLen);
      }
      msgh->ctrl = (unsigned char*)palloc(1);
      msgh->lctrl = (unsigned char*)palloc(1);
      *(msgh->ctrl)=*(msgh->lctrl)='\0';
      if (intl) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(intl)+1);
         strcat(msgh->ctrl, intl);
      }
      if (topt) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(topt)+7);
         sprintf(msgh->ctrl+strlen(msgh->ctrl), "%cTOPT %s", '\x01', topt);
      }
      if (fmpt) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(fmpt)+7);
         sprintf(msgh->ctrl+strlen(msgh->ctrl), "%cFMPT %s", '\x01', fmpt);
      }
      if (msgid) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(msgid)+1);
         strcat(msgh->ctrl, msgid);
      }
      if (reply) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(reply)+1);
         strcat(msgh->ctrl, reply);
      }
      if (pid) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(pid)+1);
         strcat(msgh->ctrl, pid);
      }
      if (kludges) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(kludges)+1);
         strcat(msgh->ctrl, kludges);
      }
      if (flags) {
         msgh->ctrl = (unsigned char*)realloc(msgh->ctrl, strlen(msgh->ctrl)+strlen(flags)+1);
         strcat(msgh->ctrl, flags);
      }

      if (seenby) {
         msgh->lctrl = (unsigned char*)realloc(msgh->lctrl, strlen(msgh->lctrl)+strlen(seenby)+1);
         strcat(msgh->lctrl, seenby);
      }
      if (path) {
         msgh->lctrl = (unsigned char*)realloc(msgh->lctrl, strlen(msgh->lctrl)+strlen(path)+1);
         strcat(msgh->lctrl, path);
      }
      if (via) {
         msgh->lctrl = (unsigned char*)realloc(msgh->lctrl, strlen(msgh->lctrl)+strlen(via)+1);
         strcat(msgh->lctrl, via);
      }
      msgh->clen = strlen(msgh->ctrl);
      msgh->lclen = strlen(msgh->lctrl);

      pfree(kludges);
      pfree(pid);
      pfree(seenby);
      pfree(path);
      pfree(flags);
      pfree(msgid);
      pfree(reply);
      pfree(intl);
      pfree(via);
}

/***********************************************************************
**
**  Crc32 lookup table
**
***********************************************************************/
static long crc32tab[256]= {
              0L,  1996959894L,  -301047508L, -1727442502L,   124634137L,
     1886057615L,  -379345611L, -1637575261L,   249268274L,  2044508324L,
     -522852066L, -1747789432L,   162941995L,  2125561021L,  -407360249L,
    -1866523247L,   498536548L,  1789927666L,  -205950648L, -2067906082L,
      450548861L,  1843258603L,  -187386543L, -2083289657L,   325883990L,
     1684777152L,   -43845254L, -1973040660L,   335633487L,  1661365465L,
      -99664541L, -1928851979L,   997073096L,  1281953886L,  -715111964L,
    -1570279054L,  1006888145L,  1258607687L,  -770865667L, -1526024853L,
      901097722L,  1119000684L,  -608450090L, -1396901568L,   853044451L,
     1172266101L,  -589951537L, -1412350631L,   651767980L,  1373503546L,
     -925412992L, -1076862698L,   565507253L,  1454621731L,  -809855591L,
    -1195530993L,   671266974L,  1594198024L,  -972236366L, -1324619484L,
      795835527L,  1483230225L, -1050600021L, -1234817731L,  1994146192L,
       31158534L, -1731059524L,  -271249366L,  1907459465L,   112637215L,
    -1614814043L,  -390540237L,  2013776290L,   251722036L, -1777751922L,
     -519137256L,  2137656763L,   141376813L, -1855689577L,  -429695999L,
     1802195444L,   476864866L, -2056965928L,  -228458418L,  1812370925L,
      453092731L, -2113342271L,  -183516073L,  1706088902L,   314042704L,
    -1950435094L,   -54949764L,  1658658271L,   366619977L, -1932296973L,
      -69972891L,  1303535960L,   984961486L, -1547960204L,  -725929758L,
     1256170817L,  1037604311L, -1529756563L,  -740887301L,  1131014506L,
      879679996L, -1385723834L,  -631195440L,  1141124467L,   855842277L,
    -1442165665L,  -586318647L,  1342533948L,   654459306L, -1106571248L,
     -921952122L,  1466479909L,   544179635L, -1184443383L,  -832445281L,
     1591671054L,   702138776L, -1328506846L,  -942167884L,  1504918807L,
      783551873L, -1212326853L, -1061524307L,  -306674912L, -1698712650L,
       62317068L,  1957810842L,  -355121351L, -1647151185L,    81470997L,
     1943803523L,  -480048366L, -1805370492L,   225274430L,  2053790376L,
     -468791541L, -1828061283L,   167816743L,  2097651377L,  -267414716L,
    -2029476910L,   503444072L,  1762050814L,  -144550051L, -2140837941L,
      426522225L,  1852507879L,   -19653770L, -1982649376L,   282753626L,
     1742555852L,  -105259153L, -1900089351L,   397917763L,  1622183637L,
     -690576408L, -1580100738L,   953729732L,  1340076626L,  -776247311L,
    -1497606297L,  1068828381L,  1219638859L,  -670225446L, -1358292148L,
      906185462L,  1090812512L,  -547295293L, -1469587627L,   829329135L,
     1181335161L,  -882789492L, -1134132454L,   628085408L,  1382605366L,
     -871598187L, -1156888829L,   570562233L,  1426400815L,  -977650754L,
    -1296233688L,   733239954L,  1555261956L, -1026031705L, -1244606671L,
      752459403L,  1541320221L, -1687895376L,  -328994266L,  1969922972L,
       40735498L, -1677130071L,  -351390145L,  1913087877L,    83908371L,
    -1782625662L,  -491226604L,  2075208622L,   213261112L, -1831694693L,
     -438977011L,  2094854071L,   198958881L, -2032938284L,  -237706686L,
     1759359992L,   534414190L, -2118248755L,  -155638181L,  1873836001L,
      414664567L, -2012718362L,   -15766928L,  1711684554L,   285281116L,
    -1889165569L,  -127750551L,  1634467795L,   376229701L, -1609899400L,
     -686959890L,  1308918612L,   956543938L, -1486412191L,  -799009033L,
     1231636301L,  1047427035L, -1362007478L,  -640263460L,  1088359270L,
      936918000L, -1447252397L,  -558129467L,  1202900863L,   817233897L,
    -1111625188L,  -893730166L,  1404277552L,   615818150L, -1160759803L,
     -841546093L,  1423857449L,   601450431L, -1285129682L, -1000256840L,
     1567103746L,   711928724L, -1274298825L, -1022587231L,  1510334235L,
      755167117L
};


/***********************************************************************
**
**  JAM_Crc32 - Calculate CRC32 on a data block
**
**  Note: This function returns data, NOT a status code!
**
***********************************************************************/
dword Jam_Crc32(unsigned char* buff, dword len)
{
    dword crc = 0xffffffff;
    unsigned char *ptr = buff;

    for (; len; len--, ptr++)
        crc=(crc >> 8) ^ crc32tab [(int) ((crc^tolower(*ptr)) & 0xffUL)];
    return crc;
}

