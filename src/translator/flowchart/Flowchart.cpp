// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>
#include "screen/Screen.h"

#include "translator/Translator.h"
#include "translator/flowchart/FlowchartLexer.h"
#include "translator/flowchart/FlowchartParser.h"
#include "util.hpp"

namespace {

class Flowchart : public Translator {
 public:
  virtual ~Flowchart() = default;

 private:
  const char* Name() final { return "Flowchart (Work in progress)"; }
  const char* Identifier() final { return "Flowchart"; }
  const char* Description() final {
    return "Transform a program into ascii art flowchart";
  }
  std::vector<Translator::OptionDescription> Options() final { return {}; }
  std::vector<Translator::Example> Examples() final;
  std::string Translate(const std::string& input,
                        const std::string& options_string) final;
};

std::vector<Translator::Example> Flowchart::Examples() {
  return {
      {
          "xkcd - Flowchart explained by flowchart",
R"source(
if ("DO YOU UNDERSTAND FLOW CHARTS?")
  "GOOD!";
else if ("OKAY, YOU SEE THE LINE LABELED 'YES'?") {
  if ("... AND YOU CAN SEE THE ONES LABELED 'NO'?") {
    "GOOD";
  } else {
    if ("BUT YOU JUST FOLLOWED THEM TWICE?")
      noop;
    else
      noop;
    "(THAT WASN'T A QUESTION)";
    "SCREW IT"
  }
} else {
  if ("BUT YOU SEE THE ONES LABELED 'NO'?") {
    return "WAIT, WHAT?";
  } else {
    "LISTEN.";
    return "I HATE YOU";
  }
}

"LET'S GO DRING";
"HEY, I SHOULD TRY INSTALLING FREEBSD!"

)source",
      },
  };
}

struct Point {
  int x;
  int y;
};

struct Draw {
  Point up;
  Point down;
  Point left;
  Point right;
  Screen screen;
  bool returned = false;
};

Point static_point;

Draw ConnectVertically(Draw a,
                       Draw b,
                       Point& a_right = static_point,
                       Point& b_left = static_point) {
  int height = a.screen.height();
  if (height == 0)
    return b;

  if (a.returned) {
    Draw out;
    out.screen.Append(a.screen, 0, 0);
    out.screen.Append(b.screen, 0, a.screen.height());

    out.left = a.left;
    out.up = a.up;
    a_right = a.right;

    out.right = b.right;
    out.down = b.down;
    b_left = b.left;

    out.right.y += a.screen.height();
    out.down.y += a.screen.height();
    b_left.y += a.screen.height();

    out.returned = b.returned;
    return out;
  }

  int a_x = 0;
  int a_y = 0;
  int b_x = a_x + a.down.x - b.up.x;
  int b_y = height;
  int shifting = std::max(0, a_x - b_x);
  a_x += shifting;
  b_x += shifting;

  Draw out;
  out.screen.Append(a.screen, a_x, a_y);
  out.screen.Append(b.screen, b_x, b_y);

  out.up.x = a_x + a.up.x;
  out.up.y = 0;
  out.down.x = b_x + b.down.x;
  out.down.y = out.screen.height() - 1;

  out.left.x = a_x + a.left.x;
  out.left.y = a.left.y;

  out.right.x = b_x + b.right.x;
  out.right.y = height + b.right.y;

  a_right.x = a_x + a.right.x;
  a_right.y = a_y + a.right.y;

  b_left.x = b_x + b.left.x;
  b_left.y = b_y + b.left.y;

  int edge_x = a_x + a.down.x;
  out.screen.DrawVerticalLineComplete(a_y + a.down.y, b_y + b.up.y, edge_x);
  out.screen.Pixel(edge_x, height) = U'▽';

  out.returned = b.returned;
  return out;
}

Draw ConnectHorizontally(Draw a,
                         Draw b,
                         Point& a_down = static_point,
                         Point& b_up = static_point) {
  int width = a.screen.width();
  if (width == 0)
    return b;

  if (a.returned) {
    Draw out;
    out.screen.Append(a.screen, 0, 0);
    out.screen.Append(b.screen, a.screen.width(), 0);

    out.up = a.up;
    out.left = a.left;
    a_down = a.down;

    out.down = b.down;
    out.right = b.right;
    b_up = b.up;

    out.down.x += a.screen.width();
    out.right.x += a.screen.width();
    b_up.x += a.screen.width();

    out.returned = b.returned;
    return out;
  }

  int a_x = 0;
  int a_y = 0;
  int b_y = a_y + a.right.y - b.left.y;
  int b_x = width;

  int shifting = std::max(0, a_y - b_y);
  a_y += shifting;
  b_y += shifting;

  Draw out;
  out.screen.Append(a.screen, a_x, a_y);
  out.screen.Append(b.screen, b_x, b_y);

  out.left.x = 0;
  out.left.y = a_y + a.left.y;

  out.right.x = out.screen.width() - 1;
  out.right.y = b_y + b.right.y;

  out.up.x = a_x + a.up.x;
  out.up.y = a_y + a.up.y;

  out.down.x = b_x + b.down.x;
  out.down.y = b_y + b.down.y;

  b_up = b.up;
  b_up.x += width;
  b_up.y += b_y;

  a_down = a.down;
  a_down.x += 0;
  a_down.y += a_y;

  int edge_y = a_y + a.right.y;

  //auto& left = out.screen.Pixel(width - 1, edge_y);
  //auto& right = out.screen.Pixel(width, edge_y);

  ////if (left == U'│')
    ////left = U'├';
  ////else
    ////left = U'─';

  ////right = U'>';

  out.screen.DrawHorizontalLine(a_x + a.right.x + 1, b_x + b.left.x - 1,
                                edge_y, U'_');
  out.returned = b.returned;
  return out;
}

Draw Boxed(std::string content, bool is_final);
Draw Unimplemented(bool is_final);
Draw Parse(FlowchartParser::ElementContext* element, bool is_final);
Draw Parse(FlowchartParser::ConditionContext* condition, bool is_final);
// std::string Parse(FlowchartParser::WhileloopContext* whileloop);
// std::string Parse(FlowchartParser::DoloopContext* doloop);
Draw Parse(FlowchartParser::GroupContext* group, bool is_final);
Draw Parse(FlowchartParser::InstructionContext* instruction, bool is_final);
Draw Parse(FlowchartParser::ProgramContext* program, bool is_final);
Draw Parse(FlowchartParser::NoopContext* instruction, bool is_final);

Draw Noop() {
  Draw draw;
  draw.screen.Resize(1,1);
  //draw.screen.Pixel(0,0) = U'─';
  draw.left = {0, 0};
  draw.right = {0, 0};
  draw.down= {0, 0};
  draw.up = {0, 0};
  draw.down.y++;
  return draw;
}

Draw Parse(FlowchartParser::NoopContext* instruction, bool is_final) {
  Draw draw = Noop();
  draw.returned = is_final;
  return draw;
}

std::vector<std::wstring_view> Split(std::wstring_view str,
                                     std::wstring_view delim) {
  std::vector<std::wstring_view> out;

  size_t start = 0;
  while (true) {
    size_t pos = str.find(delim, start);

    // Not found
    if (pos == std::wstring_view::npos) {
      if (str.size() - start)
        out.emplace_back(str.substr(start, str.size() - start));
      break;
    }

    if (pos - start)
      out.emplace_back(str.substr(start, pos - start));
    start = pos + delim.size();
  }
  return out;
}

std::vector<std::wstring_view> Broke(std::wstring_view content, int size) {
  std::vector<std::wstring_view> words = Split(content, L" ");
  std::vector<std::wstring_view> lines;
  int last_line_size = 0;
  for (auto& word : words) {
    if (last_line_size == 0 && word.size() >= size) {
      lines.push_back(word);
      last_line_size = 0;
      continue;
    }

    if (last_line_size + word.size() <= size) {
      if (last_line_size == 0) {
        lines.push_back(word);
        last_line_size = word.size();
      } else {
        int size = std::distance(lines.back().begin(), word.end());
        auto concat = std::wstring_view(lines.back().begin(), size);
        lines.back() = concat;
        last_line_size = size;
      }
      continue;
    }

    lines.push_back(word);
    last_line_size = word.size();
  }
  return lines;
}

// Say with ratio Columns/Line above 10.0. Minimize line number.
std::vector<std::wstring_view> Broke(std::wstring_view content) {
  // This is a bisection.
  int left = 0;
  int right = content.size();
  while (left + 1 < right) {
    int middle = left + (right - left) / 2;
    int lines = Broke(content, middle).size();
    float r = middle / float(lines);
    bool still_good = r >= 10.0 || lines == 1;
    (still_good ? right : left) = middle;
  }
  int lines_number = Broke(content, right).size();
  if (lines_number == 1)
    return {content};

  do {
    right--;
  } while (Broke(content, right).size() == lines_number);
  right++;

  return Broke(content, right);
}

//        ______
//       ╱      ╲
//      ╱ Do you ╲
//     ╱ Do you   ╲
//    ╱ Do you     ╲
//    ╲ Like?      ╱
//     ╲ Like?    ╱
//      ╲ Like?  ╱
//       ╲______╱
//       ______
//      ╱      ╲
//     ╱ Do you ╲
//    ╱ Do you   ╲
//    ╲ Like?    ╱
//     ╲ Like?  ╱
//      ╲______╱

//      ______
//     ╱      ╲           ┌──────┐
//    ╱ Do you ╲__________│jacket│
//    ╲ Like?  ╱yes       │      │
//     ╲______╱           └──────┘
//        │no
// ┌──────▽───────┐
// │jacket        │
// └──────────────┘

Draw Diamond(std::string content, bool is_final) {
  std::wstring content_ws = to_wstring(content);
  std::vector<std::wstring_view> lines = Broke(content_ws);
  if (lines.size() % 2) {
    lines.push_back(L"");
    std::rotate(lines.rbegin(), lines.rbegin() + 1, lines.rend());
  }

  int width = 0;
  for (auto& line : lines)
    width = std::max(width, int(line.size()));
  int height = lines.size();

  width = std::max(3, width);
  width = width + height + 2;

  Draw out;
  out.screen.Resize(width, height+3);

  for (int x = height / 2+1; x < width - height / 2-1; ++x) {
    out.screen.Pixel(x, 0) = U'_';
    out.screen.Pixel(x, height+2) = U'_';
  }

  for (int i = 0; i < height/2+1; ++i) {
    int I = width-i-1;
    out.screen.Pixel(i,1+height/2-i+0) = U'╱';
    out.screen.Pixel(i,1+height/2+i+1) = U'╲';
    out.screen.Pixel(I,1+height/2-i+0) = U'╲';
    out.screen.Pixel(I,1+height/2+i+1) = U'╱';
  }

  for(int i = 0; i<lines.size(); ++i)
    out.screen.DrawText(height/2+1, i+2, lines[i]);

  width = out.screen.width();
  height = out.screen.height();

  out.up.x = width / 2 - 1 + width%2;
  out.up.y = 0;

  out.down.x = width / 2 - 1 + width%2;
  out.down.y = height - 1;

  out.left.x = 0;
  out.left.y = height / 2;

  out.right.x = width - 1;
  out.right.y = height / 2;

  out.returned = is_final;

  return out;
}

Draw Boxed(std::string content, bool is_final) {
  std::wstring content_ws = to_wstring(content);
  std::vector<std::wstring_view> lines = Broke(content_ws);
  int width = 0;
  for (auto& line : lines)
    width = std::max(width, int(line.size()));
  width += 2;
  int height = 2 + lines.size();

  Draw draw;
  draw.screen.Resize(width, height);
  draw.screen.DrawBox(0, 0, width, height);
  for (int i = 0; i < lines.size(); ++i)
    draw.screen.DrawText(1, 1 + i, lines[i]);

  draw.up.x = width / 2;
  draw.up.y = 0;

  draw.down.x = width / 2;
  draw.down.y = height - 1;

  draw.left.x = 0;
  draw.left.y = height / 2;

  draw.right.x = width - 1;
  draw.right.y = height / 2;

  draw.returned = is_final;
  return draw;
}

Draw Text(std::string content, bool is_final) {
  Draw draw;
  draw.screen.Resize(content.size() + 5, 3);
  draw.screen.DrawText(2, 1, to_wstring(content));

  draw.up.x = draw.screen.width() / 2;
  draw.up.y = 0;

  draw.down.x = draw.screen.width() / 2;
  draw.down.y = draw.screen.height() - 1;

  draw.left.x = 0;
  draw.left.y = draw.screen.height() / 2;

  draw.right.x = draw.screen.width() - 1;
  draw.right.y = draw.screen.height() / 2;

  draw.returned = is_final;
  return draw;
}

Draw Unimplemented(bool is_final) {
  return Boxed("Unimplemented", is_final);
}

std::string Parse(FlowchartParser::StringContext* string) {
  if (string->STRING_DOUBLE_QUOTE()) {
    std::string text = string->STRING_DOUBLE_QUOTE()->getSymbol()->getText();
    text = text.substr(1, text.size() - 2);
    replaceAll(text, "\\\"", "\"");
    return text;
  }

  if (string->STRING_SIMPLE_QUOTE()) {
    std::string text = string->STRING_SIMPLE_QUOTE()->getSymbol()->getText();
    text = text.substr(1, text.size() - 2);
    replaceAll(text, "\\\'", "\'");
    return text;
  }

  return "error_Parse_string";
}

Draw Parse(FlowchartParser::ElementContext* element, bool is_final) {
  return Boxed(Parse(element->string()), is_final);
}

void AddLabel(Screen& screen, Point point, std::wstring_view label) {
  Screen label_screen(label.size(), 1);
  label_screen.DrawText(0, 0, label);
  screen.Append(label_screen, point.x + 1, point.y + 1);
}

Draw Parse(FlowchartParser::ConditionContext* condition, bool is_final) {
  if (condition->instruction().size() == 1) {
    //Point if_right;
    Draw if_ = Diamond(Parse(condition->string()), /*is_final=*/false);
    Draw then_ = Parse(condition->instruction()[0], is_final);
    Draw all = ConnectHorizontally(if_, then_);
    if (is_final)
      return all;
    all.screen.Resize(all.screen.width(), all.screen.height() + 1);
    all.screen.DrawHorizontalLine(if_.down.x, all.down.x,
                                  all.screen.height() - 1);
    all.screen.DrawVerticalLineComplete(if_.down.y, all.screen.height() - 1,
                                        if_.down.x);
    all.screen.DrawVerticalLineComplete(all.down.y, all.screen.height() - 1,
                                        all.down.x);
    all.right.x = all.down.x;
    all.right.y = all.screen.height() - 1;

    all.down.x = if_.down.x;
    all.down.y = all.screen.height() - 1;
    return all;
  }

  Point if_right;
  Draw if_ = Diamond(Parse(condition->string()), /*is_final=*/false);
  AddLabel(if_.screen, if_.down, L"no");
  AddLabel(if_.screen, if_.right, L"yes");
  if_.down.y++;

  Draw then_ = Parse(condition->instruction()[0], is_final);
  Draw else_ = Parse(condition->instruction()[1], is_final);
  Draw left = ConnectVertically(if_, else_, if_right);
  left.returned = false;
  left.right = if_right;
  Draw all = ConnectHorizontally(left, then_);
  if (is_final || (then_.returned && else_.returned)) {
    all.returned = true;
    return all;
  }
  all.returned = false;

  all.screen.Resize(all.screen.width(), all.screen.height() + 1);

  if (else_.returned) {
    all.screen.DrawVerticalLineComplete(all.down.y, all.screen.height() - 1,
                                        all.down.x);
    all.down.y = all.screen.height() - 1;
    return all;
  }

  if (then_.returned) {
    all.screen.DrawVerticalLineComplete(left.down.y, all.screen.height() - 1,
                                        left.down.x);
    all.down.x = left.down.x;
    all.down.y = all.screen.height() - 1;
    all.right.x = left.down.x;
    all.right.y = all.screen.height() - 1;
    return all;
  }

  all.screen.DrawHorizontalLine(left.down.x, all.down.x,
                                all.screen.height() - 1);
  all.screen.DrawVerticalLineComplete(left.down.y, all.screen.height() - 1,
                                      left.down.x);
  all.screen.DrawVerticalLineComplete(all.down.y, all.screen.height() - 1,
                                      all.down.x);
  all.right.x = all.down.x;
  all.right.y = all.screen.height() - 1;

  all.down.x = (left.down.x + all.down.x) / 2;
  all.down.y = all.screen.height() - 1;
  return all;
}

// std::string Parse(FlowchartParser::WhileloopContext* whileloop) {
// return "While(" + Parse(whileloop->string()) + "," +
// Parse(whileloop->instruction()) + ")";
//}
// std::string Parse(FlowchartParser::DoloopContext* doloop) {
// return "Do(" + Parse(doloop->instruction()) + "," +
// Parse(doloop->string()) + ")";
//}

Draw Parse(FlowchartParser::ReturninstructionContext* instruction) {
  Draw draw = Parse(instruction->instruction(), true);
  draw.returned = true;
  return draw;
}

Draw Parse(FlowchartParser::InstructionContext* instruction, bool is_final) {
  if (instruction->element())
    return Parse(instruction->element(), is_final);

  if (instruction->condition())
    return Parse(instruction->condition(), is_final);

  // if (instruction->whileloop())
  // return Parse(instruction->whileloop());

  // if (instruction->doloop())
  // return Parse(instruction->doloop());

  if (instruction->group())
    return Parse(instruction->group(), is_final);

  if (instruction->returninstruction())
    return Parse(instruction->returninstruction());

  if (instruction->noop())
    return Parse(instruction->noop(), is_final);

  return Unimplemented(is_final);
}

Draw Parse(FlowchartParser::GroupContext* group, bool is_final) {
  return Parse(group->program(), is_final);
}

Draw Parse(FlowchartParser::ProgramContext* program, bool is_final) {
  Draw out;
  int n = program->instruction().size();
  for (int i = 0; i < n; ++i) {
    out = ConnectVertically(std::move(out), Parse(program->instruction()[i],
                                                  is_final && (i == n - 1)));
  }
  return out;
}

std::string Flowchart::Translate(const std::string& input,
                                 const std::string& options_string) {
  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  FlowchartLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  FlowchartParser parser(&tokens);
  return Parse(parser.program(), true).screen.ToString();
}

}  // namespace

std::unique_ptr<Translator> FlowchartTranslator() {
  return std::make_unique<Flowchart>();
}
