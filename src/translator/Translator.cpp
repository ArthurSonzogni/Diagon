#include "translator/Translator.h"

#include <sstream>
#include <string>

// static
std::map<std::string, std::string> SerializeOption(
    const std::string& options) {
  std::map<std::string, std::string> m;
  std::stringstream ss(options);
  std::string label, value;
  while (std::getline(ss, label) && std::getline(ss, value)) {
    m[label] = value;
  }
  return m;
}
