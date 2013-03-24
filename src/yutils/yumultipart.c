#include "yumultipart.h"

#include "yutils/yurandom.h"

#include <string.h>

YUMultipart *yu_multipart_new() {
  YUMultipart *multipart;
  multipart = calloc(1,sizeof(YUMultipart));

  multipart->pairs    = yu_array_new(sizeof(YUMultipartPair));
  multipart->boundary = yu_string_new();

  return multipart;
}

void yu_multipart_free(YUMultipart *multipart) {
  int             i;
  YUMultipartPair pair;

  for (i = 0; i < multipart->pairs->len; i++) {
    pair = yu_array_index(multipart->pairs,YUMultipartPair,i);
    free(pair.key);
    free(pair.value);
  }
  yu_array_free(multipart->pairs);
  yu_string_free(multipart->boundary);
  free(multipart);
}

void yu_multipart_append0(YUMultipart *multipart,
                          char        *key,
                          char        *value) {
  YUMultipartPair pair;

  pair.key   = strdup(key);
  pair.value = strdup(value);

  yu_array_append(multipart->pairs,&pair);
}

void yu_multipart_generate(YUMultipart *multipart, YUString *out) {
  int              met = 0;
  int              i;
  char            *p;
  YUMultipartPair  pair;
  
  do {
    yu_string_clear(multipart->boundary);
    yu_random_string_alnum(multipart->boundary,16);

    for (i = 0; i < multipart->pairs->len; i++) {
      pair = yu_array_index(multipart->pairs,YUMultipartPair,i);
      for (p = pair.key; *p != 0; p++) {
        if (strcmp(p, multipart->boundary->str) == 0) {
          met = 1;
          break;
        }
      }
      for (p = pair.value; *p != 0; p++) {
        if (strcmp(p, multipart->boundary->str) == 0) {
          met = 1;
          break;
        }
      }
      if (met) break;
    }
  } while (met);

  for (i = 0; i < multipart->pairs->len; i++) {
    pair = yu_array_index(multipart->pairs,YUMultipartPair,i);
    yu_string_sprintfa(out, "--%s\r\n", multipart->boundary->str);
    yu_string_sprintfa(out, "Content-Disposition: form-data; "
                            "name=\"%s\"\r\n\r\n", pair.key);
    yu_string_sprintfa(out,"%s\r\n",pair.value);
  }
  yu_string_sprintfa(out, "--%s--\r\n\r\n", multipart->boundary->str);
}
