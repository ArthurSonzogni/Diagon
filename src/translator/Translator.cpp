#include "translator/Translator.h"

std::unique_ptr<Translator> TranslatorFromName(const std::string name) {
  if (name == "Sequence")
    return SequenceTranslator();
  if (name == "LineNumberASCII")
    return LineNumberASCIITranslator();
  if (name == "LineNumberUnicode")
    return LineNumberUnicodeTranslator();
  return nullptr;
}
