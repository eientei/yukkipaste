#ifndef __YU_OPTIONS__
#define __YU_OPTIONS__

#include "yutils/yustring.h"
#include "yutils/yuarray.h"

typedef enum {
  OPTION_TERMINATOR,
  OPTION_ARG_NONE,
  OPTION_ARG_STRING,
  OPTION_ARG_INT,
  OPTION_ARG_LEFTOVERS,
  OPTION_ARG_POINTER_ARRAY
} YUOptionType;

typedef struct {
  char         *long_name;
  char          short_name;
  YUOptionType  arg_type;
  void         *arg_data;
  char         *description;
  char         *arg_description;
} YUOption;

typedef struct {
  char   **argv;
  int      argc;
  char    *progname;
  char    *descr;
  int      pad;
  YUArray *options;
} YUOptionParser;

YUOptionParser *yu_options_new(int argc, char **argv);
void            yu_options_add(YUOptionParser *parser, YUOption *opts);
void            yu_options_parse(YUOptionParser *parser, YUString *error);
void            yu_options_produce_help(YUOptionParser *parser, YUString *out);
void            yu_options_free(YUOptionParser *parser);

#endif /* __YU_OPTIONS__ */
