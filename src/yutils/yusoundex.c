#include "yusoundex.h"

#include <string.h> 
#include <ctype.h> 

char *yu_soundex(char *in) {
   static  int code[] =
      {  0,1,2,3,0,1,2,0,0,2,2,4,5,5,0,1,2,6,2,3,0,1,0,2,0,2 };
   static  char key[5];
   char ch;
   int last;
   int count;

   strcpy(key, "Z000");

   /* Advance to the first letter.  If none present, 
      return default key */
   while (*in != '\0'  &&  !isalpha(*in))
      ++in;
   if (*in == '\0')
      return key;

   /* Pull out the first letter, uppercase it, and 
      set up for main loop */
   key[0] = toupper(*in);
   last = code[key[0] - 'A'];
   ++in;

   /* Scan rest of string, stop at end of string or 
      when the key is full */
   for (count = 1;  count < 4  &&  *in != '\0';  ++in) {
      /* If non-alpha, ignore the character altogether */
      if (isalpha(*in)) {
         ch = tolower(*in);
         /* Fold together adjacent letters sharing the same code */
         if (last != code[ch - 'a']) {
            last = code[ch - 'a'];
            /* Ignore code==0 letters except as separators */
            if (last != 0)
               key[count++] = '0' + last;
         }
      }
   }

   return key;
}
