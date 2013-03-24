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
#include <glob.h>

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
#include "yutils/yusectconf.h"
#include "yukkipaste-api/yukkipaste-module.h"

#include "moduleinfo.h"

#define USER_CONF_FILE "~/.config/yukkipaste/config"

static char* headers =
 "POST %s%s HTTP/1.1\r\n"
 "User-Agent: yukkipaste 1.1\r\n"
 "Host: %s:%s\r\n"
 "Accept: */*\r\n"
 "Content-Type: %s\r\n"
 "Content-Length: %d\r\n"
 "\r\n";

/* Global internals */
static YUPointerArray       *g_module_paths  = 0; /* used in CLI and config */
static YULog                *g_log_domain    = 0; /* used in module input   */
static YUArray              *g_paste_modules = 0;
static YUSectConf           *g_user_config   = 0;
static YukkipasteModuleInfo  g_active_module;
static char                 *g_module_name   = 0;
static char                 *g_pastebin_host = 0;
static char                 *g_pastebin_port = 0;
static char                 *g_pastebin_root = 0;

/* Resources */
static char                  rsr_io_buffer[BUFSIZ];
static int                   rsr_sock_fd             = 0;
static int                   rsr_file_fd             = 0;

/* Transitions */
static YUString             *trn_file_contents       = 0;
static char                 *trn_request_data        = 0;
static int                   trn_request_data_len    = 0;
static YUString             *trn_request_headers     = 0;
static char                 *trn_request_path        = 0;
static char                 *trn_content_type        = 0;
static YUString             *trn_reply_data          = 0;
static char                 *trn_uri                 = 0;
static char                 *trn_error               = 0;
static YUString             *trn_userconf_error      = 0;

/* CLI argument values */
static char                **cli_leftovers           = 0;
static int                   cli_help_flag           = 0;
static int                   cli_list_modules_flag   = 0;
static int                   cli_list_languages_flag = 0;
static int                   cli_verbose_flag        = 3;
static char                 *cli_author_arg          = 0;
static char                 *cli_uri_arg             = 0;
static char                 *cli_lang_arg            = 0;
static char                 *cli_remote_name_arg     = 0;
static char                 *cli_mime_type_arg       = 0;
static char                 *cli_module_name_arg     = 0;
static char                 *cli_parent_paste_arg    = 0;
static int                   cli_private_switch      = 0;
static int                   cli_runnable_switch     = 0;

/* Config properties values */
static int                   cfg_verbose_flag        = 0;
static char                 *cfg_author_arg          = 0;
static char                 *cfg_uri_arg             = 0;
static char                 *cfg_lang_arg            = 0;
static char                 *cfg_module_name_arg     = 0;
static int                   cfg_private_switch      = 0;
static int                   cfg_runnable_switch     = 0;

/* Module input variables */
static char                 *ptr_lang                = 0;
static char                 *ptr_uri                 = 0;
static char                 *ptr_filename            = 0;
static char                 *ptr_mime                = 0;
static char                 *ptr_parent              = 0;
static char                 *ptr_author              = 0;
static char                 *ptr_data                = 0;
static int                   ptr_private             = 0;
static int                   ptr_run                 = 0;

static YUOption options[] = 
{
  { "help", 'h', OPTION_ARG_NONE, &cli_help_flag,
    "Prints help message", 0 },

  { "modules-dir", 'd', OPTION_ARG_POINTER_ARRAY, &g_module_paths,
    "Appends module path. Stackable.", "DIR" },

  { "list-modules", 0, OPTION_ARG_NONE, &cli_list_modules_flag,
    "Lists available modules", 0 },

  { "verbose", 'v', OPTION_ARG_NONE, &cli_verbose_flag,
    "Increases verbosity level", 0 },

  { "author", 'a', OPTION_ARG_STRING, &cli_author_arg,
    "Your name", "STRING" },

  { "uri", 'u', OPTION_ARG_STRING, &cli_uri_arg,
    "Pastebin URI", "URI" },

  { "language", 'l', OPTION_ARG_STRING, &cli_lang_arg,
    "Paste language", "LANG" },

  { "list-languages", 0, OPTION_ARG_NONE, &cli_list_languages_flag,
    "Lists available languages for selected module", 0 },

  { "remote-name", 'n', OPTION_ARG_STRING, &cli_remote_name_arg,
    "Remote file name", "NAME" },

  { "mime-type", 0, OPTION_ARG_STRING, &cli_mime_type_arg,
    "Paste mime type", "MIME" },

  { "module", 'm', OPTION_ARG_STRING, &cli_module_name_arg,
    "Selects module to use", "MODULE" },

  { "parent-id", 0, OPTION_ARG_STRING, &cli_parent_paste_arg,
    "Parent paste id", "ID" },

  { "private", 'p', OPTION_ARG_NONE, &cli_private_switch,
    "Marks paste as private", 0 },

  { "run", 'r', OPTION_ARG_NONE, &cli_runnable_switch,
    "Marks paste as runnable", 0 },

  { 0, 0, OPTION_ARG_LEFTOVERS, &cli_leftovers, 0, 0 },
  
  { 0, 0, 0, 0, 0, 0 }
};


static void signal_cleanup(int sig);
static void cleanup(void);
static int init(void);
static int parse_userconfig(void);
static int parse_options(int argc, char **argv);
static int interpolate_props(void);
static int scan_module_dirs(void);
static int select_active_module(void);
static int action_list_modules(void);
static int action_list_languages(void);
static int parse_uri(void);
static int parse_language(void);
static int open_file(void);
static int read_file(void);
static int mod_init(void);
static int mod_form_request(void);
static int form_headers(void);
static int open_socket(void);
static int transfer_data(void);
static int mod_process_reply(void);

int main(int argc, char **argv) {
  atexit(cleanup);
  signal(SIGINT,  signal_cleanup);
  signal(SIGSEGV, signal_cleanup);

#define EXEC_OR_FAIL(F) \
  if (F() != 0) { \
    return 1; \
  }

  EXEC_OR_FAIL(init);
  EXEC_OR_FAIL(parse_userconfig);
  

  if (parse_options(argc, argv) != 0) {
    return 1;
  }

  EXEC_OR_FAIL(interpolate_props);
  EXEC_OR_FAIL(scan_module_dirs);
  EXEC_OR_FAIL(select_active_module);
 
  if (cli_list_modules_flag) {
    action_list_modules();
    return 0;
  }

  if (cli_list_languages_flag) {
    action_list_languages();
    return 0;
  }

  EXEC_OR_FAIL(parse_uri);
  EXEC_OR_FAIL(parse_language);
  EXEC_OR_FAIL(open_file);
  EXEC_OR_FAIL(read_file);
  EXEC_OR_FAIL(mod_init);
  EXEC_OR_FAIL(mod_form_request);
  EXEC_OR_FAIL(form_headers);
  EXEC_OR_FAIL(open_socket);
  EXEC_OR_FAIL(transfer_data);
  EXEC_OR_FAIL(mod_process_reply);

  return 0;  
}

static int mod_process_reply(void) {
  int ret;
  ret = g_active_module.PROCESS_REPLY_FUNC(trn_reply_data->str, 
                                           &trn_uri, &trn_error);

  if (trn_error != 0 && strlen(trn_error) != 0) {
    log_msg(g_log_domain, "Error received from pastebin: %s\n", trn_error);
    return 1;
  }

  if (trn_uri == 0 || strlen(trn_uri) == 0 || ret != 0) {
    log_msg(g_log_domain, "Error parsing pastebin reply.\n");
    return 1;
  }

  log_msg(g_log_domain, "%s\n", trn_uri);
  return ret;
}

static int transfer_data(void) {
  int          count = 0;
  int          nolen = 0;
  char        *p;
  char        *end;
  char        *reply_begin = 0;
  static char  cl[] = "Content-Length: ";
  static char  ch[] = "</html>";
  int          length = 0;

  log_trace(g_log_domain, "%s\n", trn_request_headers->str);

  while ((count = send(rsr_sock_fd,
                       trn_request_headers->str+count,
                       trn_request_headers->len-count,0)) > 0);
  if (count < 0) {
    log_error(g_log_domain,"send: %s\n", strerror(errno));
    return 1;
  }
  
  count = 0;
  
  log_trace(g_log_domain, "%s\n", trn_request_data);
  while ((count = send(rsr_sock_fd,
                       trn_request_data+count,
                       trn_request_data_len-count,0)) > 0);
    
  if (count < 0) {
    log_error(g_log_domain,"send: %s\n", strerror(errno));
    return 1;
  }

  count = 0;

  while ((count = recv(rsr_sock_fd,rsr_io_buffer,sizeof(rsr_io_buffer),0)) > 0) {
    yu_string_append(trn_reply_data,rsr_io_buffer,count);
    p = trn_reply_data->str;
    end = p + trn_reply_data->len;
    while (p != end) {
      if (strncmp(p,cl,sizeof(cl)-1) == 0) {
        p += sizeof(cl)-1;
        length = atoi(p);
        while (strncmp(p,"\r\n\r\n",4) != 0 && p != end) p++;
        if (p != end) {
          p += 4;
        }
        if (strnlen(p,length) >= length) {
          close(rsr_sock_fd);
        }
        break;
      } else if (strncmp(p, "\r\n\r\n",4) == 0) {
        nolen = 1;
        break;
      }
      p++;
    }
  }
  log_trace(g_log_domain, "%s\n", trn_reply_data->str);

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

  if ((status = getaddrinfo(g_pastebin_host,g_pastebin_port,&hints,&res)) != 0) {
    log_error(g_log_domain,"%s\n",gai_strerror(status));
    return 1;
  }

  rsr_sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (rsr_sock_fd == -1) {
    log_error(g_log_domain,"socket: %s\n",strerror(errno));
    goto open_socket_free_and_return;
  }

  if (connect(rsr_sock_fd, res->ai_addr, res->ai_addrlen) == -1) {
    log_error(g_log_domain,"connect: %s\n",strerror(errno));
    goto open_socket_free_and_return;
  }

open_socket_free_and_return:
  freeaddrinfo(res);
  return ret;
}

static int form_headers(void) {
  yu_string_sprintfa(trn_request_headers, headers,
                     g_pastebin_root,
                     trn_request_path,
                     g_pastebin_host,
                     g_pastebin_port,
                     trn_content_type,
                     trn_request_data_len);

  return 0;
}

static int mod_form_request(void) {
  return g_active_module.FORM_REQUEST_FUNC(&trn_request_path,
                                           &trn_content_type,
                                           &trn_request_data,
                                           &trn_request_data_len);
}

static int mod_init(void) {
  *g_active_module.PTR_LANG       = ptr_lang;
  *g_active_module.PTR_URI        = ptr_uri;
  *g_active_module.PTR_FILENAME   = ptr_filename;
  *g_active_module.PTR_MIME       = ptr_mime;
  *g_active_module.PTR_PARENT     = ptr_parent;
  *g_active_module.PTR_AUTHOR     = ptr_author;
  *g_active_module.PTR_DATA       = ptr_data;
  *g_active_module.PTR_PRIVATE    = ptr_private;
  *g_active_module.PTR_RUN        = ptr_run;
 
  return g_active_module.INIT_MODULE_FUNC();
}


static int read_file(void) {
  int         count;
#ifdef HAVE_LIBMAGIC
  magic_t     magic;
  const char *m;
#endif

  if (rsr_file_fd != STDIN_FILENO && 
      HAVE_LIBMAGIC_TEST && cli_mime_type_arg == 0) {
#ifdef HAVE_LIBMAGIC
    magic = magic_open(MAGIC_SYMLINK | MAGIC_MIME_TYPE);
    if (magic == 0) {
      log_warn(g_log_domain,"%s\n",magic_error(magic));
      goto mime_fallback;
    }
    magic_load(magic,0);
    m = magic_file(magic,ptr_filename);
    if (m == 0) {
      log_warn(g_log_domain,"%s\n",magic_error(magic));
      magic_close(magic);
      goto mime_fallback;
    }
    ptr_mime = strdup((char*)m);
    magic_close(magic);
#endif
  } else {
mime_fallback:
    if (cli_mime_type_arg == 0) {
      ptr_mime = strdup("text/plain");
    } else {
      ptr_mime = strdup(cli_mime_type_arg);
    }
  }

  while ((count = read(rsr_file_fd, rsr_io_buffer, sizeof(rsr_io_buffer))) > 0) {
    yu_string_append(trn_file_contents, rsr_io_buffer, count);
  }

  if (count < 0) {
    log_error(g_log_domain, "%s: %s\n", strerror(errno), ptr_filename);
  }

  if (trn_file_contents->len == 0) {
    log_error(g_log_domain, "Empty input, aborting.\n");
    return 1;
  }

  ptr_data = trn_file_contents->str;

  return 0;
}

static int open_file(void) {
  char *arg;

  if (cli_leftovers[0] == 0) {
    rsr_file_fd = STDIN_FILENO;
    if (ptr_filename == 0) {
      ptr_filename = "stdin";
    }
  } else {
    arg = cli_leftovers[0];
    if (strcmp("-", arg) == 0) {
      rsr_file_fd = STDIN_FILENO;
      ptr_filename = "stdin";
    } else {
      rsr_file_fd = open(arg,O_RDONLY);
      if (rsr_file_fd == -1) {
        log_error(g_log_domain, "%s: %s\n", strerror(errno), arg);
        return 1;
      }
      ptr_filename = arg;
    }
  }

  return 0;
}

static int parse_language(void) {
  char             soundex[5];
  YUPointerArray  *variants;
  char           **l;
  int              ret = 0;
  int              i;

  if (ptr_lang == 0) {
    ptr_lang = g_active_module.PASTEBIN_LANG;
  }

  for (l = g_active_module.PASTEBIN_AVAIL_LANGS; *l != 0; l++) {
    if (strcasecmp(*l,ptr_lang) == 0) {
      ptr_lang = *l;
      return 0;
    }
  }

  variants = yu_pointer_array_new(0);
  strncpy(soundex,yu_soundex(ptr_lang),5);
  
  for (l = g_active_module.PASTEBIN_AVAIL_LANGS; *l != 0; l++) {
    if (strcmp(yu_soundex(*l),soundex) == 0) {
      yu_pointer_array_append(variants,*l);
    }
  }

  if (variants->len == 0) {
    ret = 1;
    log_error(g_log_domain,
              "Language \"%s\" is not available from module \"%s\"\n",
              ptr_lang, g_active_module.MODULE_NAME);
    goto parse_language_free_and_return;
  }

  if (variants->len == 1) {
    ptr_lang = yu_pointer_array_index(variants,0);
    goto parse_language_free_and_return;
  }

  log_msg(g_log_domain, "Ambigous language variants: \n");
  for (i = 0; i < variants->len; i++) {
    log_msg(g_log_domain, "    - %s\n", yu_pointer_array_index(variants,i));
  }

  ret = 1;

parse_language_free_and_return:
  yu_pointer_array_free(variants);
  return ret;
}

static int parse_uri(void) {
  char *host_beg = 0;
  char *host_end = 0;
  char *port_beg = 0;
  char *port_end = 0;
  char *path_beg = 0;
  
  if (ptr_uri == 0) {
    ptr_uri = g_active_module.PASTEBIN_URI;
  }

  host_beg = ptr_uri;
  if (strncmp("http://",ptr_uri,7) == 0) {
    host_beg += 7;
  }

  host_end = host_beg;

  while (*host_end != ':' && *host_end != '/' && *host_end != '\0') host_end++;

  if (host_beg == host_end) {
    log_error(g_log_domain,"Empty host in URI\n");
    return 1;
  }

  g_pastebin_host = strndup(host_beg, host_end - host_beg);

  path_beg = host_end;
  
  if (*host_end == ':') {
    port_beg = host_end+1;
    port_end = port_beg;

    while (*port_end != '/' && *port_end != '\0') port_end++;
  
    path_beg = port_end;
    
    if (port_beg == port_end) {
      log_error(g_log_domain,"Empty port in URI\n");
      return 1;
    }

    g_pastebin_port = strndup(port_beg, port_end - port_beg);
  } else {
    g_pastebin_port = strdup("80");
  }

  g_pastebin_root = strdup(path_beg);
  return 0;
}

static int action_list_languages(void) {
  char **l;

  for (l = g_active_module.PASTEBIN_AVAIL_LANGS; *l != 0; l++) {
    log_msg(g_log_domain, "    - %s\n",*l);
  }
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

  for (i = 0; i < g_paste_modules->len; i++) {
    m = yu_array_index(g_paste_modules,YukkipasteModuleInfo,i);
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


  log_msg(g_log_domain, fmt->str, "name", "ver", "author", "description");
  for (i = 0; i < g_paste_modules->len; i++) {
    m = yu_array_index(g_paste_modules,YukkipasteModuleInfo,i);
    log_msg(g_log_domain, fmt->str, m.MODULE_NAME, m.MODULE_VERSION,
                                    m.MODULE_AUTHOR, m.MODULE_DESCRIPTION);
  }

  yu_string_free(fmt);
  return 0;
}

static int select_active_module(void) {
  int i;
  YukkipasteModuleInfo m;

  if (g_paste_modules->len == 0) {
    log_error(g_log_domain, "No modules were loaded. Check -d option for use\n");
    return 1;
  }
  if (g_module_name == 0) {
    g_active_module = yu_array_index(g_paste_modules,YukkipasteModuleInfo,0);
    return 0;
  }

  for (i = 0; i < g_paste_modules->len; i++) {
    m = yu_array_index(g_paste_modules,YukkipasteModuleInfo,i);
    if (strcmp(m.MODULE_NAME, g_module_name) == 0) {
      g_active_module = m;
      return 0;
    }
  }

  log_error(g_log_domain, "Wasn't able to match a module named \"%s\"\n",
            g_module_name);

  log_error(g_log_domain, "Consider reviewing --list-modules contnets\n");
  return 1; 
}

static int scan_module_dirs(void) {
  YukkipasteModuleInfo  module_info;
  YukkipasteModuleInfo  exist_info;
  YUString             *fullpath;
  int                   i;
  int                   n;
  int                   len;
  int                   existed;
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

  for (i = 0; i < g_module_paths->len; i++) {
    yu_string_clear(fullpath);
    path = yu_pointer_array_index(g_module_paths,i);
    log_debug(g_log_domain, "entering %s\n", path);
    directory = opendir(path);
    if (directory == 0) {
      log_debug(g_log_domain, "%s: %s\n", strerror(errno), path);
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
        log_debug(g_log_domain, "%s: %s\n", strerror(errno), fullpath->str);
        continue;
      }
      if (!S_ISREG(info.st_mode)) {
        log_debug(g_log_domain, "File is not a regular file: %s\n", fullpath->str);
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
        log_debug(g_log_domain, "File is not readable: %s\n", fullpath->str);
        continue;
      }
   
      /* everything is ok */
      memset(&module_info,0,sizeof(YukkipasteModuleInfo));
      module_info.handle = dlopen(fullpath->str, RTLD_LAZY | RTLD_LOCAL);

      if (module_info.handle == 0) {
        log_debug(g_log_domain, "%s\n", dlerror());
        continue;
      }

#define LOAD_SYM_OR_FAIL(ASS, SYM, SYMNAME) \
  fptr = dlsym(module_info.handle, #SYM); \
  if (fptr == 0) { \
    log_info(g_log_domain, "%s (%s = %s)\n", dlerror(), SYMNAME, #SYM); \
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
      LOAD_PPTR_OR_FAIL(PTR_DATA, char**);
      LOAD_PPTR_OR_FAIL(PTR_PRIVATE, int*);
      LOAD_PPTR_OR_FAIL(PTR_RUN, int*);
     
      existed = 0;
      for (n = 0; n < g_paste_modules->len; n++) {
        exist_info = yu_array_index(g_paste_modules, YukkipasteModuleInfo, n);
        if (strcmp(exist_info.MODULE_NAME,module_info.MODULE_NAME) == 0) {
          log_debug(g_log_domain, "Not adding duplicated module %s (%s)\n",
                    module_info.MODULE_NAME, fullpath->str);
          dlclose(module_info.handle);   
          existed = 1;
          break;
        }
      }

      if (existed) {
        continue;
      }

      yu_array_append(g_paste_modules,&module_info);
      log_trace(g_log_domain, "Loaded module %s (%s)\n", 
                module_info.MODULE_NAME, fullpath->str);
    }
    closedir(directory);
  }

  yu_string_free(fullpath);

  return 0;
}

static int interpolate_props(void) {
  YUSectConfEntry entry;
  int             verbose;

  g_module_name = cfg_module_name_arg;

  if (cli_module_name_arg != 0) {
    g_module_name = cli_module_name_arg;
  }

  entry = yu_sect_conf_get(g_user_config, g_module_name, "author");
  if (entry.value != 0) { cfg_author_arg = entry.value; }

  entry = yu_sect_conf_get(g_user_config, g_module_name, "uri");
  if (entry.value != 0) { cfg_uri_arg = entry.value; }

  entry = yu_sect_conf_get(g_user_config, g_module_name, "lang");
  if (entry.value != 0) { cfg_lang_arg = entry.value; }

  entry = yu_sect_conf_get(g_user_config, g_module_name, "private");
  if (entry.value != 0) { cfg_private_switch = atoi(entry.value); }
  
  entry = yu_sect_conf_get(g_user_config, g_module_name, "run");
  if (entry.value != 0) { cfg_runnable_switch = atoi(entry.value); }
 
  ptr_parent   = cli_parent_paste_arg;
  ptr_lang     = cfg_lang_arg;
  ptr_author   = cfg_author_arg;
  ptr_private  = cfg_private_switch;
  ptr_run      = cfg_runnable_switch;
  
  if (ptr_parent == 0) {
    ptr_parent = "";
  }

  if (cli_lang_arg != 0) {
    ptr_lang = cli_lang_arg;
  }

  if (cli_uri_arg != 0) {
    ptr_uri = cli_uri_arg;
  }

  if (cli_author_arg != 0) {
    ptr_author = cli_author_arg;
  }

  if (cli_private_switch != 0) {
    ptr_private = cli_private_switch;
  }

  if (cli_runnable_switch != 0) {
    ptr_run = cli_runnable_switch;
  }

  verbose = cfg_verbose_flag;
  if (cli_verbose_flag > 3) {
    verbose = cli_verbose_flag;
  }

  if (verbose > 0) g_log_domain->current_level |= LOG_ERROR;
  if (verbose > 1) g_log_domain->current_level |= LOG_WARN;
  if (verbose > 2) g_log_domain->current_level |= LOG_MSG;
  if (verbose > 3) g_log_domain->current_level |= LOG_INFO;
  if (verbose > 4) g_log_domain->current_level |= LOG_DEBUG;
  if (verbose > 5) g_log_domain->current_level |= LOG_TRACE;

  if (trn_userconf_error->len > 0) {
    log_info(g_log_domain, "Error parsing config file %s:\n%s\n",
             USER_CONF_FILE, trn_userconf_error->str);
  }

  return 0;
}

static int parse_options(int argc, char **argv) {
  YUOptionParser *parser;
  YUString       *help;
  YUString       *err;
  int             ret = 0;

  parser = yu_options_new(argc,argv);
  help   = yu_string_new();
  err    = yu_string_new();

  yu_options_add(parser,options);
  yu_options_produce_help(parser,help);

  yu_options_parse(parser, err);

  if (err->len != 0) {
    log_error(g_log_domain, "%s\n", err->str);
    log_msg(g_log_domain, "%s\n", help->str);

    ret = 1;
    goto parse_options_free_and_return;
  }

  if (cli_help_flag) {
    log_msg(g_log_domain, "%s\n", help->str);

    ret = 1;
    goto parse_options_free_and_return;
  }

parse_options_free_and_return:
  yu_string_free(help);
  yu_string_free(err);
  yu_options_free(parser);
  return ret;
}

static int parse_userconfig(void) {
  char           *p;
  char           *beg;
  glob_t          globbuf;
  YUSectConfEntry entry;

  glob(USER_CONF_FILE, GLOB_TILDE, 0, &globbuf);

  if (globbuf.gl_pathc == 0) {
    yu_string_sprintfa(trn_userconf_error, "Could not glob filepath: %s\n",
                       USER_CONF_FILE);
    return 0;
  }

  yu_sect_conf_parse(g_user_config, globbuf.gl_pathv[0], trn_userconf_error);

  globfree(&globbuf);

  if (trn_userconf_error->len != 0) return 0;

  entry = yu_sect_conf_get(g_user_config, 0, "module");
  cfg_module_name_arg = entry.value;

  entry = yu_sect_conf_get(g_user_config, 0, "verbosity");
  if (entry.value != 0) {
    cfg_verbose_flag = atoi(entry.value);
    if (cfg_verbose_flag < 0 || cfg_verbose_flag > 6) {
      yu_string_sprintfa(trn_userconf_error, "line %d - %s", entry.line,
                         "Value of verbosity level is not in 0..6 range");
    }
  }

  entry = yu_sect_conf_get(g_user_config, 0, "modules_dirs");
  if (entry.value != 0) {
    for (p = entry.value; *p != 0; p++) {
      beg = p;
      while (*p != 0 && *p != ':') p++;
      if (p != beg) {
        yu_pointer_array_append(g_module_paths, beg);
      }
      if (*p == 0) break;
      *p = 0;
    }
  }

  entry = yu_sect_conf_get(g_user_config, 0, "author");
  cfg_author_arg = entry.value;

  entry = yu_sect_conf_get(g_user_config, 0, "uri");
  cfg_uri_arg = entry.value;

  entry = yu_sect_conf_get(g_user_config, 0, "lang");
  cfg_lang_arg = entry.value;

  entry = yu_sect_conf_get(g_user_config, 0, "private");
  if (entry.value != 0) { cfg_private_switch = atoi(entry.value); }
  
  entry = yu_sect_conf_get(g_user_config, 0, "run");
  if (entry.value != 0) { cfg_runnable_switch = atoi(entry.value); }

  return 0;
}

static int init(void) {
  g_log_domain = yu_log_new_file(stderr,stderr,
                                 stdout,stdout,
                                 stderr,stderr);

  g_module_paths = yu_pointer_array_new();
  yu_pointer_array_append(g_module_paths, "/usr/share/yukkipaste/modules");
  yu_pointer_array_append(g_module_paths, "/usr/local/share/yukkipaste/modules");
  yu_pointer_array_append(g_module_paths, "~/.config/yukkipaste/modules");

  g_user_config = yu_sect_conf_new();

  g_paste_modules = yu_array_new(sizeof(YukkipasteModuleInfo));

  trn_file_contents   = yu_string_new();
  trn_request_headers = yu_string_new();
  trn_reply_data      = yu_string_new();
  trn_userconf_error  = yu_string_new();

  memset(&g_active_module,0,sizeof(YukkipasteModuleInfo));
  
  return 0;
}

static void signal_cleanup(int sig) {
  cleanup();
  exit(1);
}

static void cleanup(void) {
  int                  i; 
  YukkipasteModuleInfo m;

  if (g_log_domain != 0) {
    yu_log_free(g_log_domain);
    g_log_domain = 0;
  }

  if (g_module_paths != 0) {
    yu_pointer_array_free(g_module_paths);
    g_module_paths = 0;
  }

  if (g_user_config != 0) {
    yu_sect_conf_free(g_user_config);
    g_user_config = 0;
  }

  if (g_active_module.DEINIT_MODULE_FUNC != 0) {
    g_active_module.DEINIT_MODULE_FUNC();
    g_active_module.DEINIT_MODULE_FUNC = 0;
  }

  if (g_paste_modules != 0) {
    for (i = 0; i < g_paste_modules->len; i++) {
      m = yu_array_index(g_paste_modules,YukkipasteModuleInfo,i);
      if (m.handle != 0) {
        dlclose(m.handle);
      }
    }
    yu_array_free(g_paste_modules);
    g_paste_modules = 0;
  }
  
  yu_string_guarded_free0(trn_file_contents);
  yu_string_guarded_free0(trn_request_headers);
  yu_string_guarded_free0(trn_reply_data);
  yu_string_guarded_free0(trn_userconf_error);

  if (g_pastebin_host != 0) {
    free(g_pastebin_host);
    g_pastebin_host = 0;
  }

  if (g_pastebin_port != 0) {
    free(g_pastebin_port);
    g_pastebin_port = 0;
  }

  if (g_pastebin_root != 0) {
    free(g_pastebin_root);
    g_pastebin_root = 0;
  }
  
  if (ptr_mime != 0) {
    free(ptr_mime);
    ptr_mime = 0;
  }
}
