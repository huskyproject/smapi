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

#ifndef __API_SDMP_H__
#define __API_SDMP_H__

static sword _XPENTRY SdmCloseArea(MSGA * mh);
static MSGH *_XPENTRY SdmOpenMsg(MSGA * mh, word mode, dword msgnum);
static sword _XPENTRY SdmCloseMsg(MSGH * msgh);
static dword _XPENTRY SdmReadMsg(MSGH * msgh, XMSG * msg, dword offset, dword bytes, byte * text, dword clen, byte * ctxt);
static sword _XPENTRY SdmWriteMsg(MSGH * msgh, word append, XMSG * msg, byte * text, dword textlen, dword totlen, dword clen, byte * ctxt);
static sword _XPENTRY SdmKillMsg(MSGA * mh, dword msgnum);
static sword _XPENTRY SdmLock(MSGA * mh);
static sword _XPENTRY SdmUnlock(MSGA * mh);
static sword _XPENTRY SdmSetCurPos(MSGH * msgh, dword pos);
static dword _XPENTRY SdmGetCurPos(MSGH * msgh);
static UMSGID _XPENTRY SdmMsgnToUid(MSGA * mh, dword msgnum);
static dword _XPENTRY SdmUidToMsgn(MSGA * mh, UMSGID umsgid, word type);
static dword _XPENTRY SdmGetHighWater(MSGA * mh);
static sword _XPENTRY SdmSetHighWater(MSGA * sq, dword hwm);
static dword _XPENTRY SdmGetTextLen(MSGH * msgh);
static dword _XPENTRY SdmGetCtrlLen(MSGH * msgh);
static UMSGID _XPENTRY SdmGetNextUid(HAREA ha);
static dword  _XPENTRY SdmGetHash(HAREA mh, dword msgnum);


static void Convert_Fmsg_To_Xmsg(struct _omsg *fmsg, XMSG * msg, word def_zone);
static void Convert_Xmsg_To_Fmsg(XMSG * msg, struct _omsg *fmsg);
static void Init_Xmsg(XMSG * msg);
static sword near _SdmRescanArea(MSGA * mh);
static sword near _Grab_Clen(MSGH * msgh);
static void _stdc WriteToFd(byte * str);
static void near Get_Binary_Date(struct _stamp *todate, struct _stamp *fromdate, byte * asciidate);

static int statfd;  /* file handle for WriteToFd */
static byte *sd_msg = (byte *) "%s%u.msg";


static struct _apifuncs sdm_funcs =
{
    SdmCloseArea,
    SdmOpenMsg,
    SdmCloseMsg,
    SdmReadMsg,
    SdmWriteMsg,
    SdmKillMsg,
    SdmLock,
    SdmUnlock,
    SdmSetCurPos,
    SdmGetCurPos,
    SdmMsgnToUid,
    SdmUidToMsgn,
    SdmGetHighWater,
    SdmSetHighWater,
    SdmGetTextLen,
    SdmGetCtrlLen,
    SdmGetNextUid,
    SdmGetHash
};

#endif
