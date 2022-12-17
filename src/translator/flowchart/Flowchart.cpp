// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>
#include "screen/Screen.h"
#include "translator/Translator.h"
#include "translator/antlr_error_listener.h"
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
  std::string Highlight(const std::string& input) final;
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
      {
          "xkcd - local computer expert",
          R"source(
"START";

do {
  if ("FIND A MENU ITEM OR BUTTON WHICH LOOKS RELATED TO WHAT YOU WANT TO DO.") {
    "CLICK IT.";
  } else {
    if ("PICK ON AT RANDOM.") {
      "CLICK IT.";
    } else {
      "GOOGLE THE NAME OF THE PROGRAM PLUS A FEW WORDS RELATED TO WHAT YOU WANT TO DO. FOLLOW ANY INSTRUCTIONS.";
    }
  }

  if ("DID IT WORK?")
    return "YOU'RE DONE!"

} while("HAVE YOU BEEN TRYING THIS FOR LESS THAN AN HOUR?")

"ASK SOMEONE FOR HELP OR GIVE UP."
)source",
      },
  };
}

struct Point {
  int x;
  int y;
};

Point operator+(const Point& a, const Point& b) {
  return Point{
      a.x + b.x,
      a.y + b.y,
  };
}

void operator+=(Point& a, const Point& b) {
  a.x += b.x;
  a.y += b.y;
}

struct Draw {
  Screen screen;
  std::vector<Point> top;
  std::vector<Point> left;
  std::vector<Point> bottom;
  std::vector<Point> right;
  bool returned = false;
};

std::vector<Point> Merge(const std::vector<Point> a,
                         const std::vector<Point> b) {
  std::vector<Point> out;
  for (Point it : a)
    out.push_back(it);
  for (Point it : b)
    out.push_back(it);
  return out;
}

void Shift(std::vector<Point>& out, Point shift) {
  for (auto& it : out) {
    it += shift;
  }
}

void Shift(Draw& out, Point shift) {
  Shift(out.top, shift);
  Shift(out.left, shift);
  Shift(out.bottom, shift);
  Shift(out.right, shift);
}

Point static_point_1;
Point static_point_2;

Draw ConnectVertically(Draw a,
                       Draw b,
                       Point& a_shift = static_point_1,
                       Point& b_shift = static_point_2) {
  int height = a.screen.height();
  if (height == 0)
    return b;

  if (a.bottom.size() == 0 || b.top.size() == 0) {
    a_shift = Point{0, 0};
    b_shift = Point{0, height};

    Draw out;
    out.screen.Append(a.screen, a_shift.x, a_shift.y);
    out.screen.Append(b.screen, b_shift.x, b_shift.y);

    out.left = Merge(a.left, b.left);
    out.right = Merge(a.right, b.right);
    out.top = a.top;
    out.bottom = b.bottom;
    out.returned = b.returned;

    return out;
  }

  a_shift = Point{0, 0};
  b_shift = Point{a_shift.x + a.bottom[0].x - b.top[0].x, height};

  int shifting = std::max(0, a_shift.x - b_shift.x);
  a_shift.x += shifting;
  b_shift.x += shifting;

  Draw out;
  out.screen.Append(a.screen, a_shift.x, a_shift.y);
  out.screen.Append(b.screen, b_shift.x, b_shift.y);

  Shift(a, a_shift);
  Shift(b, b_shift);

  out.left = Merge(a.left, b.left);
  out.right = Merge(a.right, b.right);
  out.top = a.top;
  out.bottom = b.bottom;
  out.returned = b.returned;

  out.screen.DrawVerticalLineComplete(a.bottom[0].y + 1, b.top[0].y - 1,
                                      a.bottom[0].x);

  auto& top = out.screen.Pixel(a.bottom[0].x, a.bottom[0].y);
  if (top == L'─')
    top = L'┬';

  auto& bottom = out.screen.Pixel(b.top[0].x, b.top[0].y);
  if (bottom == L'─')
    bottom = L'▽';
  if (bottom == L'-')
    bottom = L'▽';
  if (bottom == L'_')
    bottom = L'▽';
  if (bottom == L' ')
    bottom = L'│';

  out.returned = b.returned;
  return out;
}

Draw ConnectHorizontally(Draw a,
                         Draw b,
                         Point& a_shift = static_point_1,
                         Point& b_shift = static_point_2) {
  int width = a.screen.width();
  if (width == 0) {
    return b;
  }

  a_shift = Point{
      0,
      0,
  };
  b_shift = Point{
      width,
      a_shift.y + a.right[0].y - b.left[0].y,
  };

  int shifting = std::max(0, a_shift.y - b_shift.y);
  a_shift.y += shifting;
  b_shift.y += shifting;

  Draw out;
  out.screen.Append(a.screen, a_shift.x, a_shift.y);
  out.screen.Append(b.screen, b_shift.x, b_shift.y);

  Shift(a, a_shift);
  Shift(b, b_shift);

  out.top = Merge(a.top, b.top);
  out.left = a.left;
  out.right = b.right;
  out.bottom = Merge(a.bottom, b.bottom);

  out.returned = a.returned || b.returned;

  out.screen.DrawHorizontalLine(a.right[0].x + 1, b.left[0].x - 1, a.right[0].y,
                                L'_');
  return out;
}

Draw MergeBottoms(Draw draw) {
  if (draw.bottom.size() <= 1) {
    return draw;
  }
  draw.screen.Resize(draw.screen.width(), draw.screen.height() + 1);

  int left = draw.bottom.front().x;
  int right = draw.bottom.back().x;
  draw.screen.DrawHorizontalLine(left, right, draw.screen.height() - 1, L'─');

  for (auto& it : draw.bottom) {
    draw.screen.DrawVerticalLine(it.y + 1, draw.screen.height() - 2, it.x,
                                 L'│');
    auto& top = draw.screen.Pixel(it.x, it.y);
    if (top == L'─')
      top = L'┬';

    auto& bottom = draw.screen.Pixel(it.x, draw.screen.height() - 1);
    if (bottom == L'─')
      bottom = L'┴';
  }

  draw.screen.Pixel(left, draw.screen.height() - 1) = L'└';
  draw.screen.Pixel(right, draw.screen.height() - 1) = L'┘';

  draw.bottom = {{(5 * left + 2 * right) / 7, draw.screen.height() - 1}};
  return draw;
}

Draw Boxed(std::string content, bool is_final);
Draw Unimplemented(bool is_final);
Draw Parse(FlowchartParser::ElementContext* element, bool is_final);
Draw Parse(FlowchartParser::ConditionContext* condition, bool is_final);
Draw Parse(FlowchartParser::GroupContext* group, bool is_final);
Draw Parse(FlowchartParser::InstructionContext* instruction, bool is_final);
Draw Parse(FlowchartParser::ProgramContext* program, bool is_final);
Draw Parse(FlowchartParser::NoopContext* instruction, bool is_final);
Draw Parse(FlowchartParser::WhileloopContext* whileloop, bool is_final);
Draw Parse(FlowchartParser::DoloopContext* doloop, bool is_final);

Draw ParseUnmerged(FlowchartParser::ConditionContext* condition, bool is_final);

Draw Noop() {
  Draw draw;
  draw.screen.Resize(1, 1);
  draw.left = {{0, 0}};
  draw.right = {{0, 0}};
  draw.bottom = {{0, 0}};
  draw.top = {{0, 0}};
  return draw;
}

Draw Parse(FlowchartParser::NoopContext* instruction, bool is_final) {
  Draw draw = Noop();
  draw.returned = is_final;
  if (is_final) {
    draw.bottom = {};
  }
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
  size_t last_line_size = 0;
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
        std::wstring_view last_line = lines.back();
        size_t size = std::distance(last_line.begin(), word.end());
        std::wstring_view new_last_line(&last_line[0], size);
        lines.back() = new_last_line;
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
  } while (Broke(content, right).size() == lines_number && right >= 0);
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
    // std::rotate(lines.rbegin(), lines.rbegin() + 1, lines.rend());
  }

  int width = 0;
  for (auto& line : lines)
    width = std::max(width, int(line.size()));
  int height = lines.size();

  width = std::max(3, width);
  width = width + height + 2;

  Draw out;
  out.screen.Resize(width, height + 3);

  for (int x = height / 2 + 1; x < width - height / 2 - 1; ++x) {
    out.screen.Pixel(x, 0) = L'_';
    out.screen.Pixel(x, height + 2) = L'_';
  }

  for (int i = 0; i < height / 2 + 1; ++i) {
    int I = width - i - 1;
    out.screen.Pixel(i, 1 + height / 2 - i + 0) = L'╱';
    out.screen.Pixel(i, 1 + height / 2 + i + 1) = L'╲';
    out.screen.Pixel(I, 1 + height / 2 - i + 0) = L'╲';
    out.screen.Pixel(I, 1 + height / 2 + i + 1) = L'╱';
  }

  for (int i = 0; i < lines.size(); ++i)
    out.screen.DrawText(height / 2 + 1, i + 2, lines[i]);

  width = out.screen.width();
  height = out.screen.height();

  out.top = {{width / 2 - 1 + width % 2, 0}};
  out.bottom = {{width / 2 - 1 + width % 2, height - 1}};
  out.left = {{0, height / 2}};
  out.right = {{width - 1, height / 2}};
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

  draw.top = {{width / 2, 0}};
  draw.bottom = {{width / 2, height - 1}};
  draw.left = {{0, height / 2}};
  draw.right = {{width - 1, height / 2}};

  draw.returned = is_final;
  return draw;
}

// Draw Text(std::string content, bool is_final) {
// Draw draw;
// draw.screen.Resize(content.size() + 5, 3);
// draw.screen.DrawText(2, 1, to_wstring(content));

// draw.top.x = draw.screen.width() / 2;
// draw.top.y = 0;

// draw.bottom.x = draw.screen.width() / 2;
// draw.bottom.y = draw.screen.height() - 1;

// draw.left.x = 0;
// draw.left.y = draw.screen.height() / 2;

// draw.right.x = draw.screen.width() - 1;
// draw.right.y = draw.screen.height() / 2;

// draw.returned = is_final;
// return draw;
//}

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

Draw ParseUnmerged(FlowchartParser::ConditionContext* condition,
                   bool is_final) {
  Draw if_ = Diamond(Parse(condition->string()), /*is_final=*/false);
  AddLabel(if_.screen, if_.bottom[0], L"no");
  AddLabel(if_.screen, if_.right[0], L"yes");
  Draw then_ = Parse(condition->instruction()[0], is_final);

  // An 'if' without an 'else':
  if (condition->instruction().size() == 1) {
    return ConnectHorizontally(std::move(if_), std::move(then_));
  }

  Draw else_ = Parse(condition->instruction()[1], is_final);
  else_.right = {};
  Draw if_else_ = ConnectVertically(std::move(if_), std::move(else_));
  Draw if_then_else_ =
      ConnectHorizontally(std::move(if_else_), std::move(then_));
  return if_then_else_;
}

Draw Parse(FlowchartParser::ConditionContext* condition, bool is_final) {
  return ParseUnmerged(condition, is_final);
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
  draw.bottom = {};
  draw.right = {};
  draw.returned = true;
  return draw;
}

Draw Parse(FlowchartParser::InstructionContext* instruction, bool is_final) {
  if (instruction->element())
    return Parse(instruction->element(), is_final);

  if (instruction->condition())
    return Parse(instruction->condition(), is_final);

  if (instruction->whileloop())
    return Parse(instruction->whileloop(), is_final);

  if (instruction->doloop())
    return Parse(instruction->doloop(), is_final);

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
    out = MergeBottoms(std::move(out));
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

  // Parser:
  FlowchartParser parser(&tokens);

  AntlrErrorListener error_listener;
  parser.addErrorListener(&error_listener);

  FlowchartParser::ProgramContext* context = nullptr;
  try {
    context = parser.program();
  } catch (...) {
    return "Error";
  }

  return Parse(context, true).screen.ToString();
}

std::string Flowchart::Highlight(const std::string& input) {
  std::stringstream out;

  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  FlowchartLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);

  try {
    tokens.fill();
  }
  catch (...) {  // Ignore
  }

  size_t matched = 0;
  out << "<span class='flowchart'>";
  for(antlr4::Token* token : tokens.getTokens()) {
    std::string text = token->getText();
    if (text == "<EOF>") {
      continue;
    }
    out << "<span class='";
    out << lexer.getVocabulary().getSymbolicName(token->getType());
    out << "'>";
    matched += text.size();
    out << std::move(text);
    out << "</span>";
  }

  out << input.substr(matched);
  out << "</span>";

  return out.str();
}

Draw Parse(FlowchartParser::WhileloopContext* whileloop, bool is_final) {
  Draw if_ = Diamond(Parse(whileloop->string()), /*is_final=*/false);
  Screen if_screen;
  std::swap(if_.screen, if_screen);
  if_.screen.Append(std::move(if_screen), 4, 0);
  Shift(if_, Point{4, 0});

  AddLabel(if_.screen, if_.bottom[0], L"yes");

  Point no_position = if_.left[0];
  no_position.x -= 4;
  AddLabel(if_.screen, no_position, L"no");

  Draw instruction = Parse(whileloop->instruction(), is_final);
  instruction = MergeBottoms(instruction);

  Point if_left = if_.left[0];
  Point if_right = if_.right[0];

  Point if_shift;
  Point instruction_shift;
  Draw merged = ConnectVertically(std::move(if_), std::move(instruction),
                                  if_shift, instruction_shift);
  if_left += if_shift;
  if_right += if_shift;

  Draw out;
  out.screen.Append(merged.screen, 1, 0);
  out.screen.Resize(out.screen.width() + 2, out.screen.height() + 1);

  //  --- if_left                   if_right ----
  //                                            |
  //             instruction_bottom             |
  //                     |                      |
  //                     |                      |
  //                     -----------------------

  out.screen.DrawHorizontalLine(1, if_left.x, if_left.y, '_');

  if (merged.bottom.size()) {
    out.screen.DrawHorizontalLine(if_right.x + 2, out.screen.width() - 2,
                                  if_right.y, '_');
    out.screen.DrawHorizontalLine(merged.bottom[0].x + 2,
                                  out.screen.width() - 1,
                                  out.screen.height() - 1, L'─');
    out.screen.DrawVerticalLineComplete(merged.bottom[0].y + 1,
                                        out.screen.height() - 2,
                                        merged.bottom[0].x + 1);
    out.screen.DrawVerticalLineComplete(if_right.y + 1, out.screen.height() - 1,
                                        out.screen.width() - 1);
    out.screen.Pixel(merged.bottom[0].x + 1, out.screen.height() - 1) = L'└';
    out.screen.Pixel(out.screen.width() - 1, out.screen.height() - 1) = L'┘';
    auto& connector =
        out.screen.Pixel(merged.bottom[0].x + 1, merged.bottom[0].y);
    if (connector == L'─')
      connector = L'┬';
  }

  out.top = {merged.top[0] + if_shift};
  out.left = {};
  out.right = {};
  out.bottom = {Point{0, if_left.y}};

  out.returned = false;

  return out;
}

Draw Parse(FlowchartParser::DoloopContext* doloop, bool is_final) {
  Draw instruction = MergeBottoms(Parse(doloop->instruction(), is_final));
  instruction.left = {};
  instruction.right = {};
  Draw if_ = Diamond(Parse(doloop->string()), /*is_final=*/false);
  Point instruction_position;
  Point if_position;
  Draw merged = ConnectVertically(std::move(instruction), std::move(if_),
                                  instruction_position, if_position);
  //                     |_______________
  //                     |               |
  //                  [     ]            |
  //                     |               |
  //                    / \   if_right --
  //                    \ /
  //                     |
  //                     |
  merged.left = {};
  merged.right = {};

  AddLabel(merged.screen, merged.bottom[0], L"no");
  AddLabel(merged.screen, merged.right[0], L"yes");

  // |___
  // |
  // Add 2 empty line above
  Screen merged_screen;
  std::swap(merged.screen, merged_screen);
  merged.screen.Resize(merged_screen.width() + 1, merged_screen.height() + 2);
  merged.screen.Append(std::move(merged_screen), 0, 2);
  Shift(merged, Point{0, 2});

  merged.screen.DrawHorizontalLine(merged.right[0].x + 1,
                                   merged.screen.width() - 1,
                                   merged.right[0].y + 2, L'_');
  merged.screen.DrawHorizontalLine(merged.top[0].x + 1,
                                   merged.screen.width() - 1, 1, L'─');
  merged.screen.DrawVerticalLineComplete(1, merged.right[0].y + 2,
                                         merged.screen.width() - 1);

  merged.screen.DrawVerticalLineComplete(0, 2, merged.top[0].x);

  merged.screen.Pixel(merged.screen.width() - 1, 1) = L'╮';
  merged.screen.Pixel(merged.top[0].x + 1, 1) = L'◁';
  return merged;
}

}  // namespace

std::unique_ptr<Translator> FlowchartTranslator() {
  return std::make_unique<Flowchart>();
}
