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
static UMSGID EXPENTRY JamGetNextUid(HAREA ha);
static dword  EXPENTRY JamGetHash(HAREA mh, dword msgnum);

#define fop_wpb (O_CREAT | O_TRUNC | O_RDWR | O_BINARY)
#define fop_rpb (O_RDWR | O_BINARY)

static sword MSGAPI Jam_OpenBase(MSG *jm, word *mode, unsigned char *basename);
int Jam_OpenFile(JAMBASE *jambase, word *mode, mode_t permissions);
void Jam_CloseFile(JAMBASE *jambase);
static MSGH *Jam_OpenMsg(MSG * jm, word mode, dword msgnum);
JAMSUBFIELD2ptr Jam_GetSubField(struct _msgh *msgh, dword *SubPos, word what);
dword Jam_HighMsg(JAMBASEptr jambase);
void Jam_ActiveMsgs(JAMBASEptr jambase);
static int near Jam_Lock(MSG *jm, int force);
static void near Jam_Unlock(MSG * jm);
dword Jam_PosHdrMsg(MSG * jm, dword msgnum, JAMIDXREC *jamidx, JAMHDR *jamhdr);
static dword Jam_JamAttrToMsg(MSGH *msgh);
sword Jam_WriteHdrInfo(JAMBASEptr jambase);
dword Jam_Crc32(unsigned char* buff, dword len);
static void MSGAPI ConvertXmsgToJamHdr(MSGH *msgh, XMSG *msg, JAMHDRptr jamhdr, JAMSUBFIELD2ptr *subfield);
static void MSGAPI ConvertCtrlToSubf(JAMHDRptr jamhdr, JAMSUBFIELD2ptr *subfield, dword clen, unsigned char *ctxt);
unsigned char *DelimText(JAMHDRptr jamhdr, JAMSUBFIELD2ptr *subfield, unsigned
                         char *text, size_t textlen);
int makeKludge(char **buff, char *sstr, unsigned char *str, char *ent, int len);
void parseAddr(NETADDR *netAddr, unsigned char *str, dword len);
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
    JAMSUBFIELD2ptr  SubFieldPtr;    /* Pointer to Subfield structure */

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

int read_hdrinfo(sword handle, JAMHDRINFO *HdrInfo);
int read_idx(sword handle, JAMIDXREC *Idx);
int read_hdr(sword handle, JAMHDR *Hdr);
int read_subfield(sword handle, JAMSUBFIELD2ptr *subfield, dword *SubfieldLen);

int read_allidx(JAMBASEptr jmb);

int write_hdrinfo(sword handle, JAMHDRINFO *HdrInfo);
int write_idx(sword handle, JAMIDXREC *Idx);
int write_hdr(sword handle, JAMHDR *Hdr);
int write_subfield(sword handle, JAMSUBFIELD2ptr *subfield, dword SubfieldLen);



#endif
