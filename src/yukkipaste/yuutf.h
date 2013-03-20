#ifndef __YU_UTF__
#define __YU_UTF__

#include <stdint.h>

void print_bytes(int c, int len);
uint32_t next_utf8_char(char *beg, char *end, int * skiplen);

#endif /* __YU_UTF__ */
