// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "screen/Screen.h"
#include "translator/Translator.h"

#ifndef _WIN32

#include <unistd.h>

namespace kgt {
extern "C" {
int debug = 0;
int prettify = 1;
int allow_undefined = 1;
const char* css_file = nullptr;
#include "abnf/io.h"
#include "blab/io.h"
#include "bnf/io.h"
#include "dot/io.h"
#include "ebnfhtml5/io.h"
#include "html5/io.h"
#include "iso-ebnf/io.h"
#include "rbnf/io.h"
#include "rrtext/io.h"
#include "sid/io.h"
#include "svg/io.h"
#include "wsn/io.h"
}
}  // namespace kgt

namespace {

struct StringReader {
  StringReader(std::string input) : str(input) {}
  static int Read(void* opaque) {
    auto* string_reader = static_cast<StringReader*>(opaque);
    if (string_reader->index < string_reader->str.size())
      return string_reader->str[string_reader->index++];
    else
      return -1;
  }

  std::string str;
  int index = 0;
};

using AstRulePtr = kgt::ast_rule*;
using Opaque = void*;
using OpaqueRead = int(Opaque);
using InputFunction = AstRulePtr(OpaqueRead, Opaque, kgt::parsing_error_queue*);
using OutputFunction = int(const struct kgt::ast_rule*);

std::map<std::string, InputFunction*> input_function_map = {
    {"abnf", kgt::abnf_input},         {"bnf", kgt::bnf_input},
    {"iso-ebnf", kgt::iso_ebnf_input}, {"rbnf", kgt::rbnf_input},
    {"wsn", kgt::wsn_input},
};

std::map<std::string, OutputFunction*> output_function_map = {
    {"ascii", kgt::rrtext_output},
    {"unicode", kgt::rrutf8_output},

    {"abnf", kgt::abnf_output},
    {"blab", kgt::blab_output},
    {"bnf", kgt::bnf_output},
    {"iso-ebnf", kgt::iso_ebnf_output},
    {"rbnf", kgt::rbnf_output},
    {"wsn", kgt::wsn_output},

    {"ebnfhtml5", kgt::ebnf_html5_output},
    {"ebnfxhtml5", kgt::ebnf_xhtml5_output},
    {"html5", kgt::html5_output},
    {"svg", kgt::svg_output},
    {"xhtml5", kgt::xhtml5_output},

    //{"sid", kgt::sid_output},

    //{"rrdot", kgt::rrdot_output},
    //{"rrdump", kgt::rrdump_output},
    //{"rrtdump", kgt::rrtdump_output},
    //{"rrparcon", kgt::rrparcon_output},
    //{"rrll", kgt::rrll_output},
    //{"rrta", kgt::rrta_output},
};
// clang-format on

}  // namespace

#endif

class Grammar : public Translator {
 public:
  virtual ~Grammar() = default;
  const char* Identifier() final { return "Grammar"; }
  const char* Name() final { return "Grammar (kgt - experimental)"; }
  const char* Description() final {
    return "Railroad diagram from grammar descriptions";
  }
  std::vector<Translator::OptionDescription> Options() final;
  std::vector<Translator::Example> Examples() final;
  std::string Translate(const std::string& input,
                        const std::string& options_string) final;
};

std::vector<Translator::OptionDescription> Grammar::Options() {
  return {
      {
          "input",
          {
              "abnf",
              "bnf",
              "iso-ebnf",
              "rbnf",
              "wsn",
          },
          "abnf",
          "The format of the input",
          Widget::Combobox,
      },
      {
          "output",
          {
              "unicode", "ascii",

              "svg", "html5", "xhtml5", "ebnfhtml5", "ebnfxhtml5",

              "abnf", "blab", "bnf", "iso-ebnf", "rbnf", "wsn",

              //"dot",

              //"sid",
              //"rrdot",
              //"rrdump",
              //"rrll",
              //"rrparcon",
              //"rrta",
              //"rrtdump",
          },
          "unicode",
          "The format of the output",
          Widget::Combobox,
      },
  };
}

std::vector<Translator::Example> Grammar::Examples() {
  return {
      {
          "ABNF - URL",
          R"(
URL = domain [path] [attributes] [fragment]

domain = scheme "://" [credential] host [port] 

scheme = "http" / "https"

credential = username [":" password]"@"

host = 1*(subdomain ".") domain

port = ":" number

path = "/" 

attributes = "?" attribute-key-value *("&" attribute-key-value)

attribute-key-pair = key ["=" value]
)",
      },
      {"ABNF - CSP",
       R"(
source-list       = *WSP [ source-expression *( 1*WSP source-expression ) *WSP ]
                  / *WSP "'none'" *WSP

source-expression = scheme-source
                  / host-source
                  / keyword-source
                  / nonce-source
                  / hash-source

scheme-source     = scheme-part ":"

host-source       = [ scheme-part "://" ] host-part [ port-part ] [ path-part ]

keyword-source    = "'self'"
                  / "'unsafe-inline'"
                  / "'unsafe-eval'"


base64-value      = 1*( ALPHA / DIGIT / "+" / "/" )*2( "=" )

nonce-value       = base64-value

hash-value        = base64-value

nonce-source      = "'nonce-" nonce-value "'"

hash-algo         = "sha256"
                  / "sha384"
                  / "sha512"

hash-source       = "'" hash-algo "-" hash-value "'"

scheme-part       = <scheme production from RFC 3986, section 3.1>

host-part         = "*" / [ "*." ] 1*host-char *( "." 1*host-char )

host-char         = ALPHA
                  / DIGIT
                  / "-"

path-part         = <path production from RFC 3986, section 3.3>

port-part         = ":" ( 1*DIGIT / "*" )
)"},
      {
          "ISO-EBNF - EBNF",
          R"--(
letter = "A" | "B" | "C" | "D" | "E" | "F" | "G"
       | "H" | "I" | "J" | "K" | "L" | "M" | "N"
       | "O" | "P" | "Q" | "R" | "S" | "T" | "U"
       | "V" | "W" | "X" | "Y" | "Z" | "a" | "b"
       | "c" | "d" | "e" | "f" | "g" | "h" | "i"
       | "j" | "k" | "l" | "m" | "n" | "o" | "p"
       | "q" | "r" | "s" | "t" | "u" | "v" | "w"
       | "x" | "y" | "z" ;
digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
symbol = "[" | "]" | "{" | "}" | "(" | ")" | "<" | ">"
       | "'" | '"' | "=" | "|" | "." | "," | ";" ;
character = letter | digit | symbol | "_" ;
 
identifier = letter , { letter | digit | "_" } ;
terminal = "'" , character , { character } , "'" 
         | '"' , character , { character } , '"' ;
 
lhs = identifier ;
rhs = identifier
     | terminal
     | "[" , rhs , "]"
     | "{" , rhs , "}"
     | "(" , rhs , ")"
     | rhs , "|" , rhs
     | rhs , "," , rhs ;

rule = lhs , "=" , rhs , ";" ;
grammar = { rule } ;
)--",
      },
      {"WSN - WSN",
       R"--(
SYNTAX     = { PRODUCTION } .
PRODUCTION = IDENTIFIER "=" EXPRESSION "." .
EXPRESSION = TERM { "|" TERM } .
TERM       = FACTOR { FACTOR } .
FACTOR     = IDENTIFIER
          | LITERAL
          | "[" EXPRESSION "]"
          | "(" EXPRESSION ")"
          | "{" EXPRESSION "}" .
IDENTIFIER = letter { letter } .
LITERAL    = """" character { character } """" .
)--"},
  };
}

#ifndef _WIN32
std::string Grammar::Translate(const std::string& input,
                               const std::string& options_string) {
  // Duplicate stdout, so that we can restore it later.
  int old_stdout = dup(1);

  // Redirect stdout to file.
  FILE* file_write = fopen("/tmp/diagon_grammer.txt", "w");
  dup2(fileno(file_write), 1);

  // Read string to model.
  auto string_reader = StringReader(input);

  auto options = SerializeOption(options_string);

  std::string option_input = options["input"];
  std::string option_output = options["output"];

  auto input_function = input_function_map.count(option_input)
                            ? input_function_map[option_input]
                            : kgt::abnf_input;
  auto output_function = output_function_map.count(option_output)
                             ? output_function_map[option_output]
                             : kgt::rrutf8_output;

  int error_count = 0;
  kgt::parsing_error_queue parsing_errors = NULL;
  auto* model =
      input_function(StringReader::Read, &string_reader, &parsing_errors);

  while (parsing_errors) {
    error_count++;
    kgt::parsing_error error;
    parsing_error_queue_pop(&parsing_errors, &error);
    std::cout << error.line << ":" << error.column << ":" << error.description
              << std::endl;
  }

  int error = output_function(model);
  (void)error;
  dup2(old_stdout, 1);
  close(old_stdout);
  fclose(file_write);


  auto file_read = std::ifstream("/tmp/diagon_grammer.txt");
  return std::string((std::istreambuf_iterator<char>(file_read)),
                     std::istreambuf_iterator<char>());
}
#else
std::string Grammar::Translate(const std::string& input,
                               const std::string& options_string) {
  return "Not supported on Windows";
}
#endif

std::unique_ptr<Translator> GrammarTranslator() {
  return std::make_unique<Grammar>();
}
