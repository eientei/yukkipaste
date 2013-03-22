#include "yukkipaste-api/yukkipaste-module.h"
#include "yutils/yujson.h"

#include <stdlib.h>

/* This macro defines module-specific constnats */
MODULE_INFO("pocoo", "Pocoo pastebin interface", "Iamtakingiteasy", "1.0")

/* Defines pastebin-specific constants */
PASTEBIN_INFO("http://paste.eientei.org")

/* Defines loads of variables and initializes them from host system */
/* see yukkipaste-api/yukkipaste-module.h for details */
DEFAULT_GLOBAL_VARS

static char* json_request =
  "{"
    "\"language\": \"%s\", "
    "\"filename\": \"%s\", "
    "\"mimetype\": \"%s\", "
    "\"parent_id\":  %d,   "
    "\"private\":    %d,   "
    "\"code\":     \"%s\"  "
  "}";

static YUString *escaped_lang     = 0;
static YUString *escaped_filename = 0;
static YUString *escaped_mime     = 0;
static YUString *escaped_code     = 0;


int INIT_MODULE_FUNC(void) {
  escaped_lang     = yu_string_new();
  escaped_filename = yu_string_new();
  escaped_mime     = yu_string_new();
  escaped_code     = yu_string_new();
  return 0;
}

int FORM_REQUEST_FUNC(YUString    *post,
                      YUString    *data,
                      RequestType *type) {
  int parent_id = 0;

  escape_json_string0(escaped_lang,PTR_LANG);  
  escape_json_string0(escaped_filename,PTR_FILENAME);
  escape_json_string0(escaped_mime,PTR_MIME);
  escape_json_string0(escaped_code,PTR_BODY);

  parent_id = atoi(PTR_PARENT);

  yu_string_sprintfa(data, json_request,
                     escaped_lang->str,
                     escaped_filename->str,
                     escaped_mime->str,
                     parent_id,
                     PTR_PRIVATE,
                     escaped_code->str);

  yu_string_append0(post,"/json/?method=pastes.newPaste");
  *type = HANDCRAFTED;
  return 0;
}


int PROCESS_REPLY_FUNC(char     *reply) {
  int ret = 0;
  YUString *err = 0;
  YUString *uri = 0;

  err = yu_string_new();
  uri = yu_string_new();
  yu_string_append0(uri,PTR_URI);
  
  if (uri->str[uri->len-1] != '/') {
    yu_string_append0(uri,"/");
  }
  yu_string_append0(uri,"show/");

  if (json_extract_string0(err, reply, "error") != 0) {
    ret = 1;
    goto process_reply_func_free_and_return;
  }
  if (err->len > 0) {
    ret = 1;
    log_error(PTR_LOG_DOMAIN, "Error received from pocoo: \"%s\"", err->str);
    goto process_reply_func_free_and_return;
  }
  
  if (json_extract_string0(uri, reply, "data") != 0) {
    ret = 1;
    goto process_reply_func_free_and_return;
  }

  printf("%s\n", uri->str);

process_reply_func_free_and_return:
  yu_string_free(err);
  yu_string_free(uri);
  return ret;
}

int DEINIT_MODULE_FUNC(void) {
  if (escaped_lang != 0) {
    yu_string_free(escaped_lang);
    escaped_lang = 0;
  }

  if (escaped_filename != 0) {
    yu_string_free(escaped_filename);
    escaped_filename = 0;
  }

  if (escaped_mime != 0) {
    yu_string_free(escaped_mime);
    escaped_mime = 0;
  }

  if (escaped_code != 0) {
    yu_string_free(escaped_code);
    escaped_code = 0;
  }
  return 0;
}
