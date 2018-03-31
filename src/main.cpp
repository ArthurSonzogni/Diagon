#include "translator/Translator.h"
#include <iostream>

int main(int, const char**) {
  std::string input;
  for (std::string line; std::getline(std::cin, line);) {
    input += line + "\n";
  }

  auto sequence = SequenceTranslator();
  std::cout << (*sequence)(input, "") << std::endl;

  return 0;
}
