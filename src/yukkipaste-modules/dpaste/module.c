#include "yukkipaste-api/yukkipaste-module.h"

#include <string.h>

#include "yutils/yumultipart.h"

/* This macro defines module-specific constnats */
MODULE_INFO("dpaste", "Dpaste pastebin interface", "Iamtakingiteasy", "1.0")

/* Defines pastebin-specific constants */
PASTEBIN_INFO("http://dpaste.com", "")

/* Defines loads of variables and initializes them from host system */
/* see yukkipaste-api/yukkipaste-module.h for details */
DEFAULT_GLOBAL_VARS

/* NULL-terminated list of available languages */
char *PASTEBIN_AVAIL_LANGS[] = {
  "Python",
  "PythonConsole",
  "Sql",
  "DjangoTemplate",
  "JScript",
  "Css",
  "Xml",
  "Diff",
  "Ruby",
  "Rhtml",
  "Haskell",
  "Apache",
  "Bash",
  0
};

static YUString *yus_post = 0;
static YUString *yus_type = 0;
static YUString *yus_data = 0;
static YUString *yus_uri  = 0;
static YUString *yus_err  = 0;

int INIT_MODULE_FUNC(void) {
  yus_post = yu_string_new();
  yus_type = yu_string_new();
  yus_data = yu_string_new();
  yus_uri  = yu_string_new();
  yus_err  = yu_string_new();
  return 0;
}

int FORM_REQUEST_FUNC(char **post,
                      char **type,
                      char **data,
                      int   *len) {
  YUMultipart *multipart;
  
  yu_string_append0(yus_post, "/api/v1/");

  multipart = yu_multipart_new();

  yu_multipart_append0(multipart, "content", PTR_DATA);
  yu_multipart_append0(multipart, "language", PTR_LANG);
  yu_multipart_append0(multipart, "title", PTR_FILENAME);
  yu_multipart_append0(multipart, "poster", PTR_AUTHOR);
  yu_multipart_generate(multipart, yus_data);

  yu_string_sprintfa(yus_type,"multipart/form-data; boundary=%s",
                     multipart->boundary->str);

  yu_multipart_free(multipart);

  *post = yus_post->str;
  *type = yus_type->str;
  *data = yus_data->str;
  *len  = yus_data->len;
  return 0;
}


int PROCESS_REPLY_FUNC(char *reply, char **uri, char **err) {
  char     *p;
  char     *beg;
  char      str[] = "Location: ";
  char      trm[] = "\r\n";

  for (p = reply; *p != 0; p++) {
    if (strncmp(str,p,sizeof(str)-1) == 0) {
      p += sizeof(str)-1;
      beg = p;
      while (strncmp(trm,p,sizeof(trm)-1) != 0 && *p != 0) p++;
      yu_string_append(yus_uri,beg,p-beg);
    }
  }
  
  *uri = yus_uri->str;
  *err = yus_err->str;

  return 0;
}

int DEINIT_MODULE_FUNC(void) {
  yu_string_guarded_free0(yus_post);
  yu_string_guarded_free0(yus_type);
  yu_string_guarded_free0(yus_data);
  yu_string_guarded_free0(yus_uri);
  yu_string_guarded_free0(yus_err);
  return 0;
}

