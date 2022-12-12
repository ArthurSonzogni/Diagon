#include <iostream>
#include <string>
#include "translator/Factory.h"

int GeneratorInt(const char* data, size_t& size) {
  if (size == 0)
    return 0;
  auto out = int(data[0]);
  data++;
  size--;
  return out;
}

std::string GeneratorString(const char*& data, size_t& size) {
  int index = 0;
  while (index < size && data[index])
    ++index;

  auto out = std::string(data, data + index);
  data += index;
  size -= index;

  return out;
}

extern "C" int LLVMFuzzerTestOneInput(const char* data, size_t size) {
  auto& translators = TranslatorList();
  auto& translator = translators[GeneratorInt(data, size) % translators.size()];
  std::string input = GeneratorString(data, size);
  std::string options = GeneratorString(data, size);
  if (std::string(translator->Identifier()) == "Grammar")
    return 0;
  std::cout << translator->Identifier() << std::endl;
  std::cout << input << std::endl;
  try {
    std::cout << translator->Translate(input, options) << std::endl;
  } catch (...) {
  }
  return 0;  // Non-zero return values are reserved for future use.
}

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
