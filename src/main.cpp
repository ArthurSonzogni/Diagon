#include "translator/Translator.h"
#include <iostream>

int main(int, const char**) {
  std::string input;
  for (std::string line; std::getline(std::cin, line);) {
    input += line + "\n";
  }

  auto sequence = SequenceTranslator();
  sequence->Process(input);
  std::cout << sequence->Output() << std::endl;

  return 0;
}
