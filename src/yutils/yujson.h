#ifndef __YU_JSON__
#define __YU_JSON__

#include "yutils/yustring.h"

int escape_json_string(YUString *out, char *in, size_t len);
int escape_json_string0(YUString *out, char *in);
int json_extract_string0(YUString *out, char *in, char *key);
int json_extract_string(YUString *out, char *in, size_t in_len, 
                                       char* key, size_t key_len);

#endif /* __YU_JSON__ */
