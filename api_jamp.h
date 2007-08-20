/*
** Adapted for MSGAPI by Fedor Lizunkov 2:5020/960@FidoNet
*/

#ifndef __API_JAMP_H
#define __API_JAMP_H

static sword _XPENTRY JamCloseArea(MSGA * jm);
static MSGH *_XPENTRY JamOpenMsg(MSGA * jm, word mode, dword msgnum);
static sword _XPENTRY JamCloseMsg(MSGH * msgh);
static dword _XPENTRY JamReadMsg(MSGH * msgh, XMSG * msg, dword offset, dword bytes, byte * text, dword clen, byte * ctxt);
static sword _XPENTRY JamWriteMsg(MSGH * msgh, word append, XMSG * msg, byte * text, dword textlen, dword totlen, dword clen, byte * ctxt);
static sword _XPENTRY JamKillMsg(MSGA * jm, dword msgnum);
static sword _XPENTRY JamLock(MSGA * jm);
static sword _XPENTRY JamUnlock(MSGA * jm);
static sword _XPENTRY JamSetCurPos(MSGH * msgh, dword pos);
static dword _XPENTRY JamGetCurPos(MSGH * msgh);
static UMSGID _XPENTRY JamMsgnToUid(MSGA * jm, dword msgnum);
static dword _XPENTRY JamUidToMsgn(MSGA * jm, UMSGID umsgid, word type);
static dword _XPENTRY JamGetHighWater(MSGA * jm);
static sword _XPENTRY JamSetHighWater(MSGA * sq, dword hwm);
static dword _XPENTRY JamGetTextLen(MSGH * msgh);
static dword _XPENTRY JamGetCtrlLen(MSGH * msgh);
static UMSGID _XPENTRY JamGetNextUid(HAREA ha);
static dword  _XPENTRY JamGetHash(HAREA mh, dword msgnum);

#define fop_wpb (O_CREAT | O_TRUNC | O_RDWR | O_BINARY)
#define fop_rpb (O_RDWR | O_BINARY)
#define fop_cpb (O_CREAT | O_EXCL | O_RDWR | O_BINARY)

static sword MSGAPI Jam_OpenBase(MSGA *jm, word *mode, unsigned char *basename);
int Jam_OpenFile(JAMBASE *jambase, word *mode, mode_t permissions);
void Jam_CloseFile(JAMBASE *jambase);
static MSGH *Jam_OpenMsg(MSGA * jm, word mode, dword msgnum);
JAMSUBFIELD2ptr Jam_GetSubField(struct _msgh *msgh, dword *SubPos, word what);
dword Jam_HighMsg(JAMBASEptr jambase);
void Jam_ActiveMsgs(MSGA *jm);
static int near Jam_Lock(MSGA *jm, int force);
static void near Jam_Unlock(MSGA * jm);
dword Jam_PosHdrMsg(MSGA * jm, dword msgnum, JAMIDXREC *jamidx, JAMHDR *jamhdr);
static dword Jam_JamAttrToMsg(MSGH *msgh);
sword Jam_WriteHdrInfo(JAMBASEptr jambase);
SMAPI_EXT void Jam_WriteHdr(MSGA *jm, JAMHDR *jamhdr, dword msgnum);
SMAPI_EXT JAMHDR *Jam_GetHdr(MSGA *jm, dword msgnum);
SMAPI_EXT dword Jam_Crc32(unsigned char* buff, dword len);
SMAPI_EXT char *Jam_GetKludge(MSGA *jm, dword msgnum, word what);
static void MSGAPI ConvertXmsgToJamHdr(MSGH *msgh, XMSG *msg, JAMHDRptr jamhdr, JAMSUBFIELD2LISTptr *subfield);
static void MSGAPI ConvertCtrlToSubf(JAMHDRptr jamhdr, JAMSUBFIELD2LISTptr *subfield, dword clen, unsigned char *ctxt);
unsigned char *DelimText(JAMHDRptr jamhdr, JAMSUBFIELD2LISTptr *subfield,
                         unsigned char *text, size_t textlen);
void parseAddr(NETADDR *netAddr, const unsigned char *str, dword len);
void DecodeSubf(MSGH *msgh);

struct _msgh
{
    MSGA *sq;
    dword id;                   /* Must always equal MSGH_ID */

    dword bytes_written;
    dword cur_pos;

    /* For JAM only! */

    JAMIDXREC       Idx;            /* Message index */
    JAMHDR          Hdr;            /* Message header */
    JAMSUBFIELD2LISTptr  SubFieldPtr;    /* Pointer to Subfield structure */

    dword seek_idx;
    dword seek_hdr;

    dword clen;
    byte  *ctrl;
    dword lclen;
    byte  *lctrl;
    dword msgnum;

    word  mode;
};


static struct _apifuncs jm_funcs =
{
    JamCloseArea,
    JamOpenMsg,
    JamCloseMsg,
    JamReadMsg,
    JamWriteMsg,
    JamKillMsg,
    JamLock,
    JamUnlock,
    JamSetCurPos,
    JamGetCurPos,
    JamMsgnToUid,
    JamUidToMsgn,
    JamGetHighWater,
    JamSetHighWater,
    JamGetTextLen,
    JamGetCtrlLen,
    JamGetNextUid,
    JamGetHash
};

int read_hdrinfo(int handle, JAMHDRINFO *HdrInfo);
int read_idx(int handle, JAMIDXREC *Idx);
int read_hdr(int handle, JAMHDR *Hdr);
int read_subfield(int handle, JAMSUBFIELD2LISTptr *subfield, dword *SubfieldLen);
int copy_subfield(JAMSUBFIELD2LISTptr *to, JAMSUBFIELD2LISTptr from);

int read_allidx(JAMBASEptr jmb);

int write_hdrinfo(int handle, JAMHDRINFO *HdrInfo);
int write_idx(int handle, JAMIDXREC *Idx);
int write_hdr(int handle, JAMHDR *Hdr);
int write_subfield(int handle, JAMSUBFIELD2LISTptr *subfield, dword SubfieldLen);



#endif
