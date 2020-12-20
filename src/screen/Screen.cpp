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
    : dim_x_(width),
      dim_y_(height),
      lines_(dim_y_, std::wstring(dim_x_, U' ')) {}

void Screen::DrawPixel(int x, int y, wchar_t c) {
  lines_[y][x] = c;
}

void Screen::DrawText(int x, int y, std::wstring_view text) {
  for (auto& c : text)
    lines_[y][x++] = c;
}

// ╭──╮
// │  │
// ╰──╯
//
// ─
//

// ─━│┃┄┅┆┇┈┉┊┋┌┍┎┏
// ┐┑┒┓└┕┖┗┘┙┚┛├┝┞┟
// ┠┡┢┣┤┥┦┧┨┩┪┫┬┭┮┯
// ┰┱┲┳┴┵┶┷┸┹┺┻┼┽┾┿
// ╀╁╂╃╄╅╆╇╈╉╊╋╌╍╎╏
// ═║╒╓╔╕╖╗╘╙╚╛╜╝╞╟
// ╠╡╢╣╤╥╦╧╨╩╪╫╬╭╮╯
// ╰╱╲╳╴╵╶╷╸╹╺╻╼╽╾╿

//   ______
//  ╱      ╲
// ╱        ╲
// ╲        ╱
//  ╲______╱

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
  for (int y = 0; y < dim_y_; ++y) {
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

void Screen::DrawVerticalLineComplete(int top, int bottom, int x) {
  for (int y = top; y <= bottom; ++y) {
    auto& p = Pixel(x, y);
    if (p == U'─') {
      bool left = (x != 0 && Pixel(x - 1, y) != U' ');
      bool right = (x != dim_x_ - 1 && Pixel(x + 1, y) != U' ');
      if (y == top) {
        if (left && right)
          p = U'┬';
        else if (left)
          p = U'┐';
        else if (right)
          p = U'┌';
        else
          p = U'┼';
      }
      else if (y == bottom) {
        if (left && right)
          p = U'┴';
        else if (left)
          p = U'┘';
        else if (right)
          p = U'└';
        else
          p = U'┼';
      }
      else {
        if (left && right)
          p = U'┼';
        else if (left)
          p = U'┤';
        else if (right)
          p = U'├';
        else
          p = U'┼';
      }
    } else {
      // clang-format off
      switch(p) {
        case U'┐': p = U'┤'; break;
        case U'┘': p = U'┤'; break;
        case U'┌': p = U'├'; break;
        case U'└': p = U'├'; break;
        case U'┬': p = U'┼'; break;
        case U'┴': p = U'┼'; break;
        default:   p = U'│'; break;
      }
      // clang-format on
    }
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
          case U'├': c = '-'; break;
          case U'┤': c = '-'; break;
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
          case U'├': c = '-'; break;
          case U'┤': c = '-'; break;
          case U'△': c = '^'; break;
          case U'▽': c = 'V'; break;
        }
      }
    }
    return;
  }
}

wchar_t& Screen::Pixel(int x, int y) {
  return lines_[y][x];
}

void Screen::Resize(int new_dim_x, int new_dim_y) {
  dim_x_ = new_dim_x;
  dim_y_ = new_dim_y;

  lines_.resize(dim_y_);
  for (auto& line : lines_) {
    line.resize(dim_x_, L' ');
  }
}

void Screen::Append(const Screen& other, int x, int y) {
  Resize(std::max(dim_x_, x + other.dim_x_),  //
         std::max(dim_y_, y + other.dim_y_));

  // Write
  for (size_t dy = 0; dy < other.dim_y_; ++dy) {
    for (size_t dx = 0; dx < other.dim_x_; ++dx) {
      lines_[y + dy][x + dx] = other.lines_[dy][dx];
    }
  }
}
// clang-format on
