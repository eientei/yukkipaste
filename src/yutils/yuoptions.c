#include "yuoptions.h"
#include "yutils/yupointerarray.h"

#include <ctype.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

static int rest_is_digits(char *ch) {
  while (*ch != 0) {
    if (!isdigit(*ch++)) {
      return 0;
    }
  }
  return 1; 
}

static int eqzero_len(char *ch) {
  int r = 0;
  while (*ch != 0 && *ch != '=') {
    ch++;
    r++;
  }
  return r;
}

static int dispatch_short(YUArray  *options, 
                          char     *ch, 
                          char     *next,
                          YUString *err) {
  YUOption opt;
  int      i;
  int      ret = 0;
  int      matched = 0;
  int      behaviour = 0;

  for (i = 0; i < options->len; i++) {
    opt = yu_array_index(options, YUOption, i);
    if (opt.short_name == *ch) {
      matched = 1;
      behaviour = 0;
      switch (opt.arg_type) {
        default:
          break;
        case OPTION_ARG_NONE:
          *(int*)opt.arg_data += 1;
          continue;
          break;
        case OPTION_ARG_POINTER_ARRAY:
          behaviour = 1;
        case OPTION_ARG_STRING:
          if (*(ch+1) == 0 && next != 0) {
            if (behaviour == 0) {
               *(char**)opt.arg_data = next;
            } else {
              yu_pointer_array_append(*(YUPointerArray**)opt.arg_data,
                                      (next));
            }
            ret = 1;
          } else {
            if (err->len != 0) {
              yu_string_append0(err,"\n");
            }
            yu_string_sprintfa(err,
                               "Error: option '-%c' missing string argument",
                               *ch);
          }
          break;
        case OPTION_ARG_INT:
          if (*(ch+1) == 0 && next != 0) {
            *(int*)opt.arg_data = atoi(next);
            ret = 1;
          } else if (rest_is_digits(ch+1)) {
            *(int*)opt.arg_data = atoi(ch+1);
            ret = 2;
          } else {
            if (err->len != 0) {
              yu_string_append0(err,"\n");
            }
            yu_string_sprintfa(err,
                               "Error: option '-%c' missing integer argument",
                               *ch);
          }
          break;
      }
    }
  }
  
  if (!matched) {
    if (err->len != 0) {
      yu_string_append0(err,"\n");
    }
    yu_string_sprintfa(err,"Error: option '-%c' was not recognized", *ch);
  }
  return ret;
}

static int dispatch_long(YUArray  *options, 
                         char     *opt_arg, 
                         char     *next, 
                         YUString *err) {
  
  YUOption opt;
  int      i;
  int      ret = 0;
  int      matched = 0;
  int      len = 0;
  int      behaviour = 0;

  for (i = 0; i < options->len; i++) {
    opt = yu_array_index(options, YUOption, i);
    if (opt.long_name == 0) {
      continue;
    }
    len = eqzero_len(opt_arg);
    if (strncmp(opt.long_name, opt_arg, len) == 0) {
      matched = 1;
      behaviour = 0;
      switch (opt.arg_type) {
        default:
          break;
        case OPTION_ARG_NONE:
          *(int*)opt.arg_data += 1;
          break;
        case OPTION_ARG_POINTER_ARRAY:
          behaviour = 1;
        case OPTION_ARG_STRING:
          if (*(opt_arg+len) == '=') {
            if (behaviour == 0) {
              *(char**)opt.arg_data = opt_arg+len+1;
            } else {
              yu_pointer_array_append(*(YUPointerArray**)opt.arg_data,
                                      (opt_arg+len+1));
            }
            ret = 2;
          } else if (next != 0) {
            if (behaviour == 0) {
              *(char**)opt.arg_data = next;
            } else {
              yu_pointer_array_append(*(YUPointerArray**)opt.arg_data,
                                      (next));
            }
            ret = 1;
          } else {
            if (err->len != 0) {
              yu_string_append0(err,"\n");
            }
            yu_string_sprintfa(err, 
                               "Error: option '--%s' missing string argument",
                               opt.long_name);
          }
          break;
        case OPTION_ARG_INT:
          if (*(opt_arg+len) == '=') {
            *(int*)opt.arg_data = atoi(opt_arg+len+1);
            ret = 2;
          } else if (next != 0) {
            *(int*)opt.arg_data = atoi(next);
            ret = 1;
          } else {
            if (err->len != 0) {
              yu_string_append0(err,"\n");
            }
            yu_string_sprintfa(err, 
                               "Error: option '--%s' missing integer argument",
                               opt.long_name);
          }
          break;
      }
    }
  }
  if (!matched) {
    if (err->len != 0) {
      yu_string_append0(err,"\n");
    }
    yu_string_sprintfa(err,"Error: option '--%s' was not recognized", opt_arg);
  }
  return ret;
}

YUOptionParser *yu_options_new(int argc, char **argv) {
  YUOptionParser *parser;
  parser = calloc(1, sizeof(YUOptionParser));

  parser->argv = argv;
  parser->argc = argc;
  parser->progname = argv[0];
  parser->descr = "[OPTIONS...]";
  parser->options = yu_array_new(sizeof(YUOption));
  parser->pad = 4;
  return parser;
}

void yu_options_free(YUOptionParser *parser) {
  yu_array_free(parser->options);
  free(parser);
}

void yu_options_add(YUOptionParser *parser, YUOption *opts) {
  while (opts->arg_data != 0 || opts->arg_type != OPTION_TERMINATOR) {
    yu_array_append(parser->options, opts);
    opts++;
  }
}

void yu_options_parse(YUOptionParser *parser, YUString *error) {
  YUOption  opt;
  int       i;
  int       r;
  int       ri;
  char     *tmp;
  int       consumed_next = 0;
  YUArray  *leftover_idxs;
  char     *opt_arg;
  char     *next;
  
  leftover_idxs = yu_array_new(sizeof(int));

  for (i = 1; i < parser->argc; i++) {
    if (consumed_next) {
      consumed_next = 0;
      continue;
    }
    opt_arg = parser->argv[i];
    if (i+1 < parser->argc) {
      next = parser->argv[i+1];
    } else {
      next = 0;
    }
    if (*opt_arg == '-' && *(opt_arg+1) != 0) {
      if (*(opt_arg+1) != '-') {
        opt_arg++;
        while (*opt_arg != 0) {
          consumed_next = dispatch_short(parser->options, opt_arg++, next, error);
          if (consumed_next == 2) {
            consumed_next = 0;
            break;
          }
        }
        continue;
      }
      opt_arg += 2;
      consumed_next = dispatch_long(parser->options, opt_arg, next, error); 
    } else {
      yu_array_append(leftover_idxs,&i);
    }
  }

  r = parser->argc - 1;

  for (i = leftover_idxs->len - 1; i >= 0; i--) {
    tmp = parser->argv[r];
    ri = yu_array_index(leftover_idxs, int, i);
    parser->argv[r] = parser->argv[ri];
    parser->argv[ri] = tmp;
    r--;
  }

  for (i = 0; i < parser->options->len; i++) {
    opt = yu_array_index(parser->options, YUOption, i);
    if (opt.arg_type == OPTION_ARG_LEFTOVERS) {
      *(char***)opt.arg_data = parser->argv + r+1;
    }
  }

  yu_array_free(leftover_idxs);
}

void yu_options_produce_help(YUOptionParser *parser, YUString *out) {
  YUOption       opt;
  int            total_len;
  int            intrm_len;
  int            len;
  int            long_maxlen = 0;
  int            descr_maxlen = 0;
  int            arg_descr_maxlen = 0;
  int            prefix_len = 0;
  int            descr_len = 0;
  int            was_shorts = 0;
  int            i;
  int            n;
  int            m;
  int            termwidth = 80;
  char          *p;
  char          *beg;
  struct winsize w;
  

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  if (w.ws_col > 0) {
    termwidth = w.ws_col;
  }
  
  for (i = 0; i < parser->options->len; i++) {
    opt = yu_array_index(parser->options, YUOption, i);
    if (!opt.description) continue;
    if (opt.short_name) {
      was_shorts = 1;
    }
    if (opt.long_name) {
      len = strlen(opt.long_name);
      if (len > long_maxlen) long_maxlen = len;
    }
    if (opt.description) {
      len = strlen(opt.description);
      if (len > descr_maxlen) descr_maxlen = len;
    }
    if (opt.arg_description) {
      len = strlen(opt.arg_description);
      if (len > arg_descr_maxlen) arg_descr_maxlen = len;
    }
  }
  
  prefix_len = parser->pad * 2;

  if (arg_descr_maxlen > 0) {
    prefix_len += arg_descr_maxlen + 2; /* +2 for spaces */
  }

  if (long_maxlen > 0) {
    prefix_len += long_maxlen + 2; /* +2 for -- */
  }
  
  if (was_shorts) {
    prefix_len += 2; /* '-o' */
  }

  if (was_shorts && long_maxlen > 0) {
    prefix_len += 2; /* ', ' */
  }

  descr_len = termwidth - prefix_len;

  yu_string_sprintfa(out, "%s %s\n\n", parser->progname, parser->descr);
  for (i = 0; i < parser->options->len; i++) {
    opt = yu_array_index(parser->options, YUOption, i);
    if (!opt.description) continue;

    for (n = 0; n < parser->pad; n++) {
      yu_string_append0(out, " ");
    }
    
    if (opt.long_name == 0 && long_maxlen > 0) {
      for (n = 0; n < long_maxlen+2; n++) {
        yu_string_append0(out, " ");
      }
    } else if (opt.long_name != 0) {
      yu_string_append0(out, "--");
      yu_string_append0(out, (char*)opt.long_name);
    }
    
    if (was_shorts && long_maxlen > 0) {
      if (opt.long_name == 0 || opt.short_name == 0) {
        yu_string_append0(out, "  ");
      } else {
        yu_string_append0(out, ", ");
      }
      if (opt.long_name == 0) {
        len = long_maxlen;
      } else {
        len = strlen(opt.long_name);
      }
      for (n = 0; n < long_maxlen-len; n++) {
        yu_string_append0(out, " ");
      }
    }

    if (opt.short_name == 0 && was_shorts) {
      yu_string_append0(out,"  ");
    }

    if (opt.short_name != 0) {
      yu_string_sprintfa(out,"-%c",opt.short_name);
    }

    if (arg_descr_maxlen > 0 && opt.arg_description == 0) {
      for (n = 0; n < arg_descr_maxlen+2; n++) {
        yu_string_append0(out, " ");
      }
    }

    if (opt.arg_description != 0) {
      yu_string_append0(out, " ");
      yu_string_append0(out, (char*)opt.arg_description);   
      len = strlen(opt.arg_description);
      for (n = 0; n < arg_descr_maxlen-len; n++) {
        yu_string_append0(out, " ");
      }
      yu_string_append0(out, " ");
    }

    for (n = 0; n < parser->pad; n++) {
      yu_string_append0(out, " ");
    }

    p = opt.description;
    while (isspace(*p) && *p != 0) p++;
    beg = p;
    n = 0;
    intrm_len = 0;

    while (*p != 0) {
      p++;
      if (isspace(*p) || *p == 0) {
          intrm_len += p-beg + 1;
          if (intrm_len > descr_len) {
            yu_string_append0(out,"\n");
            for (n = 0; n < prefix_len; n++) {
              yu_string_append0(out, " ");
            }
            
          }
          yu_string_append(out,beg,p-beg);
          yu_string_append0(out," ");
          while (isspace(*p) && *p != 0) p++;
          beg = p;
      }
    }

    /*
    for (n = 0; n < total_len; n += descr_len) {
      intrm_len = total_len - n;
      if (intrm_len > descr_len) {
        intrm_len = descr_len;
      }
      yu_string_append(out,(char*)opt.description+n,intrm_len);
      if ((n + descr_len) < total_len) {
        yu_string_append0(out,"\n");
        for (m = 0; m < prefix_len; m++) {
          yu_string_append0(out," ");
        }
      }
    }
    */

    yu_string_append0(out,"\n");
  }
}
