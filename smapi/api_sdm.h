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

#ifndef __API_SDM_H__
#define __API_SFM_H__

#include <huskylib/compiler.h>

#include "msgapi.h"

struct _msgh
{
    MSGA *sq;
    dword id;              /* Must always equal MSGH_ID */

    dword bytes_written;
    dword cur_pos;

    /* For *.msg only! */

    sdword clen;
    byte *ctrl;
    sdword msg_len;
    sdword msgtxt_start;
    word zplen;
    int fd;
};


/*
 *  This following junk is unique to *.msg!
 *  NO APPLICATIONS SHOULD USE THESE!
 */

struct _sdmdata
{
    byte base[80];

    unsigned *msgnum;      /* has to be of type 'int' for qksort() fn */
    word msgnum_len;

    dword hwm;
    word hwm_chgd;

    word msgs_open;
};

int _XPENTRY WriteZPInfo(XMSG * msg, void (_stdc * wfunc)(byte * str), byte * kludges);

int read_omsg(int, struct _omsg *);
int write_omsg(int, struct _omsg *);

#endif
