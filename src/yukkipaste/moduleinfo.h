#ifndef __YU_YUKKIPASTE_MODULEINFO__
#define __YU_YUKKIPASTE_MODULEINFO__

#include "yukkipaste-api/yukkipaste-module.h"
#include "yutils/yulog.h"

typedef struct {
  char            *MODULE_NAME;
  char            *MODULE_DESCRIPTION;
  char            *MODULE_AUTHOR;
  char            *MODULE_VERSION;
  char            *PASTEBIN_URI;
  char            *PASTEBIN_LANG;
  char           **PASTEBIN_AVAIL_LANGS;
  init_module_t    INIT_MODULE_FUNC;
  deinit_module_t  DEINIT_MODULE_FUNC;
  form_request_t   FORM_REQUEST_FUNC;
  process_reply_t  PROCESS_REPLY_FUNC;
  void            *handle;
  char           **PTR_LANG;
  char           **PTR_URI;
  char           **PTR_FILENAME;
  char           **PTR_MIME;
  char           **PTR_PARENT;
  char           **PTR_AUTHOR;
  char           **PTR_BODY;
  int             *PTR_PRIVATE;
  int             *PTR_RUN;
  YULog          **PTR_LOG_DOMAIN;
} YukkipasteModuleInfo;

#endif /* __YU_YUKKIPASTE_MODULEINFO__ */
