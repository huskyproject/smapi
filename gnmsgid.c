/*
   gnmsgid - standalone msgid generator using husky smapi library
   Copyright 2003 by Alexander Reznikov, 2:4600/220@fidonet,
                                         homebrewer@yadex.ru
   and HUSKY team.

   This file is part of HUSKY project

   This is free software, you can FREE redistribute or modify it.

*/
/* $Id$ */

/* $Id$
 * standalone msgid generator: print new msgid to stdout

 *  SMAPI; Modified Squish MSGAPI
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "compiler.h"
#include "msgapi.h"
#include "cvsdate.h"

#define check_stricmp(a, b) (stricmp(a, b) == 0)
#define str_or_unknown(str) (str? str: "unknown")

char gnmsgid_rev[]  = "$Revision$";
char gnmsgid_date[] = "$Date$";
extern char genmsgid_rev[];
extern char genmsgid_date[];

int outrunparse(char *line, unsigned long *seqoutrun)
{
    char *p;
    while (isspace(*line)) line++;
    if (!isdigit(*line))
    {
        fprintf(stderr, "Bad SeqOutrun value '%s'!\n", line);
        return 1;
    }
    *seqoutrun = (unsigned long)atol(line);
    p = line;
    while (isdigit(*p)) p++;
    if (*p == '\0') return 0;
    if (p[1]) {
        fprintf(stderr, "Bad SeqOutrun value '%s'!\n", line);
        return 1;
    }
    switch (tolower(*p)) {
    case 'y':	*seqoutrun *= 365;
    case 'd':	*seqoutrun *= 24;
    case 'h':	*seqoutrun *= 60*60;
        break;
    case 'w':	*seqoutrun *= 7l*24*60*60;
        break;
    case 'm':	*seqoutrun *= 31l*24*60*60;
        break;
    default:	fprintf(stderr, "Bad SeqOutrun value '%s'!\n", line);
        return 1;
    }
    return 0;
}

void printusage(void)
{
    printf("gnmsgid - standalone msgid generator using husky smapi library\n");
    printf("\nUsage:\n");
    printf("gnmsgid [-h] [-v] [-seqdir <path>] [-seqoutrun <outrun>] [<num>]\n");
    printf("where\n");
    printf("  -h         - print usage information and exit\n");
    printf("  -v         - print version and exit\n");
    printf("  -seqdir    - specify msgid sequencer directory (see husky docs)\n");
    printf("  -seqoutrun - specify msgid outrun value (see husky docs)\n");
    printf("  <num>      - number msgid to generate\n");
}

char *extract_CVS_keyword(char *str)
{
    int l;
    char *tmp, *r;

    if(!str)
        return NULL;

    tmp = strchr(str, 0x20);

    if ((!tmp)||(!*(++tmp)))
        return NULL;

    l = strlen(tmp);

    if (l<3)
        return NULL;

    r = malloc(l-1);
    strncpy(r, tmp, l-2);
    r[l-2] = 0;

    return r;
}

void printversion(void)
{
    char *rev, *date, *gen_rev, *gen_date;

    rev = extract_CVS_keyword(gnmsgid_rev);
    date = extract_CVS_keyword(gnmsgid_date);
    gen_rev = extract_CVS_keyword(genmsgid_rev);
    gen_date = extract_CVS_keyword(genmsgid_date);

    printf("gnmsgid - standalone msgid generator using husky smapi library\n");
    printf("\nCopyright (c) Alexander Reznikov, 2:4600/220@fidonet\n");
    printf("Copyright (c) HUSKY development team.\n\n");
    printf("gnmsgid.c revision:  %s\n", str_or_unknown(rev));
    printf("gnmsgid.c date:      %s\n", str_or_unknown(date));
    printf("genmsgid.c revision: %s\n", str_or_unknown(gen_rev));
    printf("genmsgid.c date:     %s\n", str_or_unknown(gen_date));
    printf("SMAPI CVS date:      %s\n", cvs_date);

    if (rev) free(rev);
    if (date) free(date);
    if (gen_rev) free(gen_rev);
    if (gen_date) free(gen_date);
}

int main(int argc, char *argv[])
{
    int i, j, perr, usage, parsed, version;
    char *s;
    char *seqdir;
    unsigned long seqoutrun;
    int num;
    dword msgid;
    char *msgiderr;

    seqdir = NULL;
    seqoutrun = 0;
    usage = 0;
    version = 0;
    perr = 0;
    num = 1;
    parsed = 0;

    for(i=1; i<argc; i++)
    {
        if ((argv[i][0] == '-')||(argv[i][0] == '/'))
        {
            s = argv[i]+1;
            if (check_stricmp(s, "dir")||check_stricmp(s, "seqdir"))
            {
                i++;
                if (i<argc)
                {
                    if (seqdir)
                        free(seqdir);
                    seqdir = strdup(argv[i]);
                }
                else
                {
                    fprintf(stderr, "'%s' option require parameter!\n", s-1);
                    perr = 1;
                    break;
                }
                continue;
            }
            if (check_stricmp(s, "outrun")||check_stricmp(s, "seqoutrun")||check_stricmp(s, "maxoutrun"))
            {
                i++;
                if (i<argc)
                {
                    if (outrunparse(argv[i], &seqoutrun)!=0)
                    {
                        perr = 1;
                        break;
                    }
                }
                else
                {
                    fprintf(stderr, "'%s' option require parameter!\n", s-1);
                    perr = 1;
                    break;
                }
            }
            if (check_stricmp(s, "?")||check_stricmp(s, "h")||check_stricmp(argv[i], "--help"))
            {
                usage = 1;
                break;
            }
            if (check_stricmp(s, "v")||check_stricmp(argv[i], "--version"))
            {
                version = 1;
                break;
            }
            fprintf(stderr, "Illegal parameter: '%s'!\n", argv[i]);
            perr = 1;
            break;
        }
        if (parsed!=0)
        {
            fprintf(stderr, "Illegal parameter: '%s'!\n", argv[i]);
            perr = 1;
            break;
        }
        s = argv[i];
        for(j=strlen(argv[i]); j>0; j--, s++)
            if (!isdigit(*s))
            {
                fprintf(stderr, "Invalid <num> parameter ('%s')!\n", argv[i]);
                perr = 1;
                break;
            }
            if (perr!=0)
                break;

            num = atoi(argv[i]);

            if (num<1)
            {
                fprintf(stderr, "Invalid <num> parameter ('%s')!\n", argv[i]);
                perr = 1;
            }
            break;
    }

    if (perr==0)
    {
        if (usage)
        {
            printusage();
            return 1;
        }

        if (version)
        {
            printversion();
            return 1;
        }

        for(i=1; i<=num; i++)
        {
            msgid = GenMsgIdEx(seqdir, seqoutrun, NULL, &msgiderr);
            if (msgiderr)
            fprintf(stderr, "warning (id #%i): %s, fall to ugly old algorythm\n", i, msgiderr);
            printf("%08lx\n", msgid);
        }
    }
    return -1;
}
