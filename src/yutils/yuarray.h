#ifndef __YU_ARRAY__
#define __YU_ARRAY__

#include <stdlib.h>

typedef struct {
  char   *data;
  size_t  len;
  size_t  allocated;
  size_t  elsize;
} YUArray;

YUArray *yu_array_new(size_t elsize);       
void     yu_array_append(YUArray *arr, const void *data);
void     yu_array_free(YUArray *arr);

#define  yu_array_index(arr,type,idx) \
  *(type*)((arr)->data + (arr)->elsize * (idx))

#endif /* __YU_ARRAY__ */
