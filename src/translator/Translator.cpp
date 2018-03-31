#include "translator/Translator.h"

#include <sstream>
#include <string>

std::unique_ptr<Translator> TranslatorFromName(const std::string name) {
  if (name == "Sequence")
    return SequenceTranslator();
  if (name == "Frame")
    return FrameTranslator();
  if (name == "PlanarGraph")
    return PlanarGraphTranslator();
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
