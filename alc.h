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

#ifndef __ALC_H__
#define __ALC_H__

#include "compiler.h"

#if defined(_MSC_VER) || defined(__TURBOC__) || defined(__EMX__) || defined(__IBMC__) || defined(__WATCOMC__) || defined(__HIGHC__) || defined(__MINGW32__)
#include <malloc.h>
#elif defined(UNIX) || defined(SASC) || defined(__DJGPP__)
#else
#error alc.h: Unknown compiler!
#endif

#ifdef __TURBOC__
#ifdef halloc
#undef halloc
#define halloc(x,y) ((char far *)farmalloc(x*y))
#endif
#ifdef hfree
#undef hfree
#define hfree(p) farfree(p)
#endif
#endif

#endif
