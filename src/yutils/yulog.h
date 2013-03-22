#ifndef __YU_LOG__
#define __YU_LOG__

#include <stdio.h>

typedef enum {
  LOG_ERROR = 1<<0,
  LOG_WARN  = 1<<1,
  LOG_MSG   = 1<<2,
  LOG_INFO  = 1<<3,
  LOG_DEBUG = 1<<4,
  LOG_TRACE = 1<<5
} YULogLevel;

typedef struct {
  FILE *err_file;
  FILE *wrn_file;
  FILE *msg_file;
  FILE *inf_file;
  FILE *dbg_file;
  FILE *trc_file;

  YULogLevel current_level;
} YULog;

YULog* yu_log_new_fd(int err, 
                     int wrn, 
                     int msg, 
                     int inf, 
                     int dbg, 
                     int trc);

YULog* yu_log_new_file(FILE *err,
                       FILE *wrn,
                       FILE *msg,
                       FILE *inf,
                       FILE *dbg,
                       FILE *trc);

void yu_log_free(YULog *domain);

void log_error(const YULog *domain, char *fmt, ...);
void log_warn(const YULog *domain, char *fmt, ...);
void log_msg(const YULog *domain, char *fmt, ...);
void log_info(const YULog *domain, char *fmt, ...);
void log_debug(const YULog *domain, char *fmt, ...);
void log_trace(const YULog *domain, char *fmt, ...);

#endif /* __YU_LOG__ */
