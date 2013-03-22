#include "yupointerarray.h"

#include <string.h>

YUPointerArray *yu_pointer_array_new(int null_terminate) {
  YUPointerArray *arr;
  arr = calloc(1,sizeof(YUPointerArray));
  arr->null_terminate = null_terminate;
 
  if (null_terminate) {
    arr->data = malloc(sizeof(void*));
    arr->allocated = sizeof(void*);
  }
  
  return arr;
}

void yu_pointer_array_free(YUPointerArray *arr) {
  free(arr->data);
  free(arr);
}

void yu_pointer_array_append(YUPointerArray *arr, void *data) {
  int was_empty = 0;
  int was_reallocd = 0;
  int addition = 1;

  if (arr->allocated == 0) {
    was_empty = 1;
    arr->allocated = 2;
  }

  if (arr->null_terminate) {
    addition += 1;
  }

  while (arr->allocated < ((arr->len + addition) * sizeof(void*))) {
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

  arr->data[arr->len] = data;

  arr->len += 1;

  if (arr->null_terminate) {
    arr->data[arr->len] = 0;
  }
}
