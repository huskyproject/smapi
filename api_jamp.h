/*
** Adapted for MSGAPI by Fedor Lizunkov 2:5020/960@FidoNet
*/

#ifndef __API_JAMP_H
#define __API_JAMP_H

static sword EXPENTRY JamCloseArea(MSG * jm);
static MSGH *EXPENTRY JamOpenMsg(MSG * jm, word mode, dword msgnum);
static sword EXPENTRY JamCloseMsg(MSGH * msgh);
static dword EXPENTRY JamReadMsg(MSGH * msgh, XMSG * msg, dword offset, dword bytes, byte * text, dword clen, byte * ctxt);
static sword EXPENTRY JamWriteMsg(MSGH * msgh, word append, XMSG * msg, byte * text, dword textlen, dword totlen, dword clen, byte * ctxt);
static sword EXPENTRY JamKillMsg(MSG * jm, dword msgnum);
static sword EXPENTRY JamLock(MSG * jm);
static sword EXPENTRY JamUnlock(MSG * jm);
static sword EXPENTRY JamSetCurPos(MSGH * msgh, dword pos);
static dword EXPENTRY JamGetCurPos(MSGH * msgh);
static UMSGID EXPENTRY JamMsgnToUid(MSG * jm, dword msgnum);
static dword EXPENTRY JamUidToMsgn(MSG * jm, UMSGID umsgid, word type);
static dword EXPENTRY JamGetHighWater(MSG * jm);
static sword EXPENTRY JamSetHighWater(MSG * sq, dword hwm);
static dword EXPENTRY JamGetTextLen(MSGH * msgh);
static dword EXPENTRY JamGetCtrlLen(MSGH * msgh);

#define fop_wpb (O_CREAT | O_TRUNC | O_RDWR | O_BINARY)
#define fop_rpb (O_RDWR | O_BINARY)

static sword MSGAPI Jam_OpenBase(MSG *jm, word *mode, char *basename);
int Jam_OpenFile(JAMBASE *jambase, word *mode);
void Jam_CloseFile(JAMBASE *jambase);
static MSGH *Jam_OpenMsg(MSG * jm, word mode, dword msgnum);
JAMSUBFIELDptr Jam_GetSubField(struct _msgh *msgh, dword *SubPos, dword what);
dword Jam_HighMsg(JAMBASEptr jambase);
static int near Jam_Lock(MSG *jm, int force);
static void near Jam_Unlock(MSG * jm);
dword Jam_PosHdrMsg(MSG * jm, dword msgnum, JAMIDXREC *jamidx, JAMHDR *jamhdr);
static dword Jam_JamAttrToMsg(MSGH *msgh);
sword Jam_WriteHdrInfo(JAMBASEptr jambase);
dword Jam_Crc32(char* buff, dword len);
static void MSGAPI ConvertXmsgToJamHdr(MSGH *msgh, XMSG *msg, JAMHDRptr jamhdr, JAMSUBFIELDptr *subfield);
static void MSGAPI ConvertCtrlToSubf(JAMHDRptr jamhdr, JAMSUBFIELDptr *subfield, dword clen, char *ctxt);
char *DelimText(JAMHDRptr jamhdr, JAMSUBFIELDptr *subfield, char *text);
int makeKludge(char **buff, char *sstr, char *str, char *ent, int len);
void parseAddr(NETADDR *netAddr, char *str, dword len);
void DecodeSubf(MSGH *msgh);

struct _msgh
{
    MSG *sq;
    dword id;                   /* Must always equal MSGH_ID */

    dword bytes_written;
    dword cur_pos;

    /* For JAM only! */

    JAMIDXREC       Idx;            /* Message index */
    JAMHDR          Hdr;            /* Message header */
    JAMSUBFIELDptr  SubFieldPtr;    /* Pointer to Subfield structure */

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
    JamGetCtrlLen
};

#endif
