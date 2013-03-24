#ifndef __YU_POINTER_ARRAY__
#define __YU_POINTER_ARRAY__

#include <stdlib.h>

typedef struct {
  void   **data;
  size_t   len;
  size_t   allocated;
  int      null_terminate;
} YUPointerArray;

YUPointerArray *yu_pointer_array_new();       
void            yu_pointer_array_append(YUPointerArray *arr, void *data);
void            yu_pointer_array_free(YUPointerArray *arr);

#define  yu_pointer_array_index(arr,idx) \
  ((arr)->data[(idx)])

#endif /* __YU_POINTER_ARRAY__ */
