{: Pascal (Delphi/ Kylix) Interface unit for the smapi. Documentation comments
   are taken from sqdev.prn, which is part of Scott Dudley's SQDEV200.zip archive. }

{: NOTE: Unfortunately it turned out that the structures that are passed
         between smapi and any calling programs are in no way align controlled,
         that is, how they are aligned is left to the compiler.
         That means that if you are not using the same compiler for your
         program that was used to compile the libraries, chances are that
         you will have an alignment problem resulting in fields of the
         structures being out of sync, e.g. you are reading something totally
         different from what the library put there.
         Because of this and because I don't want to chase a moving target
         like this, I will stop maintaining smapi.pas and fidoconf.pas.
         That probably means that husky has now become a project that can
         only be used by C programmers, unless somebody else wants
         to go through the trouble.
         One possible solution I can think of, would be having an interface
         layer that converts the randomly aligned structures to other
         structures that are fixed. I might do it, maybe, if I don't find
         anything else to waste my time on. But this is rather unlikely,
         because it has to be done in C and that is definitely not my
         favourite programming language.
         Please leave this comment in here, so nobody else will waste time
         trying to figure out why the data his programs reads is wrong.
         2001-07-22 twm }
         
unit smapi;

interface

type
  // this assumes that a word is 2 bytes and a longword is 4 bytes, true
  // for Delphi 5 and Kylix.
  sword = word;
  dword = longword;
  PPChar = ^PChar;
  PLongint = ^Longint;

const
  MSGAREA_NORMAL = $00; // read/write
  MSGAREA_CREATE = $01; // create
  MSGAREA_CRIFNEC = $02; // create if necessary

  MSGTYPE_SDM = $01; // *.MSG
  MSGTYPE_SQUISH = $02; // squish
  MSGTYPE_JAM = $08; // jam
  MSGTYPE_ECHO = $80;
  MSGTYPE_NOTH = $0100; // ??

  MSGNUM_CUR: longint = -1;
  MSGNUM_PREV: longint = -2;
  MSGNUM_NEXT: longint = -3;
  MSGNUM_current: longint = -1;
  MSGNUM_previous: longint = -2;

  MOPEN_CREATE = 0;
  MOPEN_READ = 1;
  MOPEN_WRITE = 2;
  MOPEN_RW = 3;

  MSGAPI_VERSION = 1;
  SMAPI_VERSION = '1.5.65.65.65.65.6';

  MSGPRIVATE = $00001;
  MSGCRASH = $00002;
  MSGREAD = $00004;
  MSGSENT = $00008;
  MSGFILE = $00010;
  MSGFWD = $00020;  // also called intransit flag
  MSGORPHAN = $00040;
  MSGKILL = $00080;
  MSGLOCAL = $00100;
  MSGHOLD = $00200;
  MSGXX2 = $00400;
  MSGFRQ = $00800;
  MSGRRQ = $01000;
  MSGCPT = $02000;
  MSGARQ = $04000;
  MSGURQ = $08000;
  MSGSCANNED = $10000;
  MSGLOCKED: dword = $40000000;     // seems to be a feature of GoldEd

  XMSG_NAME_SIZE = 36;
  XMSG_FROM_SIZE = XMSG_NAME_SIZE;
  XMSG_TO_SIZE = XMSG_NAME_SIZE;
  XMSG_SUBJ_SIZE = 72;
  MAX_REPLY = 9;
  FTS0001_DATE_SIZE = 20;

  XMSG_SIZE = ((94 + XMSG_FROM_SIZE) + XMSG_TO_SIZE) + XMSG_SUBJ_SIZE;
  MSGAPI_ID: dword = $0201414D;

  MSGH_ID = $0302484D;

  UID_EXACT = $00;
  UID_NEXT = $01;
  UID_PREV = $02;

// error constants

  MERR_NONE = 0; // No error
  MERR_BADH = 1; // Invalid handle passed to function
  MERR_BADF = 2; // Invalid or corrupted file
  MERR_NOMEM = 3; // Not enough memory for specified operation
  MERR_NODS = 4; // Maybe not enough disk space for operation
  MERR_NOENT = 5; // File/message does not exist
  MERR_BADA = 6; // Bad argument passed to msgapi function
  MERR_EOPEN = 7; // Couldn't close - messages still open

  BROWSE_ACUR = $0001;
  BROWSE_ATAG = $0002;
  BROWSE_AALL = $0004;
  BROWSE_ALL = $0008;
  BROWSE_NEW = $0010;
  BROWSE_SEARCH = $0020;
  BROWSE_FROM = $0040;
  BROWSE_READ = $0100;
  BROWSE_LIST = $0200;
  BROWSE_QWK = $0400;
  BROWSE_GETTXT = $0800;
  BROWSE_EXACT = $1000;
  BROWSE_HASH = $2000;
  BROWSE_AREA = (BROWSE_ACUR or BROWSE_ATAG) or BROWSE_AALL;
  BROWSE_TYPE = ((BROWSE_ALL or BROWSE_NEW) or BROWSE_SEARCH) or BROWSE_FROM;
  BROWSE_DISPLAY = (BROWSE_READ or BROWSE_LIST) or BROWSE_QWK;
  SF_HAS_ATTR = $01;
  SF_NOT_ATTR = $02;
  SF_OR = $04;
  SF_AND = $08;
  WHERE_TO = $01;
  WHERE_FROM = $02;
  WHERE_SUBJ = $04;
  WHERE_BODY = $08;
  WHERE_ALL = ((WHERE_TO or WHERE_FROM) or WHERE_SUBJ) or WHERE_BODY;
  SCAN_BLOCK_SDM = 48;
  SCAN_BLOCK_SQUISH = 512;

type
  PAreaHandle = ^TAreaHandle;
  PMsgHandle = ^TMsgHandle;
  PXMSG = ^TXMSG;
  PNETADDR = ^TNETADDR;
  PSEARCH = ^TSEARCH;
  PBROWSE = ^TBROWSE;

  TUMsgId = dword;

  { Timestamp }
  TStamp = packed record
// This was true for the original Squish MsgAPI, but smapi is double word aligned
//    case Boolean of
//      True: (Date, Time: sword);
//      False: (Value: dword);
    Date, Filler1, Time, Filler2: sword;
//    Date, Time: dword;
  end;

  PMINF = ^TMINF;
  TMINF = packed record
    req_version: sword;
    def_zone: sword;
    haveshare: sword;

    // necessary for Versions >= 1 always set to NIL
    palloc: pointer;
    pfree: pointer;
    repalloc: pointer;
    farpalloc: pointer;
    farpfree: pointer;
    farrepalloc: pointer;
  end;

  TNetAddr = packed record
    zone: sword;
    net: sword;
    node: sword;
    point: sword;
  end;

  TMsgName = packed array[0..XMSG_NAME_SIZE - 1] of char;
  TMsgSubj = packed array[0..XMSG_SUBJ_SIZE - 1] of Char;
//  TMsgSubjStr = string[XMSG_SUBJ_SIZE];

  TXMSG = packed record
    attr: dword;
    from: TMsgName;
    _to: TMsgName;
    subj: TMsgSubj;
    orig: TNetAddr;
    dest: TNetAddr;
    date_written: TStamp;
    date_arrived: TStamp;
    utc_ofs: sword;
    filler1: sword; // dword align
    replyto: TUMsgId;
    Replies: array[0..MAX_REPLY - 1] of TUMsgId;
    filler2: sword; // dword align
    FtscData: array[0..FTS0001_DATE_SIZE - 1] of Char;
  end;

  TApiFuncs = packed record
    CloseArea: function(mh: PAreaHandle): sword; cdecl;
    OpenMsg: function(mh: PAreaHandle; mode: sword; n: dword): PMsgHandle; cdecl;
    CloseMsg: function(msgh: PMsgHandle): sword; cdecl;
    ReadMsg: function(msgh: PMsgHandle; msg: PXMSG; ofs: dword; bytes: dword; text: PChar; cbyt: dword; ctxt: PChar): dword; cdecl;
    WriteMsg: function(msgh: PMsgHandle; append: sword; msg: PXMSG; text: PChar; textlen: dword; totlen: dword; clen: dword; ctxt: PChar): sword; cdecl;
    KillMsg: function(mh: PAreaHandle; msgnum: dword): sword; cdecl;
    Lock: function(mh: PAreaHandle): sword; cdecl;
    UnLock: function(mh: PAreaHandle): sword; cdecl;
    SetCurPos: function(msgh: PMsgHandle; pos: dword): sword; cdecl;
    GetCurPos: function(msgh: PMsgHandle): dword; cdecl;
    MsgnToUid: function(mh: PAreaHandle; msgnum: dword): TUMsgId; cdecl;
    UidToMsgn: function(mh: PAreaHandle; UMsgId: TUMsgId; _type: sword): dword; cdecl;
    GetHighWater: function(mh: PAreaHandle): dword; cdecl;
    SetHighWater: function(mh: PAreaHandle; hwm: dword): sword; cdecl;
    GetTextLen: function(msgh: PMsgHandle): dword; cdecl;
    GetCtrlLen: function(msgh: PMsgHandle): dword; cdecl;
  end;
  PApiFuncs = ^TApiFuncs;

  TMSGAPI = packed record
    id: dword;
    len: sword;
    _type: sword;
    num_msg: dword;
    cur_msg: dword;
    high_msg: dword;
    high_water: dword;
    sz_xmsg: sword;
    locked: byte;
    isecho: byte;
    api: PApiFuncs;
    apidata: pointer; // Pointer to application-specific data.
                      // API_SQ.C and API_SDM.C use this for
                      // different things, so again, no applications
                      // should muck with anything in here.
  end;

  TAreaHandle = TMSGAPI;

  TMsgHandle = packed record
    sq: PAreaHandle;
    id: dword;
    bytes_written: dword;
    cur_pos: dword;
  end;

  TSEARCH = packed record
    next: PSEARCH;
    attr: longint;
    flag: longint;
    txt: Pchar;
    where: char;
  end;

  TBROWSE = packed record
    path: Pchar;
    _type: sword;
    bflag: sword;
    bdata: dword;
    first: PSEARCH;
    nonstop: Pchar;
    msgn: dword;
    sq: PAreaHandle;
    m: PMsgHandle;
    msg: TXMSG;
    matched: sword;
    Begin_Ptr: function(b: pBROWSE): longint; cdecl;
    Status_Ptr: function(b: pBROWSE; aname: pchar; colour: longint): longint;
    Idle_Ptr: function(b: pBROWSE): longint;
    Display_Ptr: function(b: pBROWSE): longint;
    After_Ptr: function(b: pBROWSE): longint;
    End_Ptr: function(b: pBROWSE): longint;
    Match_Ptr: function(b: pBROWSE): longint;
  end;

//Var
//  msgapierr: sword; cvar; external;
//  mi: minf; cvar; external;

{: Helper function. Since Delphi/Kylix does not support external variables,
   this function has been introduced. It reads the smapi error variable
   msgapierr and returns its value }
function MsgGetMsgApiErr: sword; cdecl;

{: Closes a message area.
   This function performs all clean-up actions necessary, such as closing files,
   changing directories, and so on. The MsgCloseArea function should be
   called for each area opened by MsgOpenArea.
   @param AreaHandle is an HAREA handle returnded by MsgOpenArea.
   @returns 0 if the area was successfully closed, otherwise it returns -1
            and msgapierr is set to one of the following:
            MERR_BADH    An invalid handle was passed to the function.
            MERR_EOPEN   Messages are still open in this area, so the area
                         could not be closed. }
function MsgCloseArea(_AreaHandle: PAreaHandle): sword; cdecl;

{: Opens a message for access, and it must be used to read from or write to
   a given message.
   @param AreaHandle is a message area handle, as returned by the MsgOpenArea
          function.
   @param Mode is an access flag, containing one of the following manifest
          constants:
          MOPEN_CREATE  Create a new message. This mode should only
                        be used for creating new messages.
          MOPEN_READ    Open an existing message for reading ONLY.
          MOPEN_WRITE   Open an existing message for writing ONLY.
          MOPEN_RW      Open an existing message for reading AND writing.
   @param MsgNum is the specified message number to open. If mode is either
          MOPEN_READ, MOPEN_WRITE or MOPEN_RW, the message number must
          currently exist in the specified area. If mode is  set to
          MOPEN_CREATE, a value of 0 for 'msgn' indicates that a new
          message should be created, and assigned a number one higher
          than the current highest message. If MsgN is non-zero, but
          MOPEN_CREATE is set to the number of a currently-existing
          message, the specified message will be truncated and the
          new message will take its place.
          For MOPEN_READ or MOPEN_RW, the following constants can also
          be passed in place of 'msgn':
          MSGNUM_CUR  Open the last message which was accessed by MsgOpenMsg.
          MSGNUM_PREV Open the message prior to the last message accessed
                      by MsgOpenMsg.
          MSGNUM_NEXT Open the message after the last message accessed by
                      MsgOpenMsg.
          The MsgAPI maintains the number of the last message opened by
          MsgOpenMsg, which is used  when processing these constants.
          (See also MsgGetCurMsg.)
    @returns a HMSG handle ff the message was successfully opened. Otherwise,
            a value of NULL is returned, and  msgapierr will be set to one of
            the following:
             MERR_NOENT
             MERR_NOMEM
             MERR_BADF
             MERR_BADA
             MERR_BADH }
function MsgOpenMsg(_AreaHandle: PAreaHandle; _Mode: sword; _MsgNum: dword): PMsgHandle; cdecl;

{: Closes a message which has been previously opened by MsgOpenMsg.
   All messages should be closed after use, or else data loss may result.
   @params MsgHandle is the message handle that was returned by MsgOpenMsg.
   @returns 0, if the message was successfully  closed, otherwise, -1 is
            returned, and msgapierr is set to:
            MERR_BADH }
function MsgCloseMsg(_MsgHandle: PMsgHandle): sword; cdecl;

{: Reads a message from disk.
   This function can be used to read all parts of a message, including the
   message header, message body, and control information.

  @param MsgHandle is a message handle, as returned by the MsgOpenMsg function.
         The message in question must have been opened with a mode of either
         MOPEN_READ or MOPEN_RW.

  @param XMsg is a pointer to an TXMSG (extended message) structure.
         The format of this structure is detailed in the Squish File Format
         Specification section, but it contains all of the message information
         that is found in the message header, including the to/from/subject
         fields, origination and arrival dates, 4D origination and destination
         addresses, and so forth. (See the appendices for specific information
         on the XMSG structure itself.)
         If the application wishes to read the header of a given message, this
         argument should point to an XMSG structure. Otherwise, this argument
         should be NULL, which informs the API that the message header does
         not need to be read.

  @param Offset is used for reading message text in a multiple-pass environment.
         This defines the offset in the message body from which the API should
         start reading. To start reading from the beginning of the message,
         a value of 0L should be given. Otherwise, the offset into the message
         (in bytes) should be given for this argument. If the application does
         not wish to read the message body, this argument should be set to 0L.

  @param BodyLen represents the maximum number of bytes to read from the message.
         Fewer bytes may be read, but the API will read no more than BodyLen
         during this call. (See BodyLen, and also this function's return value.)
         If the application does not wish to read  the message body, this
         argument should be set to 0L.

  @param Body is a pointer to a block of memory, into which the API will place
         the message body. The message body will be read from the position
         specified by Offset, up to a maximum of BodyLen bytes. If the
         application does not wish to read the message body, this argument
         should be set to NULL.

  @param ControlLen represents the maximum number of bytes of control
         information to read from the message.

  @param Control is a pointer to a block of memory, into which the API will
         place the message control information. No more than ControlLne bytes
         of control information will be placed into the buffer.
         NOTE: unlike the message text functions, control information can only
               be read in one pass.

  The text read by this function is free-form. The message body may or may not
  contain control characters, NULs, or any other sequence of characters.
  Messages are simply treated as a block of bytes, with no interpretation
  whatsoever.

  In FidoNet areas, the  message body consists of one  or more paragraphs of
  text. Each paragraph is delimited by a hard carriage return, '\r', or
  ASCII 13. Each paragraph can be of any length, so the text should be
  wordwrapped onto logical lines before being displayed. If created by older
  applications, paragraphs  may also contain  linefeeds ('\n') and soft returns
  ('\x8d') at the end of each line, but these are optional and should always be
  ignored.

  As an example, assume that the following stream of text was returned by
  MsgReadMsg():

  "Hi!\r\rHow's it going? I got the new MsgAPI kit today!\r\rAnyhow, gotta run!"

  The "\r" marks are carriage returns, so they indicate the end of a paragraph.
  Notice that the second paragraph is fairly long, so it might have to be
  wordwrapped, depending on the screen width. Your application might wordwrap
  the text to make it look like this, if using a window 40 characters wide:

  -----------------------------------------
  | Hi!                                   |
  |                                       |
  | How's it going?  I got the new MsgAPI |
  | kit today!                            |
  |                                       |
  | Anyhow, gotta run!                    |
  -----------------------------------------

  Paragraphs should always be wordwrapped by the application, regardless
  of the screen/window size. When parsing the message text, linefeeds and
  soft carriage returns should be simply skipped.

  The 'message control information' has a somewhat more restricted format.
  The control information is passed to the application in the form of an
  ASCIIZ string. The control information is a variable-length string of text
  which contains information not found in the (fixed-size) message header.

  The format of control information is given by  the following regular
  expression:

  (group)+<NUL>

  A 'group' consists of a <SOH> and a control item.

  <SOH> is the Start Of Header character, or ASCII 01. All control information
  strings must begin with an SOH, whether or not control items are present.

  Following the <SOH> is a control item. A control item consists of a string
  which describes the type of control item, or it may consist of nothing.

  At least one group must be present in each message. If a message has no extra
  control information, this field should consists of a <SOH> followed by a
  single <NUL>.

  Although  the control  items  are free-form,  the  following format is
  suggested:

  <SOH>tag: value

  where 'tag' is a descriptive identifier, describing the type of field that
  the item represents. 'value' is simply free-form text, which continues up
  until the next SOH or <NUL>.

  The character set for the tag and value consists of those characters in the
  range 2-255, inclusive.

  As an example, a message might have the following control information:

  <SOH>CHARSET: LATIN1<SOH>REALNAME: Mark Twain<NUL>

  The trailing <NUL> byte must be included in the read count given by 'cbyt'.

  @returns the number of bytes read from the message body. If no characters
           were requested, this function returns 0.
           On error, the function returns -1 and sets msgapierr to one
           of the following:
           MERR_BADH
           MERR_BADF
           MERR_NOMEM }
function MsgReadMsg(_MsgHandle: PMsgHandle; _XMsg: PXMSG; _Offset: dword;
  _BodyLen: dword; _Body: PChar; _ControlLen: dword; _Control: PChar): integer; cdecl;

{: Writes the message header, body, and control information to a message.
   @param MsgHandle is a message handle, as returned by the MsgOpenMsg
          function. The message must have been opened with a mode of
          MOPEN_CREATE, MOPEN_WRITE or MOPEN_RW.

  @param Append is a boolean flag, indicating the state of the message body.
         If Append is zero, then the API will write the message body starting
         at offset zero. Otherwise, if Append is non-zero, the API will
         continue writing from the offset used by the last MsgWriteMsg call.
         This flag applies to the message body only; if no text is to be
         written to the body, this argument should be set to 0.

  @param XMsg is a pointer to an XMSG structure. If this pointer is non-NULL,
         then MsgWriteMsg will place the XMSG structure information into the
         message's physical header. To leave the header unmodified, NULL should
         be passed for XMsg.
         THIS PARAMETER MUST BE PASSED THE **FIRST**  TIME THAT MSGWRITEMSG()
         IS USED WITH A JUST-OPENED MESSAGE HANDLE!

  @param Body points to an array of bytes to be written to the message body.
         If no text is to be written, this argument should be NULL.

  @param Bodylen indicates the number of bytes to be written to the message
         body in this pass of the MsgWriteMsg function. The text is free-format,
         and it can consist of any characters, including NULs and control
         characters. If the application does not wish to update the message
         body, a value of 0L should be passed for this argument.

  @param Total indicates the total length of the message to be written. This
         differs from Bodylen in that the message may be written a piece at a
         time (using small Bodylen values), but the total length of the message
         will not exceed Total. This parameter can be somewhat restrictive for
         the application; however, this value is required for optimal use of
         some message base types. The Total value does not have to be the exact
         length of the message to write; however, space may be wasted if this
         value is not reasonably close to the actual length of the message.
         The rationale behind this argument is that it gives the API writer
         the most flexibility, in terms of supporting future message base
         formats. If the application can provide this information to the API,
         then almost any message base format can be supported by simply dropping
         in a new API module or DLL.
         To write text by making multiple passes, the FIRST pass should call
         MsgWriteMsg with Append set to 0, with the total length of the message
         in Total, and the length of Body in  Bodylen. Second and subsequent
         passes should set Append to 1, with the length of Body in Bodylen.
         If the application does not wish to update the message body of an
         existing message, a value of 0L should  be passed for this argument.
         This argument MUST be specified during the first call to the
         MsgWriteMsg when using a mode of MOPEN_CREATE, even if the first
         call is not requesting any text to be written.
         However, this value will be stored internally, and ignored on the
         second and later calls.
         When operating on a preexisting message (opened with MOPEN_WRITE or
         MOPEN_RW), it is an error to specify a length in 'totlen' which is
         greater than the original length of the message.

  @param ControlLen specifies the total length of the control information,
         including the trailing NUL byte. To write no control information,
         a value of 0L should be passed for this argument.

  @param Control is a pointer to the control information string. To write no
         control information, a value of 0L should be passed for this argument.
         N.B. Several restrictions apply to writing control information:
         First and foremost, control information can only be written once.
         If the control information is to be changed, the message must be read
         and copied to another message.
         Secondly, control information MUST be written during or before
         MsgWriteMsg is called with information about the message body.

  @returns a value of 0 on success, or -1 on error. If an error occurred,
           msgapierr will be set  to one of  the following values:
           MERR_BADH
           MERR_BADF
           MERR_NOMEM
           MERR_NODS }
function MsgWriteMsg(_MsgHandle: PMsgHandle; _Append: sword; _XMsg: PXMSG;
  _Body: PChar; _Bodylen: dword; _Total: dword; _ControlLen: dword;
  _Control: PChar): sword; cdecl;


{: Deletes a message from the specified message area.

   @param AreaHandle is a message area handle, as returned by MsgOpenArea.

   @param MsgNum specifies the message number to kill.

   It is an error to kill a message which is currently open.

   @returns a value of 0 if the message was successfully killed, or it
            returns -1 on error and sets msgapierr to one of the following:
            MERR_BADH
            MERR_NOENT
            MERR_BADF
            MERR_NOMEM }
function MsgKillMsg(_AreaHandle: PAreaHandle; _MsgNum: dword): sword; cdecl;

function MsgLock(mh: PAreaHandle): sword; cdecl;
function MsgUnLock(mh: PAreaHandle): sword; cdecl;

{: Sets the 'current position' in a message handle. This position is used by
   MsgReadMsg to read text from the message body.

  @param MsgHandle is a message handle, as returned by MsgOpenMsg.

  @param Pos is the number of bytes into the message from which MsgReadMsg
         should start reading.

  @returns 0 on success, or -1 on error and sets msgapierr to:
           MERR_BADH }

function MsgSetCurPos(_MsgHandle: PMsgHandle; _Pos: dword): sword; cdecl;

{: Retrieves the 'current position' of a message handle. This position is
   where the MsgReadMsg would read text from the message body next.

   @param MsgHandle is a message handle, as returned by MsgOpenMsg.

   MsgGetCurPos returns the offset into the message on success, or (dword)-1 on
   error and sets msgapierr to:
   MERR_BADH }
function MsgGetCurPos(_MsgHandle: PMsgHandle): dword; cdecl;

{: Converts a message number to a 'unique message ID', or UMSGID. This function
   can be used to maintain pointers to an 'absolute' message number, regardless
   of whether or not the area is renumbered or packed. The MsgMsgnToUid
   function converts a message number to a UMSGID, and the MsgUidToMsgn
   function converts that UMSGID back to a message number.

   @param AreaHandle is the message area handle, as returned by MsgOpenArea.

   @param MsgNum is the message number to convert.

   @returns a UMSGID on success; otherwise, it returns 0 and sets msgapierr to:
            MERR_BADH
            MERR_BADF
            MERR_NOENT }
function MsgMsgnToUid(_AreaHandle: PAreaHandle; _MsgNum: dword): TUMsgId; cdecl;

{: Converts a UMSGID to a message number.

   @param AreaHandle is the message area handle, as returned by MsgOpenArea.

   @param UMsgId is the UMSGID, as returned by a prior call to MsgMsgnToUid.

   @param Type is  the type of conversion to perform. Type can be any of the
          following values:
          UID_EXACT      Return the message number represented by the UMSGID,
                         or 0 if the message no longer exists.
          UID_PREV       Return the message number represented by the UMSGID.
                         If the message no longer exists, the number of the
                         preceding message will be returned.

          UID_NEXT       Return the message number represented by the UMSGID.
                         If the message no longer exists, the number of the
                         following message will be returned.

   @returns the requested UMSGID. If no valid message could be found,
            MsgUidToMsgn returns 0 and sets msgapierr to one of the
            following:
            MERR_BADH
            MERR_NOENT }
function MsgUidToMsgn(_AreaHandle: PAreaHandle; _UMsgId: TUMsgId; _Type: sword): dword; cdecl;

{: Returns the 'high water marker' for an area.
   This number represents the highest message number that was processed by a
   message export or import utility. The high water marker is automatically
   adjusted when messages are killed.
   @param AreaHandle is a message area handle, as returned by MsgOpenArea.
   @returns the high water mark on success, or 0 on error and
            sets msgapierr to: MERR_BADH }
function MsgGetHighWater(_AreaHandle: PAreaHandle): dword; cdecl;

{: Sets the 'high water marker' for the current area.

   @param AreaHandle is a message area handle, as returned by MsgOpenArea.

   @param HighWater is the new high water marker to use for the specified area.

   @returns 0 on success, or -1 on error and sets msgapierr to:
            MERR_BADH }
function MsgSetHighWater(_AreaHandle: PAreaHandle; _HighWater: dword): sword; cdecl;

{: Retrieves the length of the message body for the specified message.

   @param MsgHandle is a message handle, as returned by MsgOpenMsg.

   @returns the length of the body on success. On error, it returns (dword)-1
            and sets msgapierr to:
            MERR_BADHT }
function MsgGetTextLen(_MsgHandle: PMsgHandle): dword; cdecl;

{: Retrieves the length of the control information for the specified message.

   @param MsgHandle is a message handle, as returned by MsgOpenMsg.

   @returns the length of the control information on success. On error, it
            returns (dword)-1 and sets msgapierr to:
            MERR_BADH }
function MsgGetCtrlLen(_MsgHandle: PMsgHandle): dword; cdecl;

{: Initializes the MsgAPI.
   This function must be called before any of the other API functions are
   called, or else the results are undefined.
   This function serves to initialize any needed structures, and to prepare
   the message bases for use.
   @param MInf is a structure contining the following fields:

   record TMInf
     // The following fields are required for all
     // MsgAPI clients:

     word req_version;
     word def_zone;
     word haveshare;

     // The following fields are required when
     // req_version >= 1:
     void OS2FAR * (MAPIENTRY *palloc)(size_t size);
     void (MAPIENTRY *pfree)(void OS2FAR *ptr);
     void OS2FAR * (MAPIENTRY *repalloc)(void OS2FAR *ptr,
                                         size_t size);

     void far * (MAPIENTRY *farpalloc)(size_t size);
     void (MAPIENTRY *farpfree)(void far *ptr);
     void far * (MAPIENTRY *farrepalloc)(void far *ptr,
                                            size_t size);
   end;

   'req_version' indicates the MsgAPI revision level that the application is
                 requesting. The compile-time revision level can always be
                 accessed using the constant 'MSGAPI_VERSION'.

   'def_zone'    should contain a default FidoNet zone number.
                 Certain message systems, such as the FTSC-0001 *.MSG format,
                 do not store zone information with each message. When the
                 API encounters such a message and no zone is present, the
                 specified zone will be used instead. A 'def_zone' of 0
                 indicates that nothing is to be inferred about the zone
                 number of a message, and in that case, the API functions
                 will return 0 as the zone number for any message with an
                 unknown zone.

   'haveshare'   is automatically filled in by the internal API routines, and
                 this flag indicates whether or not the DOS "SHARE.EXE" program
                 is currently loaded.
                 Note that SHARE must always be loaded to access Squish-format
                 bases in a multitasking or network environment. This field is
                 not used in the OS/2 version of the MsgAPI.

   If 'req_version' is more than or equal to 1, the final six fields in the
   _minf structure must be provided. These fields are memory allocation hooks
   that MsgAPI will call whenever it needs to allocate memory.
   (If req_version is 0, or if one of the function pointers in this structure
   is NULL, then MsgAPI will use its own memory allocation routines.)

   'palloc'      is called to allocate a block of near memory. This function
                 should behave in the same manner as the ANSI malloc()
                 function. If this field is NULL, the MsgAPI will use its own
                 malloc() function.

   'pfree'       is called to free a block of near memory. This function should
                 behave in the same manner as the ANSI free() function. If this
                 field is NULL, the MsgAPI will use its own free() function.

   'repalloc'    is called to reallocate a block of near memory. This function
                 should behave in the same manner as the ANSI realloc()
                 function. If this field is NULL, the MsgAPI will use its own
                 realloc() function.

   'farpalloc'   is called to allocate a block of far memory. This function
                 should behave in the same manner as the ANSI malloc()
                 function, except that a far pointer should be returned.
                 If this field is NULL, MsgAPI will use its own malloc()
                 function.

   'farpfree'    is called to free a block of far memory. This function should
                 behave in the same manner as the ANSI free() function, except
                 that a far pointer should be accepted. If this field is NULL,
                 MsgAPI will use its own free() function.

   'farrepalloc' is called to reallocate a block of far memory. This function
                 should behave in the same manner as the ANSI realloc()
                 function, except that a far pointer should be accepted and
                 returned. If this field is NULL, MsgAPI will use its own
                 realloc() function.

   @returns a value of 0 if the initialization was performed successfully,
            and -1 if a problem was encountered. }
function MsgOpenApi(_MInf: PMINF): sword; cdecl;

{: Deinitialize theMsgAPI. This function performs any clean-up actions which
   may be necessary, including the closing of files and releasing allocated
   memory. This function should be called before the application terminates.

   @returns a value of 0 if the API was successfully deinitialized,
            and -1 otherwise. }
function MsgCloseApi: sword; cdecl;

{: Opens or creates a  message area.
   @param Name is the name of the message area to open. The contents of this
          string are implementation-defined. (See Type for more information.)

   @param Mode is the mode with which the area should be opened. Values for
          Mmode are as follows:

          MSGAREA_NORMAL   Open the message area in a normal access mode. If
                           the area does not exist, this function fails.

          MSGAREA_CRIFNEC  Open the message area in a normal access mode.
                           If the area does not exist, the MsgAPI attempts
                           to create the area. If the area cannot be created,
                           this function fails.

          MSGAREA_CREATE   Create the message area. If the area already exists,
                           it is truncated or started anew with no messages.
                           If the area cannot be created, this function fails.

  @param Type specifies the type of message area to open. Type can have any of
         the following values:

         MSGTYPE_SDM       Star Dot MSG (SDM). This specifies a FTSC-0001
                           compatible access mode, and it instructs the MsgAPI
                           to create and read Fido-compatible messages for
                           this area. If MSGTYPE_SDM is specified, Name should
                           contain the path to the *.MSG directory.

         MSGTYPE_SQUISH    Squish (*.SQ?) format. This specifies that the
                           proprietary Squish message format is to be used
                           for this area. Name should give the path and root
                           name (eight characters in DOS) for the message area.

         In addition, if the mask 'MSGTYPE_ECHO' is bitwise 'OR'ed with the '
         'MSGTYPE_SDM' value, the area in question will be treated as a
         FidoNet-style echomail area. This instructs the MsgAPI to keep
         high-water mark information in the 1.MSG file, and to stop the normal
         MsgAPI functions from writing to the first message in each area.
         Other message formats have a cleaner way of storing the high-water
         mark, so this mask is only required for *.MSG areas.

         Other values for Type' are currently reserved.

  @returns a HAREA handle on success. This handle does not contain any
           information which can be used directly by the caller; all
           interaction should be performed through the MsgAPI functions only.

           If this function fails, NULL is returned, and the global MsgApiErr
           variable is set to one of the following values:
           MERR_NOMEM     Not enough memory for requested task
           MERR_NOENT     The area did not exist or could not be created.
           MERR_BADF      The message area is structurally damaged. }
function MsgOpenArea(_Name: PChar; _Mode: sword; _Type: sword): PAreaHandle; cdecl;


{: The  MsgValidate function validates a particular message area, determining
   whether or not the area exists and if it is valid.
   @param Type is the type of the message area, using the same constants as
               specified for MsgOpenArea.
   @param Name is the name of the message area, using the same format as
               specified for MsgOpenArea.
   @returns the value 1 if the area exists and is valid. 0 otherwise. }
function MsgValidate(_Type: sword; _Name: PChar): sword; cdecl;

{: This function is not part of the original MsgApi and therefore I don't have
   any clue what it does. }
function MsgBrowseArea(b: PBROWSE): sword; cdecl;

function MsgCurMsg(_AreaHandle: PAreaHandle): integer;
function MsgNumMsg(_AreaHandle: PAreaHandle): integer;
function MsgHighMsg(_AreaHandle: PAreaHandle): integer;
function MsgGetCurMsg(_AreaHandle: PAreaHandle): integer;
function MsgGetNumMsg(_AreaHandle: PAreaHandle): integer;
function MsgGetHighMsg(_AreaHandle: PAreaHandle): integer;

function InvalidMsgh(msgh: PMsgHandle): sword; cdecl;
function InvalidMh(mh: PAreaHandle): sword; cdecl;

procedure SquishSetMaxMsg(sq: PAreaHandle; max_msgs: dword; skip_msgs: dword; age: dword); cdecl;
function SquishHash(f: PChar): dword; cdecl;

function SdmOpenArea(name: PChar; mode: sword; _type: sword): PAreaHandle; cdecl;
function SdmValidate(name: PChar): sword; cdecl;

function SquishOpenArea(name: PChar; mode: sword; _type: sword): PAreaHandle; cdecl;
function SquishValidate(name: PChar): sword; cdecl;

function JamOpenArea(name: PChar; mode: sword; _type: sword): PAreaHandle; cdecl;
function JamValidate(name: PChar): sword; cdecl;

function CvtCtrlToKludge(ctrl: PChar): PChar; cdecl;
function GetCtrlToken(where: PChar; what: PChar): PChar; cdecl;
function CopyToControlBuf(txt: PChar; newtext: PPChar; length: PLongInt): PChar; cdecl;
procedure ConvertControlInfo(ctrl: PChar; orig: PNETADDR; dest: PNETADDR); cdecl;
function NumKludges(txt: PChar): sword; cdecl;
procedure RemoveFromCtrl(ctrl: PChar; what: PChar); cdecl;

function Address(a: PNETADDR): PChar; cdecl;
function StripNasties(str: PChar): PChar; cdecl;

implementation

const LIBSMAPI_NAME = 'libsmapi.so';

function MsgCloseArea(_AreaHandle: PAreaHandle): sword; cdecl;
begin
  MsgCloseArea := _AreaHandle^.api^.CloseArea(_AreaHandle);
end;

function MsgOpenMsg(_AreaHandle: PAreaHandle; _Mode: sword; _MsgNum: dword): PMsgHandle; cdecl;
begin
  MsgOpenMsg := _AreaHandle^.api^.OpenMsg(_AreaHandle, _Mode, _MsgNum);
end;

function MsgCloseMsg(_MsgHandle: PMsgHandle): sword; cdecl;
begin
  MsgCloseMsg := _MsgHandle^.sq^.api^.CloseMsg(_MsgHandle);
end;

function MsgReadMsg(_MsgHandle: PMsgHandle; _XMsg: PXMSG; _Offset: dword;
  _BodyLen: dword; _Body: PChar; _ControlLen: dword; _Control: PChar): integer; cdecl;
begin
  MsgReadMsg := _MsgHandle^.sq^.api^.ReadMsg(_MsgHandle, _XMsg, _Offset,
    _BodyLen, _Body, _ControlLen, _Control);
end;

function MsgWriteMsg(_MsgHandle: PMsgHandle; _Append: sword; _Xmsg: PXMSG;
  _Body: PChar; _Bodylen: dword; _Total: dword; _ControlLen: dword; _Control: PChar): sword; cdecl;
begin
  MsgWriteMsg := _MsgHandle^.sq^.api^.WriteMsg(_MsgHandle, _Append, _Xmsg,
    _Body, _Bodylen, _Total, _ControlLen, _Control);
end;

function MsgKillMsg(_AreaHandle: PAreaHandle; _MsgNum: dword): sword; cdecl;
begin
  MsgKillMsg := _AreaHandle^.api^.KillMsg(_AreaHandle, _MsgNum);
end;

function MsgLock(mh: PAreaHandle): sword; cdecl;
begin
  MsgLock := mh^.api^.Lock(mh);
end;

function MsgUnLock(mh: PAreaHandle): sword; cdecl;
begin
  MsgUnLock := mh^.api^.UnLock(mh);
end;

function MsgSetCurPos(_MsgHandle: PMsgHandle; _Pos: dword): sword; cdecl;
begin
  MsgSetCurPos := _MsgHandle^.sq^.api^.SetCurPos(_MsgHandle, _Pos);
end;

function MsgGetCurPos(_MsgHandle: PMsgHandle): dword; cdecl;
begin
  MsgGetCurPos := _MsgHandle^.sq^.api^.GetCurPos(_MsgHandle);
end;

function MsgMsgnToUid(_AreaHandle: PAreaHandle; _MsgNum: dword): TUMsgId; cdecl;
begin
  MsgMsgnToUid := _AreaHandle^.api^.MsgnToUid(_AreaHandle, _MsgNum);
end;

function MsgUidToMsgn(_AreaHandle: PAreaHandle; _UMsgId: TUMsgId;
  _Type: sword): dword; cdecl;
begin
  MsgUidToMsgn := _AreaHandle^.api^.UidToMsgn(_AreaHandle, _UMsgId, _Type);
end;

function MsgGetHighWater(_AreaHandle: PAreaHandle): dword; cdecl;
begin
  MsgGetHighWater := _AreaHandle^.api^.GetHighWater(_AreaHandle);
end;

function MsgSetHighWater(_AreaHandle: PAreaHandle; _HighWater: dword): sword; cdecl;
begin
  MsgSetHighWater := _AreaHandle^.api^.SetHighWater(_AreaHandle, _HighWater);
end;

function MsgGetTextLen(_MsgHandle: PMsgHandle): dword; cdecl;
begin
  MsgGetTextLen := _MsgHandle^.sq^.api^.GetTextLen(_MsgHandle);
end;

function MsgGetCtrlLen(_MsgHandle: PMsgHandle): dword; cdecl;
begin
  MsgGetCtrlLen := _MsgHandle^.sq^.api^.GetCtrlLen(_MsgHandle);
end;

// Thes functions aren't actually functions, they are just wrappers to
// access some fields in the AreaHandle in a controlled way.
// in C they are macros.

function MsgCurMsg(_AreaHandle: PAreaHandle): integer;
begin
  Result := _AreaHandle^.cur_msg;
end;

function MsgGetCurMsg(_AreaHandle: PAreaHandle): integer;
begin
  Result := _AreaHandle^.cur_msg;
end;

function MsgNumMsg(_AreaHandle: PAreaHandle): integer;
begin
  Result := _AreaHandle^.num_msg;
end;

function MsgGetNumMsg(_AreaHandle: PAreaHandle): integer;
begin
  Result := _AreaHandle^.num_msg;
end;

function MsgHighMsg(_AreaHandle: PAreaHandle): integer;
begin
  Result := _AreaHandle^.high_msg;
end;

function MsgGetHighMsg(_AreaHandle: PAreaHandle): integer;
begin
  Result := _AreaHandle^.high_msg;
end;

function MsgGetMsgApiErr: sword; cdecl; external LIBSMAPI_NAME name 'MsgGetMsgApiErr';

function MsgOpenApi(_MInf: PMINF): sword; cdecl; external LIBSMAPI_NAME name 'MsgOpenApi';

function MsgCloseApi: sword; cdecl; external LIBSMAPI_NAME name 'MsgCloseApi';

function MsgOpenArea(_Name: PChar; _Mode: sword; _Type: sword): PAreaHandle; cdecl; external LIBSMAPI_NAME name 'MsgOpenArea';

function MsgValidate(_Type: sword; _Name: PChar): sword; cdecl; external LIBSMAPI_NAME name 'MsgValidate';

function MsgBrowseArea(b: PBROWSE): sword; cdecl; external LIBSMAPI_NAME name 'MsgBrowseArea';

function InvalidMsgh(msgh: PMsgHandle): sword; cdecl; external LIBSMAPI_NAME name 'InvalidMsgh';

function InvalidMh(mh: PAreaHandle): sword; cdecl; external LIBSMAPI_NAME name 'InvalidMh';

procedure SquishSetMaxMsg(sq: PAreaHandle; max_msgs: dword; skip_msgs: dword; age: dword); cdecl; external LIBSMAPI_NAME name 'SquishSetMaxMsg';

function SquishHash(f: PChar): dword; cdecl; external LIBSMAPI_NAME name 'SquishHash';

function SdmOpenArea(name: PChar; mode: sword; _type: sword): PAreaHandle; cdecl; external LIBSMAPI_NAME name 'SdmOpenArea';

function SdmValidate(name: PChar): sword; cdecl; external LIBSMAPI_NAME name 'SdmValidate';

function SquishOpenArea(name: PChar; mode: sword; _type: sword): PAreaHandle; cdecl; external LIBSMAPI_NAME name 'SquishOpenArea';

function SquishValidate(name: PChar): sword; cdecl; external LIBSMAPI_NAME name 'SquishValidate';

function JamOpenArea(name: PChar; mode: sword; _type: sword): PAreaHandle; cdecl; external LIBSMAPI_NAME name 'JamOpenArea';

function JamValidate(name: PChar): sword; cdecl; external LIBSMAPI_NAME name 'JamValidate';

function CvtCtrlToKludge(ctrl: PChar): PChar; cdecl; external LIBSMAPI_NAME name 'CvtCtrlToKludge';

function GetCtrlToken(where: PChar; what: PChar): PChar; cdecl; external LIBSMAPI_NAME name 'GetCtrlToken';

function CopyToControlBuf(txt: PChar; newtext: PPChar; length: PLongInt): PChar; cdecl; external LIBSMAPI_NAME name 'CopyToControlBuf';

procedure ConvertControlInfo(ctrl: PChar; orig: PNETADDR; dest: PNETADDR); cdecl; external LIBSMAPI_NAME name 'ConvertControlInfo';

function NumKludges(txt: PChar): sword; cdecl; external LIBSMAPI_NAME name 'NumKludges';

procedure RemoveFromCtrl(ctrl: PChar; what: PChar); cdecl; external LIBSMAPI_NAME name 'RemoveFromCtrl';

function Address(a: PNETADDR): PChar; cdecl; external LIBSMAPI_NAME name 'Address';

function StripNasties(str: PChar): PChar; cdecl; external LIBSMAPI_NAME name 'StripNasties';

end.
