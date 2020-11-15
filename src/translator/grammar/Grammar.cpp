// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "translator/grammar/Grammar.hpp"

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "screen/Screen.h"

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
#include "rrdot/io.h"
#include "rrdump/io.h"
#include "rrll/io.h"
#include "rrparcon/io.h"
#include "rrta/io.h"
#include "rrtdump/io.h"
#include "rrtext/io.h"
#include "sid/io.h"
#include "svg/io.h"
#include "wsn/io.h"
}
}  // namespace kgt

const char* Grammar::Name() {
  return "Grammar";
}

const char* Grammar::Description() {
  return "Railroad diagram from grammar descriptions";
}

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
      },
      {
          "output",
          {
              "bnf",    "blab",     "ebnfhtml5", "ebnfxhtml5", "wsn",
              "abnf",   "iso-ebnf", "rbnf",      "sid",        "dot",
              "rrdot",  "rrdump",   "rrtdump",   "rrparcon",   "rrll",
              "rrta",   "rrtext",   "rrutf8",    "svg",        "html5",
              "xhtml5",
          },
          "rrutf8",
          "The format of the output",
      },
  };
}

std::vector<Translator::Example> Grammar::Examples() {
  return {};
}

std::unique_ptr<Translator> GrammarTranslator() {
  return std::make_unique<Grammar>();
}

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
using InputFunction = AstRulePtr(OpaqueRead, Opaque);
using OutputFunction = void(const struct kgt::ast_rule*);

InputFunction* f = kgt::abnf_input;

std::map<std::string, InputFunction*> input_function_map = {
    {"abnf", kgt::abnf_input},
    //
    {"bnf", kgt::bnf_input},
    {"wsn", kgt::wsn_input},
    {"iso-ebnf", kgt::iso_ebnf_input},
    {"rbnf", kgt::rbnf_input},
};

std::map<std::string, OutputFunction*> output_function_map = {
    {"rrutf8", kgt::rrutf8_output},
    //
    {"bnf", kgt::bnf_output},
    {"blab", kgt::blab_output},
    {"ebnfhtml5", kgt::ebnf_html5_output},
    {"ebnfxhtml5", kgt::ebnf_xhtml5_output},
    {"wsn", kgt::wsn_output},
    {"abnf", kgt::abnf_output},
    {"iso-ebnf", kgt::iso_ebnf_output},
    {"rbnf", kgt::rbnf_output},
    {"sid", kgt::sid_output},
    {"dot", kgt::dot_output},
    {"rrdot", kgt::rrdot_output},
    {"rrdump", kgt::rrdump_output},
    {"rrtdump", kgt::rrtdump_output},
    {"rrparcon", kgt::rrparcon_output},
    {"rrll", kgt::rrll_output},
    {"rrta", kgt::rrta_output},
    {"rrtext", kgt::rrtext_output},
    {"svg", kgt::svg_output},
    {"html5", kgt::html5_output},
    {"xhtml5", kgt::xhtml5_output},
};
// clang-format on

}  // namespace

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

  auto* model = input_function(StringReader::Read, &string_reader);
  output_function(model);

  // Restore stdout
  dup2(old_stdout, 1);
  close(old_stdout);

  fclose(file_write);

  auto file_read = std::ifstream("/tmp/diagon_grammer.txt");
  return std::string((std::istreambuf_iterator<char>(file_read)),
                     std::istreambuf_iterator<char>());
}
