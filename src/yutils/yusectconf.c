#include "yusectconf.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

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
    if (section.name != 0) {
      free(section.name);
    }
    yu_array_free(section.entries);
  }
  yu_array_free(conf->sections);

  free(conf);
}

void yu_sect_conf_parse(YUSectConf *conf, char *filepath, YUString *err) {
  char             *p;
  char             *beg;
  int               fd;
  int               line = 0;
  char             *lineptr;
  YUSectConfSection section;
  YUSectConfEntry   entry;
  YUString         *contents;
  char              buf[BUFSIZ];
  int               count;

  fd = open(filepath,O_RDONLY);
  
  if (fd == -1) {
    yu_string_sprintfa(err, "%s: %s\n", strerror(errno), filepath);
    return;
  }

  contents = yu_string_new();

  while ((count = read(fd, buf, sizeof(buf))) > 0) {
    yu_string_append(contents, buf, count);
  }

  if (count < 0) {
    yu_string_sprintfa(err, "%s: %s\n", strerror(errno), filepath);
    goto yu_sect_conf_parse_free_and_return;
  }

  section.name    = 0;
  section.entries = yu_array_new(sizeof(YUSectConfEntry));
  
  yu_array_append(conf->sections, &section);

  for (p = contents->str; *p != 0; p++) {
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
      goto yu_sect_conf_parse_free_and_return;
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

yu_sect_conf_parse_free_and_return:
  yu_string_free(contents);
  return;
}

YUSectConfEntry yu_sect_conf_get(YUSectConf * conf, char *section, char *key) {
  int               i;
  int               n;
  YUSectConfSection sect;
  YUSectConfEntry   entry;
  

  for (i = 0; i < conf->sections->len; i++) {
    memset(&sect,0,sizeof(YUSectConfSection));
    sect = yu_array_index(conf->sections,YUSectConfSection,i);
    if (sect.name != section && 
        sect.name != 0 && section != 0 && 
        strcmp(sect.name,section) != 0) continue;
    for (n = 0; n < sect.entries->len; n++) {
      memset(&entry,0,sizeof(YUSectConfEntry));
      entry = yu_array_index(sect.entries,YUSectConfEntry,n);
      if (strcmp(entry.key,key) != 0) continue;
      return entry;
    }
  }
  memset(&entry,0,sizeof(YUSectConfEntry));
  return entry;
}
