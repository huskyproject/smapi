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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <huskylib/huskylib.h>

#ifdef HAS_IO_H
#  include <io.h>
#endif
#ifdef HAS_SHARE_H
#include <share.h>
#endif

#ifdef HAS_MALLOC_H
#include <malloc.h>
#endif

/* Swith for build DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"


/* Kill the specified message number.                                       *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static sword _SquishKill(HAREA ha, dword dwMsg, SQHDR *psqh, FOFS fo)
{
  assert(Sqd->fHaveExclusive);


  /* Link the existing messages over this one */

  if (psqh->prev_frame)
    if (!_SquishSetFrameNext(ha, psqh->prev_frame, psqh->next_frame))
      return FALSE;

  if (psqh->next_frame)
    if (!_SquishSetFramePrev(ha, psqh->next_frame, psqh->prev_frame))
      return FALSE;


  /* Delete this message from the index file */

  if (!_SquishRemoveIndexEntry(Sqd->hix, dwMsg, NULL, psqh, TRUE))
    return FALSE;


  /* Finally, add the freed message to the free frame list */

  return (sword)_SquishInsertFreeChain(ha, fo, psqh);
}



/* This function is used to delete a message from a Squish message base */

sword _XPENTRY apiSquishKillMsg(HAREA ha, dword dwMsg)
{
  SQHDR sqh;
  sword rc;
  FOFS fo;

  /* Validate parameters */

  if (MsgInvalidHarea(ha))
    return -1;


  /* Make sure that the message actually exists */

  if (dwMsg==0 || dwMsg > ha->num_msg)
  {
    msgapierr=MERR_NOENT;
    return -1;
  }

  /* Get the offset of the frame to delete */

  if ((fo=_SquishGetFrameOfs(ha, dwMsg))==NULL_FRAME)
  {
    return -1;
  }


  /* Read that into memory */

  if (!_SquishReadHdr(ha, fo, &sqh))
  {
    return -1;
  }


  /* Now get exclusive access for the delete operation */

  if (!_SquishExclusiveBegin(ha))
  {
    return FALSE;
  }

  /* Let _SquishKill to the dirty work */

  rc=_SquishKill(ha, dwMsg, &sqh, fo);

  /* Let go of the base */

  if (!_SquishExclusiveEnd(ha))
    rc=FALSE;

  return rc ? 0 : -1;
}

