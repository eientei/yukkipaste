#ifndef __YU_YUKKIPASTE_MODULE__
#define __YU_YUKKIPASTE_MODULE__

/* each module must have this symbols defined */
/* otherwise module would not be loaded       */
#define MODULE_NAME             yu_module_name          /* char*           */
#define MODULE_DESCRIPTION      yu_module_description   /* char*           */
#define MODULE_AUTHOR           yu_module_author        /* char*           */
#define MODULE_VERSION          yu_module_version       /* char*           */
#define PASTEBIN_URI            yu_pastebin_uri         /* char*           */
#define PASTEBIN_LANG           yu_pastebin_lang        /* char*           */
#define PASTEBIN_AVAIL_LANGS    yu_pastebin_avail_langs /* char**          */

#define PTR_LANG                yu_ptr_lang             /* char*           */
#define PTR_URI                 yu_ptr_uri              /* char*           */
#define PTR_FILENAME            yu_ptr_filename         /* char*           */
#define PTR_MIME                yu_ptr_mime             /* char*           */
#define PTR_PARENT              yu_ptr_parent           /* char*           */
#define PTR_AUTHOR              yu_ptr_author           /* char*           */
#define PTR_DATA                yu_ptr_data             /* char*           */
#define PTR_PRIVATE             yu_ptr_private          /* int             */
#define PTR_RUN                 yu_ptr_run              /* int             */

#define INIT_MODULE_FUNC        yu_init_module          /* Initialize state*/
#define DEINIT_MODULE_FUNC      yu_deinit_module        /* Deinitialize    */
#define FORM_REQUEST_FUNC       yu_form_request         /* form_request_t  */
#define PROCESS_REPLY_FUNC      yu_process_reply        /* process_reply_t */
#define POPULATE_LANGS_FUNC     yu_populate_langs       /* populate langs  */

/* Shorthand for char* fields */
#define MODULE_INFO(NAME,DESCRIPTION,AUTHOR,VERSION) \
  char* MODULE_NAME        = NAME;        \
  char* MODULE_DESCRIPTION = DESCRIPTION; \
  char* MODULE_AUTHOR      = AUTHOR;      \
  char* MODULE_VERSION     = VERSION;      

/* Shorthand for char* fields */
#define PASTEBIN_INFO(URI, LANG) \
  char * PASTEBIN_URI  = URI; \
  char * PASTEBIN_LANG = LANG;

#define DEFAULT_GLOBAL_VARS \
  char  *PTR_LANG       = 0; \
  char  *PTR_URI        = 0; \
  char  *PTR_FILENAME   = 0; \
  char  *PTR_MIME       = 0; \
  char  *PTR_PARENT     = 0; \
  char  *PTR_AUTHOR     = 0; \
  char  *PTR_DATA       = 0; \
  int    PTR_PRIVATE    = 0; \
  int    PTR_RUN        = 0;

/* type safety enforcer */
int INIT_MODULE_FUNC(void);

/* type safety enforcer */
int DEINIT_MODULE_FUNC(void);

/* type safety enforcer */
int FORM_REQUEST_FUNC(char **,
                      char **,
                      char **,
                      int   *);

/* type safety enforcer */
int PROCESS_REPLY_FUNC(char *,
                       char **,
                       char **);


typedef int (*init_module_t)(void);

typedef int (*deinit_module_t)(void);

typedef int (*form_request_t)(char **,          /* POST request       [OUT] */
                              char **,          /* Content-Type       [OUT] */
                              char **,          /* Paste data section [OUT] */
                              int   *);         /* Paste data length  [OUT] */

typedef int (*process_reply_t)(char *,          /* Received reply     [ IN] */
                               char **,         /* Resulting URI      [OUT] */
                               char **);        /* Resultting error   [OUT] */

#endif /* __YU_YUKKIPASTE_MODULE__ */
