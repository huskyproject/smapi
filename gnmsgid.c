#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "msgapi.h"
#include "prog.h"

#define check_stricmp(a, b) (stricmp(a, b) == 0)

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
    printf("gnmsgid [-seqdir <path>] [-seqoutrun <outrun>] [<num>]\n");
    printf("where\n");
    printf("  -seqdir    - specify msgid sequencer directory (see husky docs)\n");
    printf("  -seqoutrun - specify msgid outrun value (see husky docs)\n");
    printf("  <num>      - number msgid to generate\n");
}

int main(int argc, char *argv[])
{
    int i, j, perr, usage, parsed;
    char *s;
    char *seqdir;
    unsigned long seqoutrun;
    int num;
    dword msgid;
    char *msgiderr;

    seqdir = NULL;
    seqoutrun = 0;
    usage = 0;
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
            if (check_stricmp(s, "?")||check_stricmp(s, "h")||check_stricmp(s, "-help"))
            {
                usage = 1;
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
