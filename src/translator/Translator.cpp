#include "translator/Translator.h"

#include <sstream>
#include <string>

std::unique_ptr<Translator> TranslatorFromName(const std::string name) {
  if (name == "Frame")
    return FrameTranslator();
  if (name == "Math")
    return MathTranslator();
  if (name == "PlanarGraph")
    return PlanarGraphTranslator();
  if (name == "Sequence")
    return SequenceTranslator();
  if (name == "Table")
    return TableTranslator();
  if (name == "Tree")
    return TreeTranslator();
  return nullptr;
}

// static
std::map<std::string, std::string> Translator::SerializeOption(
    const std::string& options) {
  std::map<std::string, std::string> m;
  std::stringstream ss(options);
  std::string label, value;
  while (std::getline(ss, label) && std::getline(ss, value)) {
    m[label] = value;
  }
  return m;
}
