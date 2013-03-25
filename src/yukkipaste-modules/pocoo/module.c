#include "yukkipaste-api/yukkipaste-module.h"
#include "yutils/yujson.h"

#include <stdlib.h>

/* This macro defines module-specific constnats */
MODULE_INFO("pocoo", "Pocoo pastebin interface", "Iamtakingiteasy", "1.0")

/* Defines pastebin-specific constants */
PASTEBIN_INFO("http://paste.eientei.org", "text")

/* Defines loads of variables and initializes them from host system 
 * see yukkipaste-api/yukkipaste-module.h for details 
 */
DEFAULT_GLOBAL_VARS

/* NULL-terminated list of available languages */
char *PASTEBIN_AVAIL_LANGS[][2] = {
  { "abap",            "abap"            },
  { "antlr",           "antlr"           },
  { "antlr-as",        "antlr-as"        },
  { "antlr-csharp",    "antlr-csharp"    },
  { "antlr-cpp",       "antlr-cpp"       },
  { "antlr-java",      "antlr-java"      },
  { "antlr-objc",      "antlr-objc"      },
  { "antlr-perl",      "antlr-perl"      },
  { "antlr-python",    "antlr-python"    },
  { "antlr-ruby",      "antlr-ruby"      },
  { "as",              "as"              },
  { "as3",             "as3"             },
  { "ada",             "ada"             },
  { "apacheconf",      "apacheconf"      },
  { "applescript",     "applescript"     },
  { "aspectj",         "aspectj"         },
  { "asy",             "asy"             },
  { "autoit",          "autoit"          },
  { "awk",             "awk"             },
  { "bbcode",          "bbcode"          },
  { "bugs",            "bugs"            },
  { "basemake",        "basemake"        },
  { "bash",            "bash"            },
  { "console",         "console"         },
  { "bat",             "bat"             },
  { "befunge",         "befunge"         },
  { "blitzmax",        "blitzmax"        },
  { "boo",             "boo"             },
  { "brainfuck",       "brainfuck"       },
  { "bro",             "bro"             },
  { "c",               "c"               },
  { "csharp",          "csharp"          },
  { "cpp",             "cpp"             },
  { "cbmbas",          "cbmbas"          },
  { "cfengine3",       "cfengine3"       },
  { "cmake",           "cmake"           },
  { "cobol",           "cobol"           },
  { "cobolfree",       "cobolfree"       },
  { "css",             "css"             },
  { "css+django",      "css+django"      },
  { "css+genshitext",  "css+genshitext"  },
  { "css+lasso",       "css+lasso"       },
  { "css+mako",        "css+mako"        },
  { "css+myghty",      "css+myghty"      },
  { "css+php",         "css+php"         },
  { "css+erb",         "css+erb"         },
  { "css+smarty",      "css+smarty"      },
  { "csv",             "csv"             },
  { "cuda",            "cuda"            },
  { "ceylon",          "ceylon"          },
  { "cheetah",         "cheetah"         },
  { "clojure",         "clojure"         },
  { "coffee-script",   "coffee-script"   },
  { "cfm",             "cfm"             },
  { "common-lisp",     "common-lisp"     },
  { "coq",             "coq"             },
  { "creole",          "creole"          },
  { "croc",            "croc"            },
  { "cython",          "cython"          },
  { "d",               "d"               },
  { "dtd",             "dtd"             },
  { "dpatch",          "dpatch"          },
  { "dart",            "dart"            },
  { "control",         "control"         },
  { "sourceslist",     "sourceslist"     },
  { "delphi",          "delphi"          },
  { "django",          "django"          },
  { "duel",            "duel"            },
  { "dylan",           "dylan"           },
  { "dylan-console",   "dylan-console"   },
  { "dylan-lid",       "dylan-lid"       },
  { "ecl",             "ecl"             },
  { "erb",             "erb"             },
  { "elixir",          "elixir"          },
  { "iex",             "iex"             },
  { "ragel-em",        "ragel-em"        },
  { "erlang",          "erlang"          },
  { "erl",             "erl"             },
  { "evoque",          "evoque"          },
  { "fsharp",          "fsharp"          },
  { "factor",          "factor"          },
  { "fancy",           "fancy"           },
  { "fan",             "fan"             },
  { "felix",           "felix"           },
  { "fortran",         "fortran"         },
  { "Clipper",         "Clipper"         },
  { "gas",             "gas"             },
  { "gcc-messages",    "gcc-messages"    },
  { "glsl",            "glsl"            },
  { "genshi",          "genshi"          },
  { "genshitext",      "genshitext"      },
  { "pot",             "pot"             },
  { "Cucumber",        "Cucumber"        },
  { "gnuplot",         "gnuplot"         },
  { "go",              "go"              },
  { "gooddata-cl",     "gooddata-cl"     },
  { "gosu",            "gosu"            },
  { "gst",             "gst"             },
  { "groff",           "groff"           },
  { "groovy",          "groovy"          },
  { "html",            "html"            },
  { "html+cheetah",    "html+cheetah"    },
  { "html+django",     "html+django"     },
  { "html+evoque",     "html+evoque"     },
  { "html+genshi",     "html+genshi"     },
  { "html+lasso",      "html+lasso"      },
  { "html+mako",       "html+mako"       },
  { "html+myghty",     "html+myghty"     },
  { "html+php",        "html+php"        },
  { "html+smarty",     "html+smarty"     },
  { "html+velocity",   "html+velocity"   },
  { "http",            "http"            },
  { "haml",            "haml"            },
  { "haskell",         "haskell"         },
  { "haxeml",          "haxeml"          },
  { "hybris",          "hybris"          },
  { "idl",             "idl"             },
  { "ini",             "ini"             },
  { "irc",             "irc"             },
  { "io",              "io"              },
  { "ioke",            "ioke"            },
  { "jags",            "jags"            },
  { "json",            "json"            },
  { "jade",            "jade"            },
  { "java",            "java"            },
  { "jsp",             "jsp"             },
  { "js",              "js"              },
  { "js+cheetah",      "js+cheetah"      },
  { "js+django",       "js+django"       },
  { "js+genshitext",   "js+genshitext"   },
  { "js+lasso",        "js+lasso"        },
  { "js+mako",         "js+mako"         },
  { "js+myghty",       "js+myghty"       },
  { "js+php",          "js+php"          },
  { "js+erb",          "js+erb"          },
  { "js+smarty",       "js+smarty"       },
  { "julia",           "julia"           },
  { "jlcon",           "jlcon"           },
  { "kconfig",         "kconfig"         },
  { "koka",            "koka"            },
  { "kotlin",          "kotlin"          },
  { "llvm",            "llvm"            },
  { "lasso",           "lasso"           },
  { "lighty",          "lighty"          },
  { "lhs",             "lhs"             },
  { "live-script",     "live-script"     },
  { "logos",           "logos"           },
  { "logtalk",         "logtalk"         },
  { "lua",             "lua"             },
  { "maql",            "maql"            },
  { "moocode",         "moocode"         },
  { "mxml",            "mxml"            },
  { "make",            "make"            },
  { "mako",            "mako"            },
  { "mason",           "mason"           },
  { "matlab",          "matlab"          },
  { "matlabsession",   "matlabsession"   },
  { "minid",           "minid"           },
  { "modelica",        "modelica"        },
  { "modula2",         "modula2"         },
  { "trac-wiki",       "trac-wiki"       },
  { "monkey",          "monkey"          },
  { "moon",            "moon"            },
  { "mscgen",          "mscgen"          },
  { "mupad",           "mupad"           },
  { "multi",           "multi"           },
  { "mysql",           "mysql"           },
  { "myghty",          "myghty"          },
  { "nasm",            "nasm"            },
  { "nsis",            "nsis"            },
  { "nemerle",         "nemerle"         },
  { "newlisp",         "newlisp"         },
  { "newspeak",        "newspeak"        },
  { "nginx",           "nginx"           },
  { "nimrod",          "nimrod"          },
  { "numpy",           "numpy"           },
  { "ocaml",           "ocaml"           },
  { "objective-c",     "objective-c"     },
  { "objective-c++",   "objective-c++"   },
  { "objective-j",     "objective-j"     },
  { "octave",          "octave"          },
  { "ooc",             "ooc"             },
  { "opa",             "opa"             },
  { "openedge",        "openedge"        },
  { "php",             "php"             },
  { "plpgsql",         "plpgsql"         },
  { "pov",             "pov"             },
  { "perl",            "perl"            },
  { "postscript",      "postscript"      },
  { "postgresql",      "postgresql"      },
  { "psql",            "psql"            },
  { "powershell",      "powershell"      },
  { "prolog",          "prolog"          },
  { "properties",      "properties"      },
  { "protobuf",        "protobuf"        },
  { "puppet",          "puppet"          },
  { "pypylog",         "pypylog"         },
  { "python",          "python"          },
  { "python3",         "python3"         },
  { "py3tb",           "py3tb"           },
  { "pytb",            "pytb"            },
  { "pycon",           "pycon"           },
  { "qml",             "qml"             },
  { "rconsole",        "rconsole"        },
  { "rebol",           "rebol"           },
  { "rhtml",           "rhtml"           },
  { "spec",            "spec"            },
  { "racket",          "racket"          },
  { "ragel",           "ragel"           },
  { "ragel-c",         "ragel-c"         },
  { "ragel-cpp",       "ragel-cpp"       },
  { "ragel-d",         "ragel-d"         },
  { "ragel-java",      "ragel-java"      },
  { "ragel-objc",      "ragel-objc"      },
  { "ragel-ruby",      "ragel-ruby"      },
  { "raw",             "raw"             },
  { "rd",              "rd"              },
  { "redcode",         "redcode"         },
  { "robot-framework", "RobotFramework"  },
  { "rb",              "rb"              },
  { "rbcon",           "rbcon"           },
  { "rust",            "rust"            },
  { "splus",           "splus"           },
  { "scss",            "scss"            },
  { "sql",             "sql"             },
  { "sass",            "sass"            },
  { "scala",           "scala"           },
  { "ssp",             "ssp"             },
  { "scaml",           "scaml"           },
  { "scheme",          "scheme"          },
  { "scilab",          "scilab"          },
  { "shell-session",   "shell-session"   },
  { "smali",           "smali"           },
  { "smalltalk",       "smalltalk"       },
  { "smarty",          "smarty"          },
  { "snobol",          "snobol"          },
  { "sp",              "sp"              },
  { "squidconf",       "squidconf"       },
  { "stan",            "stan"            },
  { "sml",             "sml"             },
  { "tcl",             "tcl"             },
  { "tcsh",            "tcsh"            },
  { "tex",             "tex"             },
  { "tea",             "tea"             },
  { "text",            "text"            },
  { "treetop",         "treetop"         },
  { "ts",              "ts"              },
  { "diff",            "diff"            },
  { "urbiscript",      "urbiscript"      },
  { "vb.net",          "vb.net"          },
  { "vgl",             "vgl"             },
  { "vala",            "vala"            },
  { "velocity",        "velocity"        },
  { "vim",             "vim"             },
  { "xml",             "xml"             },
  { "xml+cheetah",     "xml+cheetah"     },
  { "xml+django",      "xml+django"      },
  { "xml+evoque",      "xml+evoque"      },
  { "xml+lasso",       "xml+lasso"       },
  { "xml+mako",        "xml+mako"        },
  { "xml+myghty",      "xml+myghty"      },
  { "xml+php",         "xml+php"         },
  { "xml+erb",         "xml+erb"         },
  { "xml+smarty",      "xml+smarty"      },
  { "xml+velocity",    "xml+velocity"    },
  { "xquery",          "xquery"          },
  { "xslt",            "xslt"            },
  { "xtend",           "xtend"           },
  { "yaml",            "yaml"            },
  { "aspx-cs",         "aspx-cs"         },
  { "aspx-vb",         "aspx-vb"         },
  { "ahk",             "ahk"             },
  { "c-objdump",       "c-objdump"       },
  { "ca65",            "ca65"            },
  { "cfs",             "cfs"             },
  { "cpp-objdump",     "cpp-objdump"     },
  { "d-objdump",       "d-objdump"       },
  { "dg",              "dg"              },
  { "ec",              "ec"              },
  { "hx",              "hx"              },
  { "javac-messages",  "javac-messages"  },
  { "objdump",         "objdump"         },
  { "rst",             "rst"             },
  { "registry",        "registry"        },
  { "sqlite3",         "sqlite3"         },
  { "systemverilog",   "systemverilog"   },
  { "verilog",         "verilog"         },
  { "vhdl",            "vhdl"            },
  { 0,                 0                 }
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

