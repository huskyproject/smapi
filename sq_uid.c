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
#include <string.h>
#include <fcntl.h>

#include "compiler.h"

#ifdef HAS_IO_H
#  include <io.h>
#endif
#ifdef HAS_SHARE_H
#include <share.h>
#endif
#ifdef HAS_MALLOC_H
#include <malloc.h>
#endif

#include "prog.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"
#include "unused.h"



/* This function returns the UMSGID that will be used by the next message   *
 * to be created.                                                           */

UMSGID _XPENTRY apiSquishGetNextUid(HAREA ha)
{
  return Sqd->uidNext;
}



/* This function converts the message number 'dwMsg' into a unique          *
 * message idenfitier (UMSGID).                                             */

UMSGID _XPENTRY apiSquishMsgnToUid(HAREA ha, dword dwMsg)
{
  SQIDX sqi;

  if (MsgInvalidHarea(ha))
    return (UMSGID)0L;


  /* Make sure that it's a valid message number */

  if (dwMsg==0 || dwMsg > ha->num_msg)
  {
    msgapierr=MERR_NOENT;

    return (UMSGID)0L;
  }


  if (!SidxGet(Sqd->hix, dwMsg, &sqi))
  {
    return (UMSGID)0L;
  }

  return sqi.umsgid;
}


/* This function converts the UMSGID in 'uid' into a real message number */

dword _XPENTRY apiSquishUidToMsgn(HAREA ha, UMSGID uid, word wType)
{
  SQIDX sqi;
  dword rc=0;
  sdword stLow, stHigh, stTry;
  dword dwMax;

  if (MsgInvalidHarea(ha))
    return (UMSGID)0L;

  /* Don't let the user access msg 0 */

  if (uid==(UMSGID)0L)
  {
    msgapierr=MERR_NOENT;
    return 0L;
  }

/* OG: Exlusive access is required when caching the index */
/*
  if (!_SquishExclusiveBegin(ha))
  {
    return 0;
  }
*/

  /* Read the index into memory */

  if (! _SquishBeginBuffer(Sqd->hix))
  {
    return (dword)0;
  }

  /* Set up intial bounds (inclusive) */

  dwMax=_SquishIndexSize(Sqd->hix) / SQIDX_SIZE;
  stLow=1;
  stHigh=(sdword)dwMax;
  stTry=1;

  /* Start off with a 0 umsgid */

  (void)memset(&sqi, 0, sizeof sqi);

  /* While we still have a search range... */

  while (stLow <= stHigh)
  {
    stTry=(stLow+stHigh) / 2;

    /* If we got an exact match */

    if (!SidxGet(Sqd->hix, (dword)stTry, &sqi))
      break;

    if (sqi.umsgid==uid)
    {
      rc=(dword)stTry;
      break;
    }
    else if (uid > sqi.umsgid)
      stLow=stTry+1;
    else stHigh=stTry-1;
  }


  /* If we couldn't find it exactly, try the next/prior match */

  if (!rc)
  {
    if (wType==UID_PREV)
    {
      if (sqi.umsgid < uid)
        rc=(dword)stTry;
      else if (stTry==1)
        rc=(dword)0;
      else
        rc=(dword)(stTry-1L);
    }
    else if (wType==UID_NEXT)
    {
      if (sqi.umsgid > uid || stTry==(long)dwMax)
        rc=(dword)stTry;
      else
        rc=(dword)(stTry+1L);
    }
    else
      msgapierr=MERR_NOENT;
  }


  /* Free the memory used by the index */
/*
  if (! _SquishEndBuffer(Sqd->hix))
    rc=(dword)0;

  _SquishExclusiveEnd(ha);
*/

  if (! _SquishFreeBuffer(Sqd->hix))
    rc=(dword)0;

  return rc;
}



