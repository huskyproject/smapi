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
#ifdef UNIX
#include <signal.h>
#endif
#include "alc.h"
#include "prog.h"
#include "msgapi.h"
#include "apidebug.h"
#include "unused.h"

unsigned _SquishCloseOpenAreas(void);
void _SquishInit();
void _SquishDeInit();


static byte *intl = (byte *) "INTL";
static byte *fmpt = (byte *) "FMPT";
static byte *topt = (byte *) "TOPT";
static byte *area_colon = (byte *) "AREA:";

static char *copyright = "MSGAPI - Copyright 1991 by Scott J. Dudley.  All rights reserved.";

/* Global error value for message API routines */

word _stdc msgapierr = 0;

struct _minf _stdc mi;

void _MsgCloseApi(void)
{
/*
  TODO: DeInit (close open areas etc.) for all msgbase types
*/

    _SquishDeInit();
    JamCloseOpenAreas();
}

#ifdef UNIX
/* Just a dummy alarm-fnct */
static void alrm(int x)
{}     
#endif

sword _XPENTRY MsgOpenApi(struct _minf *minf)
{
#ifdef UNIX
    struct sigaction alrmact;
#endif
    
    unused(copyright);
    mi.req_version = minf->req_version;
    mi.def_zone    = minf->def_zone;
    mi.haveshare   = minf->haveshare = shareloaded();

    /* Version 2 Requested */
    if (mi.req_version > 1 && mi.req_version < 50)
    {
       mi.smapi_version    = minf->smapi_version    = MSGAPI_VERSION;
       mi.smapi_subversion = minf->smapi_subversion = MSGAPI_SUBVERSION;
    }

    _SquishInit();

    atexit(_MsgCloseApi);
    
    /*
     * Set the dummy alarm-fcnt to supress stupid messages.
     */
#ifdef UNIX    
    memset(&alrmact, 0, sizeof(alrmact));
    alrmact.sa_handler = alrm;
    sigaction(SIGALRM, &alrmact, 0);
#endif 

    return 0;
}

sword _XPENTRY MsgCloseApi(void)
{
    _MsgCloseApi();
    return 0;
}

MSGA *_XPENTRY MsgOpenArea(byte * name, word mode, word type)
{
    if (type & MSGTYPE_SQUISH) return SquishOpenArea(name, mode, type);
    else if (type & MSGTYPE_JAM) return JamOpenArea(name, mode, type);
    else return SdmOpenArea(name, mode, type);
    
    /* Hey, i'm reading correct ? If not SQUISH and not JAM it must be SDM ?
       That's a good thing to make incompatible code, but we're not M$ !
       TODO: Use MSGTYPE_SDM for checking type-parameter. There could be
             other developer who wants to extend the API with another
             Base-Type */
}

int MsgDeleteBase(char * name, word type)
{
    if (type & MSGTYPE_SQUISH) return SquishDeleteBase(name);
    else if (type & MSGTYPE_JAM) return JamDeleteBase(name);
    else return SdmDeleteBase(name);
}

sword _XPENTRY MsgValidate(word type, byte * name)
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

sword MSGAPI InvalidMh(MSGA * mh)
{
    if (mh == NULL || mh->id != MSGAPI_ID)
    {
        msgapierr = MERR_BADH;
        return TRUE;
    }

    return FALSE;
}

/* Check to ensure that a message handle is valid. */

sword MSGAPI InvalidMsg(XMSG * msg)
{
    if (msg == NULL)
    {
        msgapierr = MERR_BADA;
        return TRUE;
    }

    return FALSE;
}


byte *StripNasties(byte * str)
{
  byte *p;

  if(str)
  {
    p = str;
    while (*p != '\0')
    {
        if (*p < ' ')
        {
            *p = ' ';
        }
        p++;
    }
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

byte *_XPENTRY CopyToControlBuf(byte * txt, byte ** newtext, unsigned *length)
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

byte *_XPENTRY GetCtrlToken(byte *where, byte *what)
{
    byte *end, *out;
    unsigned int len;

    if (where == NULL || what == NULL) return NULL;
    len = strlen(what);

    do {
	where = (byte *)strchr((char *)where, '\001');
	if (where == NULL) break;
	where++;
    } while (strncmp((char *)where, (char *)what, len));

    if (where == NULL || strlen(where)<len) return NULL;
    
    end = (byte *) strchr((char *) where, '\r');
    if (end == NULL) end = (byte *) strchr((char *) where, '\001');
    if (end == NULL) end = where + strlen((char *) where);

    out = palloc((size_t) (end - where) + 1);
    if (out == NULL) return NULL;

    memmove(out, where, (size_t) (end - where));
    out[(size_t) (end - where)] = '\0';
    return out;
}

void _XPENTRY ConvertControlInfo(byte * ctrl, NETADDR * orig, NETADDR * dest)
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

byte *_XPENTRY CvtCtrlToKludge(byte * ctrl)
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

void _XPENTRY RemoveFromCtrl(byte * ctrl, byte * what)
{
    byte *p;
    unsigned int len = strlen(what);

    while (1) {
	ctrl = strchr((char *)ctrl, '\001');
	if (ctrl == NULL) return;
	if (strncmp((char *)ctrl+1, (char *)what, len)) {
	    ctrl++;
	    continue;
	}
	if (strlen((char *)ctrl + 1) < len) return;
	/* found */
	p = strchr((char *)ctrl + 1, '\001');
	if (p == NULL) {
	    *ctrl = '\0';
	    return;
	}
	strocpy((char *)ctrl, (char *)p);
    }
}

word _XPENTRY NumKludges(char *txt)
{
    word nk = 0;
    char *p;

    for(p=txt; ((p=strchr(p, '\001'))!=NULL); p++) nk++;

    return nk;
}
