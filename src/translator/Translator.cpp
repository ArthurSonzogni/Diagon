// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "translator/Translator.h"

#include <sstream>
#include <string>

// static
std::map<std::string, std::string> SerializeOption(const std::string& options) {
  std::map<std::string, std::string> m;
  std::stringstream ss(options);
  std::string label, value;
  while (std::getline(ss, label) && std::getline(ss, value)) {
    m[label] = value;
  }
  return m;
}
