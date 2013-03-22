#ifndef __YU_STRING__
#define __YU_STRING__

#include <sys/types.h>

typedef struct {
  size_t len;
  size_t allocated;
  char * str;
} YUString;

YUString  *yu_string_new();
void       yu_string_free(YUString *str);
void       yu_string_append(YUString *str, char *data, size_t len);
void       yu_string_append0(YUString *str, char *data);
void       yu_string_append_at(YUString *str, size_t pos, char *data, size_t len);
void       yu_string_append_at0(YUString *str, size_t pos, char *data);
void       yu_string_sprintfa(YUString *str, char *fmt, ...);

#endif /* __YU_STRING__ */
