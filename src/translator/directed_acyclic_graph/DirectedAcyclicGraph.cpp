#include <sstream>
#include <vector>

#include "screen/Screen.h"
#include "translator/Translator.h"

std::string DagToText(const std::string& input);

class DirectedAcyclicGraph : public Translator {
 public:
  ~DirectedAcyclicGraph() override = default;
  const char* Identifier() final { return "DAG"; }
  const char* Name() final { return "Directed Acyclic Graph"; }
  const char* Description() final {
    return "Draw a graph, specialized for Directed Acyclic ones";
  }
  std::vector<Translator::OptionDescription> Options() final;
  std::vector<Translator::Example> Examples() final;
  std::string Translate(const std::string& input,
                        const std::string& options_string) final;
};

std::vector<Translator::OptionDescription> DirectedAcyclicGraph::Options() {
  return {
      {
          "ascii_only",
          {
              "false",
              "true",
          },
          "false",
          "Use the full unicode charset or only ASCII.",
          Widget::Checkbox,
      },
      {
          "line_number",
          {
              "false",
              "true",
          },
          "true",
          "Display the line number.",
          Widget::Checkbox,
      },
  };
}

std::vector<Translator::Example> DirectedAcyclicGraph::Examples() {
  return {
      {"1-Hello world",

       "#include <iostream>\n"
       "using namespace std;\n"
       "\n"
       "int main() \n"
       "{\n"
       "    cout << \"Hello, World!\";\n"
       "    return 0;\n"
       "}"},

  };
}

std::string DirectedAcyclicGraph::Translate(const std::string& input,
                             const std::string& options_string) {
  return DagToText(input);
}

std::unique_ptr<Translator> DirectedAcyclicGraphTranslator() {
  return std::make_unique<DirectedAcyclicGraph>();
}

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
