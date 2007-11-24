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
#include <assert.h>
#include <stdio.h>

#include <huskylib/compiler.h>

#ifdef HAS_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAS_IO_H
#  include <io.h>
#endif
#ifdef HAS_SHARE_H
#include <share.h>
#endif
#ifdef HAS_MALLOC_H
#include <malloc.h>
#endif

#include <huskylib/huskylib.h>

/* Swith for build DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"


/* Read in the binary message header from the data file */

static unsigned near _SquishReadXmsg(HMSG hmsg, PXMSG pxm, dword *pdwOfs)
{
  long ofs=hmsg->foRead + HSqd->cbSqhdr;

  if (*pdwOfs != (dword)ofs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

  if (read_xmsg(HSqd->sfd, pxm) != 1)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  /* Update our position */

  *pdwOfs=(dword)ofs + (dword) XMSG_SIZE;

  /* If there is a UMSGID associated with this message, store it in         *
   * memory in case we have to write the message later.  Blank it           *
   * out so that the application cannot access it.                          */

  if (pxm->attr & MSGUID)
  {
      hmsg->uidUs=pxm->umsgid;

      /*
       OG: I throw out this stupid 2 lines, because application SHOULD use
       the umsgid !
       */
       /*
       pxm->attr &= ~MSGUID;
       pxm->umsgid=0L;
       */
  }

/* Og: It's not the idea of the MsgApi to check all Data on every read.
*/
/*
  if (pxm->date_written.date.yr == 0 ||
      pxm->__ftsc_date[0] == 0)
  {
     MsgCvtFTSCDateToBinary(pxm->__ftsc_date, (union stamp_combo *)&pxm->date_written);
  }
*/

  return TRUE;
}


/* Read in the control information for the current message */

static unsigned near _SquishReadCtrl(HMSG hmsg, byte  *szCtrl,
                                     dword dwCtrlLen, dword *pdwOfs)
{
  long ofs=hmsg->foRead + HSqd->cbSqhdr + XMSG_SIZE;
  unsigned uMaxLen=(unsigned)min(dwCtrlLen, hmsg->sqhRead.clen);

  /* Read the specified amount of text, but no more than specified in       *
   * the frame header.                                                      */


/* Solving O.Kopp's problem with unintialised Return-Buffers */
  *szCtrl = 0;

  if (*pdwOfs != (dword)ofs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

  if (read(HSqd->sfd, (char *)szCtrl, uMaxLen) != (int)uMaxLen)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  *pdwOfs=(dword)ofs + (dword)uMaxLen;

  return TRUE;
}


/* Read in the text body for the current message */

static dword near _SquishReadTxt(HMSG hmsg, byte  *szTxt, dword dwTxtLen,
                                 dword *pdwOfs)
{
  /* Start reading from the cur_pos offset */

  long ofs=hmsg->foRead + (long)HSqd->cbSqhdr + (long)XMSG_SIZE
                        + (long)hmsg->sqhRead.clen;

  /* Max length is the size of the msg text inside the frame */

  unsigned uMaxLen=(unsigned)(hmsg->sqhRead.msg_length -
                              hmsg->sqhRead.clen - XMSG_SIZE);

/* Solving theoretical problem with 0-Byte Body's (thx to O.Kopp) */
  *szTxt = 0;

  /* Make sure that we don't try to read beyond the end of the msg */

  if (hmsg->cur_pos > uMaxLen)
    hmsg->cur_pos=uMaxLen;

  /* Now seek to the position that we are supposed to read from */

  ofs += (long)hmsg->cur_pos;

  /* Decrement the max length by the seek posn, but don't read more than    *
   * the user asked for.                                                    */

  uMaxLen -= (unsigned)hmsg->cur_pos;
  uMaxLen=min(uMaxLen, (unsigned)dwTxtLen);

  /* Now try to read that information from the file */

  if (ofs != (long)*pdwOfs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_BADF;
      return (dword)-1L;
    }


  if (read(HSqd->sfd, (char *)szTxt, uMaxLen) != (int)uMaxLen)
  {
    msgapierr=MERR_BADF;
    return (dword)-1L;
  }

  *pdwOfs = (dword)ofs + (dword)uMaxLen;


  /* Increment the current position by the number of bytes that we read */

  hmsg->cur_pos += (dword)uMaxLen;

  return (dword)uMaxLen;
}


/* Read a message from the Squish base */

dword _XPENTRY apiSquishReadMsg(HMSG hmsg, PXMSG pxm, dword dwOfs,
                              dword dwTxtLen, byte  *szTxt,
                              dword dwCtrlLen, byte  *szCtrl)
{
  dword dwSeekOfs=(dword)-1L; /* Current offset */
  unsigned fOkay=TRUE;        /* Any errors so far? */
  dword dwGot=0;              /* Bytes read from file */

  /* Make sure that we have a valid handle (and that it's in read mode) */

  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1L;

  /* Make sure that we can use szTxt and szCtrl as flags controlling what   *
   * to read.                                                               */

  if (!dwTxtLen)
    szTxt=NULL;

  if (!dwCtrlLen)
    szCtrl=NULL;


  /* Now read in the message header, the control information, and the       *
   * message text.                                                          */

 if (pxm)
    fOkay=_SquishReadXmsg(hmsg, pxm, &dwSeekOfs);

  if (fOkay && szCtrl)
    fOkay=_SquishReadCtrl(hmsg, szCtrl, dwCtrlLen, &dwSeekOfs);

  if (fOkay && szTxt)
  {
    hmsg->cur_pos=dwOfs;

    dwGot=_SquishReadTxt(hmsg, szTxt, dwTxtLen, &dwSeekOfs);
    if (dwGot==(dword)-1L)
      fOkay=FALSE;
  }

  /* If everything worked okay, return the number bytes that we read        *
   * from the message body.                                                 */

  return fOkay ? dwGot : (dword)-1L;
}



