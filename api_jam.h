/*
**  Adapted for MSGAPI by Fedor Lizunkov 2:5020/960@FidoNet
**
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**
**  Prototypes and definitions for the JAM message base format
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    JoHo
**  Initial coding.
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __API_JAM_H__
#define __API_JAM_H__

/*
**  File extensions
*/
#define EXT_HDRFILE     ".jhr"
#define EXT_TXTFILE     ".jdt"
#define EXT_IDXFILE     ".jdx"
#define EXT_LRDFILE     ".jlr"

/*
**  Revision level and header signature
*/
#define CURRENTREVLEV   1
#define HEADERSIGNATURE "JAM"

/*
**  Header file information block, stored first in all .JHR files
*/
typedef struct
    {
    byte   Signature[4];              /* <J><A><M> followed by <NUL> */
    dword  DateCreated;               /* Creation date */
    dword  ModCounter;                /* Last processed counter */
    dword  ActiveMsgs;                /* Number of active (not deleted) msgs */
    dword  PasswordCRC;               /* CRC-32 of password to access */
    dword  BaseMsgNum;                /* Lowest message number in index file */
    dword  highwater;                 /* Number of the last msg scanned */
    byte   RSRVD[996];                /* Reserved space */
    }
    JAMHDRINFO,  *JAMHDRINFOptr;

#define HDRINFO_SIZE (1000 + (4 * 6))

/*
**  Message status bits
*/
#define JMSG_LOCAL       0x00000001L    /* Msg created locally */
#define JMSG_INTRANSIT   0x00000002L    /* Msg is in-transit */
#define JMSG_PRIVATE     0x00000004L    /* Private */
#define JMSG_READ        0x00000008L    /* Read by addressee */
#define JMSG_SENT        0x00000010L    /* Sent to remote */
#define JMSG_KILLSENT    0x00000020L    /* Kill when sent */
#define JMSG_ARCHIVESENT 0x00000040L    /* Archive when sent */
#define JMSG_HOLD        0x00000080L    /* Hold for pick-up */
#define JMSG_CRASH       0x00000100L    /* Crash */
#define JMSG_IMMEDIATE   0x00000200L    /* Send Msg now, ignore restrictions */
#define JMSG_DIRECT      0x00000400L    /* Send directly to destination */
#define JMSG_GATE        0x00000800L    /* Send via gateway */
#define JMSG_FILEREQUEST 0x00001000L    /* File request */
#define JMSG_FILEATTACH  0x00002000L    /* File(s) attached to Msg */
#define JMSG_TRUNCFILE   0x00004000L    /* Truncate file(s) when sent */
#define JMSG_KILLFILE    0x00008000L    /* Delete file(s) when sent */
#define JMSG_RECEIPTREQ  0x00010000L    /* Return receipt requested */
#define JMSG_CONFIRMREQ  0x00020000L    /* Confirmation receipt requested */
#define JMSG_ORPHAN      0x00040000L    /* Unknown destination */
#define JMSG_ENCRYPT     0x00080000L    /* Msg text is encrypted */
#define JMSG_COMPRESS    0x00100000L    /* Msg text is compressed */
#define JMSG_ESCAPED     0x00200000L    /* Msg text is seven bit ASCII */
#define JMSG_FPU         0x00400000L    /* Force pickup */
#define JMSG_TYPELOCAL   0x00800000L    /* Msg is for local use only (not for export) */
#define JMSG_TYPEECHO    0x01000000L    /* Msg is for conference distribution */
#define JMSG_TYPENET     0x02000000L    /* Msg is direct network mail */
#define JMSG_NODISP      0x20000000L    /* Msg may not be displayed to user */
#define JMSG_LOCKED      0x40000000L    /* Msg is locked, no editing possible */
#define JMSG_DELETED     0x80000000L    /* Msg is deleted */

/*
**  Message header
*/
typedef struct
    {
    byte   Signature[4];              /* <J><A><M> followed by <NUL> */
    word   Revision;                  /* CURRENTREVLEV */
    word   ReservedWord;              /* Reserved */
    dword  SubfieldLen;               /* Length of subfields */
    dword  TimesRead;                 /* Number of times message read */
    dword  MsgIdCRC;                  /* CRC-32 of MSGID line */
    dword  ReplyCRC;                  /* CRC-32 of REPLY line */
    dword  ReplyTo;                   /* This msg is a reply to.. */
    dword  Reply1st;                  /* First reply to this msg */
    dword  ReplyNext;                 /* Next msg in reply chain */
    dword  DateWritten;               /* When msg was written */
    dword  DateReceived;              /* When msg was received/read */
    dword  DateProcessed;             /* When msg was processed by packer */
    dword  MsgNum;                    /* Message number (1-based) */
    dword  Attribute;                 /* Msg attribute, see "Status bits" */
    dword  Attribute2;                /* Reserved for future use */
    dword  TxtOffset;                 /* Offset of text in text file */
    dword  TxtLen;                    /* Length of message text */
    dword  PasswordCRC;               /* CRC-32 of password to access msg */
    dword  Cost;                      /* Cost of message */
    }
    JAMHDR, *JAMHDRptr;

#define HDR_SIZE (8 + (4 * 17))

/*
**  Message header subfield types
*/
#define JAMSFLD_OADDRESS    0
#define JAMSFLD_DADDRESS    1
#define JAMSFLD_SENDERNAME  2
#define JAMSFLD_RECVRNAME   3
#define JAMSFLD_MSGID       4
#define JAMSFLD_REPLYID     5
#define JAMSFLD_SUBJECT     6
#define JAMSFLD_PID         7
#define JAMSFLD_TRACE       8
#define JAMSFLD_ENCLFILE    9
#define JAMSFLD_ENCLFWALIAS 10
#define JAMSFLD_ENCLFREQ    11
#define JAMSFLD_ENCLFILEWC  12
#define JAMSFLD_ENCLINDFILE 13
#define JAMSFLD_EMBINDAT    1000
#define JAMSFLD_FTSKLUDGE   2000
#define JAMSFLD_SEENBY2D    2001
#define JAMSFLD_PATH2D      2002
#define JAMSFLD_FLAGS       2003
#define JAMSFLD_TZUTCINFO   2004
#define JAMSFLD_UNKNOWN     0xffff

/*
**  Message header subfield
*/

typedef struct JAMSUBFIELD2struct
{
	word  LoID;
	word  HiID;
	dword DatLen;
	byte *Buffer;
	struct JAMSUBFIELD2struct *next;
} JAMSUBFIELD2, *JAMSUBFIELD2ptr;


typedef struct
    {
    word  LoID;                      /* Field ID, 0 - 0xffff */
    word  HiID;                      /* Reserved for future use */
    dword DatLen;                    /* Length of buffer that follows */
    byte  Buffer[1];                 /* DatLen bytes of data */
    }
    JAMSUBFIELD1, *JAMSUBFIELD1ptr;

typedef struct
    {
    word  LoID;                      /* Field ID, 0 - 0xffff */
    word  HiID;                      /* Reserved for future use */
    dword DatLen;                    /* Length of buffer that follows */
    }
    JAMBINSUBFIELD, *JAMBINSUBFIELDptr;

/*
**  Message index record
*/
typedef struct
    {
    dword  UserCRC;                   /* CRC-32 of destination username */
    dword  HdrOffset;                 /* Offset of header in .JHR file */
    }
    JAMIDXREC, *JAMIDXRECptr;

#define IDX_SIZE 8

/*
**  Lastread structure, one per user
*/
typedef struct
    {
    dword  UserCRC;                   /* CRC-32 of user name (lowercase) */
    dword  UserID;                    /* Unique UserID */
    dword  LastReadMsg;               /* Last read message number */
    dword  HighReadMsg;               /* Highest read message number */
    }
    JAMLREAD, *JAMLREADptr;

typedef struct {
   dword     IdxOffset;         /* Offset Idx file */
   dword     TrueMsg;           /* JAMIDXREC.HdrOffset */
   } JAMACTMSG, *JAMACTMSGptr;

/* struct jam base */

typedef struct {
   byte            *BaseName;      /* Path for message base */
   int             HdrHandle;      /* File handle for .JHR file */
   int             TxtHandle;      /* File handle for .JDT file */
   int             IdxHandle;      /* File handle for .JDX file */
   int             LrdHandle;      /* File handle for .JLR file */
   JAMHDRINFO      HdrInfo;        /* Message header info */
   JAMACTMSGptr    actmsg;
   word            msgs_open;
   } JAMBASE, *JAMBASEptr;

#endif /* __JAM_H__ */

#ifdef __cplusplus
}
#endif

