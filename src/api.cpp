#include "api.hpp"

#include <nlohmann/json.hpp>
#include "translator/Factory.h"

using json = nlohmann::json;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

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
  // NOTREACHED
  return "combobox";
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

EMSCRIPTEN_KEEPALIVE
extern "C" const char* API() {
  static std::string out;
  if (out.size() == 0)
    out = API(TranslatorList()).dump(2);

  return out.c_str();
}
