#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "environment.h"
#include "translator/Translator.h"

int main(int, const char**) {
  int result = 0;

  std::string path = testing_directory;
  for (auto& dir : std::experimental::filesystem::directory_iterator(path)) {
    std::string type = dir.path().filename();
    std::cout << "Testing " << type << std::endl;
    for (auto& test :
         std::experimental::filesystem::directory_iterator(dir.path())) {
      auto sequence = SequenceTranslator();

      std::ifstream input(test.path() / "input");
      std::ifstream output(test.path() / "output");

      std::string input_string((std::istreambuf_iterator<char>(input)),
                               std::istreambuf_iterator<char>());
      std::string output_string((std::istreambuf_iterator<char>(output)),
                                std::istreambuf_iterator<char>());

      sequence->Process(input_string);

      if (sequence->Output() == output_string) {
        std::cout << "  [PASS] " << test << std::endl;
      } else {
        std::cout << "  [FAIL] " << test << std::endl;
        std::cout << "---[Output]------------------" << std::endl;
        std::cout << sequence->Output() << std::endl;
        std::cout << "---[Expected]----------------" << std::endl;
        std::cout << output_string << std::endl;
      }
    }
  }

  return result;
}
