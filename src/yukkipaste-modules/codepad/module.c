#include "yukkipaste-api/yukkipaste-module.h"

#include <string.h>

#include "yutils/yumultipart.h"

/* This macro defines module-specific constnats */
MODULE_INFO("codepad", "Codepad pastebin interface", "Iamtakingiteasy", "1.0")

/* Defines pastebin-specific constants */
PASTEBIN_INFO("http://codepad.org", "Plain Text")

/* Defines loads of variables and initializes them from host system */
/* see yukkipaste-api/yukkipaste-module.h for details */
DEFAULT_GLOBAL_VARS

/* NULL-terminated list of available languages */
char *PASTEBIN_AVAIL_LANGS[] = {
  "C",
  "C++",
  "D",
  "Haskell",
  "Lua",
  "OCaml",
  "PHP",
  "Perl",
  "Plain Text",
  "Python",
  "Ruby",
  "Scheme",
  "Tcl",
  0
};

int INIT_MODULE_FUNC(void) {
  return 0;
}

int FORM_REQUEST_FUNC(YUString    *post,
                      YUString    *type,
                      YUString    *data) {
  YUMultipart *multipart;
  
  yu_string_append0(post, "/");

  multipart = yu_multipart_new();

  yu_multipart_append0(multipart, "submit", "Submit");
  yu_multipart_append0(multipart, "client", "yukkipaste");
  yu_multipart_append0(multipart, "lang", PTR_LANG);
  yu_multipart_append0(multipart, "code", PTR_BODY);
  yu_multipart_append0(multipart, "private", PTR_PRIVATE ? "True" : "False");
  yu_multipart_append0(multipart, "run", PTR_RUN ? "True" : "False");
  yu_multipart_generate(multipart,data);

  yu_string_sprintfa(type,"multipart/form-data; boundary=%s",
                     multipart->boundary->str);

  yu_multipart_free(multipart);

  return 0;
}


int PROCESS_REPLY_FUNC(char *reply, YUString *uri) {
  char     *p;
  char     *beg;
  
  for (p = reply; *p != 0; p++) {
    if (strncmp("<a href=\"",p,9) == 0) {
      p += 9;
      beg = p;
      while (*p != '"' && *p != 0) p++;
      yu_string_append(uri,beg,p-beg);
    }
  }

  return 0;
}

int DEINIT_MODULE_FUNC(void) {
  return 0;
}

