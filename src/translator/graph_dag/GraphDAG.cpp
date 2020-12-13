#include <sstream>
#include <vector>

#include "screen/Screen.h"
#include "translator/Translator.h"

std::string DagToText(const std::string& input);

class GraphDAG : public Translator {
 public:
  ~GraphDAG() override = default;
  const char* Identifier() final { return "GraphDAG"; }
  const char* Name() final { return "Graph - DAG";}
  const char* Description() final {
    return "Draw a graph, specialized for Directed Acyclic ones";
  }
  std::vector<Translator::OptionDescription> Options() final;
  std::vector<Translator::Example> Examples() final;
  std::string Translate(const std::string& input,
                        const std::string& options_string) final;
};

std::vector<Translator::OptionDescription> GraphDAG::Options() {
  return {};
}

std::vector<Translator::Example> GraphDAG::Examples() {
  return {
      {
          "cloths",
          "socks -> shoes\n"
          "underwear -> shoes\n"
          "underwear -> pants\n"
          "pants -> shoes\n"
          "pants -> belt\n"
          "belt -> jacket\n"
          "shirt -> belt\n"
          "shirt -> tie\n"
          "tie -> jacket\n",
      },
      {
          "chromium",
          "chrome -> content\n"
          "chrome -> blink\n"
          "chrome -> base\n"
          "\n"
          "content -> blink\n"
          "content -> net\n"
          "content -> base\n"
          "\n"
          "blink -> v8\n"
          "blink -> CC\n"
          "blink -> WTF\n"
          "blink -> skia\n"
          "blink -> base\n"
          "blink -> net\n"
          "\n"
          "weblayer -> content\n"
          "weblayer -> chrome\n"
          "weblayer -> base\n"
          "\n"
          "net -> base\n"
          "WTF -> base\n",
      },
      {
          "compilation",
          "random -> pool_urbg\n"
          "random -> nonsecure_base\n"
          "random -> seed_sequence\n"
          "random -> distribution\n"
          "\n"
          "nonsecure_base -> pool_urbg\n"
          "nonsecure_base -> salted_seed_seq\n"
          "\n"
          "seed_sequence -> pool_urbg\n"
          "seed_sequence -> salted_seed_seq\n"
          "seed_sequence -> seed_material\n"
          "\n"
          "distribution -> strings\n"
          "\n"
          "pool_urbg -> seed_material\n"
          "\n"
          "salted_seed_seq -> seed_material\n"
          "\n"
          "seed_material -> strings\n",
      },
  };
}

std::string GraphDAG::Translate(const std::string& input,
                             const std::string& options_string) {
  return DagToText(input);
}

std::unique_ptr<Translator> GraphDAGTranslator() {
  return std::make_unique<GraphDAG>();
}

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
