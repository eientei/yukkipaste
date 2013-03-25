#include "yukkipaste-api/yukkipaste-module.h"

#include <string.h>

#include "yutils/yumultipart.h"

/* This macro defines module-specific constnats */
MODULE_INFO("pastie", "Pastie pastebin interface", "Iamtakingiteasy", "1.0")

/* Defines pastebin-specific constants */
PASTEBIN_INFO("http://pastie.org", "text")

/* Defines loads of variables and initializes them from host system */
/* see yukkipaste-api/yukkipaste-module.h for details */
DEFAULT_GLOBAL_VARS

/* NULL-terminated list of available languages 
 * Mapped as UI Name => Pastebin Name
 * */
char *PASTEBIN_AVAIL_LANGS[][2] = {
  { "objective-c",         "1"         },
  { "as",                  "2"         },
  { "rb",                  "3"         },
  { "ror",                 "4"         },
  { "diff",                "5"         },
  { "text",                "6"         },
  { "c",                   "7"         },
  { "cpp",                 "7"         },
  { "css",                 "8"         },
  { "java",                "9"         },
  { "js",                  "10"        },
  { "html",                "11"        },
  { "xml",                 "11"        },
  { "erb",                 "12"        },
  { "bash",                "13"        },
  { "sql",                 "14"        },
  { "php",                 "15"        },
  { "python",              "16"        },
  { "perl",                "18"        },
  { "yaml",                "19"        },
  { "csharp",              "20"        },
  { "go",                  "21"        },
  { 0,                     0           }
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
  
  yu_string_append0(yus_post, "/pastes");

  multipart = yu_multipart_new();

  yu_multipart_append0(multipart, "paste[body]",          PTR_DATA);
  yu_multipart_append0(multipart, "paste[parser_id]",     PTR_LANG);
  yu_multipart_append0(multipart, "paste[authorization]", "burger");
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
  char      str[] = "<a href=\"";
  char      trm[] = "\"";

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

