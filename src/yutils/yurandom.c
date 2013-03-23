#include "yurandom.h"

#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>

void yu_random_string_alnum(YUString *out, int len) {
  int               i;
  struct timeval    tv;
  static char       alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
  
  gettimeofday(&tv,0);

  srand(tv.tv_usec % UINT_MAX);

  for (i = 0; i < len; i++) {
    yu_string_append(out,&alphanum[rand() % (sizeof(alphanum) - 1)],1);
  }
}

