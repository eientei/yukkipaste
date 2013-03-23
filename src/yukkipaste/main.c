#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>

#include "config.h"

#ifdef HAVE_LIBMAGIC
#include <magic.h>
#endif


#include "yutils/yustring.h"
#include "yutils/yuutf.h"
#include "yutils/yulog.h"
#include "yutils/yupointerarray.h"
#include "yutils/yuoptions.h"
#include "yutils/yumacro.h"
#include "yutils/yusoundex.h"
#include "yukkipaste-api/yukkipaste-module.h"

#include "moduleinfo.h"

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

static char* headers =
 "POST %s%s HTTP/1.1\r\n"
 "User-Agent: yukkipaste 1.1\r\n"
 "Host: %s:%s\r\n"
 "Accept: */*\r\n"
 "Content-Type: %s\r\n"
 "Content-Length: %d\r\n"
 "\r\n";

static char                   buf[BUFSIZ];
static YULog                 *log_domain = 0;
static YUArray               *modules = 0;
static YUPointerArray        *module_paths = 0;
static char                 **args = 0;
static int                    print_help = 0;
static int                    list_modules = 0;
static int                    verbosity_flag = 0;
static char                  *pastebin_uri = 0;
static char                  *language_name = 0;
static char                  *remote_filename = 0;
static char                  *parent_id_name = "";
static int                    is_paste_private = 0;
static char                  *mime_type = 0;
static char                  *pastebin_host = 0;
static char                  *pastebin_port = 0;
static char                  *pastebin_root = 0;
static int                    source_fd = -1;
static char                  *source_name = 0;
static YUString              *file_data = 0;
static char                  *module_to_use = 0;
static char                  *author_name = "";
static char                  *mime = 0;
static YukkipasteModuleInfo   active_module;
static YUString              *transmit_data = 0;
static YUString              *post_path = 0;
static int                    sock_fd = 0;
static YUString              *reply = 0;
static YUString              *request_headers = 0;
static YUString              *content_type = 0;
static int                    run_code = 0;
static int                    list_languages = 0;

/* Module process reply output arguemnts */
static YUString              *mod_proc_reply_out = 0;

static void signal_cleanup(int sig);
static void cleanup(void);
static int init(void);
static int parse_options(int argc, char **argv);
static int action_list_modules(void);
static int scan_module_dirs(void);
static int parse_uri(void);
static int open_file(void);
static int read_file(void);
static int select_active_module(void);
static int mod_init(void);
static int mod_form_request(void);
static int open_socket(void);
static int form_headers(void);
static int transfer_data(void);
static int mod_process_reply(void);
static int parse_language(void);
static int action_list_languages(void);

static YUOption options[] =
{
  { "help", 'h', OPTION_ARG_NONE, &print_help, 
    "Prints help message", 0 },

  { "modules-dir", 'd', OPTION_ARG_POINTER_ARRAY, &module_paths, 
    "Appends module path. Stackable.", "DIR" },
  
  { "list-modules", 0, OPTION_ARG_NONE, &list_modules,
    "Lists available modules", 0 },

  { "verbose", 'v', OPTION_ARG_NONE, &verbosity_flag,
    "Increases verbosity level. 3 max.", 0 },

  { "author", 'a', OPTION_ARG_STRING, &author_name,
    "Your name", "STRING" },

  { "uri", 'u', OPTION_ARG_STRING, &pastebin_uri,
    "Pastebin URI", "URI" },

  { "language", 'l', OPTION_ARG_STRING, &language_name,
    "Paste language", "LANG" },

  { "list-languages", 0, OPTION_ARG_NONE, &list_languages,
    "Lists avialable languages for selected module", 0 },

  { "remote-name", 'n', OPTION_ARG_STRING, &remote_filename,
    "Remote file name", "NAME" },

  { "mime-type", 0, OPTION_ARG_STRING, &mime_type,
    "Paste mime type", "MIME" },

  { "module", 'm', OPTION_ARG_STRING, &module_to_use, 
    "Selects module to use", "MODULE" },

  { "parent-id", 'p', OPTION_ARG_STRING, &parent_id_name,
    "Parent paste id", "ID" },

  { "secret", 's', OPTION_ARG_NONE, &is_paste_private,
    "Marks a secret (private) paste", 0, },
  
  { "run", 'r', OPTION_ARG_NONE, &run_code,
    "Marks paste as runnable on pastebin side", 0, },

  { 0, 0, OPTION_ARG_LEFTOVERS, &args, 0, 0},
  
  { 0, 0, 0, 0, 0, 0 }
};

int main(int argc, char ** argv) {
  atexit(cleanup);
  signal(SIGINT, signal_cleanup);
  signal(SIGSEGV, signal_cleanup);

  if (init() != 0) {
    return 1;
  }

  if (parse_options(argc,argv) != 0) {
    return 1;
  }

  if (scan_module_dirs() != 0) {
    return 1;
  }

  if (list_modules) {
    action_list_modules();
    return 0;
  }

  if (select_active_module() != 0) {
    return 1;
  }

  if (list_languages) {
    action_list_languages();
    return 0;
  }

  if (parse_uri() != 0) {
    return 1;
  }

  if (parse_language() != 0) {
    return 1;
  }

  if (open_file() != 0) {
    return 1;
  }

  if (read_file() != 0) {
    return 1;
  }

  if (mod_init() != 0) {
    return 1;
  }

  if (mod_form_request() != 0) {
    return 1;
  }

  if (form_headers() != 0) {
    return 1;
  }

  if (open_socket() != 0) {
    return 1;
  }

  if (transfer_data() != 0) {
    return 1;
  }

  if (mod_process_reply() != 0) {
    return 1;
  }
  
  return 0;
}

     static int action_list_languages(void) {
  char **l;

  for (l = active_module.PASTEBIN_AVAIL_LANGS; *l != 0; l++) {
    log_msg(log_domain, "    - %s\n",*l);
  }
  return 0;
}

static int parse_language(void) {
  char             soundex[5];
  YUPointerArray  *variants;
  char           **l;
  int              ret = 0;
  int              i;

  if (language_name == 0) {
    language_name = active_module.PASTEBIN_LANG;
  }

  for (l = active_module.PASTEBIN_AVAIL_LANGS; *l != 0; l++) {
    if (strcasecmp(*l,language_name) == 0) {
      language_name = *l;
      return 0;
    }
  }

  variants = yu_pointer_array_new(0);
  strncpy(soundex,yu_soundex(language_name),5);
  
  for (l = active_module.PASTEBIN_AVAIL_LANGS; *l != 0; l++) {
    if (strcmp(yu_soundex(*l),soundex) == 0) {
      yu_pointer_array_append(variants,*l);
    }
  }

  if (variants->len == 0) {
    ret = 1;
    log_error(log_domain, "Language \"%s\" is not available from module \"%s\"\n",
                          language_name, active_module.MODULE_NAME);
    goto parse_language_free_and_return;
  }

  if (variants->len == 1) {
    language_name = yu_pointer_array_index(variants,0);
    goto parse_language_free_and_return;
  }

  log_msg(log_domain, "Ambigous language variants: \n");
  for (i = 0; i < variants->len; i++) {
    log_msg(log_domain, "    - %s\n", yu_pointer_array_index(variants,i));
  }

  ret = 1;

parse_language_free_and_return:
  yu_pointer_array_free(variants);
  return ret;
}

static int mod_process_reply(void) {
  int ret;
  ret = active_module.PROCESS_REPLY_FUNC(reply->str, mod_proc_reply_out);
  log_msg(log_domain, "%s\n", mod_proc_reply_out->str);
  return ret;
}

static int transfer_data(void) {
  int          count = 0;
  char        *p;
  char        *end;
  char        *reply_begin = 0;
  static char  cl[] = "Content-Length: ";
  int          length = 0;
  YUString    *tmp = 0;

  log_trace(log_domain, "%s\n", request_headers->str);

  while ((count = send(sock_fd,
                       request_headers->str+count,
                       request_headers->len-count,0)) > 0);
  if (count < 0) {
    log_error(log_domain,"send: %s\n", strerror(errno));
    return 1;
  }
  
  count = 0;
  
  log_trace(log_domain, "%s\n", transmit_data->str);
  while ((count = send(sock_fd,
                       transmit_data->str+count,
                       transmit_data->len-count,0)) > 0);
    
  if (count < 0) {
    log_error(log_domain,"send: %s\n", strerror(errno));
    return 1;
  }

  count = 0;

  tmp = yu_string_new();

  while ((count = recv(sock_fd,buf,sizeof(buf),0)) > 0) {
    yu_string_append(tmp,buf,count);
    p = tmp->str;
    end = p + tmp->len;
    while (p != end) {
      if (strncmp(p,cl,sizeof(cl)-1) == 0) {
        p += sizeof(cl)-1;
        length = atoi(p);
        while (strncmp(p,"\r\n\r\n",4) != 0) p++;
        p += 4;
        if (strnlen(p,length) >= length) {
          reply_begin = p;
          close(sock_fd);
        }
        break;
      }
      p++;
    }
  }
  log_trace(log_domain, "%s\n", tmp->str);

  yu_string_append(reply,reply_begin,length);

  yu_string_free(tmp);
  return 0;
}

static int form_headers(void) {
  
  
  yu_string_sprintfa(request_headers,headers,
                     pastebin_root,
                     post_path->str,
                     pastebin_host,
                     pastebin_port,
                     content_type->str,
                     transmit_data->len);

  return 0;
}

static int open_socket(void) {
  int              status;
  struct addrinfo  hints;
  struct addrinfo *res;
  int              ret = 0;

  memset(&hints,0,sizeof(hints));

  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  if ((status = getaddrinfo(pastebin_host,pastebin_port,&hints,&res)) != 0) {
    log_error(log_domain,"%s\n",gai_strerror(status));
    return 1;
  }

  sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (sock_fd == -1) {
    log_error(log_domain,"socket: %s\n",strerror(errno));
    goto open_socket_free_and_return;
  }

  if (connect(sock_fd, res->ai_addr, res->ai_addrlen) == -1) {
    log_error(log_domain,"connect: %s\n",strerror(errno));
    goto open_socket_free_and_return;
  }

open_socket_free_and_return:
  freeaddrinfo(res);
  return ret;
}

static int mod_form_request(void) {
  return active_module.FORM_REQUEST_FUNC(post_path,
                                         content_type,
                                         transmit_data);

}

static int mod_init(void) {
  *active_module.PTR_LANG       = language_name;
  *active_module.PTR_URI        = pastebin_uri;
  *active_module.PTR_FILENAME   = source_name;
  *active_module.PTR_MIME       = mime;
  *active_module.PTR_PARENT     = parent_id_name;
  *active_module.PTR_AUTHOR     = author_name;
  *active_module.PTR_BODY       = file_data->str;
  *active_module.PTR_PRIVATE    = is_paste_private;
  *active_module.PTR_RUN        = run_code;
  *active_module.PTR_LOG_DOMAIN = log_domain;
 
  return active_module.INIT_MODULE_FUNC();
}

static int select_active_module(void) {
  int i;
  YukkipasteModuleInfo m;

  if (modules->len == 0) {
    log_error(log_domain, "No modules were loaded. Check -d option for use\n");
    return 1;
  }
  if (module_to_use == 0) {
    active_module = yu_array_index(modules,YukkipasteModuleInfo,0);
    return 0;
  }

  for (i = 0; i < modules->len; i++) {
    m = yu_array_index(modules,YukkipasteModuleInfo,i);
    if (strcmp(m.MODULE_NAME, module_to_use) == 0) {
      active_module = m;
      return 0;
    }
  }

  log_error(log_domain, "Wasn't able to match a module named \"%s\"\n",
            module_to_use);

  log_error(log_domain, "Consider reviewing --list-modules contnets\n");
  return 1; 
}

static int read_file(void) {
  int         count;
#ifdef HAVE_LIBMAGIC
  magic_t     magic;
  const char *m;
#endif

  if (source_fd != STDIN_FILENO && HAVE_LIBMAGIC_TEST && mime_type == 0) {
#ifdef HAVE_LIBMAGIC
    magic = magic_open(MAGIC_SYMLINK | MAGIC_MIME_TYPE);
    if (magic == 0) {
      log_warn(log_domain,"%s\n",magic_error(magic));
      goto mime_fallback;
    }
    magic_load(magic,0);
    m = magic_file(magic,source_name);
    if (m == 0) {
      log_warn(log_domain,"%s\n",magic_error(magic));
      magic_close(magic);
      goto mime_fallback;
    }
    mime = strdup((char*)m);
    magic_close(magic);
#endif
  } else {
mime_fallback:
    if (mime_type == 0) {
      mime = strdup("text/plain");
    } else {
      mime = strdup(mime_type);
    }
  }

  while ((count = read(source_fd, buf, sizeof(buf))) > 0) {
    yu_string_append(file_data, buf, count);
  }

  if (count < 0) {
    log_error(log_domain, "%s: %s\n", strerror(errno), source_name);
  }

  if (file_data->len == 0) {
    log_error(log_domain, "Empty input, aborting.\n");
    return 1;
  }

  return 0;
}

static int open_file(void) {
  char       *arg;

  if (args[0] == 0) {
    source_fd = STDIN_FILENO;
    if (remote_filename == 0) {
      source_name = "stdin";
    }
  } else {
    arg = args[0];
    if (strcmp("-", arg) == 0) {
      source_fd = STDIN_FILENO;
      source_name = "stdin";
    } else {
      source_fd = open(arg,O_RDONLY);
      if (source_fd == -1) {
        log_error(log_domain, "%s: %s\n", strerror(errno), arg);
        return 1;
      }
      source_name = arg;
    }
  }

  return 0;
}

static int parse_uri(void) {
  char *host_beg = 0;
  char *host_end = 0;
  char *port_beg = 0;
  char *port_end = 0;
  char *path_beg = 0;
  
  if (pastebin_uri == 0) {
    pastebin_uri = active_module.PASTEBIN_URI;
  }
  host_beg = pastebin_uri;
  if (strncmp("http://",pastebin_uri,7) == 0) {
    host_beg += 7;
  }

  host_end = host_beg;

  while (*host_end != ':' && *host_end != '/' && *host_end != '\0') host_end++;

  if (host_beg == host_end) {
    log_error(log_domain,"Empty host in URI\n");
    return 1;
  }

  pastebin_host = strndup(host_beg, host_end - host_beg);

  path_beg = host_end;
  
  if (*host_end == ':') {
    port_beg = host_end+1;
    port_end = port_beg;

    while (*port_end != '/' && *port_end != '\0') port_end++;
  
    path_beg = port_end;
    
    if (port_beg == port_end) {
      log_error(log_domain,"Empty port in URI\n");
      return 1;
    }

    pastebin_port = strndup(port_beg, port_end - port_beg);
  } else {
    pastebin_port = strdup("80");
  }

  pastebin_root = strdup(path_beg);
  return 0;
}

static int action_list_modules(void) {
  YukkipasteModuleInfo  m;
  YUString             *fmt;
  int                   name_maxlen = 4;    /* "name" */
  int                   descr_maxlen = 11;  /* "description" */
  int                   author_maxlen = 5;  /* "author" */
  int                   version_maxlen = 3; /* "ver" */
  static int            pad = 4;
  int                   len;
  int                   i;

  fmt = yu_string_new();

  for (i = 0; i < modules->len; i++) {
    m = yu_array_index(modules,YukkipasteModuleInfo,i);
    len = strlen(m.MODULE_NAME);
    if (len > name_maxlen) name_maxlen = len;

    len = strlen(m.MODULE_DESCRIPTION);
    if (len > descr_maxlen) descr_maxlen = len;

    len = strlen(m.MODULE_AUTHOR);
    if (len > author_maxlen) author_maxlen = len;

    len = strlen(m.MODULE_VERSION);
    if (len > version_maxlen) version_maxlen = len;
  }


  yu_string_sprintfa(fmt,"%%%ds %%%ds %%%ds %%%ds\n",name_maxlen    + pad,
                                                     version_maxlen + pad,
                                                     author_maxlen  + pad, 
                                                     descr_maxlen   + pad);


  log_msg(log_domain, fmt->str, "name", "ver", "author", "description");
  for (i = 0; i < modules->len; i++) {
    m = yu_array_index(modules,YukkipasteModuleInfo,i);
    log_msg(log_domain, fmt->str, m.MODULE_NAME, m.MODULE_VERSION,
                                  m.MODULE_AUTHOR, m.MODULE_DESCRIPTION);
  }

  yu_string_free(fmt);
  return 0;
}

static int scan_module_dirs(void) {
  YukkipasteModuleInfo  module_info;
  YUString             *fullpath;
  int                   i;
  int                   len;
  char                 *path;
  DIR                  *directory;
  struct dirent        *entry;
  struct stat           info;
  uid_t                 uid;
  uid_t                 gid;
  void                 *fptr;

  uid = getuid();
  gid = getgid();

  fullpath = yu_string_new();

  for (i = 0; i < module_paths->len; i++) {
    path = yu_pointer_array_index(module_paths,i);
    directory = opendir(path);
    if (directory == 0) {
      log_debug(log_domain, "%s: %s\n", strerror(errno), path);
      continue;
    }
    yu_string_append0(fullpath,path);
    if (fullpath->str[fullpath->len-1] != '/') {
      yu_string_append0(fullpath,"/");
    }
    len = fullpath->len;
    while ((entry = readdir(directory)) != 0) {
      if (strcmp(".", entry->d_name) == 0 &&
          strcmp(".", path) != 0) continue;
      if (strcmp("..", entry->d_name) == 0 &&
          strcmp("..", path) != 0) continue;
      yu_string_append_at0(fullpath, len, entry->d_name);
      if (stat(fullpath->str,&info) != 0) {
        log_debug(log_domain, "%s: %s\n", strerror(errno), fullpath->str);
        continue;
      }
      if (!S_ISREG(info.st_mode)) {
        log_debug(log_domain, "File is not a regular file: %s\n", fullpath->str);
        continue;
      }

      if (uid == info.st_uid && info.st_mode & S_IRUSR) {
        /* ok */
      } else if (gid == info.st_gid && info.st_mode & S_IRGRP) {
        /* ok */
      } else if (info.st_mode & S_IROTH) {
        /* ok */
      } else {
        /* fail */
        log_debug(log_domain, "File is not readable: %s\n", fullpath->str);
        continue;
      }
   
      /* everything is ok */
      memset(&module_info,0,sizeof(YukkipasteModuleInfo));
      module_info.handle = dlopen(fullpath->str, RTLD_LAZY | RTLD_LOCAL);

      if (module_info.handle == 0) {
        log_debug(log_domain, "%s\n", dlerror());
        continue;
      }

#define LOAD_SYM_OR_FAIL(ASS, SYM, SYMNAME) \
  fptr = dlsym(module_info.handle, #SYM); \
  if (fptr == 0) { \
    log_info(log_domain, "%s (%s = %s)\n", dlerror(), SYMNAME, #SYM); \
    dlclose(module_info.handle); \
    continue; \
  } \
  ASS;

#define LOAD_PROP_OR_FAIL(SYM, TYPE) \
  LOAD_SYM_OR_FAIL(module_info.SYM = *(TYPE*) fptr, SYM, #SYM)

#define LOAD_PPTR_OR_FAIL(SYM, TYPE) \
  LOAD_SYM_OR_FAIL(module_info.SYM = (TYPE) fptr, SYM, #SYM)

#define LOAD_FUNC_OR_FAIL(SYM) \
  LOAD_SYM_OR_FAIL(*(void **)&module_info.SYM = fptr, SYM, #SYM)

      LOAD_PROP_OR_FAIL(MODULE_NAME, char*);
      LOAD_PROP_OR_FAIL(MODULE_DESCRIPTION, char*);
      LOAD_PROP_OR_FAIL(MODULE_AUTHOR, char*);
      LOAD_PROP_OR_FAIL(MODULE_VERSION, char*);
      LOAD_PROP_OR_FAIL(PASTEBIN_URI, char*);
      LOAD_PROP_OR_FAIL(PASTEBIN_LANG, char*);
      LOAD_PPTR_OR_FAIL(PASTEBIN_AVAIL_LANGS, char**);

      LOAD_FUNC_OR_FAIL(FORM_REQUEST_FUNC);
      LOAD_FUNC_OR_FAIL(PROCESS_REPLY_FUNC);
      LOAD_FUNC_OR_FAIL(INIT_MODULE_FUNC);
      LOAD_FUNC_OR_FAIL(DEINIT_MODULE_FUNC);
  
      LOAD_PPTR_OR_FAIL(PTR_LANG, char**);
      LOAD_PPTR_OR_FAIL(PTR_URI, char**);
      LOAD_PPTR_OR_FAIL(PTR_FILENAME, char**);
      LOAD_PPTR_OR_FAIL(PTR_MIME, char**);
      LOAD_PPTR_OR_FAIL(PTR_PARENT, char**);
      LOAD_PPTR_OR_FAIL(PTR_AUTHOR, char**);
      LOAD_PPTR_OR_FAIL(PTR_BODY, char**);
      LOAD_PPTR_OR_FAIL(PTR_PRIVATE, int*);
      LOAD_PPTR_OR_FAIL(PTR_RUN, int*);
      LOAD_PPTR_OR_FAIL(PTR_LOG_DOMAIN, YULog**);
        
      yu_array_append(modules,&module_info);
    }
    closedir(directory);
  }

  yu_string_free(fullpath);

  return 0;
}

static int parse_options(int argc, char **argv) {
  YUOptionParser *parser;
  YUString       *help;
  YUString       *error;
  int             ret = 0;

  parser = yu_options_new(argc, argv);
  help   = yu_string_new();
  error  = yu_string_new();

  yu_options_add(parser, options);
  yu_options_produce_help(parser, help);

  yu_options_parse(parser, error);

  if (error->len != 0) {
    log_error(log_domain, "%s\n", error->str);
    log_msg(log_domain, "%s\n", help->str);
    ret = 1;
    goto parse_options_free_and_return;
  }

  if (print_help) {
    log_msg(log_domain, "%s\n", help->str);
    ret = 1;
    goto parse_options_free_and_return;
  }

  if (verbosity_flag > 0) {
    log_domain->current_level |= LOG_INFO;
  }

  if (verbosity_flag > 1) {
    log_domain->current_level |= LOG_DEBUG;
  }
  if (verbosity_flag > 2) {
    log_domain->current_level |= LOG_TRACE;
  }

parse_options_free_and_return:
  yu_options_free(parser);
  yu_string_free(error);
  yu_string_free(help);
  return ret;
}

static int init(void) {
  log_domain = yu_log_new_file(stderr,
                               stderr,
                               stdout,
                               stdout,
                               stderr,
                               stderr);

  module_paths = yu_pointer_array_new(1);
  yu_pointer_array_append(module_paths, "/usr/share/yukkipaste/modules");
  yu_pointer_array_append(module_paths, "/usr/local/share/yukkipaste/modules");
  yu_pointer_array_append(module_paths, "~/.config/yukkipaste/modules");
 
  modules = yu_array_new(sizeof(YukkipasteModuleInfo));
  
  file_data          = yu_string_new();
  transmit_data      = yu_string_new();
  post_path          = yu_string_new();
  reply              = yu_string_new();
  request_headers    = yu_string_new();
  content_type       = yu_string_new();
  mod_proc_reply_out = yu_string_new();

  memset(&active_module,0,sizeof(YukkipasteModuleInfo));
  return 0;
}

static void signal_cleanup(int sig) {
  cleanup();
  exit(1);
}

static void cleanup(void) {
  YukkipasteModuleInfo m;
  int                  i;

  if (log_domain != 0) {
    yu_log_free(log_domain);
    log_domain = 0;
  }
  if (module_paths != 0) {
    yu_pointer_array_free(module_paths);
    module_paths = 0;
  }
  
  if (pastebin_host != 0) {
    free(pastebin_host);
    pastebin_host = 0;
  }

  if (pastebin_port != 0) {
    free(pastebin_port);
    pastebin_port = 0;
  }

  if (pastebin_root != 0) {
    free(pastebin_root);
    pastebin_root = 0;
  }

  if (file_data != 0) {
    yu_string_free(file_data);
    file_data = 0;
  }

  if (mime != 0) {
    free(mime);
    mime = 0;
  }

  if (transmit_data != 0) {
    yu_string_free(transmit_data);
    transmit_data = 0;
  }

  if (post_path != 0) {
    yu_string_free(post_path);
    post_path = 0;
  }

  if (active_module.DEINIT_MODULE_FUNC != 0) {
    active_module.DEINIT_MODULE_FUNC();
    active_module.DEINIT_MODULE_FUNC = 0;
  }

  if (modules != 0) {
   for (i = 0; i < modules->len; i++) {
      m = yu_array_index(modules,YukkipasteModuleInfo,i);
      if (m.handle != 0) {
        dlclose(m.handle);
      }
    }
    yu_array_free(modules);
    modules = 0;
  }

  if (reply != 0) {
    yu_string_free(reply);
    reply = 0;
  }

  if (request_headers != 0) {
    yu_string_free(request_headers);
    request_headers = 0;
  }

  if (content_type != 0) {
    yu_string_free(content_type);
    content_type = 0;
  }

  if (mod_proc_reply_out != 0) {
    yu_string_free(mod_proc_reply_out);
    mod_proc_reply_out = 0;
  }
}

