#include "yustring.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

YUString * yu_string_new() {
  YUString *str;
  str = malloc(sizeof(YUString));
  memset(str,0,sizeof(YUString));
  str->str = malloc(2 * sizeof(char));
  str->str[0] = 0;
  str->allocated = 2;
  return str;
}

void yu_string_clear(YUString *str) {
  str->str[0] = 0;
  str->len = 0;
}

void yu_string_free(YUString *str) {
  free(str->str);
  free(str);
}

void yu_string_sprintfa(YUString *str, char *fmt, ...) {
  int count;
  char *tmp;
  va_list args;
  
  va_start(args,fmt);
  count = vsnprintf(0,0,fmt,args);
  va_end(args);

  va_start(args,fmt);
  tmp = malloc(count+1);
  vsnprintf(tmp,count+1,fmt,args);
  va_end(args);

  yu_string_append(str, tmp, count);

  free(tmp);
}

void yu_string_append0(YUString *str, char *data) {
  yu_string_append(str,data,strlen(data));
}

void yu_string_append_at0(YUString *str, size_t pos, char *data) {
  yu_string_append_at(str,pos,data,strlen(data));
}

void yu_string_append_at(YUString *str, size_t pos, char *data, size_t len) {
  str->len = pos;
  yu_string_append(str,data,len);
}


void yu_string_append(YUString *str, char *data, size_t len) {
  int was_empty = 0;
  int was_reallocd = 0;

  if (len == 0) {
    return;
  }
  
  if (str->allocated == 0) {
    was_empty = 1;
    str->allocated = 2;
  }

  while (str->allocated < (len + str->len + 1)) {
    str->allocated <<= 1;
    was_reallocd = 1;
  }

  if (was_reallocd || was_empty) {
    if (was_empty) {
      str->str = malloc(str->allocated);
    } else {
      str->str = realloc(str->str, str->allocated);
    }
  }
  
  memmove(str->str + str->len, data, len);

  str->len += len;

  str->str[str->len] = '\0';
}

