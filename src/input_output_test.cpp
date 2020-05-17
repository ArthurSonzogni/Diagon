// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <filesystem>
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

int main(int, const char**) {
  auto translator_list = TranslatorList();
  int result = 0;

  std::string path = test_directory;
  std::cout << "test_directory = " << test_directory << std::endl;
  for (auto& dir : std::filesystem::directory_iterator(path)) {
    std::string type = dir.path().filename();
    for (auto& test :
         std::filesystem::directory_iterator(dir.path())) {
      auto translator = translator_list[type]();

      std::string input = ReadFile(test.path() / "input");
      std::string output = ReadFile(test.path() / "output");

      std::string output_computed = translator->Translate(input, "");

      if (output_computed == output) {
        std::cout << "  [PASS] " << test << std::endl;
      } else {
        std::cout << "  [FAIL] " << test << std::endl;
        std::cout << "---[Output]------------------" << std::endl;
        std::cout << output_computed << std::endl;
        std::cout << "---[Expected]----------------" << std::endl;
        std::cout << output << std::endl;
        std::cout << "---------------------" << std::endl;
      }
    }
  }

  return result;
}
