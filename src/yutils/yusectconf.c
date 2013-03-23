#include "yusectconf.h"

#include <string.h>
#include <ctype.h>

YUSectConf *yu_sect_conf_new() {
  YUSectConf *conf;

  conf = calloc(1,sizeof(YUSectConf));

  conf->sections = yu_array_new(sizeof(YUSectConfSection));

  return conf;
}

void yu_sect_conf_free(YUSectConf *conf) {
  int               i;
  int               n;
  YUSectConfSection section;
  YUSectConfEntry   entry;
  
  for (i = 0; i < conf->sections->len; i++) {
    section = yu_array_index(conf->sections,YUSectConfSection,i);
    for (n = 0; n < section.entries->len; n++) {
      entry = yu_array_index(section.entries,YUSectConfEntry,n);
      free(entry.key);
      free(entry.value);
    }
    free(section.name);
    yu_array_free(section.entries);
  }
  yu_array_free(conf->sections);

  free(conf);
}

void yu_sect_conf_parse(YUSectConf *conf, char *data, YUString *err) {
  char             *p;
  char             *beg;
  int               line = 0;
  char             *lineptr;
  YUSectConfSection section;
  YUSectConfEntry   entry;

  section.name    = strdup("global");
  section.entries = yu_array_new(sizeof(YUSectConfEntry));
  
  yu_array_append(conf->sections, &section);

  for (p = data; *p != 0; p++) {
    line++;
    lineptr = p;
    if (*p == '\n') continue;
    while (isblank(*p) && *p != 0) p++;
    if (*p == '#') {
      while (*p != '\n' && *p != 0) p++;
      continue;
    }
    if (*p == '[') {
      p+=1;
      beg = p;
      while (*p != '\n' && *p != 0 && *p != ']') p++;
      section.name    = strndup(beg, p-beg);
      section.entries = yu_array_new(sizeof(YUSectConfEntry));
      yu_array_append(conf->sections, &section);

      while (*p != '\n' && *p != 0) p++;
      continue;
    }
    beg = p;
    while (!isblank(*p) && *p != 0 && *p != '\n' && *p != '=') p++;
    entry.key = strndup(beg, p-beg);
    while (isblank(*p) && *p != 0 && *p != '\n' && *p != '=') p++;
    if (*p != '=') {
      free(entry.key);
      yu_string_sprintfa(err,"line %d col %d - equal sign (=) expected",
                         line, p-lineptr);
      return;
    }
    p++;
    while (isblank(*p) && *p != 0 && *p != '\n') p++;
    beg = p;
    while (!isblank(*p) && *p != 0 && *p != '\n') p++;
    entry.value = strndup(beg, p-beg);
    entry.line  = line;
    yu_array_append(section.entries,&entry);
    while (*p != '\n' && *p != 0) p++;
  }
}

YUSectConfEntry yu_sect_conf_get(YUSectConf * conf, char *section, char *key) {
  int               i;
  int               n;
  YUSectConfSection sect;
  YUSectConfEntry   entry;
  
  memset(&entry,0,sizeof(YUSectConfEntry));

  for (i = 0; i < conf->sections->len; i++) {
    sect = yu_array_index(conf->sections,YUSectConfSection,i);
    if (strcmp(sect.name,section) != 0) continue;
    for (n = 0; n < sect.entries->len; n++) {
      entry = yu_array_index(sect.entries,YUSectConfEntry,n);
      if (strcmp(entry.key,key) != 0) continue;
      return entry;
    }
  }
  return entry;
}
