#ifndef TRANSLATOR_EXAMPLE_EXAMPLE_H
#define TRANSLATOR_EXAMPLE_EXAMPLE_H

#include <string>
#include <vector>

#include "translator/Translator.h"
#include "translator/example/Example.h"
#include "translator/example/ExampleLexer.h"
#include "translator/example/ExampleParser.h"

class Example : public Translator {
 public:
  virtual ~Example() = default;
  void Process(const std::string& input) override;
  std::string Output() override;
 private:
  std::string output_;
};

#endif /* end of include guard: TRANSLATOR_EXAMPLE_EXAMPLE_H */
