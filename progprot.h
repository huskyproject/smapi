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

#include <stdio.h>
#include <time.h>

#include <huskylib/compiler.h>

/*#if !( (defined(__WATCOMC__) && !defined(__DOS__)) || defined(__MSVC__) )*/
#ifdef __DOS__
void pascal far flush_handle2(int fd);  /* flushasm.asm for DOS, redefined for known implementations in flush.c */
#endif

void _fast flush_handle(FILE * fp);
char *_fast stristr(char *string, char *search);
void _fast qksort(int a[], size_t n);

#endif
