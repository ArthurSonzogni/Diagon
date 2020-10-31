#ifndef TRANSLATOR_TRANSLATOR_FACTORY
#define TRANSLATOR_TRANSLATOR_FACTORY

#include <map>
#include <memory>
#include "translator/Translator.h"

using TranslatorPtr = std::unique_ptr<Translator>;
using TranslatorFactory = TranslatorPtr (*)();

// List of exported translator.
TranslatorPtr FrameTranslator();
TranslatorPtr MathTranslator();
TranslatorPtr PlanarGraphTranslator();
TranslatorPtr SequenceTranslator();
TranslatorPtr TableTranslator();
TranslatorPtr TreeTranslator();
TranslatorPtr GrammarTranslator();

// Return a map "string" -> "Translator generator function".
static std::map<std::string, TranslatorFactory> TranslatorList() {
  return {
      {"Frame", FrameTranslator},
      {"Grammar", GrammarTranslator},
      {"Math", MathTranslator},
      {"PlanarGraph", PlanarGraphTranslator},
      {"Sequence", SequenceTranslator},
      {"Table", TableTranslator},
      {"Tree", TreeTranslator},
  };
}

#endif /* end of include guard: TRANSLATOR_TRANSLATOR_FACTORY */
