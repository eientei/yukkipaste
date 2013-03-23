#ifndef __YU_MULTIPART__
#define __YU_MULTIPART__

#include "yutils/yuarray.h"
#include "yutils/yustring.h"

typedef struct {
  char     *key;
  char     *value;
} YUMultipartPair;

typedef struct {
  YUArray  *pairs;
  YUString *boundary;
} YUMultipart;

YUMultipart *yu_multipart_new();
void         yu_multipart_free(YUMultipart *multipart);
void         yu_multipart_append0(YUMultipart *multipart, 
                                  char     *key, 
                                  char     *value);
void         yu_multipart_generate(YUMultipart *multipart, YUString *out);

#endif /* __YU_MULTIPART__ */
