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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MSGAPI
#define MSGAPI

#include "compiler.h"
#include "typedefs.h"
#include "stamp.h"

#ifdef __BEOS__
#include <OS.h>
#endif

#define MSGAREA_NORMAL  0x00
#define MSGAREA_CREATE  0x01
#define MSGAREA_CRIFNEC 0x02

/* Message & messagebase types (FIDO/OPUS, jam, squish, passthrough, ...) */
/*  - types by storage type */
#define MSGTYPE_STORAGES    0x0F /* MSGTYPE_SDM | MSGTYPE_SQUISH | MSGTYPE_PASSTHROUGH | MSGTYPE_JAM */
#define MSGTYPE_SDM         0x01
#define MSGTYPE_SQUISH      0x02
#define MSGTYPE_PASSTHROUGH 0x04
#define MSGTYPE_JAM         0x08
/*  - types by area type (echo, net, ...) */
#define MSGTYPE_AREAS       0x1C0  /* MSGTYPE_ECHO | ... */
#define MSGTYPE_ECHO        0x80
#define MSGTYPE_NOTH        0x0100 /* What is it ?... unknown... */

/* Special message number values */
#define MSGNUM_CUR      ((dword)-1L)
#define MSGNUM_PREV     ((dword)-2L)
#define MSGNUM_NEXT     ((dword)-3L)

#define MSGNUM_current  MSGNUM_CUR
#define MSGNUM_previous MSGNUM_PREV
#define MSGNUM_next     MSGNUM_NEXT

#define MOPEN_CREATE    0
#define MOPEN_READ      1
#define MOPEN_WRITE     2
#define MOPEN_RW        3

#ifdef __UNIX__
#define FILEMODE_NETMAIL (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define FILEMODE_ECHOMAIL (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#else
#define FILEMODE_NETMAIL (S_IREAD | S_IWRITE)
#define FILEMODE_ECHOMAIL (S_IREAD | S_IWRITE)
#endif
#define FILEMODE(a) ((a) ? FILEMODE_ECHOMAIL : FILEMODE_NETMAIL)

struct _msgapi;
struct _msgh;
struct _xmsg;
struct _netaddr;

typedef struct _msgapi MSGA;
typedef struct _msgapi *HAREA;
typedef struct _msgh MSGH;
typedef struct _msgh *HMSG;
typedef struct _xmsg *PXMSG;
typedef struct _netaddr NETADDR;
typedef NETADDR *PNETADDR;
typedef dword UMSGID;

#define MSGAPI_VERSION    2
#define MSGAPI_SUBVERSION 0x240
#define SMAPI_VERSION   "2.4.0"

struct _minf
{
    word req_version;
    word def_zone;
    word haveshare;  /* filled in by msgapi routines - no need to set this */

    /* Version 2 Information */
    word smapi_version;
    word smapi_subversion;
};

/*
 *  The network address structure.  The zone/net/node/point fields are
 *  always maintained in parallel to the 'ascii' field, which is simply
 *  an ASCII representation of the address.  In addition, the 'ascii' field
 *  can be used for other purposes (such as Internet addresses), so the
 *  contents of this field are implementation-defined, but for most cases,
 *  should be in the format "1:123/456.7" for Fido addresses.
 */

struct _netaddr
{
    word zone;
    word net;
    word node;
    word point;
};

/*
 *  The eXtended message structure.  Translation between this structure,
 *  and the structure used by the individual message base formats, is done
 *  on-the-fly by the API routines.
 */

typedef struct _xmsg
{
    /* Bitmasks for 'attr' */

#define MSGPRIVATE 0x0001
#define MSGCRASH   0x0002
#define MSGREAD    0x0004
#define MSGSENT    0x0008
#define MSGFILE    0x0010
#define MSGFWD     0x0020     /* is also called intransit flag */
#define MSGORPHAN  0x0040
#define MSGKILL    0x0080
#define MSGLOCAL   0x0100
#define MSGHOLD    0x0200
#define MSGXX2     0x0400     /* you can use this flag as "Direct" attribute */
#define MSGFRQ     0x0800
#define MSGRRQ     0x1000
#define MSGCPT     0x2000
#define MSGARQ     0x4000
#define MSGURQ     0x8000
#define MSGSCANNED 0x00010000L
#define MSGUID     0x00020000L /* xmsg.uid field contains umsgid of msg */
#define MSGIMM     0x00040000L /* Use only if msgtype == MSGTYPE_JAM !
                                  Used to map the Jam "immediate" attribute. */
#define MSGLOCKED  0x40000000L /* this seems to be a feature of golded  */


    dword attr;

#define XMSG_FROM_SIZE  36
#define XMSG_TO_SIZE    36
#define XMSG_SUBJ_SIZE  72

    byte from[XMSG_FROM_SIZE];
    byte to[XMSG_TO_SIZE];
    byte subj[XMSG_SUBJ_SIZE];

    NETADDR orig;               /* Origination and destination addresses */
    NETADDR dest;

    struct _stamp date_written; /* When user wrote the msg (UTC) */
    struct _stamp date_arrived; /* When msg arrived on-line (UTC) */
    sword utc_ofs;              /* Offset from UTC of message writer, in
                                 * minutes. */

#define MAX_REPLY 9            /* Max number of stored replies to one msg */

    UMSGID replyto;
    UMSGID replies[MAX_REPLY];
    dword umsgid;               /* UMSGID of this message, if (attr&MSGUID) */
                                /* This field is only stored on disk -- it  *
                                 * is not read into memory.                 */

    byte __ftsc_date[20];       /* Obsolete date information.  If it weren't
                                 * for the fact that FTSC standards say that
                                 * one cannot modify an in-transit message,
                                 * I'd be VERY tempted to axe this field
                                 * entirely, and recreate an FTSC-compatible
                                 * date field using the information in
                                 * 'date_written' upon export.  Nobody should
                                 * use this field, except possibly for tossers
                                 * and scanners.  All others should use one
                                 * of the two binary datestamps, above. */
}
XMSG;

/* XMSG_SIZE is not sizeof(XMSG) but size of squish message header.
 * This value used for disk i/o operations with squish messagebase.
 */
#define XMSG_SIZE (94 + XMSG_FROM_SIZE + XMSG_TO_SIZE + XMSG_SUBJ_SIZE)

#define xmreply1st replies[0]
#define xmreplynext replies[MAX_REPLY-1]
#define xmtimesread replies[MAX_REPLY-2]
#define xmcost replies[MAX_REPLY-3]

/*
 *  This is a 'message area handle', as returned by MsgOpenArea(), and
 *  required by calls to all other message functions.  This structure must
 *  always be accessed through the API functions, and never modified
 *  directly.
 */

struct _msgapi
{
#define MSGAPI_ID   0x0201414DL

    dword id;                   /* Must always equal MSGAPI_ID */

    word len;                   /* LENGTH OF THIS STRUCTURE! */
    word type;

    dword num_msg;
    dword cur_msg;
    dword high_msg;
    dword high_water;

    word sz_xmsg;

    byte locked;                /* Base is locked from use by other tasks */
    byte isecho;                /* Is this an EchoMail area?              */

    /* Function pointers for manipulating messages within this area. */

    struct _apifuncs
    {
        sword(_XPENTRY * CloseArea) (HAREA mh);
        MSGH *(_XPENTRY * OpenMsg) (HAREA mh, word mode, dword n);
        sword(_XPENTRY * CloseMsg) (MSGH * msgh);
        dword(_XPENTRY * ReadMsg) (MSGH * msgh, XMSG * msg, dword ofs, dword bytes, byte * text, dword cbyt, byte * ctxt);
        sword(_XPENTRY * WriteMsg) (MSGH * msgh, word append, XMSG * msg, byte * text, dword textlen, dword totlen, dword clen, byte * ctxt);
        sword(_XPENTRY * KillMsg) (HAREA mh, dword msgnum);
        sword(_XPENTRY * Lock) (HAREA mh);
        sword(_XPENTRY * Unlock) (HAREA mh);
        sword(_XPENTRY * SetCurPos) (MSGH * msgh, dword pos);
        dword(_XPENTRY * GetCurPos) (MSGH * msgh);
        UMSGID(_XPENTRY * MsgnToUid) (HAREA mh, dword msgnum);
        dword(_XPENTRY * UidToMsgn) (HAREA mh, UMSGID umsgid, word type);
        dword(_XPENTRY * GetHighWater) (HAREA mh);
        sword(_XPENTRY * SetHighWater) (HAREA mh, dword hwm);
        dword(_XPENTRY * GetTextLen) (MSGH * msgh);
        dword(_XPENTRY * GetCtrlLen) (MSGH * msgh);
        /* Version 1 Functions */
        UMSGID (_XPENTRY * GetNextUid)(HAREA harea);
        /* Version 2 Functions */
        dword  (_XPENTRY * GetHash)(HAREA harea, dword msgnum);
    } *api;

    /*
     *  Pointer to application-specific data.  API_SQ.C and API_SDM.C use
     *  this for different things, so again, no applications should muck
     *  with anything in here.
     */

    void *apidata;


#ifdef ALTLOCKING
    char  *lck_path;
    int    lck_handle;
#endif
};


/*
 *  This is a 'dummy' message handle.  The other message handlers (contained
 *  in API_SQ.C and API_SDM.C) will define their own structures, with some
 *  application-specified variables instead of other[].  Applications should
 *  not mess with anything inside the _msgh (or MSGH) structure.
 */

#define MSGH_ID  0x0302484DL

#if !defined(MSGAPI_HANDLERS) /*&& !defined(NO_MSGH_DEF) *//* NO_MSGH_DEF unused */
struct _msgh
{
    MSGA *sq;
    dword id;

    dword bytes_written;
    dword cur_pos;
};
#endif

#include "api_brow.h"

/*
 *  This variable is modified whenever an error occurs with the MsgXxx()
 *  functions.  If msgapierr == 0, then no error occurred.
 */
SMAPI_EXT word _stdc msgapierr;

extern struct _minf _stdc mi;

#define palloc(s)     malloc(s)
#define pfree(s)      { if (s) { free(s); s = NULL; } }
#define farpalloc(s)  farmalloc(s)
#define farpfree(s)   { if (s) { farfree(s); s = NULL; } }

/* Constants for 'type' argument of MsgUidToMsgn() */

#define UID_EXACT     0x00
#define UID_NEXT      0x01
#define UID_PREV      0x02

/* Values for 'msgapierr', above. */

#define MERR_NONE   0           /* No error  */
#define MERR_BADH   1           /* Invalid handle passed to function */
#define MERR_BADF   2           /* Invalid or corrupted file */
#define MERR_NOMEM  3           /* Not enough memory for specified operation */
#define MERR_NODS   4           /* Maybe not enough disk space for operation */
#define MERR_NOENT  5           /* File/message does not exist */
#define MERR_BADA   6           /* Bad argument passed to msgapi function */
#define MERR_EOPEN  7           /* Couldn't close - messages still open */
#define MERR_NOLOCK 8     /* Base needs to be locked to perform operation   */
#define MERR_SHARE  9     /* Resource in use by other process               */
#define MERR_EACCES 10    /* Access denied (can't write to read-only, etc)  */
#define MERR_BADMSG 11    /* Bad message frame (Squish)                     */
#define MERR_TOOBIG 12    /* Too much text/ctrlinfo to fit in frame (Squish)*/
#define MERR_BADNAME 13   /* Bad area name or file name */

/*
 *  Now, a set of macros, which call the specified API function.  These
 *  will map calls for 'MsgOpenMsg()' into 'SquishOpenMsg()', 'SdmOpenMsg()',
 *  or '<insert fave message type here>'.  Applications should always call
 *  these macros, instead of trying to call the manipulation functions
 *  directly.
 */

#define MsgCloseArea(mh)                    (*(mh)->api->CloseArea) (mh)
#define MsgOpenMsg(mh,mode,n)               (*(mh)->api->OpenMsg)(mh,mode,n)
#define MsgCloseMsg(msgh)                   ((*(((struct _msgh *)msgh)->sq->api->CloseMsg))(msgh))
#define MsgReadMsg(msgh,msg,ofs,b,t,cl,ct)  (*(((struct _msgh *)msgh)->sq->api->ReadMsg))(msgh,msg,ofs,b,t,cl,ct)
#define MsgWriteMsg(gh,a,m,t,tl,ttl,cl,ct)  (*(((struct _msgh *)gh)->sq->api->WriteMsg))(gh,a,m,t,tl,ttl,cl,ct)
#define MsgKillMsg(mh,msgnum)               (*(mh)->api->KillMsg)(mh,msgnum)
#define MsgLock(mh)                         (*(mh)->api->Lock)(mh)
#define MsgUnlock(mh)                       (*(mh)->api->Unlock)(mh)
#define MsgGetCurPos(msgh)                  (*(((struct _msgh *)msgh)->sq->api->GetCurPos))(msgh)
#define MsgSetCurPos(msgh,pos)              (*(((struct _msgh *)msgh)->sq->api->SetCurPos))(msgh,pos)
#define MsgMsgnToUid(mh,msgn)               (*(mh)->api->MsgnToUid)(mh,msgn)
#define MsgUidToMsgn(mh,umsgid,t)           (*(mh)->api->UidToMsgn)(mh,umsgid,t)
#define MsgGetHighWater(mh)                 (*(mh)->api->GetHighWater)(mh)
#define MsgSetHighWater(mh,n)               (*(mh)->api->SetHighWater)(mh,n)
#define MsgGetTextLen(msgh)                 (*(((struct _msgh *)msgh)->sq->api->GetTextLen))(msgh)
#define MsgGetCtrlLen(msgh)                 (*(((struct _msgh *)msgh)->sq->api->GetCtrlLen))(msgh)
#define MsgGetNextUid(ha)                   (*(ha)->api->GetNextUid)(ha)

/*
 *  These don't actually call any functions, but are macros used to access
 *  private data inside the _msgh structure.
 */

#define MsgCurMsg(mh)      ((mh)->cur_msg)
#define MsgNumMsg(mh)      ((mh)->num_msg)
#define MsgHighMsg(mh)     ((mh)->high_msg)
#define MsgGetCurMsg(mh)   ((mh)->cur_msg)
#define MsgGetNumMsg(mh)   ((mh)->num_msg)
#define MsgGetHighMsg(mh)  ((mh)->high_msg)

#define MsgStripDebris(str)          StripNasties(str)
#define MsgCreateCtrlBuf(t, n, l)    CopyToControlBuf(t, n, l)
#define MsgGetCtrlToken(where, what) GetCtrlToken(where, what)
#define MsgCvt4D(c, o, d)            ConvertControlInfo(c, o, d)
#define MsgCvtCtrlToKludge(ctrl)     CvtCtrlToKludge(ctrl)
#define MsgRemoveToken(c, w)         RemoveFromCtrl(c, w)
#define MsgGetNumKludges(txt)        NumKludges(txt)
#define MsgWrite4D(msg, wf, ctrl)    WriteZPInfo(msg, wf, ctrl)
#define MsgInvalidHmsg(mh)           InvalidMsgh(mh)
#define MsgInvalidHarea(mh)          InvalidMh(mh)

#define MsgCvtFTSCDateToBinary(a, b) ASCII_Date_To_Binary(a,b)

SMAPI_EXT sword _XPENTRY MsgOpenApi(struct _minf *minf);
SMAPI_EXT sword _XPENTRY MsgCloseApi(void);

SMAPI_EXT MSGA *_XPENTRY MsgOpenArea(byte * name, word mode, word type);
SMAPI_EXT int MsgDeleteBase(char * name, word type);
sword _XPENTRY MsgValidate(word type, byte * name);
sword _XPENTRY MsgBrowseArea(BROWSE * b);

sword MSGAPI InvalidMsgh(MSGH * msgh);
sword MSGAPI InvalidMh(MSGA * mh);
sword MSGAPI InvalidMsg(XMSG * msg);

void _XPENTRY SquishSetMaxMsg(MSGA * sq, dword max_msgs, dword skip_msgs, dword age);
SMAPI_EXT dword _XPENTRY SquishHash(byte * f);

MSGA *MSGAPI SdmOpenArea(byte * name, word mode, word type);
sword MSGAPI SdmValidate(byte * name);
int SdmDeleteBase(char * name);

MSGA *MSGAPI SquishOpenArea(byte * name, word mode, word type);
sword MSGAPI SquishValidate(byte * name);
int SquishDeleteBase(char * name);

MSGA *MSGAPI JamOpenArea(byte * name, word mode, word type);
sword MSGAPI JamValidate(byte * name);
int JamDeleteBase(char * name);
void JamCloseOpenAreas();

SMAPI_EXT byte *_XPENTRY CvtCtrlToKludge(byte * ctrl);
SMAPI_EXT byte *_XPENTRY GetCtrlToken(byte * where, byte * what);
SMAPI_EXT byte *_XPENTRY CopyToControlBuf(byte * txt, byte ** newtext, unsigned *length);
void _XPENTRY ConvertControlInfo(byte * ctrl, NETADDR * orig, NETADDR * dest);
word _XPENTRY NumKludges(char *txt);
SMAPI_EXT void _XPENTRY RemoveFromCtrl(byte * ctrl, byte * what);
SMAPI_EXT dword _XPENTRY GenMsgId(char *seqdir, unsigned long max_outrun);
SMAPI_EXT dword _XPENTRY GenMsgIdEx(char *seqdir, unsigned long max_outrun, dword (*altGenMsgId)(void), char **errstr);

/* Check version of smapi library
 * return zero if test passed
 * test cvs need for DLL version only, using #include <smapi/cvsdate.h>
  const char *smapidate(){
  static const
  #include "../smapi/cvsdate.h"
  return cvs_date;
  }
  CheckSmapiVersion( ..., smapidate());
 */
SMAPI_EXT int _XPENTRY CheckSmapiVersion( int need_major, int need_minor,
                	    int need_patch, const char *cvs_date_string );

/*  Return MSGAPI error text (string constant).
 */
SMAPI_EXT char * _XPENTRY  strmerr(int msgapierr);


#if !defined(__FLAT__)
#ifndef farread
sword far pascal farread(sword handle, byte far * buf, word len);
#endif
#ifndef farwrite
sword far pascal farwrite(sword handle, byte far * buf, word len);
#endif
#endif

byte *_fast Address(NETADDR * a);
byte *StripNasties(byte * str);

#if defined(__DOS__)
sword far pascal shareloaded(void);
#elif defined(__OS2__) || defined(__NT__) || defined(__UNIX__)
#define shareloaded() TRUE
#else
#define shareloaded() FALSE
#endif

#endif

#ifdef __cplusplus
}
#endif

