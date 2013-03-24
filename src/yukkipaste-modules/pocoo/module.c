#include "yukkipaste-api/yukkipaste-module.h"
#include "yutils/yujson.h"

#include <stdlib.h>

/* This macro defines module-specific constnats */
MODULE_INFO("pocoo", "Pocoo pastebin interface", "Iamtakingiteasy", "1.0")

/* Defines pastebin-specific constants */
PASTEBIN_INFO("http://paste.eientei.org", "text")

/* Defines loads of variables and initializes them from host system */
/* see yukkipaste-api/yukkipaste-module.h for details */
DEFAULT_GLOBAL_VARS

/* NULL-terminated list of available languages */
char *PASTEBIN_AVAIL_LANGS[] = {
  "abap",
  "antlr",
  "antlr-as",
  "antlr-csharp",
  "antlr-cpp",
  "antlr-java",
  "antlr-objc",
  "antlr-perl",
  "antlr-python",
  "antlr-ruby",
  "as",
  "as3",
  "ada",
  "apacheconf",
  "applescript",
  "aspectj",
  "asy",
  "autoit",
  "awk",
  "bbcode",
  "bugs",
  "basemake",
  "bash",
  "console",
  "bat",
  "befunge",
  "blitzmax",
  "boo",
  "brainfuck",
  "bro",
  "c",
  "csharp",
  "cpp",
  "cbmbas",
  "cfengine3",
  "cmake",
  "cobol",
  "cobolfree",
  "css",
  "css+django",
  "css+genshitext",
  "css+lasso",
  "css+mako",
  "css+myghty",
  "css+php",
  "css+erb",
  "css+smarty",
  "csv",
  "cuda",
  "ceylon",
  "cheetah",
  "clojure",
  "coffee-script",
  "cfm",
  "common-lisp",
  "coq",
  "creole",
  "croc",
  "cython",
  "d",
  "dtd",
  "dpatch",
  "dart",
  "control",
  "sourceslist",
  "delphi",
  "django",
  "duel",
  "dylan",
  "dylan-console",
  "dylan-lid",
  "ecl",
  "erb",
  "elixir",
  "iex",
  "ragel-em",
  "erlang",
  "erl",
  "evoque",
  "fsharp",
  "factor",
  "fancy",
  "fan",
  "felix",
  "fortran",
  "Clipper",
  "gas",
  "gcc-messages",
  "glsl",
  "genshi",
  "genshitext",
  "pot",
  "Cucumber",
  "gnuplot",
  "go",
  "gooddata-cl",
  "gosu",
  "gst",
  "groff",
  "groovy",
  "html",
  "html+cheetah",
  "html+django",
  "html+evoque",
  "html+genshi",
  "html+lasso",
  "html+mako",
  "html+myghty",
  "html+php",
  "html+smarty",
  "html+velocity",
  "http",
  "haml",
  "haskell",
  "haxeml",
  "hybris",
  "idl",
  "ini",
  "irc",
  "io",
  "ioke",
  "jags",
  "json",
  "jade",
  "java",
  "jsp",
  "js",
  "js+cheetah",
  "js+django",
  "js+genshitext",
  "js+lasso",
  "js+mako",
  "js+myghty",
  "js+php",
  "js+erb",
  "js+smarty",
  "julia",
  "jlcon",
  "kconfig",
  "koka",
  "kotlin",
  "llvm",
  "lasso",
  "lighty",
  "lhs",
  "live-script",
  "logos",
  "logtalk",
  "lua",
  "maql",
  "moocode",
  "mxml",
  "make",
  "mako",
  "mason",
  "matlab",
  "matlabsession",
  "minid",
  "modelica",
  "modula2",
  "trac-wiki",
  "monkey",
  "moon",
  "mscgen",
  "mupad",
  "multi",
  "mysql",
  "myghty",
  "nasm",
  "nsis",
  "nemerle",
  "newlisp",
  "newspeak",
  "nginx",
  "nimrod",
  "numpy",
  "ocaml",
  "objective-c",
  "objective-c++",
  "objective-j",
  "octave",
  "ooc",
  "opa",
  "openedge",
  "php",
  "plpgsql",
  "pov",
  "perl",
  "postscript",
  "postgresql",
  "psql",
  "powershell",
  "prolog",
  "properties",
  "protobuf",
  "puppet",
  "pypylog",
  "python",
  "python3",
  "py3tb",
  "pytb",
  "pycon",
  "qml",
  "rconsole",
  "rebol",
  "rhtml",
  "spec",
  "racket",
  "ragel",
  "ragel-c",
  "ragel-cpp",
  "ragel-d",
  "ragel-java",
  "ragel-objc",
  "ragel-ruby",
  "raw",
  "rd",
  "redcode",
  "RobotFramework",
  "rb",
  "rbcon",
  "rust",
  "splus",
  "scss",
  "sql",
  "sass",
  "scala",
  "ssp",
  "scaml",
  "scheme",
  "scilab",
  "shell-session",
  "smali",
  "smalltalk",
  "smarty",
  "snobol",
  "sp",
  "squidconf",
  "stan",
  "sml",
  "tcl",
  "tcsh",
  "tex",
  "tea",
  "text",
  "treetop",
  "ts",
  "diff",
  "urbiscript",
  "vb.net",
  "vgl",
  "vala",
  "velocity",
  "vim",
  "xml",
  "xml+cheetah",
  "xml+django",
  "xml+evoque",
  "xml+lasso",
  "xml+mako",
  "xml+myghty",
  "xml+php",
  "xml+erb",
  "xml+smarty",
  "xml+velocity",
  "xquery",
  "xslt",
  "xtend",
  "yaml",
  "aspx-cs",
  "aspx-vb",
  "ahk",
  "c-objdump",
  "ca65",
  "cfs",
  "cpp-objdump",
  "d-objdump",
  "dg",
  "ec",
  "hx",
  "javac-messages",
  "objdump",
  "rst",
  "registry",
  "sqlite3",
  "systemverilog",
  "verilog",
  "vhdl",
  0
};


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

static YUString *yus_data = 0;
static YUString *yus_uri  = 0;
static YUString *yus_err  = 0;


int INIT_MODULE_FUNC(void) {
  escaped_lang     = yu_string_new();
  escaped_filename = yu_string_new();
  escaped_mime     = yu_string_new();
  escaped_code     = yu_string_new();
  yus_data         = yu_string_new();
  yus_uri          = yu_string_new();
  yus_err          = yu_string_new();
  return 0;
}

int FORM_REQUEST_FUNC(char **post,
                      char **type,
                      char **data,
                      int   *len) {
  int parent_id = 0;

  escape_json_string0(escaped_lang,PTR_LANG);  
  escape_json_string0(escaped_filename,PTR_FILENAME);
  escape_json_string0(escaped_mime,PTR_MIME);
  escape_json_string0(escaped_code,PTR_DATA);

  parent_id = atoi(PTR_PARENT);

  yu_string_sprintfa(yus_data, json_request,
                     escaped_lang->str,
                     escaped_filename->str,
                     escaped_mime->str,
                     parent_id,
                     PTR_PRIVATE,
                     escaped_code->str);

  *post = "/json/?method=pastes.newPaste";
  *type = "application/json";
  *data = yus_data->str;
  *len  = yus_data->len;
  return 0;
}


int PROCESS_REPLY_FUNC(char *reply, char **uri, char **err) {
  int ret = 0;

  yu_string_append0(yus_uri,PTR_URI);
  
  if (yus_uri->str[yus_uri->len-1] != '/') {
    yu_string_append0(yus_uri,"/");
  }
  yu_string_append0(yus_uri,"show/");

  if (json_extract_string0(yus_err, reply, "error") != 0) {
    ret = 1;
    goto process_reply_func_free_and_return;
  }
  if (yus_err->len > 0) {
    ret = 1;
    goto process_reply_func_free_and_return;
  }
  
  if (json_extract_string0(yus_uri, reply, "data") != 0) {
    ret = 1;
    goto process_reply_func_free_and_return;
  }

  *uri = yus_uri->str;
  *err = yus_err->str;

process_reply_func_free_and_return:
  return ret;
}

int DEINIT_MODULE_FUNC(void) {
  yu_string_guarded_free0(escaped_lang);
  yu_string_guarded_free0(escaped_filename);
  yu_string_guarded_free0(escaped_mime);
  yu_string_guarded_free0(escaped_code);
  yu_string_guarded_free0(yus_data);
  yu_string_guarded_free0(yus_uri);
  yu_string_guarded_free0(yus_err);
  return 0;
}

