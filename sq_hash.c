/***************************************************************************
 *                                                                         *
 *  Squish Developers Kit Source, Extended Version 2.00                             *
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

#if !defined(UNIX) && !defined(SASC)
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if !defined(UNIX) && !defined(SASC)
#include <share.h>
#endif

#include "prog.h"
#include "alc.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"
#include "unused.h"

dword _XPENTRY apiSquishGetHash(HAREA ha, dword dwMsg)
{
  SQIDX sqi;

  if (!SidxGet(Sqd->hix, dwMsg, &sqi))
  {
    return (dword)0L;
  }

  msgapierr=MERR_NONE;
  return sqi.hash;
}

