/*
 *  PATMAT.C - Pattern matching.
 *
 *  Written November 29, 1988 by Sreenath Chary.  Minor maintenance on
 *  December 21, 1996 by Andrew Clarke.  Released to the public domain.
 *
 *  Pass two string pointers as parameters.  The first being a raw
 *  string and the second a pattern the raw string is to be matched
 *  against.  If the raw string matches the pattern, the function returns
 *  1, otherwise it returns 0.
 *
 *      patmat("abcdefghi", "*ghi") returns 1
 *      patmat("abcdefghi", "??c??f*") returns 1
 *      patmat("abcdefghi", "*dh*") returns 0
 *      patmat("abcdefghi", "*def") returns 0
 *
 *  The asterisk is a wildcard to allow any characters from its first
 *  appearance to the next specific character.  The character ? is a
 *  wildcard for only one character in the position it appears.
 *
 *  Combinations such as "*?" or "?*" or "**" are illegal for obvious
 *  reasons, and the functions may goof, although I think it will still
 *  work.
 *
 *  The only simple way I could devise is to use recursion.  Each
 *  character in the pattern is taken and compared with the character in
 *  the raw string.  If it matches then the remaining amount of the
 *  string and the remaining amount of the pattern are passed as parameters
 *  to patmat again until the end of the pattern.  If at any stage the
 *  pattern does not match, then we go back one level and at this level
 *  if the previous character was an asterisk in the pattern, we hunt
 *  again from where we left off, otherwise we return back one more level
 *  with a not found and the process goes on till the first level call.
 *
 *  Only one character at a time is considered, except when the character
 *  is an asterisk. You'll get the logic as the program unfolds.
 */

#include <string.h>

int patmat(char *raw, char *pat)
{
    /* if it's the end of both strings, then match */
    if (*pat == '\0' && *raw == '\0')
    {
        return 1;
    }

    /* if it's the end of only pat, then mismatch */
    if (*pat == '\0')
    {
        return 0;
    }

    /* if pattern is a '*' */
    if (*pat == '*')
    {
        int i;

        /* if it is the end of pat, then match */
        if (*(pat + 1) == '\0')
        {
            return 1;
        }

        /* else hunt for match or wildcard */
        for (i = 0; i <= strlen(raw); i++)
        {
            if (*(raw + i) == *(pat + 1) || *(pat + 1) == '?')
            {
                /* if found, match rest of pat */
                if (patmat(raw + i + 1, pat + 2) == 1)
                {
                    return 1;
                }
            }
        }
    }
    else
    {
        /* if end of raw, then mismatch */
        if (*raw == '\0')
        {
            return 0;
        }

        /* if chars match then try and match the rest of it */
        if (*pat == '?' || *pat == *raw)
        {
            if (patmat(raw + 1, pat + 1) == 1)
            {
                return 1;
            }
        }
    }

    /* fell through, no match was found */
    return 0;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
    printf("patmat(\"abcdefghi\", \"*ghi\"): %d\n", patmat("abcdefghi", "*ghi"));
    printf("patmat(\"abcdefghi\", \"??c??f*\"): %d\n", patmat("abcdefghi", "??c??f*"));
    printf("patmat(\"abcdefghi\", \"*dh*\"): %d\n", patmat("abcdefghi", "*dh*"));
    printf("patmat(\"abcdefghi\", \"*def\"): %d\n", patmat("abcdefghi", "*def"));
    return 0;
}

#endif
