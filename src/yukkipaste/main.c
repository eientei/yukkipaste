#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>

#include "yustring.h"
#include "yuutf.h"

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

static char* headers =
 "POST %s/json/?method=pastes.newPaste HTTP/1.1\r\n"
 "User-Agent: yukkipaste\r\n"
 "Host: %s:%s\r\n"
 "Accept: */*\r\n"
 "Content-Type: application/json\r\n"
 "Content-Length: %d\r\n"
 "\r\n";

static char* json_request =
  "{"
    "\"language\": \"%s\", "
    "\"filename\": \"%s\", "
    "\"mimetype\": \"%s\", "
    "\"parent_id\":  %d,   "
    "\"private\":    %d,   "
    "\"code\":     \"%s\"  "
  "}";

static char   buf[BUFSIZ];

static YUString  *reply = 0;
static YUString  *request_headers = 0;
static YUString  *request_data = 0;

static YUString  *escaped_code = 0;
static YUString  *escaped_language = 0;
static YUString  *escaped_mime = 0;
static YUString  *escaped_filename = 0;

static YUString  *filedata    = 0;
static int        filefd;
static int        sockfd;
static char      *pastebin_host = 0;
static char      *pastebin_port = 0;
static char      *pastebin_root = 0;
static char     **args;

static char *pastebin_uri     = "http://paste.eientei.org";
static char *language_id      = "text";
static char *mime_type        = "text/plain";
static char *remote_filename  = 0;
static int   parent_id        = 1;
static int   is_paste_private = 0;

static int      parse_options(int argc, char **argv);
static void     print_help(char *argv0);
static void     cleanup(void);
static void     signal_cleanup(int sig);
static int      parse_uri(void);
static int      open_file(void);
static int      read_file(void);
static int      escape_data(void);
static int      escape_json_string(YUString *out, char *in, size_t len);
static int      open_socket(void);
static int      form_request(void);
static int      transfer_data(void);
static char*    json_extract_p(char **p, char *end, char *key, size_t len);

int main(int argc, char ** argv) {
  atexit(cleanup);
  signal(SIGINT, signal_cleanup);

  if (parse_options(argc,argv) != 0) {
    return 1;
  }

  if (parse_uri() != 0) {
    return 1;
  }

  if (open_file() != 0) {
    return 1;
  }

  if (read_file() != 0) {
    return 1;
  }

  if (escape_data() != 0) {
    return 1;
  }

  if (form_request() != 0) {
    return 1;
  }

  if (open_socket() != 0) {
    return 1;
  }

  if (transfer_data() != 0) {
    return 1;
  }

  return 0;
}
static char* json_extract_p(char **p, char *end, char *key, size_t len) {
  char *beg;
  while (*p != end) {
    if (strncmp(*p,key,len) == 0) {
      *p += len;
      if (*((*p)+1) != '"') {
        beg = *p;
        (*p)++;
        while (**p != '\n' && **p != '}' && *p != end) {
          (*p)++;
          if (**p == '"' && *((*p)-1) != '\\') {
            break;
          }
        }
        return beg;
      }
    }
    (*p)++;
  }
  return 0;
}

static int transfer_data(void) {
  int          count = 0;
  char        *p;
  char        *end;
  char        *beg;
  char        *json_begin;
  static char  cl[] = "Content-Length: ";
  static char  er[] = "\"error\": ";
  static char  dt[] = "\"data\": ";
  int          length = 0;

  while ((count = send(sockfd,
                       request_headers->str+count,
                       request_headers->len-count,0)) > 0);
  if (count < 0) {
    perror("send");
    return 1;
  }
  
  count = 0;
  
  while ((count = send(sockfd,
                       request_data->str+count,
                       request_data->len-count,0)) > 0);
    
  if (count < 0) {
    perror("send");
    return 1;
  }

  count = 0;

  reply = yu_string_new();

  while ((count = recv(sockfd,buf,sizeof(buf),0)) > 0) {
    yu_string_append(reply,buf,count);
    p = reply->str;
    end = p + reply->len;
    while (p != end) {
      if (strncmp(p,cl,sizeof(cl)-1) == 0) {
        p += sizeof(cl)-1;
        length = atoi(p);
        while (strncmp(p,"\r\n\r\n",4) != 0) p++;
        p += 4;
        if (strnlen(p,length) >= length) {
          json_begin = p;
          close(sockfd);
        }
        break;
      }
      p++;
    }
  }

  beg = json_extract_p(&p,end,er,sizeof(er)-1);
  
  if (beg == 0) {
    printf("Error parsing respnse from server\n");
    return 1;
  }

  if (strncmp(beg,"null",4) != 0) {
    printf("Error received from server: "); 
    fflush(stdout);
    write(STDOUT_FILENO,beg,p-beg+1);
    printf("\n");
    return 1;
  }

  p = json_begin;
  beg = json_extract_p(&p,end,dt,sizeof(dt)-1);

  if (beg == 0) {
    printf("Error parsing respnse from server\n");
    return 1;
  }
  
  printf("%s/show/",pastebin_uri); 
  fflush(stdout);
  write(STDOUT_FILENO,beg+1,p-beg-1);
  printf("/\n");

  return 0;
}

static int form_request(void) {
  request_data    = yu_string_new();
  request_headers = yu_string_new();
  
  yu_string_sprintfa(request_data,json_request,
                     escaped_language->str,
                     escaped_filename->str,
                     escaped_mime->str,
                     parent_id,
                     is_paste_private,
                     escaped_code->str);

  yu_string_sprintfa(request_headers,headers,
                     pastebin_root,
                     pastebin_host,
                     pastebin_port,
                     request_data->len);
  return 0;
}

static int escape_data(void) {
  escaped_code     = yu_string_new();
  escaped_language = yu_string_new();
  escaped_mime     = yu_string_new();
  escaped_filename = yu_string_new();
  
  if (escape_json_string(escaped_code,filedata->str,filedata->len) != 0) {
    return 1;
  }

  if (escape_json_string(escaped_language,language_id,strlen(language_id)) != 0) {
    return 1;
  }

  if (escape_json_string(escaped_mime,mime_type,strlen(mime_type)) != 0) {
    return 1;
  }

  if (escape_json_string(escaped_filename,
                         remote_filename,strlen(remote_filename)) != 0) {
    return 1;
  }

  return 0;
}

static int escape_json_string(YUString *out, char *in, size_t len) {
  unsigned char *p;
  unsigned char *oldp;
  unsigned char *end;
  int skiplen;
  uint32_t unichar;

  p = (unsigned char*)in;
  end = p + len;

  while (p != end) {
    unichar = next_utf8_char((char*)p,(char*)end,&skiplen);
    if (skiplen == 0) {
      yu_string_sprintfa(out, "\\u%04x",*p++);
      continue;
    }

    oldp = p;
    p += skiplen;
    switch (unichar) {
      case  ' ': yu_string_append0(out,  " "); continue;
      case '\b': yu_string_append0(out,"\\b"); continue;
      case '\f': yu_string_append0(out,"\\f"); continue;
      case '\n': yu_string_append0(out,"\\n"); continue;
      case '\r': yu_string_append0(out,"\\r"); continue;
      case '\t': yu_string_append0(out,"\\t"); continue;
      case '\v': yu_string_append0(out,"\\v"); continue;
      case  '"': yu_string_append0(out,"\\\"");continue;
      case '\\': yu_string_append0(out,"\\\\");continue;
    }
    if (unichar < 32) {
      yu_string_sprintfa(out, "\\u%04x",unichar);
      continue;
    }
    yu_string_append(out,(char*)oldp,(size_t)skiplen);
  }

  return 0;
}

static int read_file(void) {
  int count;

  filedata = yu_string_new();

  while ((count = read(filefd,buf,sizeof(buf))) > 0) {
    yu_string_append(filedata,buf,count);
  }

  if (count < 0) {
    perror("read");
    return 1;
  }

  if (filedata->len == 0) {
    fprintf(stderr,"Empty file, not pasting anything\n");
    return 1;
  }

  return 0;
}

static int open_socket(void) {
  int              status;
  struct addrinfo  hints;
  struct addrinfo *res;

  memset(&hints,0,sizeof(hints));

  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  if ((status = getaddrinfo(pastebin_host,pastebin_port,&hints,&res)) != 0) {
    fprintf(stderr, "%s\n", gai_strerror(status));
    return 1;
  }

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (sockfd == -1) {
    perror("socket");
    freeaddrinfo(res);
    return 1;
  }

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    perror("connect");
    freeaddrinfo(res);
    return 1;
  }

  freeaddrinfo(res);
  return 0;
}

static void signal_cleanup(int sig) {
  cleanup();
  exit(0);
}

static void cleanup(void) {
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
  if (filedata != 0) {
    yu_string_free(filedata);
    filedata = 0;
  }
  if (escaped_code != 0) {
    yu_string_free(escaped_code);
    escaped_code = 0;
  }
  if (escaped_language != 0) {
    yu_string_free(escaped_language);
    escaped_language = 0;
  }
  if (escaped_mime != 0) {
    yu_string_free(escaped_mime);
    escaped_mime = 0;
  }
  if (escaped_filename != 0) {
    yu_string_free(escaped_filename);
    escaped_filename = 0;
  }
  if (request_data != 0) {
    yu_string_free(request_data);
    request_data = 0;
  }
  if (request_headers != 0) {
    yu_string_free(request_headers);
    request_headers = 0;
  }
  if (reply != 0) {
    yu_string_free(reply);
    reply = 0;
  }
}

static int parse_uri(void) {
  char *host_beg = 0;
  char *host_end = 0;
  char *port_beg = 0;
  char *port_end = 0;
  char *path_beg = 0;
  
  host_beg = pastebin_uri;
  if (strncmp("http://",pastebin_uri,7) == 0) {
    host_beg += 7;
  }

  host_end = host_beg;

  while (*host_end != ':' && *host_end != '/' && *host_end != '\0') host_end++;

  if (host_beg == host_end) {
    fprintf(stderr, "Empty host in URI\n");
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
      fprintf(stderr, "Empty port in URI\n");
      return 1;
    }

    pastebin_port = strndup(port_beg, port_end - port_beg);
  } else {
    pastebin_port = strdup("80");
  }

  pastebin_root = strdup(path_beg);
  return 0;
}

static int open_file(void) {
  char *arg;

  if (args[0] == 0) {
    filefd = STDIN_FILENO;
    if (remote_filename == 0) {
      remote_filename = "stdin";
    }
  } else {
    arg = args[0];
    if (strncmp("-",arg,1) == 0) {
      filefd = STDIN_FILENO;
      if (remote_filename == 0) {
        remote_filename = "stdin";
      }
    } else { 
      filefd = open(arg,O_RDONLY);
      if (filefd == -1) {
        perror(arg);
        return 1;
      } else if (remote_filename == 0) {
        remote_filename = arg;
      }
    }
  }
  return 0;
}

static void print_help(char *argv0) {
  fprintf(stdout, "%s %s\n", argv0,
      "[OPTION...] <FILE> or - for stdin (default)\n"
      "\n" 
      "    -h        Prints help message\n"
      "    -u URI    Sets pastebin URI\n"
      "    -l LANG   Sets paste language\n"
      "    -m MIME   Sets mime type of paste\n"
      "    -f NAME   Sets remote name of paste\n"
      "    -i ID     Sets parent id\n"
      "    -p        Marks paste as private\n"
      );
  exit(0);
}

static int parse_options(int argc, char **argv) {
  int opt;
  opterr = 1;
  while ((opt = getopt(argc, argv, "hu:l:m:f:i:p")) != -1) {
    switch (opt) {
      case 'u': pastebin_uri = optarg;    break;
      case 'l': language_id = optarg;     break;
      case 'm': mime_type = optarg;       break;
      case 'f': remote_filename = optarg; break;
      case 'i': parent_id = atoi(optarg); break;
      case 'p': is_paste_private = 1;     break;
      default: /* FALLTHROUGH */
      case 'h': print_help(argv[0]);      break;
    }
  }

  args = argv + optind;
  return 0;
}
