#ifndef __YU_SECTCONF__
#define __YU_SECTCONF__

#include <stdint.h>

#include "yutils/yustring.h"
#include "yutils/yuarray.h"

typedef struct {
  char *key;
  char *value;
  int   line;
} YUSectConfEntry;

typedef struct {
  char    *name;
  YUArray *entries;
} YUSectConfSection;

typedef struct {
  YUArray *sections;
} YUSectConf;

YUSectConf      *yu_sect_conf_new();
void             yu_sect_conf_free(YUSectConf *conf);
void             yu_sect_conf_parse(YUSectConf *conf, char *filepath, 
                                    YUString *err);
YUSectConfEntry  yu_sect_conf_get(YUSectConf *conf, char *section, char *key);

#endif /* __YU_SECTCONF__ */
