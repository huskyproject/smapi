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

#ifndef __PROGPROT_H__
#define __PROGPROT_H__

#if !((defined(__WATCOMC__) && !defined(MSDOS)) || (defined(_MSC_VER) && (_MSC_VER >= 1200)))
void pascal far flush_handle2(int fd);
#endif
void _fast flush_handle(FILE * fp);
int _fast fexist(char *filename);
long _fast fsize(char *filename);
char *_fast firstchar(char *strng, char *delim, int findword);
char *_fast stristr(char *string, char *search);
void _fast qksort(int a[], size_t n);
int _fast direxist(char *directory);
void _fast ASCII_Date_To_Binary(char *msgdate, union stamp_combo *d_written);
union stamp_combo *_fast Get_Dos_Date(union stamp_combo *st);
struct tm *_fast DosDate_to_TmDate(union stamp_combo *dosdate, struct tm *tmdate);
union stamp_combo *_fast TmDate_to_DosDate(struct tm *tmdate, union stamp_combo *dosdate);
char *_fast Strip_Trailing(char *str, char strip);
char *_fast Add_Trailing(char *str, char add);
void _fast Parse_NetNode(char *netnode, word * zone, word * net, word * node, word * point);
void _fast ParseNN(char *netnode, word * zone, word * net, word * node, word * point, word all);
char *_fast sc_time(union stamp_combo *sc, char *string);
char *_fast strocpy(char *d, char *s);
void _fast tdelay(int);
int _fast setfsize(int fd, long size);
#ifdef INTEL
#define put_dword(ptr, val)	(*(dword *)(ptr) = (val))
#define put_word(ptr, val)	(*(word *)(ptr) = (val))
#else
void put_word(byte *ptr, word value);
void put_dword(byte *ptr, dword value);
#endif

int  _createDirectoryTree(const char *pathName);
/*DOC
  Input:  a pointer to a \0 terminated string
  Output: 0 if successfull, 1 else
  FZ:     pathName is a correct directory name
          createDirectoryTree creates the directory and all parental directories
          if they do not exist.

  was taken from hpt\fcommon
*/

#endif
