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
/*
#pragma off(unreferenced)
static char rcs_id[]="$Id$";
#pragma on(unreferenced)
*/
#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <huskylib/compiler.h>

#ifdef HAS_IO_H
#  include <io.h>
#endif
#ifdef HAS_SHARE_H
#include <share.h>
#endif
#ifdef HAS_MALLOC_H
#include <malloc.h>
#endif

#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"

/* Set the "current position" pointer in a message handle */

sword _XPENTRY apiSquishSetCurPos(HMSG hmsg, dword dwOfs)
{
  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return -1;

  hmsg->cur_pos=dwOfs;
  return 0;
}



/* Return the current read position within a message */

dword _XPENTRY apiSquishGetCurPos(HMSG hmsg)
{
  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1;

  return hmsg->cur_pos;
}


/* Return the length of the text body of this message */

dword _XPENTRY apiSquishGetTextLen(HMSG hmsg)
{
  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1L;

  return hmsg->sqhRead.msg_length - XMSG_SIZE - hmsg->sqhRead.clen;
}




/* Return the length of this message's control information */

dword _XPENTRY apiSquishGetCtrlLen(HMSG hmsg)
{
  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1L;

  return hmsg->sqhRead.clen;
}


/* Return the number of the high water marker */

dword _XPENTRY apiSquishGetHighWater(HAREA ha)
{
  if (MsgInvalidHarea(ha))
    return (dword)-1L;

  return apiSquishUidToMsgn(ha, ha->high_water, UID_PREV);
}




/* Set the high water marker for this area */

sword _XPENTRY apiSquishSetHighWater(HAREA ha, dword dwMsg)
{
  if (MsgInvalidHarea(ha))
    return -1;

  /* Make sure that the message exists */

  if (dwMsg > ha->num_msg)
  {
    msgapierr=MERR_NOENT;

    return -1;
  }

  if (!_SquishExclusiveBegin(ha))
    return -1;

  ha->high_water=apiSquishMsgnToUid(ha, dwMsg);


  if (!_SquishExclusiveEnd(ha))
  {
    return -1;
  }

  return 0;
}



/* Function to set the highest/skip message numbers for a *.SQ? base */

void _XPENTRY apiSquishSetMaxMsg(HAREA ha, dword dwMaxMsgs, dword dwSkipMsgs, dword dwMaxDays)
{
  if (MsgInvalidHarea(ha))
    return;

  /* Update base only if max msg settings have changed */

  if ((dwMaxMsgs  != (dword)-1L && dwMaxMsgs  != Sqd->dwMaxMsg) ||
      (dwSkipMsgs != (dword)-1L && dwSkipMsgs != Sqd->wSkipMsg) ||
      (dwMaxDays  != (dword)-1L && dwMaxDays  != Sqd->wMaxDays))
  {
    if (!_SquishExclusiveBegin(ha))
    {
      return;
    }

    if (dwMaxMsgs != (dword)-1L)
      Sqd->dwMaxMsg=dwMaxMsgs;

    if (dwSkipMsgs != (dword)-1L)
      Sqd->wSkipMsg=(word)dwSkipMsgs;

    if (dwMaxDays != (dword)-1L)
      Sqd->wMaxDays=(word)dwMaxDays;

    (void)_SquishExclusiveEnd(ha);
  }

}

void _XPENTRY apiSquishGetMaxMsg(HAREA ha, dword *dwMaxMsgs, dword *dwSkipMsgs, dword *dwMaxDays)
{
  if (MsgInvalidHarea(ha))
    return;

  if (dwMaxMsgs)
    *dwMaxMsgs = Sqd->dwMaxMsg;

  if (dwSkipMsgs)
    *dwSkipMsgs = (dword) Sqd->wSkipMsg;

  if (dwMaxDays)
    *dwMaxDays = (dword) Sqd->wMaxDays;
}

/* Hash function used for calculating the hashes in the .sqi file */

dword _XPENTRY SquishHash(byte  *f)
{
  dword hash=0, g;

  while (*f)
  {
    hash=(hash << 4) + (dword)tolower(*f);

    if ((g=(hash & 0xf0000000L)) != 0L)
    {
      hash |= g >> 24;
      hash |= g;
    }
    f++;
  }


  /* Strip off high bit */

  return (hash & 0x7fffffffLu);
}

