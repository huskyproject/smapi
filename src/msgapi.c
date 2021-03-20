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

#include <huskylib/compiler.h>


#ifdef HAS_SIGNAL_H
#include <signal.h>
#endif

#include <huskylib/huskylib.h>
/* Swith for build DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "msgapi.h"
#include "apidebug.h"

unsigned _SquishCloseOpenAreas(void);
void _SquishInit();
void _SquishDeInit();

static byte * intl       = (byte *)"INTL";
static byte * fmpt       = (byte *)"FMPT";
static byte * topt       = (byte *)"TOPT";
static byte * area_colon = (byte *)"AREA:";
/*static char *copyright = "MSGAPI - Copyright 1991 by Scott J. Dudley.  All rights
   reserved.";*/
/* Global error value for message API routines */
word _stdc msgapierr = 0;
struct _minf _stdc mi = {0};
const char * _XPENTRY smapi_cvs_date()
{
    static
#include "../cvsdate.h"
    return cvs_date;
}

int _MsgCloseApi(void)
{
    int result = 0;

/*
   TODO: DeInit (close open areas etc.) for all msgbase types
 */
    _SquishDeInit();
    result = JamCloseOpenAreas();
    return result;
}

#ifdef HAS_SIGNAL_H /* old: #ifdef __UNIX__ */
/* Just a dummy alarm-fnct */
static void alrm(int x)
{
    unused(x);
}

#endif

sword _XPENTRY MsgOpenApi(struct _minf * minf)
{
/*    unused(copyright);*/
    mi.req_version = minf->req_version;
    mi.def_zone    = minf->def_zone;
    mi.haveshare   = minf->haveshare = shareloaded();

    /* Version 2 Requested */
    if(mi.req_version > 1 && mi.req_version < 50)
    {
        mi.smapi_version    = minf->smapi_version = MSGAPI_VERSION;
        mi.smapi_subversion = minf->smapi_subversion = MSGAPI_SUBVERSION;
    }

    _SquishInit();
    atexit((void (*)(void))_MsgCloseApi);
    /*
     * Set the dummy alarm-fcnt to supress stupid messages.
     */
#if defined(HAS_SIGNAL_H) && !defined(__WATCOMC__)
    {
        struct sigaction alrmact;
        memset(&alrmact, 0, sizeof(alrmact));
        alrmact.sa_handler = alrm;
        sigaction(SIGALRM, &alrmact, 0);
    }
#endif

    return 0;
} /* MsgOpenApi */

sword _XPENTRY MsgCloseApi(void)
{
    return (sword)_MsgCloseApi();
}

MSGA * _XPENTRY MsgOpenArea(byte * name, word mode, word type)
{
    switch(type & MSGTYPE_STORAGES)
    {
        case MSGTYPE_SQUISH:
            return SquishOpenArea(name, mode, type);

        case MSGTYPE_JAM:
            return JamOpenArea(name, mode, type);

        case MSGTYPE_SDM:
            return SdmOpenArea(name, mode, type);

        case MSGTYPE_PASSTHROUGH:
            msgapierr = MERR_NONE;                   /* Try to open pssthrough area */
            return NULL;

        default:
            msgapierr = MERR_BADA;                   /* illegal msgbase type */
            return NULL;
    }
}

int MsgDeleteBase(char * name, word type)
{
    if(!name)
    {
        return FALSE;
    }

    switch(type & MSGTYPE_STORAGES)
    {
        case MSGTYPE_SQUISH:
            return SquishDeleteBase(name);

        case MSGTYPE_JAM:
            return JamDeleteBase(name);

        case MSGTYPE_SDM:
            return SdmDeleteBase(name);

        default:
            return TRUE;
    }
}

sword _XPENTRY MsgValidate(word type, byte * name)
{
    switch(type & MSGTYPE_STORAGES)
    {
        case MSGTYPE_SQUISH:
            return SquishValidate(name);

        case MSGTYPE_JAM:
            return JamValidate(name);

        case MSGTYPE_SDM:
            return SdmValidate(name);

        default:
            return TRUE;
    }
}

/*
 *  Check to see if a message handle is valid.  This function should work
 *  for ALL handlers tied into MsgAPI.  This also checks to make sure that
 *  the area which the message is from is also valid (ie. the message handle
 *  isn't valid unless the area handle of that message is also valid).
 */
sword MSGAPI InvalidMsgh(MSGH * msgh)
{
    if(msgh == NULL || msgh->id != MSGH_ID || InvalidMh(msgh->sq))
    {
        msgapierr = MERR_BADH;
        return TRUE;
    }

    return FALSE;
}

/* Check to ensure that a message area handle is valid. */
sword MSGAPI InvalidMh(MSGA * mh)
{
    if(mh == NULL || mh->id != MSGAPI_ID)
    {
        msgapierr = MERR_BADH;
        return TRUE;
    }

    return FALSE;
}

/* Check to ensure that a message handle is valid. */
sword MSGAPI InvalidMsg(XMSG * msg)
{
    if(msg == NULL)
    {
        msgapierr = MERR_BADA;
        return TRUE;
    }

    return FALSE;
}

byte * StripNasties(byte * str)
{
    byte * p;

    if(str)
    {
        p = str;

        while(*p != '\0')
        {
            if(*p < ' ')
            {
                *p = ' ';
            }

            p++;
        }
    }

    return str;
}

/* Copy the text itself to a buffer, or count its length if out==NULL */
static dword near _CopyToBuf(byte * p, byte * out, byte ** end)
{
    dword len     = 0;
    byte * p_text = p;

    if(out)
    {
        *out++ = '\001';
    }

    len++;

    while(*p == '\015' || *p == '\012')
    {
        p++;
    }

    while(*p == '\001' || strncmp((char *)p, (char *)area_colon, 5) == 0)
    {
        /* Skip over the first ^a */
        if(*p == '\001')
        {
            p++;
        }

        while(*p && *p != '\015' && *p != '\012')
        {
            if(out)
            {
                *out++ = *p;
            }

            p++;
            len++;
        }

        if(out)
        {
            *out++ = '\001';
        }

        len++;

        /* preserve empty lines between kludges and text */
        if(*p == '\015' || *p == '\012')
        {
            if((p[0] == '\015' && p[1] == '\012') || (p[0] == '\012' && p[1] == '\015'))
            {
                p_text = p += 2;
            }
            else
            {
                p_text = ++p;
            }
        }

        while(*p == '\015' || *p == '\012')
        {
            p++;
        }
    }

    /* Cap the string */
    if(out)
    {
        *out = '\0';
    }

    len++;

    /* Make sure to leave no trailing x01's. */
    if(out && out[-1] == '\001')
    {
        out[-1] = '\0';
    }

    /* Now store the new end location of the kludge lines */
    if(end)
    {
        *end = p_text;
    }

    return len;
} /* _CopyToBuf */

byte * _XPENTRY CopyToControlBuf(byte * txt, byte ** newtext, unsigned * length)
{
    byte * cbuf, * end;
    dword clen;

    /* Figure out how long the control info is */
    clen = _CopyToBuf(txt, NULL, NULL);
    /* Allocate memory for it */

    #define SAFE_CLEN 5

    cbuf = palloc(clen + SAFE_CLEN * sizeof(byte *));

    if(cbuf == NULL)
    {
        return NULL;
    }

    memset(cbuf, '\0', clen + SAFE_CLEN);
    /* Now copy the text itself */
    clen = _CopyToBuf(txt, cbuf, &end);

    if(length)
    {
        *length -= (unsigned int)(end - txt);
    }

    if(newtext)
    {
        *newtext = end;
    }

    return cbuf;
} /* CopyToControlBuf */

byte * _XPENTRY GetCtrlToken(byte * where, byte * what)
{
    byte * end, * out;
    size_t len;

    if(where == NULL || what == NULL)
    {
        return NULL;
    }

    len = strlen((const char *)what);

    do
    {
        where = (byte *)strchr((const char *)where, '\001');

        if(where == NULL)
        {
            break;
        }

        where++;
    }
    while(strncmp((const char *)where, (const char *)what, len));

    if(where == NULL || strlen((const char *)where) < len)
    {
        return NULL;
    }

    end = (byte *)strchr((const char *)where, '\r');

    if(end == NULL)
    {
        end = (byte *)strchr((const char *)where, '\001');
    }

    if(end == NULL)
    {
        end = where + strlen((const char *)where);
    }

    out = palloc((size_t)(end - where) + 1);

    if(out == NULL)
    {
        return NULL;
    }

    memmove(out, where, (size_t)(end - where));
    out[(size_t)(end - where)] = '\0';
    return out;
} /* GetCtrlToken */

/* NETADDR content is rewritten with discovered information */
/* If some address pieces aren't discovered then they stay intact */
/* Be sure these structures don't contain junk before this call */
void _XPENTRY ConvertControlInfo(byte * ctrl, NETADDR * orig, NETADDR * dest)
{
    byte * s;
    const char * p;

    s = GetCtrlToken(ctrl, intl);

    if(s != NULL)
    {
        NETADDR norig, ndest;
        p = (char *)s;

        /* Copy the defaults from the original address */
        norig = *orig;
        ndest = *dest;
        /* Parse the destination part of the kludge */
        parseFtnAddrZ(p + 5, &ndest, FTNADDR_GOOD, &p);

        while(*p != ' ' && *p)  /* TODO: Should'n be the case! Maybe remove it and skip intl
                                   processing. */
        {
            p++;
        }
        parseFtnAddrZS(p, &norig);
        pfree(s);

        /*
         *  Only use this as the "real" zonegate address if the net/node
         *  addresses in the INTL line match those in the message body.
         *  Otherwise, it's probably a gaterouted message!
         */
        if(ndest.net == dest->net && ndest.node == dest->node && norig.net == orig->net &&
           norig.node == orig->node)
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

    if(s != NULL)
    {
        orig->point = (word)atoi((char *)s + 5);
        pfree(s);
        /* mtt: DO NO CHANGE THE MSGTEXT!!!! */
        /* RemoveFromCtrl(ctrl, fmpt);       */
    }

    /* Handle TOPT too */
    s = GetCtrlToken(ctrl, topt);

    if(s != NULL)
    {
        dest->point = (word)atoi((char *)s + 5);
        pfree(s);
        /* mtt: DO NOT CHANGE THE MSGTEXT!!! */
        /* RemoveFromCtrl(ctrl, topt);       */
    }
} /* ConvertControlInfo */

byte * _XPENTRY CvtCtrlToKludge(byte * ctrl)
{
    byte * from, * to, * buf;
    size_t clen;

    clen = strlen((char *)ctrl) + NumKludges((char *)ctrl) + 20;
    buf  = palloc(clen);

    if(buf == NULL)
    {
        return NULL;
    }

    to = buf;

    /* Convert ^aKLUDGE^aKLUDGE... into ^aKLUDGE\r^aKLUDGE\r... */
    from = ctrl;

    while(*from == '\001' && from[1])
    {
        /* Only copy out the ^a if it's NOT the area: line */
        if(strncmp((char *)from + 1, (char *)area_colon, 5) != 0)
        {
            *to++ = *from;
        }

        from++;

        while(*from && *from != '\001')
        {
            *to++ = *from++;
        }
        *to++ = '\r';
    }
    *to = '\0';
    return buf;
} /* CvtCtrlToKludge */

void _XPENTRY RemoveFromCtrl(byte * ctrl, byte * what)
{
    byte * p;
    size_t len = strlen((char *)what);

    for( ; ; )
    {
        ctrl = (byte *)strchr((const char *)ctrl, '\001');

        if(ctrl == NULL)
        {
            return;
        }

        if(strncmp((const char *)ctrl + 1, (const char *)what, len))
        {
            ctrl++;
            continue;
        }

        if(strlen((const char *)ctrl + 1) < len)
        {
            return;
        }

        /* found */
        p = (byte *)strchr((const char *)ctrl + 1, '\001');

        if(p == NULL)
        {
            *ctrl = '\0';
            return;
        }

        strocpy((char *)ctrl, (const char *)p);
    }
} /* RemoveFromCtrl */

word _XPENTRY NumKludges(char * txt)
{
    word nk = 0;
    char * p;

    for(p = txt; ((p = strchr(p, '\001')) != NULL); p++)
    {
        nk++;
    }
    return nk;
}

/*  Return MSGAPI error text (string constant).
 */
char * _XPENTRY strmerr(int my_msgapierr)
{
    switch(my_msgapierr)
    {
        case MERR_NONE:
            return "No error";

        case MERR_BADH:
            return "Invalid handle passed to function";

        case MERR_BADF:
            return "Invalid or corrupted file";

        case MERR_NOMEM:
            return "Not enough memory for specified operation";

        case MERR_NODS:
            return "Maybe not enough disk space for operation";

        case MERR_NOENT:
            return "File/message does not exist";

        case MERR_BADA:
            return "Bad argument passed to msgapi function";

        case MERR_EOPEN:
            return "Couldn't close - messages still open";

        case MERR_NOLOCK:
            return "Base needs to be locked to perform operation";

        case MERR_SHARE:
            return "Resource in use by other process";

        case MERR_EACCES:
            return "Access denied (can't write to read-only, etc)";

        case MERR_BADMSG:
            return "Bad message frame (Squish)";

        case MERR_TOOBIG:
            return "Too much text/ctrlinfo to fit in frame (Squish)";

        case MERR_BADNAME:
            return "Bad area name or file name";

        case MERR_LIMIT:
            return "Messagebase limit is reached";

        case MERR_INTERN:
            return "Internal progran error";

        default:
            break;
    } /* switch */
    return "Unknown error";
} /* strmerr */

/* Check version of smapi library
 * return zero if test failed; non-zero if passed
 * test cvs need for DLL version only, using #include <smapi/cvsdate.h>
   const char *smapidate(){
   static const
 #include "../smapi/smapi/cvsdate.h"
   return cvs_date;
   }
   CheckSmapiVersion( ..., smapidate());
 */
int _XPENTRY CheckSmapiVersion(int need_major,
                               int need_minor,
                               int need_patch,
                               const char * cvs_date_string)
{
    static
  #include "../cvsdate.h"   /* char cvs_date[]=datestring; */

    if(need_major == MSGAPI_VERSION && need_minor == ((MSGAPI_SUBVERSION & 0x0F0) >> 4) &&
       need_patch == (MSGAPI_SUBVERSION & 0x00F))
    {
        return !(cvs_date_string && strcmp(cvs_date_string, cvs_date));
    }

    return 0;
}
