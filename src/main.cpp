// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <iostream>

#include "environment.h"
#include "translator/Factory.h"

namespace {

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

int PrintHelp() {
  std::string output = R"description(

NAME
  diagon - Generate ascii art diagram.
  
SYNOPSIS
  diagon [options] (translator) [translator-options] 

EXAMPLE:
  diagon Math -- "1+1/2 + sum(i,0,10) = 112/2"
            10         
          ___        
      1   ╲       112
  1 + ─ + ╱   i = ───
      2   ‾‾‾      2 
           0         

OPTIONS:
  -h, --help:    Print this page.
  -v, --version: Print the version.
  -l, --list:    List the available translators.

TRANSLATOR:
)description";

  for (auto& it : TranslatorList()) {
    output += " - " + it.first + "\n";
  }

  output += R"(
TRANSLATOR-OPTIONS:

  --help       : Display translator specific help. For example:
                 * diagon Math --help
                 * diagon Table --help

  -- <input>   : Read the input from the command line. Without this option, it
                 is read from the standard input.

  -option=value: Provide a translator specific option.

COOKBOOK:
  Reading from:
    * command_line: diagon Math -- 1+1/2
    * file        : diagon Math < filename
    * stdin       : diagon Math

  Providing options:
    diagon Math -style=Unicode -- 1 + 1/2
    diagon Math -style=Ascii   -- 1 + 1/2
    diagon Math -style=Latex   -- 1 + 1/2
  
WEBSITE:
  This tool can also be used as a WebAssembly application on the website:
  https://arthursonzogni.com/Diagon/

)";

  std::cout << output;
  return EXIT_SUCCESS;
}

int PrintVersion() {
  std::cout << "version: " << git_version << std::endl;
  std::cout << "Written by Arthur Sonzogni" << std::endl;
  std::cout << "License MIT" << std::endl;
  return EXIT_SUCCESS;
}

int PrintError(std::string error) {
  std::cout << error << std::endl;
  return EXIT_FAILURE;
}

int PrintTranslatorExamples(TranslatorFactory factory) {
  auto translator = factory();
  if (!translator->Examples().size()) {
    std::cout << "No examples" << std::endl;
    return EXIT_SUCCESS;
  }

  std::cout << "EXAMPLES:" << std::endl;
  int i = 0;
  for (auto& it : translator->Examples()) {
    std::string input = TranslatorList()["Frame"]()->Translate(
        it.input, "line_number\nfalse");
    replaceAll(input, "\n", "\n     ");
    std::cout << "  " << (++i) << ") input" << std::endl;
    std::cout << "     " << input;

    std::string output = factory()->Translate(it.input, "");
    output =
        TranslatorList()["Frame"]()->Translate(output, "line_number\nfalse");
    replaceAll(output, "\n", "\n     ");
    std::cout << " output" << std::endl;
    std::cout << "     " << output << std::endl;
  }

  return EXIT_SUCCESS;
}

int PrintTranslatorHelp(TranslatorFactory factory) {
  auto translator = factory();
  std::cout << "SYNOPSIS:" << std::endl;
  std::cout << "  diagon " << translator->Name() << " [--option=value]*"
            << std::endl
            << std::endl;

  std::cout << "DESCRIPTION:" << std::endl;
  std::string description = translator->Description();
  replaceAll(description, "\n", "\n  ");
  std::cout << "  " << description << std::endl << std::endl;

  if (translator->Options().size()) {
    std::cout << "OPTIONS:" << std::endl;
    int i = 0;
    for (auto& it : translator->Options()) {
      std::cout << "   " << (++i) << ") " << it.description << std::endl;
      for (auto& value : it.values) {
        if (value == it.default_value)
          std::cout << "     --" << it.name << "=" << value << " (default)"
                    << std::endl;
        else
          std::cout << "     --" << it.name << "=" << value << std::endl;
      }
      std::cout << std::endl;
    }
  }
  if (translator->Examples().size() > 2) {
    std::cout << "EXAMPLES:" << std::endl;
    std::cout << "  " << translator->Examples().size() << " examples found. Print them using:"
              << std::endl;
    std::cout << "  diagon " << translator->Name() << " --examples" << std::endl;
    return EXIT_SUCCESS;
  }

  if (translator->Examples().size() > 0)
    return PrintTranslatorExamples(factory);

  return EXIT_SUCCESS;
}

int Translate(TranslatorFactory translator_factory,
              int argument_count,
              const char** arguments) {
  // Read the options
  auto next_argument = [&]() {
    argument_count--;
    arguments++;
    return arguments[-1];
  };

  auto read_stdin = []() {
    std::string input;
    for (std::string line; std::getline(std::cin, line);)
      input += line + "\n";
    return input;
  };

  auto read_remaining_args = [&]() {
    std::string output;
    while (argument_count) {
      if (output.size())
        output += " ";
      output += next_argument();
    }
    return output;
  };

  bool has_input = false;
  std::string input;
  std::string option_list;

  while (argument_count) {
    std::string argument = next_argument();

    if (argument == "--help")
      return PrintTranslatorHelp(translator_factory);

    if (argument == "--examples")
      return PrintTranslatorExamples(translator_factory);

    if (argument == "--") {
      input = read_remaining_args();
      has_input = true;
      continue;
    }

    if (argument.size() == 0)
      return PrintError("weird input encountered");

    if (argument[0] != '-')
      return PrintError("Unexpected translator argument: " + argument);

    int space_position = 1;
    while (argument[space_position] && argument[space_position] != '=')
      space_position++;

    if (!argument[space_position])
      return PrintError("Unexpected translator argument: " + argument);

    std::string name = argument.substr(2, space_position - 2);
    std::string value = argument.substr(space_position + 1, -1);

    if (name.size() == 0 || value.size() == 0)
      return PrintError("Unexpected translator argument: " + argument);

    option_list += name + '\n';
    option_list += value + '\n';
  }

  if (!has_input) {
    has_input = true;
    input = read_stdin();
  }

  auto translator = translator_factory();
  std::string output = translator->Translate(input, option_list);
  std::cout << output << std::endl;
  return EXIT_SUCCESS;
}

int PrintTranslatorNotFound(const std::string& translator) {
  std::cout << "The translator: " << translator << " doesn't exist"
            << std::endl;
  std::cout << "List of available translator:" << std::endl;
  for (auto& it : TranslatorList())
    std::cout << "  - " + it.first << std::endl;

  std::cout << "Please read the manual by using diagon --help" << std::endl;
  return EXIT_SUCCESS;
}

}  // namespace

int main(int argument_count, const char** arguments) {
  if (argument_count <= 1)
    return PrintHelp();

  std::string argument_1 = arguments[1];
  if (argument_1 == "-h" || argument_1 == "--help")
    return PrintHelp();

  if (argument_1 == "-v" || argument_1 == "--version")
    return PrintVersion();

  std::string translator_name = arguments[1];
  auto translator = TranslatorList()[translator_name];

  if (translator)
    return Translate(std::move(translator), argument_count - 2, arguments + 2);

  return PrintTranslatorNotFound(translator_name);
}
