#ifndef TRANSLATOR_TRANSLATOR
#define TRANSLATOR_TRANSLATOR

#include <map>
#include <memory>
#include <string>

class Translator {
 public:
  virtual ~Translator() = default;
  virtual std::string operator()(const std::string& input,
                                 const std::string& options) = 0;
  static std::map<std::string, std::string> SerializeOption(
      const std::string& options);
};

std::unique_ptr<Translator> FrameTranslator();
std::unique_ptr<Translator> MathTranslator();
std::unique_ptr<Translator> PlanarGraphTranslator();
std::unique_ptr<Translator> SequenceTranslator();
std::unique_ptr<Translator> TableTranslator();
std::unique_ptr<Translator> TreeTranslator();

std::unique_ptr<Translator> TranslatorFromName(const std::string name);

#endif /* end of include guard: TRANSLATOR_TRANSLATOR */
