#include "yuarray.h"

#include <string.h>

YUArray *yu_array_new(size_t elsize) {
  YUArray *arr;
  arr = calloc(1,sizeof(YUArray));
  arr->elsize = elsize;
  return arr;
}

void yu_array_free(YUArray *arr) {
  free(arr->data);
  free(arr);
}

void yu_array_append(YUArray *arr, const void *data) {
  int was_empty = 0;
  int was_reallocd = 0;

  if (arr->allocated == 0) {
    was_empty = 1;
    arr->allocated = 2;
  }

  while (arr->allocated < ((arr->len + 1) * arr->elsize)) {
    arr->allocated <<= 1;
    was_reallocd = 1;
  }

  if (was_reallocd) {
    if (was_empty) {
      arr->data = malloc(arr->allocated);
    } else {
      arr->data = realloc(arr->data,arr->allocated);
    }
  }

  memmove(arr->data + arr->len * arr->elsize, data, arr->elsize);

  arr->len += 1;
}
