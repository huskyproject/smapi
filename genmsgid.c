#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(UNIX) && !defined(SASC)
#   include <io.h>
#endif

#include "msgapi.h"
#include "prog.h"
#include "ffind.h"

#define MAX_OUTRUN	(3ul*365*24*60*60) /* 3 year */

dword oldGenMsgId(void)
{
	dword seq = (dword)time(NULL);
	sleep(1);
	return seq;
}

dword _XPENTRY GenMsgId(char *seqdir, unsigned long max_outrun)
{
	dword seq, n, curtime;
	FFIND *ff;
	char *seqpath, max_fname[13], *new_fname, *pname, *p;
	int  h, try;

	if (seqdir == NULL || *seqdir == '\0') {
		seqdir = getenv("SEQDIR");
		if (seqdir == NULL || *seqdir == '\0') {
			/* warning: no SEQDIR defined, fall to ugly old algorythm */
			return oldGenMsgId();
		}
	}
	seqpath = malloc(strlen(seqdir)+13);
	strcpy(seqpath, seqdir);
	pname = seqpath + strlen(seqpath);
	if (*seqpath && strchr("/\\", seqpath[strlen(seqpath)-1]) == NULL)
		*pname++ = PATH_DELIM;
	new_fname = NULL;
	if (max_outrun == 0) {
		p = getenv("SEQOUT");
		if (p && isdigit(*p)) {
			max_outrun = (unsigned long)atol(p);
			switch (tolower(p[strlen(p) - 1])) {
				case 'y':	max_outrun *= 365;
				case 'd':	max_outrun *= 24;
				case 'h':	max_outrun *= 60*60;
						break;
				case 'w':	max_outrun *= (7l*24*60*60);
						break;
				case 'm':	max_outrun *= (31l*24*60*60);
						break;
			}
		}
		else max_outrun = MAX_OUTRUN;
	}
	for (try=0;;try++) {
		curtime = (dword)time(NULL);
		seq = 0;
		max_fname[0] = '\0';
		strcpy(pname, "*.*");
		ff = FFindOpen(seqpath, 0);
		if (ff == NULL) {
			*pname = '\0';
			if (try == 0 && !direxist(seqpath))
				if (_createDirectoryTree(seqpath) == 0)
					continue;
			if (try == 0 && direxist(seqpath))
				goto emptydir;
			free(seqpath);
			if (new_fname) free(new_fname);
			return oldGenMsgId();
		}
		do {
			for (p=ff->ff_name; isxdigit(*p); p++);
			if (stricmp(p, ".seq") != 0) continue;
			if (strlen(ff->ff_name) > 12) continue;
			n = strtol(ff->ff_name, NULL, 16);
			if (n > curtime && n - curtime > max_outrun) {
				/* counter too large, remove */
				strcpy(pname, ff->ff_name);
				unlink(seqpath);
				continue;
			}
			if (n >= seq) {
				if (max_fname[0]) {
					strcpy(pname, max_fname);
					unlink(seqpath);
				}
				strcpy(max_fname, ff->ff_name);
				seq = n;
			} else {
				strcpy(pname, ff->ff_name);
				unlink(seqpath);
			}
		} while (FFindNext(ff) == 0);
		if (ff) FFindClose(ff);
emptydir:
		if (seq < curtime) seq = curtime;
		if (new_fname == NULL)
			new_fname = malloc(strlen(seqpath) + 13);
		*pname = '\0';
		sprintf(new_fname, "%s%08lx.seq", seqpath, (unsigned long)(seq + 1));
		if (max_fname[0] == '\0') {
			/* No files found, create new */
			h = open(new_fname, O_CREAT|O_BINARY|O_EXCL, 0666);
			if (h != -1) {
				/* ok, scan again */
				close(h);
				continue;
			}
			/* error creating file */
			if (errno == EEXIST) continue;
			free(seqpath);
			free(new_fname);
			return oldGenMsgId();
		}
		/* rename max_fname to new_fname */
		strcpy(pname, max_fname);
		if (rename(seqpath, new_fname) == 0) {
			free(seqpath);
			free(new_fname);
			return seq;
		}
		if (errno == ENOENT || errno == EEXIST ||
		    ((errno == EPERM || errno == EACCES) && try < 16))
			continue;
		free(seqpath);
		free(new_fname);
		return oldGenMsgId();
	}
}
