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
  std::vector<std::string> parts;
  int left = 0;
  int right = 0;
  while (right < name.size()) {
    if (name[right] == '_' || name[right] == '=') {
      parts.push_back(name.substr(left, right - left));
      left = right + 1;
    }
    ++right;
  }
  parts.push_back(name.substr(left, right - left));

  *translator_name = parts[0];
  for (int i = 1; i < parts.size(); ++i) {
    *options += parts[i] + "\n";
  }
}

int main(int, const char**) {
  int result = EXIT_SUCCESS;
  std::string path = test_directory;
  std::cout << "test_directory = " << test_directory << std::endl;
  for (auto& dir : std::filesystem::directory_iterator(path)) {
    std::string translator_name;
    std::string options;
    ParseDirectoryName(dir.path().filename(), &translator_name, &options);

    for (auto& test : std::filesystem::directory_iterator(dir.path())) {
      auto translator = FindTranslator(translator_name);
      if (!translator) {
        std::cout << "Translator " << translator_name << " not found."
                  << std::endl;
        result = EXIT_FAILURE;
        continue;
      }

      std::string input = ReadFile(test.path() / "input");
      std::string output = ReadFile(test.path() / "output");

      std::string output_computed = translator->Translate(input, options);

      if (output_computed == output) {
        std::cout << "  [PASS] " << test.path() << std::endl;
      } else {
        std::cout << "  [FAIL] " << test.path() << std::endl;
        std::cout << "---[Output]------------------" << std::endl;
        std::cout << output_computed << std::endl;
        std::cout << "---[Expected]----------------" << std::endl;
        std::cout << output << std::endl;
        std::cout << "---------------------" << std::endl;
        result = EXIT_FAILURE;
      }
    }
  }

  return result;
}
