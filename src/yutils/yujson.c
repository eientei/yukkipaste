#include "yujson.h"

#include "yutils/yuutf.h"

#include <string.h>

int escape_json_string0(YUString *out, char *in) {
  return escape_json_string(out,in,strlen(in));
}

int escape_json_string(YUString *out, char *in, size_t len) {
  unsigned char *p;
  unsigned char *oldp;
  unsigned char *end;
  int skiplen;
  uint32_t unichar;

  p = (unsigned char*)in;
  end = p + len;

  while (p != end) {
    unichar = next_utf8_char((char*)p,(char*)end,&skiplen);
    if (skiplen == 0) {
      yu_string_sprintfa(out, "\\u%04x",*p++);
      continue;
    }

    oldp = p;
    p += skiplen;
    switch (unichar) {
      case  ' ': yu_string_append0(out,  " "); continue;
      case '\b': yu_string_append0(out,"\\b"); continue;
      case '\f': yu_string_append0(out,"\\f"); continue;
      case '\n': yu_string_append0(out,"\\n"); continue;
      case '\r': yu_string_append0(out,"\\r"); continue;
      case '\t': yu_string_append0(out,"\\t"); continue;
      case '\v': yu_string_append0(out,"\\v"); continue;
      case  '"': yu_string_append0(out,"\\\"");continue;
      case '\\': yu_string_append0(out,"\\\\");continue;
    }
    if (unichar < 32) {
      yu_string_sprintfa(out, "\\u%04x",unichar);
      continue;
    }
    yu_string_append(out,(char*)oldp,(size_t)skiplen);
  }

  return 0;
}

int json_extract_string0(YUString *out, char *in, char *key) {
  return json_extract_string(out, in, strlen(in), key, strlen(key));
}

int json_extract_string(YUString *out, char *in, size_t in_len,
                                       char* key, size_t key_len) {
  char *beg;
  char *in_end = in + in_len;
  char *p;

  for (p = in; p != in_end; p++) {
    if (strncmp(p,key,key_len) == 0) {
      p += key_len+1;
      while (*p != ':' && p != in_end) p++;
      if (p == in_end) return 1;
      p++;
      while (*p == ' ' && p != in_end) p++;
      if (p == in_end) return 1;
      if (strncmp(p,"null",4) == 0) return 0;
      p++;
      beg = p;
      while (p != in_end && (*p != '"' || *(p-1) == '\\')) p++;
      if (p == in_end) return 1;
      yu_string_append(out,beg,p-beg);
      return 0;
    }
  }

  return 0;
}

