/*
 *  STREXTRA.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Modified in July 1994 by John Dennis.
 *  Additional modifications by Paul Edwards and Andrew Clarke.
 *
 *  Originally distributed with the Msged mail reader source code and
 *  modified for use in SMAPI in January 1997.  Released to the public domain.
 */

#ifndef __STREXTRA_H__
#define __STREXTRA_H__

int stricmp(const char *s, const char *t);

#if !defined(__IBMPC__) && !defined(__linux__)
char *strdup(const char *s);
#endif

#endif
