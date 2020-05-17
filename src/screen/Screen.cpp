// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "screen/Screen.h"

#include <codecvt>
#include <locale>
#include <sstream>

std::string to_string(const std::wstring& s) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(s);
}

std::wstring to_wstring(const std::string& s) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}

Screen::Screen(int width, int height)
    : width_(width),
      height_(height),
      lines_(height_, std::wstring(width_, U' ')) {}

void Screen::DrawPixel(int x, int y, wchar_t c) {
  lines_[y][x] = c;
}

void Screen::DrawText(int x, int y, const std::wstring& text) {
  for (auto& c : text)
    lines_[y][x++] = c;
}

void Screen::DrawBox(int x, int y, int w, int h) {
  lines_[y][x + w - 1] = U'┐';
  lines_[y + h - 1][x + w - 1] = U'┘';
  lines_[y][x] = U'┌';
  lines_[y + h - 1][x] = U'└';
  for (int xx = 1; xx < w - 1; ++xx) {
    lines_[y][x + xx] = U'─';
    lines_[y + h - 1][x + xx] = U'─';
  }
  for (int yy = 1; yy < h - 1; ++yy) {
    lines_[y + yy][x] = U'│';
    lines_[y + yy][x + w - 1] = U'│';
  }
}

void Screen::DrawBoxedText(int x, int y, const std::wstring& text) {
  DrawText(x + 1, y + 1, text);
  DrawBox(x, y, text.size() + 2, 3);
}

std::string Screen::ToString() {
  std::stringstream ss;
  for (int y = 0; y < height_; ++y) {
    ss << to_string(lines_[y]) << '\n';
  }
  return ss.str();
}

void Screen::DrawHorizontalLine(int left, int right, int y, wchar_t c) {
  for (int x = left; x <= right; ++x) {
    lines_[y][x] = c;
  }
}

void Screen::DrawVerticalLine(int top, int bottom, int x, wchar_t c) {
  for (int y = top; y <= bottom; ++y) {
    lines_[y][x] = U'│';
  }
}

// clang-format off
void Screen::ASCIIfy(int style) {
  if (style == 0) {
    for(auto& line : lines_) {
      for(auto& c : line) {
        switch(c) {
          case U'─': c = '-'; break;
          case U'│': c = '|'; break;
          case U'┐': c = '.'; break;
          case U'┘': c = '\''; break;
          case U'┌': c = '.'; break;
          case U'└': c = '\''; break;
          case U'┬': c = '-'; break;
          case U'┴': c = '-'; break;
          case U'△': c = '^'; break;
          case U'▽': c = 'V'; break;
        }
      }
    }
    return;
  }

  if (style == 1) {
    for(auto& line : lines_) {
      for(auto& c : line) {
        switch(c) {
          case U'─': c = '-'; break;
          case U'│': c = '|'; break;
          case U'┐': c = '.'; break;
          case U'┘': c = '\''; break;
          case U'┌': c = '.'; break;
          case U'└': c = '\''; break;
          case U'┬': c = '.'; break;
          case U'┴': c = '\''; break;
          case U'△': c = '^'; break;
          case U'▽': c = 'V'; break;
        }
      }
    }
    return;
  }
}
// clang-format on
