#ifndef TRANSLATOR_TRANSLATOR
#define TRANSLATOR_TRANSLATOR

#include <string>
#include <memory>

class Translator {
 public:
  virtual ~Translator() = default;
  virtual void Process(const std::string& input) = 0;
  virtual std::string Output() = 0;
};

std::unique_ptr<Translator> SequenceTranslator();
std::unique_ptr<Translator> LineNumberASCIITranslator();
std::unique_ptr<Translator> LineNumberUnicodeTranslator();
std::unique_ptr<Translator> PlanarGraphTranslator();

std::unique_ptr<Translator> TranslatorFromName(const std::string name);

#endif /* end of include guard: TRANSLATOR_TRANSLATOR */
