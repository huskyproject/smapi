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

/* This file contents our own implementation of strftime() function for using
 * with compilers where this function not implemented in clib.
 */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#ifdef HAS_strftime
/* Use function instead macro to prevent compiler warning or error.
 * (If file do not content any code some compilers report about error.)
 */
size_t cdecl strftim(char *string, size_t maxsize, const char *format, const struct tm *current_time)
{ return strftime(string, maxsize, format, current_time); }

#else
/* We own implementation instead strftime() */

/* Note: TZ environment variable MUST be defined to use the %Z function
 * at least in form "SET TZ=XYZ"
 */

size_t cdecl strftim(char *string, size_t maxsize, const char *format, const struct tm *current_time)
{
    const char *in;

    char *out, *scrptr;

    char temp[250];

    maxsize = min(maxsize, 230);

    for (in = format, out = temp; *in; in++)
    {
        if ((int)(out - (int)temp) >= maxsize)
        {
            break;
        }

        if (*in == '%')
        {
            switch (*++in)
            {
            case 'a':
                strcpy(out, weekday_ab[current_time->tm_wday]);
                break;

            case 'A':
                strcpy(out, weekday[current_time->tm_wday]);
                break;

            case 'b':
                strcpy(out, months_ab[current_time->tm_mon]);
                break;

            case 'B':
                strcpy(out, months[current_time->tm_mon]);
                break;

            case 'c':
                sprintf(out, "%02d-%02d-%02d %02d:%02d:%02d",
                  current_time->tm_mon + 1, current_time->tm_mday,
                  current_time->tm_year, current_time->tm_hour,
                  current_time->tm_min, current_time->tm_sec);
                break;

            case 'd':
                sprintf(out, "%02d", current_time->tm_mday);
                break;

            case 'H':
                sprintf(out, "%02d", current_time->tm_hour);
                break;

            case 'I':
                sprintf(out, "%02d",
                  current_time->tm_hour >= 0 && current_time->tm_hour <= 12 ?
                  current_time->tm_hour : current_time->tm_hour - 12);
                break;

            case 'j':
                sprintf(out, "%03d", current_time->tm_yday + 1);
                break;

            case 'm':
                sprintf(out, "%02d", (current_time->tm_mon) + 1);
                break;

            case 'M':
                sprintf(out, "%02d", current_time->tm_min);
                break;

            case 'p':
                strcpy(out, current_time->tm_hour < 12 ? "am" : "pm");
                break;

            case 'S':
                sprintf(out, "%02d", current_time->tm_sec);
                break;

            case 'U': /* Not from Sunday, BUG!!! */
                sprintf(out, "%02d", current_time->tm_yday / 7);
                break;

            case 'w':
                sprintf(out, "%d", current_time->tm_wday);
                break;

            case 'W': /* Not from Monday, BUG!!! */
                sprintf(out, "%02d", (current_time->tm_yday) / 7);
                break;

            case 'x':
                sprintf(out, "%02d-%02d-%02d", current_time->tm_mon + 1,
                  current_time->tm_mday, current_time->tm_year);
                break;

            case 'X':
                sprintf(out, "%02d:%02d:%02d", current_time->tm_hour,
                  current_time->tm_min, current_time->tm_sec);
                break;

            case 'y':
                sprintf(out, "%02d", current_time->tm_year % 100);
                break;

            case 'Y':
                sprintf(out, "%02d", current_time->tm_year + 1900);
                break;

            case 'Z':
                scrptr = getenv("TZ");
                if (scrptr != NULL)
                {
                    /*!!! todo1: replace with extract 3 first alpha chars
                                 to prevent illegal value
                          todo2: implement full parsing TZ env.var */
                    strncpy(out, scrptr, 3);
                    out[3] = '\0';
                    strupr(out);
                }
                else
                {
                    static char firstcall=1;
                    if(firstcall){ firstcall=0;
                      fprintf( stderr, "Please set the TZ enviroment variable!");
                    }
                    strcpy(out, "??T");
                }
                break;

            case '%':
                strcpy(out, "%");
                break;
            }

            out += strlen(out);
        }
        else
        {
            *out++ = *in;
        }
    }

    *out = '\0';

    strcpy(string, temp);

    return strlen(string);
}
#endif
