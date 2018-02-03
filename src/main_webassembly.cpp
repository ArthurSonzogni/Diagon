#include "translator/Translator.h"
#include <iostream>
#include <emscripten.h>

void replaceAll(std::string& str,
                const std::string& from,
                const std::string& to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();  // In case 'to' contains 'from', like replacing
                               // 'x' with 'yx'
  }
}

extern "C" {

void translate(const char* input) {
  auto sequence = SequenceTranslator();
  sequence->Process(input);
  std::string command = "output.value='" + sequence->Output() + "';";
  replaceAll(command, "\n", "\\n");
  emscripten_run_script(command.c_str());
}
}

int main(int, const char**) {
  std::cout << "main_webassembly loaded" << std::endl;
  return 0;
}
