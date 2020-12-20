// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <emscripten.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include "util.hpp"

#include "translator/Factory.h"

extern "C" void translate(const char* translator_name,
                          const char* input,
                          const char* options) {
  auto* translator = FindTranslator(translator_name);
  if (!translator)
    std::cerr << "Translator not found" << std::endl;

  std::string command = translator->Translate(input, options);
  replaceAll(command, "\\", "\\\\");
  replaceAll(command, "\"", "\\\"");
  replaceAll(command, "\n", "\\n");
  command = "output.value=\"" + command + "\";";
  emscripten_run_script(command.c_str());
}

int main(int, const char**) {
  return 0;
}

using json = nlohmann::json;

template <class T>
json API(const std::vector<T>& container) {
  auto json = json::array();
  for (auto& it : container)
    json.push_back(API(it));
  return json;
}

std::string API(const Translator::Widget& type) {
  switch (type) {
    case Translator::Widget::Combobox:
      return "combobox";
    case Translator::Widget::Checkbox:
      return "checkbox";
  }
}

json API(const Translator::OptionDescription& option) {
  auto json = json::object();
  json["name"] = option.name;
  json["label"] = option.name;
  json["values"] = option.values;
  json["default"] = option.default_value;
  json["type"] = API(option.type);
  return json;
}

json API(const Translator::Example& example) {
  auto json = json::object();
  json["title"] = example.title;
  json["content"] = example.input;
  return json;
}

json API(const TranslatorPtr& translator) {
  auto json = json::object();
  json["tool"] = translator->Identifier();
  json["description"] = translator->Name();
  json["options"] = API(translator->Options());
  json["examples"] = API(translator->Examples());
  return json;
}

extern "C" const char* API() {
  static std::string out;
  if (out.size() == 0)
    out = API(TranslatorList()).dump();

  return out.c_str();
}
