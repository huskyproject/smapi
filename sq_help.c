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
#include <errno.h>
#include <assert.h>
#include <stdlib.h>


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

//#include "prog.h"
//#include "progprot.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"

/* Read the base header from the beginning of the .sqd file */

unsigned _SquishReadBaseHeader(HAREA ha, SQBASE *psqb)
{
  if (lseek(Sqd->sfd, 0L, SEEK_SET) != 0 ||
      read_sqbase(Sqd->sfd, psqb) != 1)
  {
    if (errno==EACCES || errno==-1)
      msgapierr=MERR_SHARE;
    else
      msgapierr=MERR_BADF;

    return FALSE;
  }

  return TRUE;
}




/* Write the base header back to the beginning of the .sqd file */

unsigned _SquishWriteBaseHeader(HAREA ha, SQBASE *psqb)
{
  if (lseek(Sqd->sfd, 0L, SEEK_SET) != 0 ||
      write_sqbase(Sqd->sfd, psqb) != 1)
  {
    msgapierr=MERR_NODS;
    return FALSE;
  }

  return TRUE;
}



/* Release the specified frame to the free chain.  The frame is             *
 * located at offset 'fo', and the header at that offset has                *
 * already been loaded into *psqh.  This function does **not**              *
 * change the links of other messages that may point to this                *
 * message; it simply threads the current message into the free chain.      *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

unsigned _SquishInsertFreeChain(HAREA ha, FOFS fo, SQHDR *psqh)
{
  SQHDR sqh=*psqh;

  assert(Sqd->fHaveExclusive);

  sqh.id=SQHDRID;
  sqh.frame_type=FRAME_FREE;
  sqh.msg_length=sqh.clen=0L;

  /* If we have no existing free frames, then this is easy */

  if (Sqd->foLastFree==NULL_FRAME)
  {
    sqh.prev_frame=NULL_FRAME;
    sqh.next_frame=NULL_FRAME;

    /* Try to write this frame back to the file */

    if (! _SquishWriteHdr(ha, fo, &sqh))
      return FALSE;

    Sqd->foFree=Sqd->foLastFree=fo;
    return TRUE;
  }


  /* There is an existing frame, so we must append to the end of the        *
   * chain.                                                                 */

  sqh.prev_frame=Sqd->foLastFree;
  sqh.next_frame=NULL_FRAME;


  /* Update the last chain in the free list, pointing it to us */

  if (!_SquishSetFrameNext(ha, sqh.prev_frame, fo))
    return FALSE;


  /* Try to write the current frame to disk */

  if (_SquishWriteHdr(ha, fo, &sqh))
  {
    Sqd->foLastFree=fo;
    return TRUE;
  }
  else
  {
    /* The write failed, so just hope that we can undo what we did        *
     * earlier.                                                           */

    (void)_SquishSetFrameNext(ha, sqh.prev_frame, NULL_FRAME);
    return FALSE;
  }
}



/* Change the 'next' link of the foModify frame to the value foValue */

unsigned _SquishSetFrameNext(HAREA ha, FOFS foModify, FOFS foValue)
{
  SQHDR sqh;

  if (!_SquishReadHdr(ha, foModify, &sqh))
    return FALSE;

  sqh.next_frame=foValue;

  return _SquishWriteHdr(ha, foModify, &sqh);
}



/* Change the 'prior' link of the foModify frame to the value foValue */

unsigned _SquishSetFramePrev(HAREA ha, FOFS foModify, FOFS foValue)
{
  SQHDR sqh;

  if (!_SquishReadHdr(ha, foModify, &sqh))
    return FALSE;

  sqh.prev_frame=foValue;

  return _SquishWriteHdr(ha, foModify, &sqh);
}



/* This function ensures that the message handle is readable */

unsigned _SquishReadMode(HMSG hmsg)
{
  if (hmsg->wMode != MOPEN_READ && hmsg->wMode != MOPEN_RW)
  {
    msgapierr=MERR_EACCES;
    return FALSE;
  }

  return TRUE;
}



/* This function ensures that the message handle is writable */

unsigned _SquishWriteMode(HMSG hmsg)
{
  if (hmsg->wMode != MOPEN_CREATE && hmsg->wMode != MOPEN_WRITE &&
      hmsg->wMode != MOPEN_RW)
  {
    msgapierr=MERR_EACCES;
    return FALSE;
  }

  return TRUE;
}



/* Translate a message number into a frame offset for area 'ha' */

FOFS _SquishGetFrameOfs(HAREA ha, dword dwMsg)
{
  SQIDX sqi;


  msgapierr=MERR_NOENT;

  /* Check for simple stuff that we can handle by following our own         *
   * linked list.                                                           */

  if (dwMsg==ha->cur_msg)
    return Sqd->foCur;
  else if (dwMsg==ha->cur_msg-1)
    return Sqd->foPrev;
  else if (dwMsg==ha->cur_msg+1)
    return Sqd->foNext;

  /* We couldn't just follow the linked list, so we will have to consult    *
   * the Squish index file to find it.                                      */

  if (! SidxGet(Sqd->hix, dwMsg, &sqi))
    return NULL_FRAME;

  return sqi.ofs;
}


/* Read the Squish header 'psqh' from the specified frame offset */

unsigned _SquishReadHdr(HAREA ha, FOFS fo, SQHDR *psqh)
{
  /* Ensure that we are reading a valid frame header */

  if (fo < SQBASE_SIZE)
  {
    msgapierr=MERR_BADA;
    return FALSE;
  }

  /* Seek and read the header */

  if (fo >= Sqd->foEnd ||
      lseek(Sqd->sfd, fo, SEEK_SET) != fo ||
      read_sqhdr(Sqd->sfd, psqh) != 1 ||
      psqh->id != SQHDRID)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  return TRUE;
}



/* Write the Squish header 'psqh' to the specified frame offset */

unsigned _SquishWriteHdr(HAREA ha, FOFS fo, SQHDR *psqh)
{
  /* Make sure that we don't write over the file header */

  if (fo < SQBASE_SIZE)
  {
    msgapierr=MERR_BADA;
    return FALSE;
  }

  if (lseek(Sqd->sfd, fo, SEEK_SET) != fo ||
      write_sqhdr(Sqd->sfd, psqh) != 1)
  {
    msgapierr=MERR_NODS;
    return FALSE;
  }

  return TRUE;
}


/* This function fixes the in-memory pointers after message dwMsg was       *
 * removed from the index file.                                             *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

unsigned _SquishFixMemoryPointers(HAREA ha, dword dwMsg, SQHDR *psqh)
{
  assert(Sqd->fHaveExclusive);

  /* Adjust the first/last message pointers */

  if (dwMsg==1)
    Sqd->foFirst=psqh->next_frame;

  if (dwMsg==ha->num_msg)
    Sqd->foLast=psqh->prev_frame;


  /* Now fix up the in-memory version of the prior/next links */

  if (dwMsg==ha->cur_msg+1)
    Sqd->foNext=psqh->next_frame;

  if (dwMsg==ha->cur_msg-1)
    Sqd->foPrev=psqh->prev_frame;


  /* If we killed the message that we are on, it's a special case */

  if (dwMsg==ha->cur_msg)
  {
    SQHDR sqh;

    /* Go to the header of the prior msg */

    if (!_SquishReadHdr(ha, psqh->prev_frame, &sqh))
    {
      /* That does not exist, so go to msg 0 */

      Sqd->foCur=Sqd->foPrev=NULL_FRAME;
      Sqd->foNext=Sqd->foFirst;
      ha->cur_msg=0;
    }
    else
    {
      /* Otherwise, adjust pointers appropriately */

      Sqd->foCur=psqh->prev_frame;
      Sqd->foPrev=sqh.prev_frame;
      Sqd->foNext=sqh.next_frame;
      ha->cur_msg--;
    }
  }
  else
  {
    /* We didn't kill the current msg, so just decrement cur_msg if         *
     * we were higher than the deleted message.                             */

    if (ha->cur_msg >= dwMsg)
      ha->cur_msg--;
  }


  /* Adjust the message numbers appropriately */

  ha->num_msg--;
  ha->high_msg--;

  if (ha->high_water >= dwMsg)
    ha->high_water--;

  return TRUE;
}





/* Write the index back to disk and free the associated memory */

unsigned _SquishFreeIndex(HAREA ha, dword dwMsg, SQIDX *psqi,
                          dword dwIdxSize, unsigned fWrite)
{
  unsigned rc=TRUE;
  long ofs;

#ifdef __WATCOMC__
dwIdxSize=dwIdxSize; /* To prevent warning */
#endif

  if (fWrite)
  {
    /* Seek to the offset of the message that we want to delete */

    ofs=((long)dwMsg-1L) * (long)SQIDX_SIZE;

    /* Write it back out to disk at the same position */

    rc=(lseek(Sqd->ifd, ofs, SEEK_SET)==ofs &&
        write_sqidx(Sqd->ifd, psqi, ((long)dwMsg-1L)) == 1);
  }

  pfree(psqi);

  return rc;
}



#if 0
/* Read from the index file, starting at the dwMsg'th record */

SQIDX * _SquishAllocIndex(HAREA ha, dword dwMsg, dword *pdwIdxSize)
{
  SQIDX *psqi;
  dword dwIdxSize;
  long ofs;

  /* We only need enough memory to read in the index file from the point    *
   * that we are deleting a message.                                        */

  dwIdxSize = ((long)ha->num_msg - (long)dwMsg + 1L) * (long)sizeof(SQIDX);


  /* Handle problems that we may have when working on a 16-bit platform */

  if (dwIdxSize > 65000L)
  {
  }

  /* Allocate memory for handling the index */

  if ((psqi=palloc((size_t)dwIdxSize))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return NULL;
  }


  /* Seek to the offset of the message that we want to delete */

  ofs=((long)dwMsg-1L) * (long)SQIDX_SIZE;


  /* Now read it from disk */

  if (lseek(Sqd->ifd, ofs, SEEK_SET) != ofs ||
      read_sqidx(Sqd->ifd, psqi, ((long)ha->num_msg - (long)dwMsg + 1L)) != 1)
  {
    msgapierr=MERR_BADF;
    pfree(psqi);
    return NULL;
  }

  *pdwIdxSize=dwIdxSize;
  return psqi;
}

/* This function removes the specified message number from the Squish index *
 * file, moving the rest of the index file back over the message number to  *
 * fill in the gaps.                                                        *
 *                                                                          *
 * This function also adjusts the message number pointers everywhere to     *
 * accomodate for the deletion of this message.                             *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

unsigned _SquishRemoveIndex(HAREA ha, dword dwMsg, SQIDX *psqiOut, SQHDR *psqh)
{
  dword dwIdxSize;
  SQIDX *psqiLast;
  SQIDX *psqi;
  unsigned rc;

  assert(Sqd->fHaveExclusive);

  /* Read the index from disk */

  if ((psqi=_SquishAllocIndex(ha, dwMsg, &dwIdxSize))==NULL)
    return FALSE;


  /* If the caller wants a copy of the record that we are deleting... */

  if (psqiOut)
    memmove(psqiOut, psqi, sizeof(SQIDX));


  /* Shift everything over by one to accomodate for the deleted msg */

  memmove(psqi, psqi+1, (size_t)dwIdxSize-sizeof(SQIDX));


  /* Blank out the last index pointer in the file so that it is invalid */

  psqiLast=psqi + (ha->num_msg - (long)dwMsg);

  psqiLast->ofs=NULL_FRAME;
  psqiLast->umsgid=(UMSGID)-1L;
  psqiLast->hash=(dword)-1L;


  /* Write it back to disk and free memory */

  rc=_SquishFreeIndex(ha, dwMsg, psqi, dwIdxSize, TRUE);


  /* If the delete succeeded, adjust the memory pointers */

  if (rc)
    rc=_SquishFixMemoryPointers(ha, dwMsg, psqh);

  return rc;
}
#endif

