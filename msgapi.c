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

#include <string.h>
#include <stdlib.h>
#include "alc.h"
#include "prog.h"
#include "msgapi.h"
#include "apidebug.h"
#include "unused.h"

static byte *intl = (byte *) "INTL";
static byte *fmpt = (byte *) "FMPT";
static byte *topt = (byte *) "TOPT";
static byte *area_colon = (byte *) "AREA:";

static char *copyright = "MSGAPI - Copyright 1991 by Scott J. Dudley.  All rights reserved.";

/* Global error value for message API routines */

word _stdc msgapierr = 0;

struct _minf _stdc mi;

sword EXPENTRY MsgOpenApi(struct _minf *minf)
{
    unused(copyright);
    mi = *minf;
    mi.haveshare = shareloaded();
    return 0;
}

sword EXPENTRY MsgCloseApi(void)
{
    return 0;
}

MSG *EXPENTRY MsgOpenArea(byte * name, word mode, word type)
{
    if (type & MSGTYPE_SQUISH) return SquishOpenArea(name, mode, type);
    else if (type & MSGTYPE_JAM) return JamOpenArea(name, mode, type);
    else return SdmOpenArea(name, mode, type);
}

int MsgDeleteBase(char * name, word type)
{
    if (type & MSGTYPE_SQUISH) return SquishDeleteBase(name);
    else if (type & MSGTYPE_JAM) return JamDeleteBase(name);
    else return SdmDeleteBase(name);
}

sword EXPENTRY MsgValidate(word type, byte * name)
{
    if (type & MSGTYPE_SQUISH) return SquishValidate(name);
    else if (type & MSGTYPE_JAM) return JamValidate(name);
    else return SdmValidate(name);
}

/*
 *  Check to see if a message handle is valid.  This function should work
 *  for ALL handlers tied into MsgAPI.  This also checks to make sure that
 *  the area which the message is from is also valid (ie. the message handle
 *  isn't valid unless the area handle of that message is also valid).
 */

sword MSGAPI InvalidMsgh(MSGH * msgh)
{
    if (msgh == NULL || msgh->id != MSGH_ID || InvalidMh(msgh->sq))
    {
        msgapierr = MERR_BADH;
        return TRUE;
    }

    return FALSE;
}

/* Check to ensure that a message area handle is valid. */

sword MSGAPI InvalidMh(MSG * mh)
{
    if (mh == NULL || mh->id != MSGAPI_ID)
    {
        msgapierr = MERR_BADH;
        return TRUE;
    }

    return FALSE;
}

byte *StripNasties(byte * str)
{
    byte *p;

    p = str;
    while (*p != '\0')
    {
        if (*p < ' ')
        {
            *p = ' ';
        }
        p++;
    }

    return str;
}

/* Copy the text itself to a buffer, or count its length if out==NULL */

static word near _CopyToBuf(byte * p, byte * out, byte ** end)
{
    word len = 0;

    if (out)
    {
        *out++ = '\001';
    }

    len++;

    while (*p == '\015' || *p == '\012' || *p == (byte) '\215')
    {
        p++;
    }

    while (*p == '\001' || strncmp((char *) p, (char *) area_colon, 5) == 0)
    {
        /* Skip over the first ^a */

        if (*p == '\001')
        {
            p++;
        }

        while (*p && *p != '\015' && *p != '\012' && *p != (byte) '\215')
        {
            if (out)
            {
                *out++ = *p;
            }

            p++;
            len++;
        }

        if (out)
        {
            *out++ = '\001';
        }

        len++;

        while (*p == '\015' || *p == '\012' || *p == (byte) '\215')
        {
            p++;
        }
    }

    /* Cap the string */

    if (out)
    {
        *out = '\0';
    }

    len++;

    /* Make sure to leave no trailing x01's. */

    if (out && out[-1] == '\001')
    {
        out[-1] = '\0';
    }

    /* Now store the new end location of the kludge lines */

    if (end)
    {
        *end = p;
    }

    return len;
}

byte *EXPENTRY CopyToControlBuf(byte * txt, byte ** newtext, unsigned *length)
{
    byte *cbuf, *end;

    word clen;

    /* Figure out how long the control info is */

    clen = _CopyToBuf(txt, NULL, NULL);

    /* Allocate memory for it */

#define SAFE_CLEN 20

    cbuf = palloc(clen + SAFE_CLEN);
    if (cbuf == NULL)
    {
        return NULL;
    }

    memset(cbuf, '\0', clen + SAFE_CLEN);

    /* Now copy the text itself */

    clen = _CopyToBuf(txt, cbuf, &end);

    if (length)
    {
        *length -= (size_t) (end - txt);
    }

    if (newtext)
    {
        *newtext = end;
    }

    return cbuf;
}

byte *EXPENTRY GetCtrlToken(byte * where, byte * what)
{
    byte *end, *found, *out;

    found = NULL;

    if (where != NULL)
    {
        found = (byte *) strstr((char *) where, (char *) what);
    }

    if (where != NULL && found != NULL && found[-1] == '\001')
    {
        end = (byte *) strchr((char *) found, '\001');

        if (!end)
        {
            end = found + strlen((char *) found);
        }

        out = palloc((size_t) (end - found) + 1);
        if (out == NULL)
        {
            return NULL;
        }

        memmove(out, found, (size_t) (end - found));
        out[(size_t) (end - found)] = '\0';
        return out;
    }

    return NULL;
}

void EXPENTRY ConvertControlInfo(byte * ctrl, NETADDR * orig, NETADDR * dest)
{
    byte *p, *s;

    s = GetCtrlToken(ctrl, intl);

    if (s != NULL)
    {
        NETADDR norig, ndest;

        p = s;

        /* Copy the defaults from the original address */

        norig = *orig;
        ndest = *dest;

        /* Parse the destination part of the kludge */

        s += 5;
        Parse_NetNode((char *) s, &ndest.zone, &ndest.net, &ndest.node, &ndest.point);

        while (*s != ' ' && *s)
        {
            s++;
        }

        if (*s)
        {
            s++;
        }

        Parse_NetNode((char *) s, &norig.zone, &norig.net, &norig.node, &norig.point);

        pfree(p);

        /*
         *  Only use this as the "real" zonegate address if the net/node
         *  addresses in the INTL line match those in the message body.
         *  Otherwise, it's probably a gaterouted message!
         */

        if (ndest.net == dest->net && ndest.node == dest->node &&
          norig.net == orig->net && norig.node == orig->node)
        {
            *dest = ndest;
            *orig = norig;

            /*
             *  Only remove the INTL line if it's not gaterouted, which is
             *  why we do it here.
             */

/* mtt: DO NOT CHANGE THE MSGTEXT!!!      */
/*            RemoveFromCtrl(ctrl, intl); */
        }
    }

    /* Handle the FMPT kludge */

    s = GetCtrlToken(ctrl, fmpt);
    if (s != NULL)
    {
        orig->point = (word) atoi((char *) s + 5);
        pfree(s);
        /* mtt: DO NO CHANGE THE MSGTEXT!!!! */
        /* RemoveFromCtrl(ctrl, fmpt);       */
    }

    /* Handle TOPT too */

    s = GetCtrlToken(ctrl, topt);
    if (s != NULL)
    {
        dest->point = (word) atoi((char *) s + 5);
        pfree(s);
        /* mtt: DO NOT CHANGE THE MSGTEXT!!! */
        /* RemoveFromCtrl(ctrl, topt);       */
    }
}

byte *EXPENTRY CvtCtrlToKludge(byte * ctrl)
{
    byte *from, *to, *buf;
    size_t clen;

    clen = strlen((char *) ctrl) + NumKludges((char *) ctrl) + 20;

    buf = palloc(clen);
    if (buf == NULL)
    {
        return NULL;
    }

    to = buf;

    /* Convert ^aKLUDGE^aKLUDGE... into ^aKLUDGE\r^aKLUDGE\r... */

    from = ctrl;
    while (*from == '\001' && from[1])
    {
        /* Only copy out the ^a if it's NOT the area: line */

        if (!eqstrn((char *) from + 1, (char *) area_colon, 5))
        {
            *to++ = *from;
        }

        from++;

        while (*from && *from != '\001')
        {
            *to++ = *from++;
        }

        *to++ = '\r';
    }

    *to = '\0';

    return buf;
}

void EXPENTRY RemoveFromCtrl(byte * ctrl, byte * what)
{
    byte *search, *p, *s;

    search = palloc(strlen((char *) what) + 2);
    if (search == NULL)
    {
        return;
    }

    strcpy((char *) search, "\001");
    strcat((char *) search, (char *) what);

    /* Now search for this token in the control buffer, and remove it. */

    p = (byte *) strstr((char *) ctrl, (char *) search);
    while (p != NULL)
    {
        s = p + 1;
        while (*s && *s != '\001')
        {
            s++;
        }
        strocpy((char *) p, (char *) s);
        p = (byte *) strstr((char *) ctrl, (char *) search);
    }

    pfree(search);
}

word EXPENTRY NumKludges(char *txt)
{
    word nk = 0;
    char *p;

    for(p=txt; ((p=strchr(p, '\001'))!=NULL); p++) nk++;

    return nk;
}
