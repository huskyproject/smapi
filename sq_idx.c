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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

#include "compiler.h"

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

#include "prog.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"
#include "unused.h"

#define HixSqd            ((struct _sqdata *)(hix)->ha->apidata)


#ifdef __FLAT__
  #define MORE_SPACE       256      /* Allow for up to 256 additions */
  #define SEGMENT_SIZE    (LONG_MAX/(long)SQIDX_SIZE)
  #define SHIFT_SIZE      32768
#else
  #define MORE_SPACE        16      /* Allow for up to 16 additions */
  #define SEGMENT_SIZE    (32767L/(long)SQIDX_SIZE)
  #define SHIFT_SIZE      8192
#endif

#define fmemmove memmove

/* Open the index file and read the index for this area */

HIDX _SquishOpenIndex(HAREA ha)
{
  HIDX hix;

  if ((hix=palloc(sizeof(*hix)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  /* Store the current area handle */

  hix->id=ID_HIDX;
  hix->ha=ha;
  hix->lDeltaLo=-1;
  hix->lDeltaHi=-1;
  hix->cSeg=0;
  hix->fBuffer=0;

  return hix;
}


/* This function returns the size of the virtual index */

dword _SquishIndexSize(HIDX hix)
{
  dword lSize;
  int i;

  assert(hix->id==ID_HIDX);

  if (!hix->fBuffer)
    lSize=(dword)lseek(HixSqd->ifd, 0L, SEEK_END);
  else
  {
    for (i=0, lSize=0; i < hix->cSeg; i++)
      lSize += hix->pss[i].dwUsed * (dword)SQIDX_SIZE;
  }

  return lSize;
}



/* Start buffering reads/writes to the index file */

int _SquishBeginBuffer(HIDX hix)
{
  dword dwMsgs;
  int i;

  assert(hix->id==ID_HIDX);

  /* Multiple buffers are ok, but we only need to do it once */

  if (hix->fBuffer++ != 0)
    return TRUE;

  hix->cSeg=(int)(hix->ha->num_msg / SEGMENT_SIZE) + 1;

  /* Allocate memory for the array of segments */

  if ((hix->pss=palloc(sizeof(SQIDXSEG) * (unsigned)hix->cSeg))==NULL)
  {
    msgapierr=MERR_NOMEM;
    hix->fBuffer=0;
    return FALSE;
  }
  dwMsgs=hix->ha->num_msg;                /* Read all messages into memory */
  /* Find out how many records are in the file */

  if ((hix->lAllocatedRecords=lseek(HixSqd->ifd, 0L, SEEK_END)) < 0)
  {
    msgapierr=MERR_BADF;
    hix->fBuffer=0;
    return FALSE;
  }
  /* Find out the number of records, not the number of bytes */

  hix->lAllocatedRecords /= SQIDX_SIZE;


  /* Read from head of index file */

  (void)lseek(HixSqd->ifd, 0L, SEEK_SET);

  /* Repeat for each segment in the index file */

  for (i=0; i < hix->cSeg; i++)
  {
    dword dwSize=min(dwMsgs+MORE_SPACE, (long)SEGMENT_SIZE);
    unsigned uiSize;

    /* Try to allocate memory for this segment */

    if ((hix->pss[i].psqi=farpalloc((size_t)dwSize * (size_t)sizeof(SQIDX)))==NULL)
    {
      while (i--)
        farpfree(hix->pss[i].psqi);

      pfree(hix->pss);

      msgapierr=MERR_NOMEM;
      hix->fBuffer=0;
      return FALSE;
    }

    hix->pss[i].dwMax=dwSize;

    /* Now read in the messages for this segment */

    dwSize=min(dwMsgs, SEGMENT_SIZE);

    uiSize=(unsigned)dwSize * (unsigned)SQIDX_SIZE;

    if (read_sqidx(HixSqd->ifd, hix->pss[i].psqi, dwSize) != 1)
    {

      do
      {
        farpfree(hix->pss[i].psqi);
      }
      while (i--);

      pfree(hix->pss);

      msgapierr=MERR_BADF;
      hix->fBuffer=0;
      return FALSE;
    }

    /* Decrement the count for msgs in the next segment, if necessary */

    if (dwSize != SEGMENT_SIZE)
      dwMsgs=0;
    else
      dwMsgs -= SEGMENT_SIZE;


    hix->pss[i].dwUsed=dwSize;
  }

  /* Now we have the whole file in memory */

  return TRUE;
}


/* Return a pointer to the 'dwMsg'th message in the buffered index */

static SQIDX far *sidx(HIDX hix, dword dwMsg)
{
  dword dwStart=1L;
  int i;

  for (i=0; i < hix->cSeg; i++)
  {
    if (dwMsg >= dwStart && dwMsg < dwStart + hix->pss[i].dwUsed)
      return hix->pss[i].psqi + (size_t)(dwMsg - dwStart);

    dwStart += hix->pss[i].dwUsed;
  }

  return NULL;
}


/* Get an index from the virtual index file */

int SidxGet(HIDX hix, dword dwMsg, SQIDX *psqi)
{
  SQIDX far *psqiFound;

  assert(hix->id==ID_HIDX);

  if (!hix->fBuffer)
  {
    (void)lseek(HixSqd->ifd, (long)(dwMsg-1) * (long)SQIDX_SIZE, SEEK_SET);

    if (read_sqidx(HixSqd->ifd, psqi, 1) != 1)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

    return TRUE;
  }

  psqiFound=sidx(hix, dwMsg);

  if (!psqiFound)
    return FALSE;

  *psqi=*psqiFound;
  return TRUE;
}




/* Add a new index record to the end of the array */

static int near _SquishAppendIndexRecord(HIDX hix, SQIDX *psqi)
{
  SQIDXSEG *pss;


  /* If we need to expand the index file on disk, do so now */

  if ((long)hix->ha->num_msg > hix->lAllocatedRecords)
  {
    long lSize;
    SQIDX sqi;

    /* Make room for up to 64 new records */

    hix->lAllocatedRecords=hix->ha->num_msg+64;
    lSize=(hix->lAllocatedRecords-1) * (long)SQIDX_SIZE;

    sqi.ofs=0L;
    sqi.umsgid=(UMSGID)-1L;
    sqi.hash=(UMSGID)-1L;

    /* Write a blank index entry at the appropriate location to fill        *
     * up the file.                                                         */

    if (lseek(HixSqd->ifd, lSize, SEEK_SET) != lSize ||
        write_sqidx(HixSqd->ifd, &sqi, 1) != 1)
    {
      msgapierr=MERR_NODS;
      return FALSE;
    }
  }

  /* If we already have some segments... */

  if (hix->cSeg)
  {
    /* Add to an existing segment */

    pss=hix->pss + hix->cSeg-1;

    /* If the record fits within this segment, just append it. */

    if (pss->dwUsed < pss->dwMax)
    {
      pss->psqi[(size_t)pss->dwUsed]=*psqi;
      pss->dwUsed++;
      return TRUE;
    }

    /* If we can expand this segment by reallocating memory... */

    if (pss->dwMax < SEGMENT_SIZE)
    {
      SQIDX far *psqiNew;

      assert(pss->dwMax >= pss->dwUsed);

      /* Don't use realloc because we cannot afford to lose the info that we  *
       * already have!                                                        */

      if ((psqiNew=farpalloc(((size_t)pss->dwMax + MORE_SPACE) * SQIDX_SIZE))==NULL)
      {
        msgapierr=MERR_NOMEM;
        return FALSE;
      }

      (void) fmemmove(psqiNew,
                      pss->psqi,
                      (size_t)pss->dwUsed * (size_t)SQIDX_SIZE);

      psqiNew[(size_t)pss->dwUsed]=*psqi;

      pss->dwUsed++;
      pss->dwMax += MORE_SPACE;

      farpfree(pss->psqi);
      pss->psqi=psqiNew;
      return TRUE;
    }
  }


  /* If we arrived here, we either have no segments, or all of our          *
   * existing segments are full.  To handle this, we need to reallocate     *
   * the array of pointers to segments and add a new one.                   */

  if ((pss=palloc(sizeof(SQIDXSEG) * (size_t)(hix->cSeg+1)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  (void)memmove(pss, hix->pss, (size_t)hix->cSeg * sizeof(SQIDXSEG));
  hix->pss=pss;

  /* Allocate memory for the new segment */

  if ((hix->pss[hix->cSeg].psqi=farpalloc(MORE_SPACE * SQIDX_SIZE))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  pss=hix->pss + hix->cSeg;

  /* Add the specified record to our indices */

  pss->dwUsed=1;
  pss->dwMax=MORE_SPACE;
  *pss->psqi=*psqi;

  /* Increment the segment count */

  hix->cSeg++;
  return TRUE;
}


/* Store an index entry in the virtual index file */

int SidxPut(HIDX hix, dword dwMsg, SQIDX *psqi)
{
  SQIDX far *psqiFound;
  int rc;

  assert(hix->id==ID_HIDX);

  if (!hix->fBuffer)
  {
    (void)lseek(HixSqd->ifd, (long)(dwMsg-1) * (long)SQIDX_SIZE, SEEK_SET);

    if (write_sqidx(HixSqd->ifd, psqi, 1) != 1)
    {
      msgapierr=MERR_NODS;
      return FALSE;
    }

    return TRUE;
  }

  /* If we can't find the appropriate index record */

  if ((psqiFound=sidx(hix, dwMsg))==NULL)
  {
    rc=FALSE;

    /* If the index is out of range, only create a new record if it's       *
     * to be placed at EOF.                                                 */

    if (dwMsg==hix->ha->num_msg+1)
      rc=_SquishAppendIndexRecord(hix, psqi);
  }
  else
  {
    *psqiFound=*psqi;
    rc=TRUE;
  }

  if (rc)
  {
    if (hix->lDeltaLo==-1 || hix->lDeltaLo > (long)dwMsg)
      hix->lDeltaLo=(long)dwMsg;

    if (hix->lDeltaHi==-1 || hix->lDeltaHi < (long)dwMsg)
      hix->lDeltaHi=(long)dwMsg;
  }

  return rc;
}


/* Delete an entry from the index */

unsigned _SquishRemoveIndexEntry(HIDX hix, dword dwMsg, SQIDX *psqiOut,
                                 SQHDR *psqh, int fFixPointers)
{
  SQIDX sqi;
  char *pcBuf;
  int got, i;

  assert(hix->id==ID_HIDX);

  /* Create a blank record for writing at the end */

  sqi.ofs=NULL_FRAME;
  sqi.umsgid=(UMSGID)-1L;
  sqi.hash=(dword)-1L;

  if (hix->fBuffer)
  {
    dword dwStart=1L;

    /* Find the segment containing the deleted message */

    for (i=0; i < hix->cSeg; i++)
    {
      /* If it's in this segment */

      if (dwMsg >= dwStart && dwMsg < dwStart + hix->pss[i].dwUsed)
      {
        int j=(int)(dwMsg-dwStart);
        unsigned rc=TRUE;

        /* If caller wants copy of deleted record */

        if (psqiOut)
          *psqiOut=hix->pss[i].psqi[j];

        /* Shift the rest of the text over this segment */

        (void)fmemmove(hix->pss[i].psqi+j, hix->pss[i].psqi+j+1,
                       (size_t)(hix->pss[i].dwUsed - (dword)j - (dword)1)
                          * (size_t)SQIDX_SIZE);

        hix->pss[i].dwUsed--;

        if (!_SquishAppendIndexRecord(hix, &sqi))
          rc=FALSE;

        if (hix->lDeltaLo==-1 || hix->lDeltaLo > (long)dwMsg)
          hix->lDeltaLo=(long)dwMsg;

        hix->lDeltaHi=(long)_SquishIndexSize(hix) / (long)SQIDX_SIZE;

        if (fFixPointers && rc)
          return _SquishFixMemoryPointers(hix->ha, dwMsg, psqh);
        else
          return rc;
      }

      dwStart += hix->pss[i].dwUsed;
    }

    /* Huh?  Message not in index! */

    return FALSE;
  }


  /* Else if it's not buffered: */


  (void)lseek(HixSqd->ifd, (long)dwMsg * (long)SQIDX_SIZE, SEEK_SET);


  if ((pcBuf=palloc(SHIFT_SIZE))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  /* Only shifting - read_sqidx() is not required */
  while ((got=read(HixSqd->ifd, pcBuf, SHIFT_SIZE)) > 0)
  {
    /* Skip back to one position before this index entry */

    (void)lseek(HixSqd->ifd, -(long)got - SQIDX_SIZE, SEEK_CUR);

    if (write(HixSqd->ifd, pcBuf, (unsigned)got) != got)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

    (void)lseek(HixSqd->ifd, (long)SQIDX_SIZE, SEEK_CUR);
  }

  pfree(pcBuf);

  /* Now write the last entry to stomp over the index element that is at    *
   * the end of the file.                                                   */

  (void)lseek(HixSqd->ifd, -(long)SQIDX_SIZE, SEEK_CUR);

  if (write_sqidx(HixSqd->ifd, &sqi, 1) != 1)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  if (fFixPointers)
    return _SquishFixMemoryPointers(hix->ha, dwMsg, psqh);
  else
    return TRUE;
}



/* Close an index file handle */

unsigned _SquishCloseIndex(HIDX hix)
{
  assert(hix->id==ID_HIDX);

  while (hix->fBuffer)
    if (!_SquishEndBuffer(hix))
      return FALSE;

  hix->id=0;

  pfree(hix);

  return TRUE;
}



/* Dump the index file buffer */

int _SquishEndBuffer(HIDX hix)
{
  int i;
  int rc=TRUE;
  long lSize;

  assert(hix->id==ID_HIDX);

  if (hix->fBuffer==0)
    return FALSE;

  if (--hix->fBuffer != 0)
    return TRUE;


  /* Reduce the index file to the size that it really should be */

  lSize=(long)hix->ha->num_msg * (long)SQIDX_SIZE;
  setfsize(HixSqd->ifd, lSize);


  /* If we need to rewrite the index */

  if (hix->lDeltaLo != -1 && hix->lDeltaHi != -1)
  {
    dword dwStart=1;

    (void) lseek(HixSqd->ifd,
                 (hix->lDeltaLo - 1L) * (long)SQIDX_SIZE,
                 SEEK_SET);

    for (i=0; i < hix->cSeg; i++)
    {
      unsigned uiWriteSize;

      /* If this buffer is within the "delta" range */

      if ((long)dwStart + (long)hix->pss[i].dwUsed > hix->lDeltaLo &&
          (long)dwStart <= hix->lDeltaHi)
      {
        size_t j, size;

        if ((long)dwStart > hix->lDeltaLo)
          j=0;
        else
          j=(size_t)(hix->lDeltaLo-(long)dwStart);

        if ((long)dwStart + (long)hix->pss[i].dwUsed > hix->lDeltaHi)
          size = (size_t)(hix->lDeltaHi - (long)dwStart + 1L);
        else
          size = (size_t)(hix->pss[i].dwUsed);

        size -= j;

        uiWriteSize=(size_t)size * (size_t)SQIDX_SIZE;

        if (rc)
        {
          if (write_sqidx(HixSqd->ifd, (hix->pss[i].psqi+j), size)
                 != 1)

          {
            msgapierr=MERR_NODS;
            rc=FALSE;
          }
        }
      }

      dwStart += hix->pss[i].dwUsed;
    }
  }


  /* Free the memory used by these segments */

  for (i=0; i < hix->cSeg; i++)
    farpfree(hix->pss[i].psqi);

  pfree(hix->pss);
  hix->cSeg=0;

  return rc;
}

/* Free the index file buffer */

int _SquishFreeBuffer(HIDX hix)
{
  int i;
  int rc=TRUE;

  assert(hix->id==ID_HIDX);

  if (hix->fBuffer==0)
    return FALSE;

  if (--hix->fBuffer != 0)
    return TRUE;


  /* Free the memory used by these segments */

  for (i=0; i < hix->cSeg; i++)
    farpfree(hix->pss[i].psqi);

  pfree(hix->pss);
  hix->cSeg=0;

  return rc;
}

