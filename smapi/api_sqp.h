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

#ifndef __API_SQP_H_DEFINED
#define __API_SQP_H_DEFINED

#include <huskylib/compiler.h>

#include "msgapi.h"


sword _XPENTRY apiSquishCloseArea(HAREA sq);
HMSG  _XPENTRY apiSquishOpenMsg(HAREA sq,word mode,dword msgnum);
sword _XPENTRY apiSquishCloseMsg(HMSG msgh);
dword _XPENTRY apiSquishReadMsg(HMSG msgh, PXMSG msg, dword offset, dword bytes,
                              byte  *szText, dword clen, byte  *ctxt);
sword _XPENTRY apiSquishWriteMsg(HMSG msgh,word append,PXMSG msg,byte  *text,dword textlen,dword totlen,dword clen,byte  *ctxt);
sword _XPENTRY apiSquishKillMsg(HAREA sq,dword msgnum);
sword _XPENTRY apiSquishLock(HAREA sq);
sword _XPENTRY apiSquishUnlock(HAREA sq);
sword _XPENTRY apiSquishSetCurPos(HMSG msgh,dword pos);
dword _XPENTRY apiSquishGetCurPos(HMSG msgh);
UMSGID _XPENTRY apiSquishMsgnToUid(HAREA sq,dword msgnum);
dword _XPENTRY apiSquishUidToMsgn(HAREA sq,UMSGID umsgid,word type);
dword _XPENTRY apiSquishGetHash(HAREA sq,dword msgnum);
dword _XPENTRY apiSquishGetHighWater(HAREA mh);
sword _XPENTRY apiSquishSetHighWater(HAREA sq,dword hwm);
dword _XPENTRY apiSquishGetTextLen(HMSG msgh);
dword _XPENTRY apiSquishGetCtrlLen(HMSG msgh);
UMSGID _XPENTRY apiSquishGetNextUid(HAREA ha);

/* Private functions */
unsigned _SquishReadMode(HMSG hmsg);
unsigned _SquishWriteMode(HMSG hmsg);
unsigned _SquishCopyBaseToData(HAREA ha, SQBASE *psqb);
unsigned _SquishWriteBaseHeader(HAREA ha, SQBASE *psqb);
unsigned _SquishReadBaseHeader(HAREA ha, SQBASE *psqb);
unsigned _SquishExclusiveBegin(HAREA ha);
unsigned _SquishExclusiveEnd(HAREA ha);
unsigned _SquishCopyDataToBase(HAREA ha, SQBASE *psqb);
unsigned _SquishReadHdr(HAREA ha, FOFS fo, SQHDR *psqh);
unsigned _SquishWriteHdr(HAREA ha, FOFS fo, SQHDR *psqh);
/*unsigned _SquishReadIndexRecord(HAREA ha, dword dwMsg, SQIDX *psqi);*/
/*unsigned _SquishWriteIndexRecord(HAREA ha, dword dwMsg, SQIDX *psqi);*/
FOFS _SquishGetFrameOfs(HAREA ha, dword dwMsg);
/*unsigned _SquishRemoveIndex(HAREA ha, dword dwMsg, SQIDX *psqiOut, SQHDR *psqh);*/
unsigned _SquishSetFrameNext(HAREA ha, FOFS foModify, FOFS foValue);
unsigned _SquishSetFramePrev(HAREA ha, FOFS foModify, FOFS foValue);
unsigned _SquishInsertFreeChain(HAREA ha, FOFS fo, SQHDR *psqh);
/*SQIDX * _SquishAllocIndex(HAREA ha, dword dwMsg, dword *pdwIdxSize);*/
/*unsigned _SquishFreeIndex(HAREA ha, dword dwMsg, SQIDX *psqi,
                          dword dwIdxSize, unsigned fWrite);*/

HIDX _SquishOpenIndex(HAREA ha);
int _SquishBeginBuffer(HIDX hix);
int SidxGet(HIDX hix, dword dwMsg, SQIDX *psqi);
int SidxPut(HIDX hix, dword dwMsg, SQIDX *psqi);
unsigned _SquishRemoveIndexEntry(HIDX hix, dword dwMsg, SQIDX *psqiOut,
                                 SQHDR *psqh, int fFixPointers);
unsigned _SquishCloseIndex(HIDX hix);
int _SquishEndBuffer(HIDX hix);
int _SquishFreeBuffer(HIDX hix);
dword _SquishIndexSize(HIDX hix);
unsigned _SquishFixMemoryPointers(HAREA ha, dword dwMsg, SQHDR *psqh);

#endif /* __API_SQP_H_DEFINED */

