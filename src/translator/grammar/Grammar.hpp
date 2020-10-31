#ifndef TRANSLATOR_MATH_H
#define TRANSLATOR_MATH_H

#include <string>
#include <vector>

#include "translator/Translator.h"
#include "translator/grammar/Grammar.hpp"

class Screen;

class Grammar : public Translator {
 public:
  virtual ~Grammar() = default;
  std::string Translate(const std::string& input,
                        const std::string& options) final;

  const char* Name() final;
  const char* Description() final;
  std::vector<OptionDescription> Options() final;
  std::vector<Example> Examples() final;
};

#endif /* end of include guard: TRANSLATOR_MATH_H */
