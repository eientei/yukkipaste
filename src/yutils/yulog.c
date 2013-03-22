#include "yulog.h"

#include <stdarg.h>
#include <stdlib.h>

YULog* yu_log_new_fd(int err, 
                     int wrn, 
                     int msg, 
                     int inf, 
                     int dbg, 
                     int trc) {
  YULog *domain;
  domain = calloc(1,sizeof(YULog));
  domain->err_file = fdopen(err,"a+");
  domain->wrn_file = fdopen(wrn,"a+");
  domain->msg_file = fdopen(msg,"a+");
  domain->inf_file = fdopen(inf,"a+");
  domain->dbg_file = fdopen(dbg,"a+");
  domain->trc_file = fdopen(trc,"a+");

  domain->current_level = LOG_ERROR | LOG_WARN | LOG_MSG;

  return domain;
}

YULog* yu_log_new_file(FILE *err,
                       FILE *wrn,
                       FILE *msg,
                       FILE *inf,
                       FILE *dbg,
                       FILE *trc) {
  YULog *domain;
  domain = calloc(1,sizeof(YULog));
  domain->err_file = err;
  domain->wrn_file = wrn;
  domain->msg_file = msg;
  domain->inf_file = inf;
  domain->dbg_file = dbg;
  domain->trc_file = trc;

  domain->current_level = LOG_ERROR | LOG_WARN | LOG_MSG;
  return domain;
}

void yu_log_free(YULog *domain) {
  fclose(domain->err_file);
  fclose(domain->wrn_file);
  fclose(domain->msg_file);
  fclose(domain->inf_file);
  fclose(domain->dbg_file);
  fclose(domain->trc_file);
  free(domain);
}

#define ERROR_FUNC(LEVEL,F) \
  { \
    va_list args; \
    if (!(domain->current_level & (LEVEL))) { \
      return; \
    } \
    va_start(args,fmt); \
    vfprintf(domain->msg_file,fmt,args); \
    va_end(args); \
  }


void log_error(const YULog *domain, char *fmt, ...) ERROR_FUNC(LOG_ERROR,err)
void log_warn(const YULog *domain, char *fmt, ...) ERROR_FUNC(LOG_WARN,wrn)
void log_info(const YULog *domain, char *fmt, ...) ERROR_FUNC(LOG_INFO,inf)
void log_msg(const YULog *domain, char *fmt, ...) ERROR_FUNC(LOG_MSG,msg)
void log_debug(const YULog *domain, char *fmt, ...) ERROR_FUNC(LOG_DEBUG,dbg)
void log_trace(const YULog *domain, char *fmt, ...) ERROR_FUNC(LOG_TRACE,trc)
