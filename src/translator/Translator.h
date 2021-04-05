#ifndef TRANSLATOR_TRANSLATOR
#define TRANSLATOR_TRANSLATOR

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct TranslatorError {
  TranslatorError(int line, int column, std::string description)
      : line(line), column(column), description(description) {}
  int line;
  int column;
  std::string description;
};

struct TranslatorResult {
  TranslatorResult() {}
  TranslatorResult(const char* output) : output(output) {}
  TranslatorResult(std::string output) : output(output) {}
  TranslatorResult(TranslatorError error) : errors({std::move(error)}) {}
  TranslatorResult(int line, int column, std::string description)
      : TranslatorResult(TranslatorError(line, column, description)) {}

  std::string output;
  std::vector<TranslatorError> errors;
};

class Translator {
 public:
  // Main API implemented by translator. ---------------------------------------
  virtual TranslatorResult Translate(const std::string& input,
                                     const std::string& option) = 0;
  virtual ~Translator() = default;

  // Reflection API ------------------------------------------------------------
  virtual const char* Identifier() { return ""; }
  virtual const char* Name() { return ""; }
  virtual const char* Description() { return ""; }

  enum Widget {
    Combobox,
    Checkbox,
  };

  struct OptionDescription {
    std::string name;
    std::vector<std::string> values;
    std::string default_value;
    std::string description;
    Widget type;
  };
  virtual std::vector<OptionDescription> Options() { return {}; }

  struct Example {
    std::string title;
    std::string input;
  };
  virtual std::vector<Example> Examples() { return {}; }
};

std::map<std::string, std::string> SerializeOption(const std::string& options);

#endif /* end of include guard: TRANSLATOR_TRANSLATOR */
