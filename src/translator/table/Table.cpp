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
      separator = U',';
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
    int line_count = data.size();
    int row_count = 0;
    for (const auto& line : data) {
      row_count = std::max(row_count, (int)line.size());
    }

    // Uniformize the number of cells per lines.
    for (auto& line : data) {
      line.resize(row_count);
    }

    // Compute row_width;
    std::vector<int> row_width(row_count, 0);
    for (const auto& line : data) {
      for(int i = 0; i<line.size(); ++i) {
        row_width[i] = std::max(row_width[i], (int)line[i].size());
      }
    }

    // Compute sum_row_width;
    int row_width_global = 0;
    for(const auto it : row_width) {
      row_width_global += it;
    }

    // Compute screen dimension.
    int width = style.width[0] + style.width[1] * (row_count - 1) +
                style.width[2] + row_width_global;
    int height = style.height[0] + style.height[1] +
                 style.height[2] * (line_count - 2) + style.height[3] + line_count;

    Screen screen(width, height);

    // Draw table.
    int Y = 0;
    for (int y = 0; y < line_count; ++y) {
      bool last_line = (y == line_count - 1);
      int X = 0;

      const int cell_top = Y + style.height[std::min(2,y)];
      const int cell_bottom = cell_top + 1;

      for (int x = 0; x < data[y].size(); ++x) {
        bool last_row = (x == row_count - 1);
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
        const int cell_right = cell_left + row_width[x];

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
        for(int yy = cell_top; yy < cell_bottom; ++yy) {
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
          for(int yy = Y; yy<cell_top; ++yy) {
            for(int xx = X; xx<cell_left; ++xx) {
              screen.DrawPixel(xx,yy,style.charset[top_left_char][i]);
              ++i;
            }
          }
        }

        // Draw Right/Top
        if (last_row) {
          int i = 0;
          for (int yy = Y; yy < cell_top; ++yy) {
            for (int xx = cell_right; xx < width; ++xx) {
              screen.DrawPixel(xx,yy,style.charset[top_right_char][i]);
              ++i;
            }
          }
        }

        // Draw Left/Bottom
        if (last_line)
        {
          int i = 0;
          for (int yy = cell_bottom; yy < height; ++yy) {
            for (int xx = X; xx < cell_left; ++xx) {
              screen.DrawPixel(xx, yy, style.charset[bottom_left_char][i]);
              ++i;
            }
          }
        }

        // Draw Right/Bottom
        if (last_row && last_line)
        {
          int i = 0;
          for(int yy = cell_bottom; yy<height; ++yy) {
            for (int xx = cell_right; xx < width; ++xx) {
              screen.DrawPixel(xx,yy,style.charset[bottom_right_char][i]);
              ++i;
            }
          }
        }
        if (false) {
        }

        // Draw Text.
        screen.DrawText(cell_left, cell_top, data[y][x]);
        X = cell_right;
      }
      Y = cell_bottom;
    }

    return screen.ToString();
  }

  // ----------------------------------------------
  const char* Name() override;
  const char* Description() override;
  std::vector<OptionDescription> Options() override;
  std::vector<Example> Examples() override;
};

std::unique_ptr<Translator> TableTranslator() {
  return std::make_unique<Table>();
}

const char* Table::Name() {
  return "Table";
}

const char* Table::Description() {
  return "Draw table";
}

std::vector<Translator::OptionDescription> Table::Options() {
  return {
      {
        "style",
        "The style of the table.\n"
        "Possible values:\n"
        " - unicode\n"
        " - unicode rounded\n"
        " - unicode bold\n"
        " - unicode double\n"
        " - unicode with bold header\n"
        " - unicode with double header\n"
        " - unicode cells\n"
        " - unicode cells 2\n"
        " - ascii\n"
        " - ascii rounded\n"
        " - ascii with header 1\n"
        " - ascii with header 2\n"
        " - ascii light header\n"
        " - ascii light header/separator\n"
        " - ascii light header/separator/border\n"
        " - ascii light separator/border\n"
        " - ascii light border\n"
        " - conceptual"
      }
  };
}

std::vector<Translator::Example> Table::Examples() {
  return {
      {"1-simple",
       "Column 1,Column 2,Column 3\n"
       "C++,Web,Assembly\n"
       "Javascript,CSS,HTML"},
  };
}
