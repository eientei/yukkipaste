#include "yuutf.h"

#include <unistd.h>

/* debugging function */
void print_bytes(int c, int len) {
  unsigned int x = 1;
  int i;
  
  x <<= (len-1);
  
  for (i = 0; i < len; i++) {
    if (c&x) {
      write(1,"1",1);
    } else {
      write(1,"0",1);
    }
    x>>=1;
  }
  write(1,"\n",1);
}



/**
 * This function returns uint32 representation of unicode character or
 * (uint32_t)-1 on parsing failure.
 **/
uint32_t next_utf8_char(char *beg, char *end, int *skiplen) {
  /* available bytes */
  int siz = end-beg;
  *skiplen = 0;

  if (siz > 0 && !(*beg & 0x80)) {
    /* since character is 7-bits only, return it without any modifications */
    *skiplen = 1;
    return *beg;
  } else if (siz > 1 && (*beg & 0xE0) == 0xC0) {
    /* 2-byte character mark */
    if ((((*(beg+1) & 0xFF) << 0) & 0xC0) == 0x80) {
      /* there is a valid continuation character */
      *skiplen = 2;
      return ((*(beg+0) & 31) << 6)|
             ((*(beg+1) & 63) << 0);
    } else {
      /* there is no valid continuation character */
      return -1;
    }
  } else if (siz > 2 && (*beg & 0xF0) == 0xE0) {
    /* 3-byte character mark */
    if (((*(uint16_t*)(beg+1)) & 0xC0C0) == 0x8080) {
      /* there is a two valid continuation characters */
      *skiplen = 3;
      return ((*(beg+0) & 15) << 12)|
             ((*(beg+1) & 63) <<  6)|
             ((*(beg+2) & 63) <<  0);
    } else {
      /* there is no[t enough] valid continuation characters */
      return -1;
    }
  } else if (siz > 3 && (*beg & 0xF8) == 0xF0) {
    /* 4-byte character mark */
    /* couldn't thought of anything better than this */
    if (((((*(beg+1) & 0xFF) << 16) | 
          ((*(beg+2) & 0xFF) <<  8) |
          ((*(beg+3) & 0xFF) <<  0)) & 0xC0C0C0) == 0x808080) {
      /* there is three valid continuation characters */
      *skiplen = 4;
      return ((*(beg+0) &  7) << 18)|
             ((*(beg+1) & 63) << 12)|
             ((*(beg+2) & 63) <<  6)|
             ((*(beg+3) & 63) <<  0);
    } else {
      /* there is no[t enough] valid continuation characters */
      return -1;
    }
  } else if (siz > 4 && (*beg & 0xFC) == 0xF8) {
    /* 5-byte character mark */
    if (((*(uint32_t*)(beg+1)) & 0xC0C0C0C0) == 0x80808080) {
      /* there is four valid continuation characters */
      *skiplen = 5;
      return ((*(beg+0) &  3) << 24)|
             ((*(beg+1) & 63) << 18)|
             ((*(beg+2) & 63) << 12)|
             ((*(beg+3) & 63) <<  6)|
             ((*(beg+4) & 63) <<  0);
    } else {
      /* there is no[t enough] valid continuation characters */
      return -1;
    }
  } else if (siz > 5 && (*beg & 0xFE) == 0xFC) {
    /* 6-byte character mark */
    if (((*(uint32_t*)(beg+1)) & 0xC0C0C0C0) == 0x80808080 &&
         (((*(beg+5) & 0xFF) <<  0)  & 0xC0) == 0x80) {
      /* there is five valid continuation characters */
      *skiplen = 6;
      return ((*(beg+0) &  1) << 30)|
             ((*(beg+1) & 63) << 24)|
             ((*(beg+2) & 63) << 18)|
             ((*(beg+3) & 63) << 12)|
             ((*(beg+4) & 63) <<  6)|
             ((*(beg+5) & 63) <<  0);
    } else {
      /* there is no[t enough] valid continuation characters */
      return -1;
    }
  }

  /* fallback to failure */
  return -1;
}

