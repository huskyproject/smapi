/* Don't call this routine from user applications!  This routine is NOT part of
   the SMAPI 2.0 interface, because it has been introduced after 2.0 was
   already released.

   _createDirectoryTree will be part of the SMAPI 2.2 interface.

   Until that time, either COPY this code to your application, or require link
   your application against current smapi or smapi 2.2
*/

static int _createDirectoryTree(const char *pathName) {

   char *start, *slash;
   char limiter=PATH_DELIM;
   int i;

   start = (char *) malloc(strlen(pathName)+2);
   strcpy(start, pathName);
   i = strlen(start)-1;
   if (start[i] != limiter) {
      start[i+1] = limiter;
      start[i+2] = '\0';
   }
   slash = start;

#ifndef UNIX
   // if there is a drivename, jump over it
   if (slash[1] == ':') slash += 2;
#endif

   // jump over first limiter
   slash++;

   while ((slash = strchr(slash, limiter)) != NULL) {
      *slash = '\0';

      if (!direxist(start)) {
         if (!fexist(start)) {
            // this part of the path does not exist, create it
            if (mymkdir(start) != 0) {
               free(start);
               return 1;
            }
         } else {
            free(start);
            return 1;
         }
      }

      *slash++ = limiter;
   }

   free(start);

   return 0;
}
