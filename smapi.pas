unit smapi;

interface

{ C default packing is dword }

{$PACKRECORDS 4}

  type
     sword = Integer;
     PPChar = ^PChar;
     PLongint = ^Longint;

  const
     MSGAREA_NORMAL = $00;
     MSGAREA_CREATE = $01;
     MSGAREA_CRIFNEC = $02;
     MSGTYPE_SDM = $01;
     MSGTYPE_SQUISH = $02;
     MSGTYPE_JAM = $08;
     MSGTYPE_ECHO = $80;
     MSGNUM_CUR : longint = -1;
     MSGNUM_PREV : longint = -2;
     MSGNUM_NEXT : longint = -3;
     MSGNUM_current : longint = -1;
     MSGNUM_previous : longint = -2;

     MOPEN_CREATE = 0;
     MOPEN_READ = 1;
     MOPEN_WRITE = 2;
     MOPEN_RW = 3;

     MSGAPI_VERSION = 1;
     SMAPI_VERSION = '1.5.65.65.65.65.6';

     MSGPRIVATE = $00001;
     MSGCRASH   = $00002;
     MSGREAD    = $00004;
     MSGSENT    = $00008;
     MSGFILE    = $00010;
     MSGFWD     = $00020;
     MSGORPHAN  = $00040;
     MSGKILL    = $00080;
     MSGLOCAL   = $00100;
     MSGHOLD    = $00200;
     MSGXX2     = $00400;
     MSGFRQ     = $00800;
     MSGRRQ     = $01000;
     MSGCPT     = $02000;
     MSGARQ     = $04000;
     MSGURQ     = $08000;
     MSGSCANNED = $10000;
     MSGLOCKED : dword = $40000000;
     XMSG_FROM_SIZE = 36;
     XMSG_TO_SIZE = 36;
     XMSG_SUBJ_SIZE = 72;
     MAX_REPLY  = 10;
     
     XMSG_SIZE = ((94 + XMSG_FROM_SIZE) + XMSG_TO_SIZE) + XMSG_SUBJ_SIZE;
     MSGAPI_ID : dword = $0201414D;

     MSGH_ID = $0302484D;

     UID_EXACT = $00;
     UID_NEXT = $01;
     UID_PREV = $02;
     MERR_NONE = 0;
     MERR_BADH = 1;
     MERR_BADF = 2;
     MERR_NOMEM = 3;
     MERR_NODS = 4;
     MERR_NOENT = 5;
     MERR_BADA = 6;
     MERR_EOPEN = 7;
       
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
     PMSG = ^MSG;
     PMSGH = ^MSGH;
     PXMSG = ^XMSG;
     PNETADDR = ^NETADDR;
     PSEARCH = ^SEARCH;
     PBROWSE = ^BROWSE;

     UMSGID = dword;

     _stamp = record
          date: word;
          time: word;
       end;

     PMINF = ^MINF;
     MINF = record
          req_version : word;
          def_zone : word;
          haveshare : word;
       end;

     NETADDR = record
          zone : word;
          net : word;
          node : word;
          point : word;
       end;


     XMSG = record
          attr: dword;
          from: PChar;
          _to: PChar;
          subj: PChar;
          orig: NETADDR;
          dest: NETADDR;
          date_written: _stamp;
          date_arrived: _stamp;
          utc_ofs: integer;
          replyto: UMSGID;
          replies: ^UMSGID;   {MAX_REPLY entries}
          __ftsc_date: PChar; {20 bytes}
       end;

    _apifuncs = record
        CloseArea: function (mh: PMSG): sword; cdecl;
        OpenMsg: function (mh: PMSG; mode: word; n: dword): PMSGH; cdecl;
        CloseMsg: function (msgh: PMSGH): sword; cdecl;
        ReadMsg: function (msgh: PMSGH; msg: PXMSG; ofs: dword; bytes: dword; text: PChar; cbyt: dword; ctxt: PChar): dword; cdecl;
        WriteMsg: function (msgh: PMSGH; append: word; msg: PXMSG; text: PChar; textlen: dword; totlen: dword; clen: dword; ctxt: PChar): sword; cdecl;
        KillMsg: function (mh: PMSG; msgnum: dword): sword; cdecl;
        Lock: function (mh: PMSG): sword; cdecl;
        UnLock: function (mh: PMSG): sword; cdecl;
        SetCurPos: function (msgh: PMSGH; pos: dword): sword; cdecl;
        GetCurPos: function (msgh: PMSGH): dword; cdecl;
        MsgnToUid: function (mh: PMSG; msgnum: dword): UMSGID; cdecl;
        UidToMsgn: function (mh: PMSG; umsgid: UMSGID; _type: word): dword; cdecl;
        GetHighWater: function (mh: PMSG): dword; cdecl;
        SetHighWater: function (mh: PMSG; hwm: dword): sword; cdecl;
        GetTextLen: function (msgh: PMSGH): dword; cdecl;
        GetCtrlLen: function (msgh: PMSGH): dword; cdecl;
        end;
    P_apifuncs = ^_apifuncs;

    MSGAPI = record
          id: dword;
          len: word;
          _type: word;
          num_msg: dword;
          cur_msg: dword;
          high_msg: dword;
          high_water: dword;
          sz_xmsg: word;
          locked: byte;
          isecho: byte;
          api: P_apifuncs;
          apidata: pointer;
          end;
    MSG = MSGAPI;

    MSGH = record
          sq : PMSG;
          id : dword;
          bytes_written : dword;
          cur_pos : dword;
       end;

     SEARCH = record
          next : PSEARCH;
          attr : longint;
          flag : longint;
          txt : Pchar;
          where : char;
       end;

     BROWSE = record
          path : Pchar;
          _type : word;
          bflag : word;
          bdata : dword;
          first : PSEARCH;
          nonstop : Pchar;
          msgn : dword;
          sq : PMSG;
          m : PMSGH;
          msg : XMSG;
          matched : word;
          Begin_Ptr : function (b:pBROWSE):longint;cdecl;
          Status_Ptr : function (b:pBROWSE; aname:pchar; colour:longint):longint;
          Idle_Ptr : function (b:pBROWSE):longint;
          Display_Ptr : function (b:pBROWSE):longint;
          After_Ptr : function (b:pBROWSE):longint;
          End_Ptr : function (b:pBROWSE):longint;
          Match_Ptr : function (b:pBROWSE):longint;
       end;

Var
  msgapierr: word; cvar; external;
  mi: minf; cvar; external;

  function MsgCloseArea(mh: PMSG): sword; cdecl;
  function MsgOpenMsg(mh: PMSG; mode: word; n: dword): PMSGH; cdecl;
  function MsgCloseMsg(msgh: PMSGH): sword; cdecl;
  function MsgReadMsg(msgh: PMSGH; msg: PXMSG; ofs: dword; bytes: dword; text: PChar; cbyt: dword; ctxt: PChar): dword; cdecl;
  function MsgWriteMsg(msgh: PMSGH; append: word; msg: PXMSG; text: PChar; textlen: dword; totlen: dword; clen: dword; ctxt: PChar): sword; cdecl;
  function MsgKillMsg(mh: PMSG; msgnum: dword): sword; cdecl;
  function MsgLock(mh: PMSG): sword; cdecl;
  function MsgUnLock(mh: PMSG): sword; cdecl;
  function MsgSetCurPos(msgh: PMSGH; pos: dword): sword; cdecl;
  function MsgGetCurPos(msgh: PMSGH): dword; cdecl;
  function MsgMsgnToUid(mh: PMSG; msgnum: dword): UMSGID; cdecl;
  function MsgUidToMsgn(mh: PMSG; umsgid: UMSGID; _type: word): dword; cdecl;
  function MsgGetHighWater(mh: PMSG): dword; cdecl;
  function MsgSetHighWater(mh: PMSG; hwm: dword): sword; cdecl;
  function MsgGetTextLen(msgh: PMSGH): dword; cdecl;
  function MsgGetCtrlLen(msgh: PMSGH): dword; cdecl;

  function MsgOpenApi(minf: PMINF): sword; cdecl;
  function MsgCloseApi: sword; cdecl;

  function MsgOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl;
  function MsgValidate(_type: word; name: PChar): sword; cdecl;
  function MsgBrowseArea(b: PBROWSE): sword; cdecl;

  function InvalidMsgh(msgh: PMSGH): sword; cdecl;
  function InvalidMh(mh: PMSG): sword; cdecl;

  procedure SquishSetMaxMsg(sq: PMSG; max_msgs: dword; skip_msgs: dword; age: dword); cdecl;
  function SquishHash(f: PChar): dword; cdecl;

  function SdmOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl;
  function SdmValidate(name: PChar): sword; cdecl;

  function SquishOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl;
  function SquishValidate(name: PChar): sword; cdecl;

  function JamOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl;
  function JamValidate(name: PChar): sword; cdecl;

  function CvtCtrlToKludge(ctrl: PChar): PChar; cdecl;
  function GetCtrlToken(where: PChar; what: PChar): PChar; cdecl;
  function CopyToControlBuf(txt: PChar; newtext: PPChar; length: PLongInt): PChar; cdecl;
  procedure ConvertControlInfo(ctrl: PChar; orig: PNETADDR; dest: PNETADDR); cdecl;
  function NumKludges(txt: PChar): word; cdecl;
  procedure RemoveFromCtrl(ctrl: PChar; what: PChar); cdecl;

  function Address(a: PNETADDR): PChar; cdecl;
  function StripNasties(str: PChar): PChar; cdecl;

implementation

const External_library='smapilnx';


  function MsgCloseArea(mh: PMSG): sword; cdecl;
    Begin
    MsgCloseArea := mh^.api^.CloseArea(mh);
    End;

  function MsgOpenMsg(mh: PMSG; mode: word; n: dword): PMSGH; cdecl;
    Begin
    MsgOpenMsg := mh^.api^.OpenMsg(mh, mode, n);
    End;

  function MsgCloseMsg(msgh: PMSGH): sword; cdecl;
    Begin
    MsgCloseMsg := msgh^.sq^.api^.CloseMsg(msgh);
    End;

  function MsgReadMsg(msgh: PMSGH; msg: PXMSG; ofs: dword; bytes: dword; text: PChar; cbyt: dword; ctxt: PChar): dword; cdecl;
    Begin
    MsgReadMsg := msgh^.sq^.api^.ReadMsg(msgh, msg, ofs, bytes, text, cbyt, ctxt);
    End;

  function MsgWriteMsg(msgh: PMSGH; append: word; msg: PXMSG; text: PChar; textlen: dword; totlen: dword; clen: dword; ctxt: PChar): sword; cdecl;
    Begin
    MsgWriteMsg := msgh^.sq^.api^.WriteMsg(msgh, append, msg, text, textlen, totlen, clen, ctxt);
    End;

  function MsgKillMsg(mh: PMSG; msgnum: dword): sword; cdecl;
    Begin
    MsgKillMsg := mh^.api^.KillMsg(mh, msgnum);
    End;

  function MsgLock(mh: PMSG): sword; cdecl;
    Begin
    MsgLock := mh^.api^.Lock(mh);
    End;

  function MsgUnLock(mh: PMSG): sword; cdecl;
    Begin
    MsgUnLock := mh^.api^.UnLock(mh);
    End;

  function MsgSetCurPos(msgh: PMSGH; pos: dword): sword; cdecl;
    Begin
    MsgSetCurPos := msgh^.sq^.api^.SetCurPos(msgh, pos);
    End;

  function MsgGetCurPos(msgh: PMSGH): dword; cdecl;
    Begin
    MsgGetCurPos := msgh^.sq^.api^.GetCurPos(msgh);
    End;

  function MsgMsgnToUid(mh: PMSG; msgnum: dword): UMSGID; cdecl;
    Begin
    MsgMsgnToUid := mh^.api^.MsgnToUid(mh, msgnum);
    End;

  function MsgUidToMsgn(mh: PMSG; umsgid: UMSGID; _type: word): dword; cdecl;
    Begin
    MsgUidToMsgn := mh^.api^.UidToMsgn(mh, umsgid, _type);
    End;

  function MsgGetHighWater(mh: PMSG): dword; cdecl;
    Begin
    MsgGetHighWater := mh^.api^.GetHighWater(mh);
    End;

  function MsgSetHighWater(mh: PMSG; hwm: dword): sword; cdecl;
    Begin
    MsgSetHighWater := mh^.api^.SetHighWater(mh, hwm);
    End;

  function MsgGetTextLen(msgh: PMSGH): dword; cdecl;
    Begin
    MsgGetTextLen := msgh^.sq^.api^.GetTextLen(msgh);
    End;

  function MsgGetCtrlLen(msgh: PMSGH): dword; cdecl;
    Begin
    MsgGetCtrlLen := msgh^.sq^.api^.GetCtrlLen(msgh);
    End;


  function MsgOpenApi(minf: PMINF): sword; cdecl; external External_library name 'MsgOpenApi';
  function MsgCloseApi: sword; cdecl; external External_library name 'MsgCloseApi';

  function MsgOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl; external External_library name 'MsgOpenArea';
  function MsgValidate(_type: word; name: PChar): sword; cdecl; external External_library name 'MsgValidate';
  function MsgBrowseArea(b: PBROWSE): sword; cdecl; external External_library name 'MsgBrowseArea';

  function InvalidMsgh(msgh: PMSGH): sword; cdecl; external External_library name 'InvalidMsgh';
  function InvalidMh(mh: PMSG): sword; cdecl; external External_library name 'InvalidMh';

  procedure SquishSetMaxMsg(sq: PMSG; max_msgs: dword; skip_msgs: dword; age: dword); cdecl; external External_library name 'SquishSetMaxMsg';
  function SquishHash(f: PChar): dword; cdecl; external External_library name 'SquishHash';

  function SdmOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl; external External_library name 'SdmOpenArea';
  function SdmValidate(name: PChar): sword; cdecl; external External_library name 'SdmValidate';

  function SquishOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl; external External_library name 'SquishOpenArea';
  function SquishValidate(name: PChar): sword; cdecl; external External_library name 'SquishValidate';

  function JamOpenArea(name: PChar; mode: word; _type: word): PMSG; cdecl; external External_library name 'JamOpenArea';
  function JamValidate(name: PChar): sword; cdecl; external External_library name 'JamValidate';

  function CvtCtrlToKludge(ctrl: PChar): PChar; cdecl; external External_library name 'CvtCtrlToKludge';
  function GetCtrlToken(where: PChar; what: PChar): PChar; cdecl; external External_library name 'GetCtrlToken';
  function CopyToControlBuf(txt: PChar; newtext: PPChar; length: PLongInt): PChar; cdecl; external External_library name 'CopyToControlBuf';
  procedure ConvertControlInfo(ctrl: PChar; orig: PNETADDR; dest: PNETADDR); cdecl; external External_library name 'ConvertControlInfo';
  function NumKludges(txt: PChar): word; cdecl; external External_library name 'NumKludges';
  procedure RemoveFromCtrl(ctrl: PChar; what: PChar); cdecl; external External_library name 'RemoveFromCtrl';

  function Address(a: PNETADDR): PChar; cdecl; external External_library name 'Address';
  function StripNasties(str: PChar): PChar; cdecl; external External_library name 'StripNasties';

end.
