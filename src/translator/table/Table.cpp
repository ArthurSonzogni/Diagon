// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <iostream>
#include <sstream>
#include <vector>

#include "screen/Screen.h"
#include "translator/Translator.h"

namespace {
// clang-format off
  //---------------------
  // 
  //    0   1   1   2
  //  
  // 0  ABBBCBBBCBBBD
  //    E   F   F   G
  // 1  HIIIJIIIJIIIK
  //    L   M   M   N
  // 2  OPPPQPPPQPPPR
  //    L   M   M   N
  // 2  OPPPQPPPQPPPR
  //    L   M   M   N
  // 3  STTTUTTTUTTTV
  //
  // A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V
  // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21
  // 
  //---------------------
  struct Style {
    std::wstring charset[22];
    int width[3];
    int height[4];
  };
  
  std::map<std::string, Style> styles = {
    {
      "ascii",
      Style{
        // charset
        {
          L"+" , L"-" , L"+" , L"+" ,
          L"|"        , L"|" , L"|" ,
          L"+" , L"-" , L"+" , L"+" ,

          L"|"        , L"|" , L"|" ,
          L"+" , L"-" , L"+" , L"+" ,

          L"+" , L"-" , L"+" , L"+" ,
        },
        // width/height
        {1,1,1}, {1,1,1,1},
      }
    },
    {
      "ascii rounded",
      Style{
        // charset
        {
          L"." , L"-" , L"+" , L"." ,
          L"|"        , L"|" , L"|" ,
          L"|" , L"-" , L"+" , L"|" ,

          L"|"        , L"|" , L"|" ,
          L"|" , L"-" , L"+" , L"|" ,

          L"'" , L"-" , L"+" , L"'" ,
        },
        // width/height
        {1,1,1}, {1,1,1,1},
      }
    },
    {
      "ascii with header 1",
      Style{
        // charset
        {
          L"|=" , L"=" , L"=" , L"=|" ,
          L"| "        , L"|" , L" |" ,
          L"|=" , L"=" , L"=" , L"=|" ,

          L" |"        , L"|" , L"| " ,
          L" +" , L"-" , L"+" , L"+ " ,

          L" +" , L"-" , L"+" , L"+ " ,
        },
        // width
        {2,1,2},{1,1,1,1},
      }
    },
    {
      "ascii with header 2",
      Style{
        // charset
        {
          L"=" , L"=" , L"=" , L"=" ,
          L"|"        , L"|" , L"|" ,
          L"=" , L"=" , L"=" , L"=" ,

          L"|"        , L"|" , L"|" ,
          L"+" , L"-" , L"+" , L"+" ,

          L"+" , L"-" , L"+" , L"+" ,
        },
        // width
        {1,1,1},{1,1,1,1},
      }
    },
    {
      "ascii light header",
      Style{
        // charset
        {
          L"" , L"" , L"" , L"" ,
          L"" ,       L" " , L""  ,
          L"" , L"-", L" " , L"" ,

          L"" ,       L" ", L""  ,
          L"" , L"",  L"" , L"" ,

          L"" , L"",  L"" , L"" ,
        },
        // width
        {0,1,0},{0,1,0,0},
      }
    },
    {
      "ascii light header/separator",
      Style{
        // charset
        {
          L"" , L"" , L"" , L"" ,
          L"" ,       L"|" , L""  ,
          L"" , L"-", L"|" , L"" ,

          L"" ,       L"|", L""  ,
          L"" , L"",  L"" , L"" ,

          L"" , L"",  L"" , L"" ,
        },
        // width
        {0,1,0},{0,1,0,0},
      }
    },
    {
      "ascii light header/separator/border",
      Style{
        // charset
        {
          L"+" , L"-", L"+" , L"+" ,
          L"|" ,       L"|" , L"|"  ,
          L"+" , L"-", L"+" , L"|" ,

          L"|" ,       L"|" , L"|"  ,
          L"|" , L"",  L"|" , L"|" ,

          L"+" , L"-", L"+" , L"+" ,
        },
        // width
        {1,1,1},{1,1,0,1},
      }
    },
    {
      "ascii light separator/border",
      Style{
        // charset
        {
          L"+", L"-" , L"+" , L"+" ,
          L"|",        L"|" , L"|"  ,
          L"" , L"",   L""  , L"" ,

          L"|" ,       L"|" , L"|"  ,
          L"|" , L"",  L"|" , L"|" ,

          L"+" , L"-", L"+" , L"+" ,
        },
        // width
        {1,1,1},{1,0,0,1},
      }
    },
    {
      "ascii light border",
      Style{
        // charset
        {
          L"+" , L"-" , L"-" , L"+" ,
          L"|",        L" " , L"|"  ,
          L"" , L"",   L""  , L"" ,

          L"|" ,       L" " , L"|"  ,
          L"|" , L"",  L" " , L"|" ,

          L"+" , L"-", L"-" , L"+" ,
        },
        // width
        {1,1,1},{1,0,0,1},
      }
    },
    {
      "unicode",
      Style{
        // charset
        {
          L"┌" , L"─" , L"┬" , L"┐" ,
          L"│"        , L"│" , L"│" ,
          L"├" , L"─" , L"┼" , L"┤" ,

          L"│"        , L"│" , L"│" ,
          L"├" , L"─" , L"┼" , L"┤" ,

          L"└" , L"─" , L"┴" , L"┘" ,
        },
        // width
        {1,1,1},{1,1,1,1},
      }
    },
    {
      "unicode rounded",
      Style{
        // charset
        {
          L"╭" , L"─" , L"┬" , L"╮" ,
          L"│"        , L"│" , L"│" ,
          L"├" , L"─" , L"┼" , L"┤" ,

          L"│"        , L"│" , L"│" ,
          L"├" , L"─" , L"┼" , L"┤" ,

          L"╰" , L"─" , L"┴" , L"╯" ,
        },
        // width
        {1,1,1},{1,1,1,1},
      }
    },
    {
      "unicode bold",
      Style{
        // charset
        {
          L"┏" , L"━" , L"┳" , L"┓" ,
          L"┃"        , L"┃" , L"┃" ,
          L"┣" , L"━" , L"╋" , L"┫" ,

          L"┃"        , L"┃" , L"┃" ,
          L"┣" , L"━" , L"╋" , L"┫" ,

          L"┗" , L"━" , L"┻" , L"┛" ,
        },
        // width
        {1,1,1},{1,1,1,1},
      }
    },
    {
      "unicode double",
      Style{
        // charset
        {
          L"╔" , L"═" , L"╦" , L"╗" ,
          L"║"        , L"║" , L"║" ,
          L"╠" , L"═" , L"╬" , L"╣" ,

          L"║"        , L"║" , L"║" ,
          L"╠" , L"═" , L"╬" , L"╣" ,

          L"╚" , L"═" , L"╩" , L"╝" ,
        },
        // width
        {1,1,1},{1,1,1,1},
      }
    },
    {
      "unicode with bold header",
      Style{
        // charset
        {
          L"┏" , L"━" , L"┳" , L"┓" ,
          L"┃"        , L"┃" , L"┃" ,
          L"┡" , L"━" , L"╇" , L"┩" ,

          L"│"        , L"│" , L"│" ,
          L"├" , L"─" , L"┼" , L"┤" ,

          L"└" , L"─" , L"┴" , L"┘" ,
        },
        // width
        {1,1,1},{1,1,1,1},
      }
    },
    {
      "unicode with double header",
      Style{
        // charset
        {
          L"╒" , L"═" , L"╤" , L"╕" ,
          L"│"        , L"│" , L"│" ,
          L"╞" , L"═" , L"╪" , L"╡" ,

          L"│"        , L"│" , L"│" ,
          L"├" , L"─" , L"┼" , L"┤" ,

          L"└" , L"─" , L"┴" , L"┘" ,
        },
        // width
        {1,1,1},{1,1,1,1},
      }
    },
    {
      "unicode cells",
      Style{
        // charset
        {
          L"╭"  , L"─"  , L"╮╭"   , L"╮"  ,
          L"│"          , L"││"   , L"│"  ,
          L"╰╭" , L"──" , L"╯╰╮╭" , L"╯╮" ,

          L"│"          , L"││"   , L"│"  ,
          L"╰╭" , L"──" , L"╯╰╮╭" , L"╯╮" ,

          L"╰"  , L"─"  , L"╯╰"   , L"╯"  ,
        },
        // width
        {1,2,1},{1,2,2,1},
      }
    },
    {
      "unicode cells 2",
      Style{
        // charset
        {
          L"╭─│╭"  , L"──"  , L"──╮╭"   , L"─╮╮│"  ,
          L"││"             , L"││" , L"││"    ,
          L"│╰│╭" , L"──" , L"╯╰╮╭" , L"╯│╮│" ,

          L"││"  , L"││" , L"││"    ,
          L"│╰│╭" , L"──" , L"╯╰╮╭" , L"╯│╮│" ,

          L"│╰╰─"  , L"──"  , L"╯╰──"   , L"╯│─╯"  ,
        },
        // width
        {2,2,2},{2,2,2,2},
      }
    },
    {
      "conceptual",
      Style{
        // charset
        {
          L" " , L"_" , L"  " , L" " ,
          L"/"        , L"\\/" , L"\\" ,
          L"\\" , L"_" , L"/\\" , L"/" ,

          L"/"        , L"\\/" , L"\\" ,
          L"\\" , L"_" , L"/\\" , L"/" ,

          L"\\" , L"_" , L"/\\" , L"/" ,
        },
        // width
        {1,2,1},{1,1,1,1},
      }
    },
  };
};
// clang-format on

class Table : public Translator {
 public:
  virtual ~Table() = default;

 private:
  const char* Name() final { return "Table"; }
  const char* Identifier() final { return "Table"; }
  const char* Description() final { return "Draw table"; }

  std::vector<Translator::OptionDescription> Options() final {
    return {
        {
            "style",
            {
                "unicode",
                "unicode rounded",
                "unicode bold",
                "unicode double",
                "unicode with bold header",
                "unicode with double header",
                "unicode cells",
                "unicode cells 2",
                "ascii",
                "ascii rounded",
                "ascii with header 1",
                "ascii with header 2",
                "ascii light header",
                "ascii light header/separator",
                "ascii light header/separator/border",
                "ascii light separator/border",
                "ascii light border",
                "conceptual",
            },
            "unicode",
            "The style of the table.",
            Widget::Combobox,
        },
    };
  }

  std::vector<Translator::Example> Examples() final {
    return {
        {"1-simple",
         "Column 1,Column 2,Column 3\n"
         "C++,Web,Assembly\n"
         "Javascript,CSS,HTML"},
    };
  }
  std::string Translate(const std::string& input,
                        const std::string& options_string) override {
    auto options = SerializeOption(options_string);

    // Style.
    std::string style_option = options["style"];
    Style style = styles["unicode"];
    if (styles.count(style_option)) {
      style = styles[style_option];
    }

    // Separator.
    std::wstring separator = to_wstring(options["separator"]);
    if (separator.size() != 1) {
      separator = L',';
    }

    // Parse data.
    std::vector<std::vector<std::wstring>> data;
    std::wstring line;
    std::wstringstream ss(to_wstring(input));
    while (std::getline(ss, line)) {
      data.emplace_back();
      std::wstring cell;
      std::wstringstream ss_line(line);
      while (std::getline(ss_line, cell, separator[0])) {
        data.back().push_back(cell);
      }
    }

    // Compute row/line count.
    int row_count = data.size();
    int column_count = 0;
    for (const auto& line : data) {
      column_count = std::max(column_count, (int)line.size());
    }

    // Uniformize the number of cells per lines.
    for (auto& line : data) {
      line.resize(column_count);
    }

    // Compute column_width;
    std::vector<int> column_width(column_count, 0);
    for (const auto& line : data) {
      for (int i = 0; i < line.size(); ++i) {
        column_width[i] = std::max(column_width[i], (int)line[i].size());
      }
    }

    // Compute sum_column_width;
    int column_width_global = 0;
    for (const auto it : column_width) {
      column_width_global += it;
    }

    // Compute screen dimension.
    int width = style.width[0] + style.width[1] * (column_count - 1) +
                style.width[2] + column_width_global;
    int height = style.height[0] + style.height[1] +
                 style.height[2] * (row_count - 2) + style.height[3] +
                 row_count;

    Screen screen(width, height);

    // Draw table.
    int Y = 0;
    for (int y = 0; y < row_count; ++y) {
      bool last_line = (y == row_count - 1);
      int X = 0;

      const int cell_top = Y + style.height[std::min(2, y)];
      const int cell_bottom = cell_top + 1;

      for (int x = 0; x < data[y].size(); ++x) {
        bool last_row = (x == column_count - 1);
        // clang-format off
        const int top_char = 
          y == 0 ? 1 :
          y == 1 ? 8 :
                   15;
        const int left_char =
          y == 0 ? (x == 0 ? 4 : 5) :
                   (x == 0 ? 11:12) ;
        const int right_char = 
          y == 0 ? 6:
                   13;
        const int bottom_char = 19;
        const int top_left_char =
            y == 0 ? (x == 0 ? 0 : 2):
            y == 1 ? (x == 0 ? 7 : 9):
                     (x == 0 ? 14:16);
        const int top_right_char =
            y == 0 ? 3 :
            y == 1 ? 10:
                     17;
        const int bottom_left_char =
            x == 0 ? 18:
                     20;
        const int bottom_right_char = 21;

        const int cell_left = X + style.width[std::min(1,x)];
        const int cell_right = cell_left + column_width[x];

        // clang-format on

        // Draw Top.
        {
          int i = 0;
          for (int yy = Y; yy < cell_top; ++yy) {
            for (int xx = cell_left; xx < cell_right; ++xx) {
              screen.DrawPixel(xx, yy, style.charset[top_char][i]);
            }
            ++i;
          }
        }
        // Draw Down.
        if (last_line) {
          int i = 0;
          for (int yy = cell_bottom; yy < height; ++yy) {
            for (int xx = cell_left; xx < cell_right; ++xx) {
              screen.DrawPixel(xx, yy, style.charset[bottom_char][i]);
            }
            ++i;
          }
        }

        // Draw Left.
        for (int yy = cell_top; yy < cell_bottom; ++yy) {
          screen.DrawText(X, yy, style.charset[left_char]);
        }

        // Draw Right.
        if (last_row) {
          for (int yy = cell_top; yy < cell_bottom; ++yy) {
            screen.DrawText(cell_right, yy, style.charset[right_char]);
          }
        }

        // Draw Left/Top
        {
          int i = 0;
          for (int yy = Y; yy < cell_top; ++yy) {
            for (int xx = X; xx < cell_left; ++xx) {
              screen.DrawPixel(xx, yy, style.charset[top_left_char][i]);
              ++i;
            }
          }
        }

        // Draw Right/Top
        if (last_row) {
          int i = 0;
          for (int yy = Y; yy < cell_top; ++yy) {
            for (int xx = cell_right; xx < width; ++xx) {
              screen.DrawPixel(xx, yy, style.charset[top_right_char][i]);
              ++i;
            }
          }
        }

        // Draw Left/Bottom
        if (last_line) {
          int i = 0;
          for (int yy = cell_bottom; yy < height; ++yy) {
            for (int xx = X; xx < cell_left; ++xx) {
              screen.DrawPixel(xx, yy, style.charset[bottom_left_char][i]);
              ++i;
            }
          }
        }

        // Draw Right/Bottom
        if (last_row && last_line) {
          int i = 0;
          for (int yy = cell_bottom; yy < height; ++yy) {
            for (int xx = cell_right; xx < width; ++xx) {
              screen.DrawPixel(xx, yy, style.charset[bottom_right_char][i]);
              ++i;
            }
          }
        }

        // Draw Text.
        screen.DrawText(cell_left, cell_top, data[y][x]);
        X = cell_right;
      }
      Y = cell_bottom;
    }

    return screen.ToString();
  }
};

std::unique_ptr<Translator> TableTranslator() {
  return std::make_unique<Table>();
}
