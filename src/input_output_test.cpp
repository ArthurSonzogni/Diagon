#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "environment.h"
#include "translator/Translator.h"

std::string ReadFile(std::experimental::filesystem::path path) {
  std::ifstream file(path);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

int main(int, const char**) {
  int result = 0;

  std::string path = test_directory;
  std::cout << "test_directory = " << test_directory << std::endl;
  for (auto& dir : std::experimental::filesystem::directory_iterator(path)) {
    std::string type = dir.path().filename();
    std::cout << "Testing " << type << std::endl;
    for (auto& test :
         std::experimental::filesystem::directory_iterator(dir.path())) {
      auto translator = TranslatorFromName(type);

      std::string input = ReadFile(test.path() / "input");
      std::string output = ReadFile(test.path() / "output");

      std::string output_computed = (*translator)(input, "");

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
