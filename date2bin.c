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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "prog.h"
#include "progprot.h"
#include "stamp.h"

static void near StandardDate(union stamp_combo *d_written);


void _fast ASCII_Date_To_Binary(char *msgdate, union stamp_combo *d_written)
{
    char temp[80];

    int dd, yy, mo, hh, mm, ss, x;

    time_t timeval;
    struct tm *tim;

    timeval = time(NULL);
    tim = localtime(&timeval);

  if (*msgdate=='\0') /* If no date... */
  {
    /* OG: localtime must only generated, if msgadate == '' & yr = 1980
           A little bit more speed !
    */
    /* max: and why you leave generation upper *msgdate=='\0' ?
            but you can't remove, because it needed at the end of function
	    think you must remove localtime generation below */

    if (d_written->msg_st.date.yr == 0)
    {
      timeval=time(NULL);
      tim=localtime(&timeval);

      /* Insert today's date */
      fts_time(msgdate, tim);

      StandardDate(d_written);
    }
    else /* If msgdate = '' & yr > 1980, date_written seems to be ok ! */
    {
      if (d_written->msg_st.date.mo == 0 ||
          d_written->msg_st.date.mo > 12)
        d_written->msg_st.date.mo = 1;
      sprintf(msgdate,
             "%02d %s %02d  %02d:%02d:%02d",
             d_written->msg_st.date.da,
             months_ab[d_written->msg_st.date.mo-1],
             (d_written->msg_st.date.yr+80) % 100,
             d_written->msg_st.time.hh,
             d_written->msg_st.time.mm,
             d_written->msg_st.time.ss);
    }
    return;
  }

    if (sscanf(msgdate, "%d %s %d %d:%d:%d", &dd, temp, &yy, &hh, &mm, &ss) == 6)
    {
        x = 1;
    }
    else if (sscanf(msgdate, "%d %s %d %d:%d", &dd, temp, &yy, &hh, &mm) == 5)
    {
        ss = 0;
        x = 1;
    }
    else if (sscanf(msgdate, "%*s %d %s %d %d:%d", &dd, temp, &yy, &hh, &mm) == 5)
    {
        x = 2;
    }
    else if (sscanf(msgdate, "%d/%d/%d %d:%d:%d", &mo, &dd, &yy, &hh, &mm, &ss) == 6)
    {
        x = 3;
    }
    else
    {
        x = 0;
        StandardDate(d_written);
        return;
    }

    if (x == 1 || x == 2)
    {
        /* Formats one and two have ASCII date, so compare to list */

        for (x = 0; x < 12; x++)
        {
            if (stricmp(temp, months_ab[x])==0)
            {
                d_written->msg_st.date.mo = x + 1;
                break;
            }
        }

        if (x == 12)
        {
            /* Invalid month, use January instead. */
            d_written->msg_st.date.mo = 1;
        }
    }
    else
    {
        /* Format 3 don't need no ASCII month */
        d_written->msg_st.date.mo = mo;
    }

    /* Use sliding window technique to interprete the year number */
    while (yy <= tim->tm_year - 50) yy += 100;
    while (yy >  tim->tm_year + 50) yy -= 100;

    d_written->msg_st.date.yr = yy - 80;
    d_written->msg_st.date.da = dd;

    d_written->msg_st.time.hh = hh;
    d_written->msg_st.time.mm = mm;
    d_written->msg_st.time.ss = ss >> 1;
}

/* Date couldn't be determined, so set it to Jan 1st, 1980 */

static void near StandardDate(union stamp_combo *d_written)
{
    d_written->msg_st.date.yr = 0;
    d_written->msg_st.date.mo = 1;
    d_written->msg_st.date.da = 1;

    d_written->msg_st.time.hh = 0;
    d_written->msg_st.time.mm = 0;
    d_written->msg_st.time.ss = 0;
}
