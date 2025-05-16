// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "filesystem.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "environment.h"
#include "translator/Factory.h"

std::string ReadFile(std::filesystem::path path) {
  std::ifstream file(path);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void ParseDirectoryName(std::string name,
                        std::string* translator_name,
                        std::string* options) {

  // Split at the first '_' the first part is the translator name, the second is
  // the options.
  // The options follows name=value, split by '_', but name can contain '_'.

  std::string::size_type pos = name.find('_');

  if (pos == std::string::npos) {
    *translator_name = name;
    *options = "";
    return;
  }

  *translator_name = name.substr(0, pos);

  // Parse a key=value list.
  while(true) {
    // Parse the name.
    std::string::size_type pos2 = name.find('=', pos);
    if (pos2 == std::string::npos) {
      break;
    }

    // Parse the value.
    std::string::size_type pos3 = name.find('_', pos2);
    if (pos3 == std::string::npos) {
      pos3 = name.length();
    }
    std::string key = name.substr(pos + 1, pos2 - pos - 1);
    std::string value = name.substr(pos2 + 1, pos3 - pos2 - 1);

    *options += key + '\n' + value + '\n';
    pos = pos3;
  }

  // Remove the last '\n' if any.
  if (options->length() > 0) {
    options->erase(options->length() - 1);
  }
}

int main(int, const char**) {
  int result = EXIT_SUCCESS;
  std::string path = test_directory;
  //std::cout << "test_directory = " << test_directory << std::endl;

  for (auto& dir : std::filesystem::directory_iterator(path)) {
    std::string translator_name;
    std::string options;
    ParseDirectoryName(dir.path().filename(), &translator_name, &options);

    for (auto& test : std::filesystem::directory_iterator(dir.path())) {
      std::cout << "  [TEST] " << test.path() << std::endl;
      auto translator = FindTranslator(translator_name);
      if (!translator) {
        std::cout << "Translator " << translator_name << " not found."
                  << std::endl;
        continue;
      }

      std::string input = ReadFile(test.path() / "input");
      std::string output_computed = translator->Translate(input, options);

      if (!std::filesystem::exists(test.path() / "output")) {
        std::cout << "  [RUN ] " << test.path() << std::endl;
        std::cout << "  [Create output] " << std::endl;
        std::cout << output_computed;
        std::ofstream(test.path() / "output") << output_computed;
        continue;
      }

      std::string output = ReadFile(test.path() / "output");
      if (output_computed == output) {
        continue;
      }

      std::cout << "  [FAIL] " << test.path() << std::endl;
      std::cout << "---[Output]------------------" << std::endl;
      std::cout << output_computed << std::endl;
      std::cout << "---[Expected]----------------" << std::endl;
      std::cout << output << std::endl;
      std::cout << "---------------------" << std::endl;
      result = EXIT_FAILURE;

      // Fix the expected output if it is not correct.
      // Uncomment the following lines to create the output file.
      //std::cout << "  [Create output] " << std::endl;
      //std::cout << output_computed;
      //std::ofstream(test.path() / "output") << output_computed;
    }
  }

  return result;
}
