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

#ifndef __API_SQP_H__
#define __API_SQP_H__

static sword EXPENTRY SquishCloseArea(MSG * sq);
static MSGH *EXPENTRY SquishOpenMsg(MSG * sq, word mode, dword msgnum);
static sword EXPENTRY SquishCloseMsg(MSGH * msgh);
static dword EXPENTRY SquishReadMsg(MSGH * msgh, XMSG * msg, dword offset, dword bytes, byte * text, dword clen, byte * ctxt);
static sword EXPENTRY SquishWriteMsg(MSGH * msgh, word append, XMSG * msg, byte * text, dword textlen, dword totlen, dword clen, byte * ctxt);
static sword EXPENTRY SquishKillMsg(MSG * sq, dword msgnum);
static sword EXPENTRY SquishLock(MSG * sq);
static sword EXPENTRY SquishUnlock(MSG * sq);
static sword EXPENTRY SquishSetCurPos(MSGH * msgh, dword pos);
static dword EXPENTRY SquishGetCurPos(MSGH * msgh);
static UMSGID EXPENTRY SquishMsgnToUid(MSG * sq, dword msgnum);
static dword EXPENTRY SquishUidToMsgn(MSG * sq, UMSGID umsgid, word type);
static dword EXPENTRY SquishGetHighWater(MSG * mh);
static sword EXPENTRY SquishSetHighWater(MSG * sq, dword hwm);
static dword EXPENTRY SquishGetTextLen(MSGH * msgh);
static dword EXPENTRY SquishGetCtrlLen(MSGH * msgh);
static sword MSGAPI _OpenSquish(MSG * sq, word * mode);
static SQHDR *MSGAPI _SquishGotoMsg(MSG * sq, dword msgnum, FOFS * seek_frame, SQIDX * idx, word updptrs);
static MSGH *_SquishOpenMsgRead(MSG * sq, word mode, dword msgnum);
static sword MSGAPI _SquishReadHeader(MSG * sq, dword ofs, SQHDR * hdr);
static sword MSGAPI _SquishWriteHeader(MSG * sq, dword ofs, SQHDR * hdr);
static sword MSGAPI _SquishUpdateHeaderNext(MSG * sq, dword ofs, SQHDR * hdr, dword newval);
static sword MSGAPI _SquishUpdateHeaderPrev(MSG * sq, dword ofs, SQHDR * hdr, dword newval);
static sword MSGAPI _SquishWriteSq(MSG * sq);
static sword MSGAPI _SquishUpdateSq(MSG * sq, word force);
static void MSGAPI Init_Hdr(SQHDR * sh);
static void SqbaseToSq(struct _sqbase *sqbase, MSG * sq);
static void SqToSqbase(MSG * sq, struct _sqbase *sqbase);
static sword near AddIndex(MSG * sq, SQIDX * ix, dword msgnum);
static sword near Add_To_Free_Chain(MSG * sq, FOFS killofs, SQHDR * killhdr);
static sword near _SquishReadIndex(MSG * sq);
static sword near _SquishWriteIndex(MSG * sq);
static sword near _SquishGetIdxFrame(MSG * sq, dword num, SQIDX * idx);
static void far *near farmemmove(void far * destin, const void far * source, unsigned n);
static void far *near farmemset(void far * s, int c, size_t length);
static int near _SquishLock(MSG * sq, int force);
static void near _SquishUnlock(MSG * sq);
static sword near _SquishFindFree(MSG * sq, FOFS * this_frame, dword totlen, dword clen, SQHDR * freehdr, FOFS * last_frame, SQHDR * lhdr, MSGH * msgh);

#define fop_wpb (O_CREAT | O_TRUNC | O_RDWR | O_BINARY)
#define fop_rpb (O_RDWR | O_BINARY)

#define Sqd ((struct _sqdata *)(sq->apidata))
#define MsghSqd ((struct _sqdata *)(((struct _msgh far *)msgh)->sq->apidata))

static struct _apifuncs sq_funcs =
{
    SquishCloseArea,
    SquishOpenMsg,
    SquishCloseMsg,
    SquishReadMsg,
    SquishWriteMsg,
    SquishKillMsg,
    SquishLock,
    SquishUnlock,
    SquishSetCurPos,
    SquishGetCurPos,
    SquishMsgnToUid,
    SquishUidToMsgn,
    SquishGetHighWater,
    SquishSetHighWater,
    SquishGetTextLen,
    SquishGetCtrlLen
};

static byte *ss_sqd = (byte *) "%s.sqd";
static byte *ss_sqi = (byte *) "%s.sqi";
/*static byte *ss_sql = (byte *) "%s.sql";*/

static struct _sqdata *_junksq;

#endif
