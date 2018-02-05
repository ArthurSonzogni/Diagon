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

void translate(const char* translator_name, const char* input) {
  std::string translator_string = translator_name;
  std::unique_ptr<Translator> translator;
  if (translator_string == "Example") 
    translator = ExampleTranslator();
  else if (translator_string == "Sequence") 
    translator = SequenceTranslator();
  else if (translator_string == "LineNumber") 
    translator = LineNumberTranslator();
  else
    return;

  translator->Process(input);
  std::string command = "output.value='" + translator->Output() + "';";
  replaceAll(command, "\n", "\\n");
  emscripten_run_script(command.c_str());
}
}

int main(int, const char**) {
  std::cout << "main_webassembly loaded" << std::endl;
  return 0;
}
