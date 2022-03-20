// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <string>
#include <vector>
#include "screen/Screen.h"
#include "translator/Translator.h"
#include "translator/math/MathLexer.h"
#include "translator/math/MathParser.h"

class Screen;

struct Draw {
  int dim_x = 0;
  int dim_y = 0;
  int center_x = 0;
  int center_y = 0;
  std::vector<std::vector<wchar_t>> content;

  Draw() = default;
  Draw(const std::wstring text);
  void Append(const Draw& other, int x, int y);
  void Resize(int dim_x, int dim_y);
};

struct Style {
  wchar_t divide;
  wchar_t multiply;
  std::wstring lower_or_equal;
  std::wstring greater_or_equal;

  wchar_t left_parenthesis_0;
  wchar_t left_parenthesis_1;
  wchar_t left_parenthesis_2;
  wchar_t left_parenthesis_3;

  wchar_t right_parenthesis_0;
  wchar_t right_parenthesis_1;
  wchar_t right_parenthesis_2;
  wchar_t right_parenthesis_3;

  wchar_t sqrt_0;
  wchar_t sqrt_1;
  wchar_t sqrt_2;
  wchar_t sqrt_3;
  wchar_t sqrt_4;

  std::map<std::wstring, std::wstring> variable_transform;

  wchar_t summation_top;
  wchar_t summation_bottom;
  wchar_t summation_diagonal_top;
  wchar_t summation_diagonal_bottom;

  wchar_t mult_top;
  wchar_t mult_bottom;
  wchar_t mult_intersection;

  std::vector<wchar_t> integral_top;
  std::vector<wchar_t> integral_middle;
  std::vector<wchar_t> integral_bottom;
  int integral_min_height;
};

Draw Parse(MathParser::MultilineEquationContext* context, Style* style);
Draw Parse(MathParser::NewlinesContext*);
Draw Parse(MathParser::EquationContext*, Style*);
Draw Parse(MathParser::ExpressionContext*, Style*);
Draw Parse(MathParser::TermContext*, Style*);
Draw Parse(MathParser::FactorContext*, Style*, bool suppress_parenthesis);
Draw Parse(MathParser::FunctionContext*, Style*);
Draw Parse(MathParser::MatrixContext*, Style*);
Draw Parse(MathParser::ValueBangContext* context, Style*, bool);
Draw Parse(MathParser::ValueContext*, Style*, bool suppress_parenthesis);
Draw Parse(MathParser::AtomContext*, Style*, bool suppress_parenthesis);
Draw Parse(MathParser::VariableContext*, Style* style);
Draw ComposeHorizontal(const Draw& left, const Draw& right, int spaces);
Draw ComposeVertical(const Draw& top, const Draw& down, int spaces);
Draw ComposeDiagonal(const Draw& A, const Draw& B);
Draw WrapWithParenthesis(const Draw& element, Style* style);
std::string to_string(const Draw& draw);

std::wstring ParseLatex(MathParser::MultilineEquationContext* context, Style*);
std::wstring ParseLatex(MathParser::NewlinesContext*, Style*);
std::wstring ParseLatex(MathParser::EquationContext*, Style*);
std::wstring ParseLatex(MathParser::ExpressionContext*, Style*);
std::wstring ParseLatex(MathParser::TermContext*, Style*);
std::wstring ParseLatex(MathParser::FactorContext*,
                        Style*,
                        bool suppress_parenthesis);
std::wstring ParseLatex(MathParser::FunctionContext*, Style*);
std::wstring ParseLatex(MathParser::MatrixContext*, Style*);
std::wstring ParseLatex(MathParser::ValueBangContext* context,
                        Style*,
                        bool suppress_parenthesis);
std::wstring ParseLatex(MathParser::ValueContext*,
                        Style*,
                        bool suppress_parenthesis);
std::wstring ParseLatex(MathParser::AtomContext*,
                        Style*,
                        bool suppress_parenthesis);
std::wstring ParseLatex(MathParser::VariableContext*, Style*);

Draw::Draw(const std::wstring text) {
  content.resize(1);
  for (const auto& c : text) {
    content[0].push_back(c);
  }
  dim_x = content[0].size();
  dim_y = 1;
  center_x = dim_x / 2;
  center_y = 0;
}

void Draw::Append(const Draw& other, int x, int y) {
  Resize(std::max(dim_x, x + other.dim_x),  //
         std::max(dim_y, y + other.dim_y));

  // Write
  for (size_t dy = 0; dy < other.dim_y; ++dy) {
    for (size_t dx = 0; dx < other.dim_x; ++dx) {
      content[y + dy][x + dx] = other.content[dy][dx];
    }
  }
}

void Draw::Resize(int new_dim_x, int new_dim_y) {
  dim_x = new_dim_x;
  dim_y = new_dim_y;

  content.resize(dim_y);
  for (auto& line : content) {
    line.resize(dim_x, L' ');
  }
}

Draw ComposeHorizontal(const Draw& left, const Draw& right, int spaces) {
  int center_y = std::max(left.center_y, right.center_y);

  Draw composition;
  composition.Append(left, 0, center_y - left.center_y);
  composition.Append(right, left.dim_x + spaces, center_y - right.center_y);

  composition.center_x = composition.dim_x / 2;
  composition.center_y = center_y;

  return composition;
}

Draw ComposeVertical(const Draw& top, const Draw& down, int spaces) {
  int center_x = std::max(top.center_x, down.center_x);

  Draw composition;
  composition.Append(top, center_x - top.center_x, 0);
  composition.Append(down, center_x - down.center_x, top.dim_y + spaces);

  composition.center_x = center_x;
  composition.center_y = composition.dim_y / 2;

  return composition;
}

Draw ComposeDiagonalUp(const Draw& A, const Draw& B) {
  Draw composition;
  composition.Append(A, 0, B.dim_y);
  composition.Append(B, A.dim_x, 0);

  composition.center_x = composition.dim_x / 2;
  composition.center_y = A.center_y + B.dim_y;

  return composition;
}

Draw ComposeDiagonalDown(const Draw& A, const Draw& B) {
  Draw composition;
  composition.Append(A, 0, 0);
  composition.Append(B, A.dim_x, A.dim_y);

  composition.center_x = composition.dim_x / 2;
  composition.center_y = A.center_y;

  return composition;
}

// Useful for x_a^b
Draw ComposeDiagonalUpAndDown(const Draw& A, const Draw& B, const Draw& C) {
  Draw composition;
  composition.Append(B, A.dim_x, 0);
  composition.Append(A, 0, B.dim_y);
  composition.Append(C, A.dim_x, B.dim_y + A.dim_y);

  composition.center_x = composition.dim_x / 2;
  composition.center_y = A.center_y + B.dim_y;

  return composition;
}

Draw WrapWithParenthesis(const Draw& element, Style* style) {
  Draw draw;
  draw.Resize(element.dim_x + 2, element.dim_y);

  for (auto& line : draw.content) {
    line.front() = L'|';
    line.back() = L'|';
  }

  for (int i = 0; i < draw.content.size(); ++i) {
    auto& line = draw.content[i];
    bool is_first = (i == 0);
    bool is_last = (i == draw.content.size() - 1);

    if (is_first && is_last) {
      line.front() = style->left_parenthesis_0;
      line.back() = style->right_parenthesis_0;
      continue;
    }

    if (is_first) {
      line.front() = style->left_parenthesis_1;
      line.back() = style->right_parenthesis_1;
      continue;
    }

    if (is_last) {
      line.front() = style->left_parenthesis_3;
      line.back() = style->right_parenthesis_3;
      continue;
    }

    line.front() = style->left_parenthesis_2;
    line.back() = style->right_parenthesis_2;
  }

  draw.Append(element, 1, 0);

  draw.center_x = draw.dim_x / 2;
  draw.center_y = element.center_y;

  return draw;
}

std::wstring WrapWithParenthesisLatex(std::wstring element) {
  return L"\\left(" + element + L"\\right)";
}

Draw Parse(MathParser::MultilineEquationContext* context, Style* style) {
  Draw draw;
  for (int i = 0; i < context->equation().size(); ++i) {
    draw = ComposeVertical(draw, Parse(context->equation(i), style), 0);
    if (i < context->newlines().size())
      draw = ComposeVertical(draw, Parse(context->newlines(i)), 0);
  }
  return draw;
}

std::wstring ParseLatex(MathParser::MultilineEquationContext* context,
                        Style* style) {
  std::wstring out;
  for (int i = 0; i < context->equation().size(); ++i) {
    out += ParseLatex(context->equation(i), style);
    if (i < context->newlines().size())
      out += ParseLatex(context->newlines(i), style);
  }
  return out;
}

Draw Parse(MathParser::NewlinesContext* context) {
  Draw draw;
  draw.Resize(0, context->EOL().size() - 1);
  return draw;
}

std::wstring ParseLatex(MathParser::NewlinesContext* context, Style* style) {
  std::wstring out;
  for (int i = 0; i < context->EOL().size(); ++i)
    if (i == 0)
      out += L" \\\\\n";
    else
      out += L"\\\\\n";
  return out;
}

Draw Parse(MathParser::EquationContext* context, Style* style) {
  Draw draw = Parse(context->expression(0), style);
  for (int i = 1; i < context->expression().size(); ++i) {
    auto op = context->relop(i - 1);
    std::wstring symbol;
    // clang-format off
         if (op->LT()) symbol = L'<';
    else if (op->GT()) symbol = L'>';
    else if (op->LE()) symbol = style->lower_or_equal;
    else if (op->GE()) symbol = style->greater_or_equal;
    else if (op->EQ()) symbol = L'=';
    // clang-format on

    int op_x = draw.dim_x + 1;
    draw = ComposeHorizontal(draw, Parse(context->expression(i), style),
                             2 + symbol.size());

    for (int j = 0; j < symbol.size(); ++j) {
      draw.content[draw.center_y][op_x + j] = symbol[j];
    }
  }
  draw.center_x = 0;
  return draw;
}

std::wstring ParseLatex(MathParser::EquationContext* context, Style* style) {
  std::wstring out = ParseLatex(context->expression(0), style);
  for (int i = 1; i < context->expression().size(); ++i) {
    auto op = context->relop(i - 1);
    std::wstring symbol;
    // clang-format off
         if (op->LT()) out += L" < ";
    else if (op->GT()) out += L" > ";
    else if (op->LE()) out += L" \\leq ";
    else if (op->GE()) out += L" \\geq ";
    else if (op->EQ()) out += L" = ";
    // clang-format on

    out += ParseLatex(context->expression(i), style);
  }
  return out;
}

Draw Parse(MathParser::ExpressionContext* context, Style* style) {
  Draw draw = Parse(context->term(0), style);
  for (int i = 1; i < context->term().size(); ++i) {
    int op_x = draw.dim_x + 1;
    draw = ComposeHorizontal(draw, Parse(context->term(i), style), 3);
    draw.content[draw.center_y][op_x] =
        context->addop(i - 1)->PLUS() ? L'+' : L'-';
  }
  return draw;
}

std::wstring ParseLatex(MathParser::ExpressionContext* context, Style* style) {
  std::wstring out = ParseLatex(context->term(0), style);
  for (int i = 1; i < context->term().size(); ++i) {
    out += context->addop(i - 1)->PLUS() ? L" + " : L" - ";
    out += ParseLatex(context->term(i), style);
  }
  return out;
}

Draw Parse(MathParser::TermContext* context, Style* style) {
  bool suppress_parenthesis_first =
      context->mulop().size() && context->mulop(0)->DIV();
  Draw draw = Parse(context->factor(0), style, suppress_parenthesis_first);
  for (int i = 1; i < context->factor().size(); ++i) {
    if (context->mulop(i - 1)->DIV()) {
      int op_y = draw.dim_y;
      draw = ComposeVertical(draw, Parse(context->factor(i), style, true), 1);
      for (int x = 0; x < draw.dim_x; ++x) {
        draw.content[op_y][x] = style->divide;
      }
      draw.center_y = op_y;
    } else {
      int op_x = draw.dim_x + 1;
      draw =
          ComposeHorizontal(draw, Parse(context->factor(i), style, false), 3);
      draw.content[draw.center_y][op_x] = style->multiply;
    }
  }
  return draw;
}

std::wstring ParseLatex(MathParser::TermContext* context, Style* style) {
  bool suppress_parenthesis_first =
      context->mulop().size() && context->mulop(0)->DIV();
  std::wstring out =
      ParseLatex(context->factor(0), style, suppress_parenthesis_first);
  for (int i = 1; i < context->factor().size(); ++i) {
    if (context->mulop(i - 1)->DIV()) {
      out = L"\\frac{" + out + L"}{" +
            ParseLatex(context->factor(i), style, true) + L"}";
    } else {
      out += L" \\cdot " + ParseLatex(context->factor(i), style, false);
    }
  }
  return out;
}

Draw Parse(MathParser::FactorContext* context,
           Style* style,
           bool suppress_parenthesis) {
  suppress_parenthesis &= (context->valueBang().size() == 1);
  Draw draw = Parse(context->valueBang(0), style, suppress_parenthesis);

  // Optimization for a_b^c and a^c:
  if (context->valueBang().size() == 3) {
    if (context->powop(0)->POW() && context->powop(1)->SUBSCRIPT()) {
      return ComposeDiagonalUpAndDown(
          draw, Parse(context->valueBang(1), style, false),
          Parse(context->valueBang(2), style, false));
    }
    if (context->powop(1)->POW() && context->powop(0)->SUBSCRIPT()) {
      return ComposeDiagonalUpAndDown(
          draw, Parse(context->valueBang(2), style, false),
          Parse(context->valueBang(1), style, false));
    }
  }

  for (int i = 1; i < context->valueBang().size(); ++i) {
    auto* compose =
        context->powop(i - 1)->POW() ? ComposeDiagonalUp : ComposeDiagonalDown;
    draw = compose(draw, Parse(context->valueBang(i), style, false));
  }
  return draw;
}

std::wstring ParseLatex(MathParser::FactorContext* context,
                        Style* style,
                        bool suppress_parenthesis) {
  suppress_parenthesis &= (context->valueBang().size() == 1);
  std::wstring out =
      ParseLatex(context->valueBang(0), style, suppress_parenthesis);
  for (int i = 1; i < context->valueBang().size(); ++i) {
    out += context->powop(i - 1)->POW() ? L"^" : L"_";
    out += L"{" + ParseLatex(context->valueBang(i), style, false) + L"}";
  }
  return out;
}

Draw Parse(MathParser::ValueBangContext* context,
           Style* style,
           bool suppress_parenthesis) {
  if (context->value()) {
    return Parse(context->value(), style, suppress_parenthesis);
  } else {
    return ComposeHorizontal(
        Parse(context->valueBang(), style, suppress_parenthesis), Draw(L"!"),
        0);
  }
}

std::wstring ParseLatex(MathParser::ValueBangContext* context,
                        Style* style,
                        bool suppress_parenthesis) {
  if (context->value())
    return ParseLatex(context->value(), style, suppress_parenthesis);
  else
    return ParseLatex(context->valueBang(), style, suppress_parenthesis) + L"!";
}

bool CheckFunctionSqrt(MathParser::FunctionContext* context) {
  int num_arguments = context->equation().size();
  if (num_arguments != 1) {
    std::cerr << "Square root function (sqrt) only handle one argument, "
              << num_arguments << " provided" << std::endl;
    return false;
  }
  return true;
}

Draw ParseFunctionSqrt(MathParser::FunctionContext* context, Style* style) {
  if (!CheckFunctionSqrt(context))
    return Draw(L"(error)");

  Draw content = Parse(context->equation(0), style);
  Draw draw;
  draw.Append(content, 1 + content.dim_y, 1);
  draw.content.back().front() = style->sqrt_0;
  for (int y = 0; y < draw.content.size() - 1; ++y)
    draw.content[draw.content.size() - 1 - y][1 + y] = style->sqrt_1;
  for (int x = draw.content.size(); x < draw.content[0].size(); ++x)
    draw.content[0][x] = style->sqrt_2;
  draw.center_x = draw.dim_x / 2;
  draw.center_y = content.center_y + 1;
  return draw;
}

bool CheckFunctionSum(MathParser::FunctionContext* context) {
  int num_arguments = context->equation().size();
  if (num_arguments > 3) {
    std::cerr << "Summation function (sum) only handle 1,2 or 3 arguments, "
              << num_arguments << " provided" << std::endl;
    return false;
  }
  return true;
}

Draw ParseFunctionSum(MathParser::FunctionContext* context, Style* style) {
  if (!CheckFunctionSum(context))
    return Draw(L"(error)");

  Draw content = Parse(context->equation(0), style);
  Draw down =
      context->equation(1) ? Parse(context->equation(1), style) : Draw();
  Draw top = context->equation(2) ? Parse(context->equation(2), style) : Draw();

  int sigma_height = std::max(4, (content.dim_y + 1) / 2 * 2 + 2);
  int sigma_width = (sigma_height - 2) / 2 + 2;

  Draw sigma;
  sigma.Resize(sigma_width, sigma_height);

  for (int x = 0; x < sigma.dim_x; ++x) {
    sigma.content.front()[x] = style->summation_top;
    sigma.content.back()[x] = style->summation_bottom;
  }

  {
    int x = 0;
    int y_1 = 1;
    int y_2 = sigma.content.size() - 2;
    while (y_1 < y_2) {
      sigma.content[y_1][x] = style->summation_diagonal_top;
      sigma.content[y_2][x] = style->summation_diagonal_bottom;
      ++x;
      ++y_1;
      --y_2;
    }
  }

  // Align top, sigma, and bottom on center.
  top.center_x = top.dim_x / 2;
  sigma.center_x = sigma.dim_x / 2;
  down.center_x = down.dim_x / 2;

  Draw sum = ComposeVertical(ComposeVertical(top, sigma, 0), down, 0);
  sum.center_y =
      top.dim_y + sigma.content.size() - content.dim_y + content.center_y - 1;

  return ComposeHorizontal(sum, content, 1);
}

std::wstring ParseFunctionSumLatex(MathParser::FunctionContext* context,
                                   Style* style) {
  if (!CheckFunctionSum(context))
    return L"(error)";

  std::wstring out = L"\\sum";
  if (context->equation(1))
    out += L"_{" + ParseLatex(context->equation(1), style) + L"}";
  if (context->equation(2))
    out += L"^{" + ParseLatex(context->equation(2), style) + L"}";
  return out + L" " + ParseLatex(context->equation(0), style);
}

bool CheckFunctionMult(MathParser::FunctionContext* context) {
  int num_arguments = context->equation().size();
  if (num_arguments > 3) {
    std::cerr
        << "Multiplication function (mult) only handle 1,2 or 3 arguments, "
        << num_arguments << " provided" << std::endl;
    return false;
  }
  return true;
}

Draw ParseFunctionMult(MathParser::FunctionContext* context, Style* style) {
  if (!CheckFunctionMult(context))
    return Draw(L"(error)");

  Draw content = Parse(context->equation(0), style);
  Draw down =
      context->equation(1) ? Parse(context->equation(1), style) : Draw();
  Draw top = context->equation(2) ? Parse(context->equation(2), style) : Draw();

  int mult_height = std::max(2, content.dim_y);
  int mult_width = mult_height + 2;

  Draw mult;
  mult.Resize(mult_width, mult_height);
  for (int x = 0; x < mult_width; ++x) {
    mult.content[0][x] = style->mult_top;
  }
  for (int y = 1; y < mult_height; ++y) {
    mult.content[y][1] = style->mult_bottom;
    mult.content[y][mult_width - 2] = style->mult_bottom;
  }
  mult.content[0][1] = style->mult_intersection;
  mult.content[0][mult_width - 2] = style->mult_intersection;

  // Align top, mult, and bottom on center.
  top.center_x = top.dim_x / 2;
  mult.center_x = mult.dim_x / 2;
  down.center_x = down.dim_x / 2;

  Draw ret = ComposeVertical(ComposeVertical(top, mult, 0), down, 0);
  ret.center_y =
      top.dim_y + mult.content.size() - content.dim_y + content.center_y;

  return ComposeHorizontal(ret, content, 1);
}

std::wstring ParseFunctionMultLatex(MathParser::FunctionContext* context,
                                    Style* style) {
  if (!CheckFunctionMult(context))
    return L"(error)";

  std::wstring out = L"\\prod";
  if (context->equation(1))
    out += L"_{" + ParseLatex(context->equation(1), style) + L"}";
  if (context->equation(2))
    out += L"^{" + ParseLatex(context->equation(2), style) + L"}";
  return out + L" " + ParseLatex(context->equation(0), style);
}

Draw ParseFunctionMathBB(MathParser::FunctionContext* context, Style* style) {
  static const std::map<std::string, std::string> known = {
      {"0", "𝟘"},      //
      {"1", "𝟙"},      //
      {"2", "𝟚"},      //
      {"3", "𝟛"},      //
      {"4", "𝟜"},      //
      {"5", "𝟝"},      //
      {"6", "𝟞"},      //
      {"7", "𝟟"},      //
      {"8", "𝟠"},      //
      {"9", "𝟡"},      //
      {"A", "𝔸"},      //
      {"B", "𝔹"},      //
      {"C", "ℂ"},      //
      {"D", "𝔻"},      //
      {"E", "𝔼"},      //
      {"F", "𝔽"},      //
      {"G", "𝔾"},      //
      {"H", "ℍ"},      //
      {"I", "𝕀"},      //
      {"J", "𝕁"},      //
      {"K", "𝕂"},      //
      {"L", "𝕃"},      //
      {"M", "𝕄"},      //
      {"N", "ℕ"},      //
      {"O", "𝕆"},      //
      {"P", "ℙ"},      //
      {"PI", "ℿ"},     //
      {"Q", "ℚ"},      //
      {"R", "ℝ"},      //
      {"S", "𝕊"},      //
      {"T", "𝕋"},      //
      {"U", "𝕌"},      //
      {"V", "𝕍"},      //
      {"W", "𝕎"},      //
      {"X", "𝕏"},      //
      {"Y", "𝕐"},      //
      {"Z", "ℤ"},      //
      {"a", "𝕒"},      //
      {"b", "𝕓"},      //
      {"c", "𝕔"},      //
      {"d", "𝕕"},      //
      {"e", "𝕖"},      //
      {"f", "𝕗"},      //
      {"g", "𝕘"},      //
      {"h", "𝕙"},      //
      {"i", "ⅈ"},      //
      {"i", "𝕚"},      //
      {"j", "𝕛"},      //
      {"k", "𝕜"},      //
      {"l", "𝕝"},      //
      {"m", "𝕞"},      //
      {"n", "𝕟"},      //
      {"o", "𝕠"},      //
      {"p", "𝕡"},      //
      {"pi", "ℼ"},     //
      {"q", "𝕢"},      //
      {"r", "𝕣"},      //
      {"s", "𝕤"},      //
      {"t", "𝕥"},      //
      {"u", "𝕦"},      //
      {"v", "𝕧"},      //
      {"w", "𝕨"},      //
      {"x", "𝕩"},      //
      {"y", "𝕪"},      //
      {"z", "𝕫"},      //
  };

  std::string name;
  for (int i = 0; i < context->equation().size(); ++i)
    name += context->equation(i)->getText();
  Draw draw;
  while (name.size() > 0) {
    bool found = false;
    for (const auto& it : known) {
      if (name.rfind(it.first) == 0) {
        name = name.substr(it.first.size());
        draw = ComposeHorizontal(draw, Draw(to_wstring(it.second)), 0);
        found = true;
        break;
      }
    }
    if (!found) {
      name = name.substr(1);
      draw = ComposeHorizontal(draw, Draw(L"?"), 0);
    }
  }
  return draw;
}

std::wstring ParseFunctionMathBBLatex(MathParser::FunctionContext* context,
                                      Style* style) {
  std::string name = context->equation(0)->getText();
  for (int i = 1; i < context->equation().size(); ++i)
    name += context->equation(i)->getText();
  return L"\\mathbb{" + to_wstring(name) + L"}";
}

bool CheckFunctionIntegral(MathParser::FunctionContext* context) {
  int num_arguments = context->equation().size();
  if (num_arguments > 3) {
    std::cerr << "Integral function (int) only handle 1,2 or 3 arguments, "
              << num_arguments << " provided" << std::endl;
    return false;
  }

  return true;
}

Draw ParseFunctionIntegral(MathParser::FunctionContext* context, Style* style) {
  if (!CheckFunctionIntegral(context))
    return Draw(L"(error)");

  Draw content = Parse(context->equation(0), style);
  Draw down =
      context->equation(1) ? Parse(context->equation(1), style) : Draw();
  Draw top = context->equation(2) ? Parse(context->equation(2), style) : Draw();

  int integral_height = std::max(style->integral_min_height, content.dim_y);
  int integral_width = style->integral_top.size();

  Draw integral;
  integral.Resize(integral_width, integral_height);

  integral.content.front() = style->integral_top;
  integral.content.back() = style->integral_bottom;
  for (int y = 1; y < integral.content.size() - 1; ++y)
    integral.content[y] = style->integral_middle;

  // Align top, integral, and bottom on center.
  top.center_x = top.dim_x / 2;
  integral.center_x = integral.dim_x / 2;
  down.center_x = down.dim_x / 2;

  Draw sum = ComposeVertical(ComposeVertical(top, integral, 0), down, 0);
  sum.center_y =
      top.dim_y + integral.content.size() - content.dim_y + content.center_y;

  return ComposeHorizontal(sum, content, 1);
}

std::wstring ParseFunctionIntegralLatex(MathParser::FunctionContext* context,
                                        Style* style) {
  if (!CheckFunctionIntegral(context))
    return L"(error)";

  std::wstring out = L"\\int";
  if (context->equation(1))
    out += L"_{" + ParseLatex(context->equation(1), style) + L"}";
  if (context->equation(2))
    out += L"^{" + ParseLatex(context->equation(2), style) + L"}";
  return out + L" " + ParseLatex(context->equation(0), style);
}

Draw ParseFunctionCommon(MathParser::FunctionContext* context, Style* style) {
  Draw content = Parse(context->equation(0), style);
  for (int i = 1; i < context->equation().size(); ++i) {
    int x = content.dim_x;
    content = ComposeHorizontal(content, Parse(context->equation(i), style), 2);
    content.content[content.center_y][x] = L',';
  }
  return ComposeHorizontal(Parse(context->variable(), style),
                           WrapWithParenthesis(content, style),
                           content.dim_y == 1 ? 0 : 1);
}

std::wstring ParseFunctionCommonLatex(MathParser::FunctionContext* context,
                                      Style* style) {
  std::wstring content = ParseLatex(context->equation(0), style);
  for (int i = 1; i < context->equation().size(); ++i)
    content += L"," + ParseLatex(context->equation(0), style);
  return ParseLatex(context->variable(), style) +
         WrapWithParenthesisLatex(content);
}

std::wstring ParseFunctionSqrtLatex(MathParser::FunctionContext* context,
                                    Style* style) {
  std::wstring content = ParseLatex(context->equation(0), style);
  for (int i = 1; i < context->equation().size(); ++i)
    content += L"," + ParseLatex(context->equation(0), style);
  return L"\\sqrt{" + content + L"}";
}

std::wstring ParseFunctionKnownLatex(MathParser::FunctionContext* context,
                                     Style* style,
                                     const std::wstring& name) {
  std::wstring content = ParseLatex(context->equation(0), style);
  for (int i = 1; i < context->equation().size(); ++i)
    content += L"," + ParseLatex(context->equation(0), style);
  return name + WrapWithParenthesisLatex(content);
}

Draw Parse(MathParser::FunctionContext* context, Style* style) {
  std::string function_name = context->variable()->VARIABLE()->getText();
  if (function_name == "sqrt")
    return ParseFunctionSqrt(context, style);
  if (function_name == "sum")
    return ParseFunctionSum(context, style);
  if (function_name == "int")
    return ParseFunctionIntegral(context, style);
  if (function_name == "mult")
    return ParseFunctionMult(context, style);
  if (function_name == "mathbb" || function_name == "bb")
    return ParseFunctionMathBB(context, style);
  return ParseFunctionCommon(context, style);
}

std::wstring ParseLatex(MathParser::FunctionContext* context, Style* style) {
  static const std::map<std::string, std::wstring> known = {
      {"arccos", L"\\arccos"},  //
      {"arcsin", L"\\arcsin"},  //
      {"arctan", L"\\arctan"},  //
      {"cos", L"\\cos"},        //
      {"cosh", L"\\cosh"},      //
      {"cot", L"\\cot"},        //
      {"coth", L"\\coth"},      //
      {"exp ", L"\\exp "},      //
      {"ker", L"\\ker"},        //
      {"ln", L"\\ln"},          //
      {"log", L"\\log"},        //
      {"max", L"\\max"},        //
      {"min", L"\\min"},        //
      {"sin", L"\\sin"},        //
      {"sinh", L"\\sinh"},      //
      {"tan", L"\\tan"},        //
      {"tanh", L"\\tanh"},      //
  };

  std::string function_name = context->variable()->VARIABLE()->getText();
  if (function_name == "sqrt")
    return ParseFunctionSqrtLatex(context, style);
  if (function_name == "sum")
    return ParseFunctionSumLatex(context, style);
  if (function_name == "int")
    return ParseFunctionIntegralLatex(context, style);
  if (function_name == "mult")
    return ParseFunctionMultLatex(context, style);
  if (const auto it = known.find(function_name); it != known.end())
    return ParseFunctionKnownLatex(context, style, it->second);
  if (function_name == "mathbb" || function_name == "bb")
    return ParseFunctionMathBBLatex(context, style);
  return ParseFunctionCommonLatex(context, style);
}

Draw Parse(MathParser::ValueContext* context,
           Style* style,
           bool suppress_parenthesis) {
  suppress_parenthesis &= (!context->PLUS() && !context->MINUS());
  Draw atom = Parse(context->atom(), style, suppress_parenthesis);
  if (context->MINUS())
    return ComposeHorizontal(Draw(L"-"), atom, 0);
  if (context->PLUS())
    return ComposeHorizontal(Draw(L"+"), atom, 0);
  return atom;
}

std::wstring ParseLatex(MathParser::ValueContext* context,
                        Style* style,
                        bool suppress_parenthesis) {
  suppress_parenthesis &= (!context->PLUS() && !context->MINUS());
  std::wstring atom = ParseLatex(context->atom(), style, suppress_parenthesis);
  if (context->MINUS())
    return L"-" + atom;
  if (context->PLUS())
    return L"+" + atom;
  return atom;
}

Draw ParseString(antlr4::tree::TerminalNode* node) {
  std::wstring s = to_wstring(node->getText());
  s = s.substr(1, s.length() - 2);  // Remove quotes.
  return Draw(s);
}

std::wstring ParseStringLatex(antlr4::tree::TerminalNode* node) {
  return to_wstring(node->getText());
}

Draw Parse(MathParser::AtomContext* context,
           Style* style,
           bool suppress_parenthesis) {
  if (context->variable())
    return Parse(context->variable(), style);

  if (context->expression()) {
    Draw draw = Parse(context->expression(), style);
    if (suppress_parenthesis || context->RBRACE()) {
      return draw;
    } else {
      return WrapWithParenthesis(draw, style);
    }
  }

  if (context->function())
    return Parse(context->function(), style);

  if (context->matrix())
    return Parse(context->matrix(), style);

  if (context->STRING())
    return ParseString(context->STRING());

  // XXX
  return Draw();
}

std::wstring ParseLatex(MathParser::AtomContext* context,
                        Style* style,
                        bool suppress_parenthesis) {
  if (context->variable())
    return ParseLatex(context->variable(), style);

  if (context->expression()) {
    std::wstring out = ParseLatex(context->expression(), style);
    if (suppress_parenthesis || context->RBRACE())
      return out;
    else
      return WrapWithParenthesisLatex(out);
  }

  if (context->function())
    return ParseLatex(context->function(), style);

  if (context->matrix())
    return ParseLatex(context->matrix(), style);

  if (context->STRING())
    return ParseStringLatex(context->STRING());

  return L"";
}

Draw Parse(MathParser::VariableContext* context, Style* style) {
  std::wstring label = to_wstring(context->VARIABLE()->getText());
  if (style->variable_transform.count(label))
    label = style->variable_transform.at(label);
  return Draw(label);
}

std::wstring ParseLatex(MathParser::VariableContext* context, Style* style) {
  std::wstring label = to_wstring(context->VARIABLE()->getText());
  if (style->variable_transform.count(label))
    label = style->variable_transform.at(label);
  return label;
}

Draw Parse(MathParser::MatrixContext* context, Style* style) {
  // 1) Get matrix content.
  std::vector<std::vector<Draw>> content;
  for (const auto& line : context->matrixLine()) {
    std::vector<Draw> line_content;
    for (const auto& content : line->expression()) {
      line_content.emplace_back(Parse(content, style));
    }
    content.push_back(std::move(line_content));
  }

  // 2) Sanitize
  size_t height = content.size();
  size_t width = 0;
  for (const auto& it : content)
    width = std::max(width, it.size());
  for (auto& it : content)
    it.resize(width);

  // 3) Compute element sizes.
  std::vector<int> y_size(height, 0);
  std::vector<int> x_size(width, 0);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      x_size[x] = std::max(x_size[x], content[y][x].dim_x);
      y_size[y] = std::max(y_size[y], content[y][x].dim_y);
    }
  }
  std::vector<int> y_top(height, 0);
  std::vector<int> x_left(width, 0);
  for (int i = 1; i < height; ++i) {
    y_top[i] = y_top[i - 1] + y_size[i - 1] + 1;
  }
  for (int i = 1; i < width; ++i) {
    x_left[i] = x_left[i - 1] + x_size[i - 1] + 1;
  }

  // 4) Draw
  Draw draw;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto& c = content[y][x];
      draw.Append(std::move(c), x_left[x], y_top[y] + y_size[y] - c.dim_y);
    }
  }

  draw.center_x = draw.dim_x / 2;
  draw.center_y = draw.dim_y / 2;
  return WrapWithParenthesis(draw, style);
}

std::wstring ParseLatex(MathParser::MatrixContext* context, Style* style) {
  std::wstring out = L"\\begin{pmatrix} ";
  bool first_line = true;
  for (const auto& line : context->matrixLine()) {
    if (!first_line)
      out += L" \\\\ ";
    first_line = false;
    bool first_column = true;
    for (const auto& content : line->expression()) {
      if (!first_column)
        out += L" & ";
      first_column = false;
      out += ParseLatex(content, style);
    }
  }

  return out + L" \\end{pmatrix}";
}

std::string to_string(const Draw& draw) {
  std::wstring s;
  for (const auto& line : draw.content) {
    for (const auto& c : line) {
      s += c;
    }
    s += L'\n';
  }
  return to_string(s);
}

class Math : public Translator {
 public:
  ~Math() override = default;

  const char* Name() final { return "Mathematical expression"; }
  const char* Identifier() final { return "Math"; }
  const char* Description() final { return "Math description"; }

  std::vector<Translator::OptionDescription> Options() final {
    return {
        {
            "style",
            {
                "Unicode",
                "ASCII",
                "Latex",
            },
            "Unicode",
            "Use the full unicode charset or only ASCII. Or even latex.",
            Widget::Combobox,
        },
        {
            "transform_math_letters",
            {
                "false",
                "true",
            },
            "true",
            "Transform letter name into their unicode glyph. alpha -> α.",
            Widget::Checkbox,
        },
    };
  }

  std::vector<Translator::Example> Examples() final {
    return {
        {"1-fraction", "f(x) = 1 + x / (1 + x)"},
        {"2-square-root", "sqrt(1+sqrt(1+x/2))"},
        {"3-power", "f(x) = 1 + x^2 + x^3 + x^(1+1/2)"},
        {"4-subscript", "S_n = u_1 + u_2 + ... + u_n"},
        {"5-summation", "sum(i^2,i=0,n) = n^3/2+n^2/2+n/6"},
        {"6-integral", "int(x^2 * dx ,0,1) = n^3/3"},
        {"7-product",
         "mult(i^2,i=1,n) = (mult(i,i=1,n))^2\n\n\n\nmult(1/2,1,100) = "
         "7.8886091e-31"},
        {"8-vector", "[a;b] + [c;d] = [a+c; b+d]"},
        {"9-matrix", "[1,2;3,4] * [x;y] = [1*x+2*y; 3*x+4*y]"},
        {"10-factorial", "[n;k] = n! / (k! *(n-k)!)"},
        {"11-quoted-string",
         "\"x_n\"\n"
         " x_n\n"},
        {"12-braces-vs-parenthesis",
         "A_(1+2)\n"
         "\n"
         "A_{1+2}\n"
         "\n"
         "A^{1+2}\n"},
        {"13-Math-symbols",
         "Alpha + alpha + Digamma + digamma + Kappa + kappa + Omicron \n"
         "omicron + Upsilon + upsilon + Beta + beta + Zeta + zeta + Lambda \n"
         "lambda + Pi + pi + Phi + phi + Gamma + gamma + Eta + eta + Mu + mu \n"
         "Rho + rho + Chi + chi + Delta + delta + Theta + theta + Nu + nu \n"
         "Sigma + sigma + Psi + psi + Epsilon + epsilon + Iota + iota + Xi\n"
         "xi + Tau + tau + Omega + omega"},
        {"14-mathbb",
         "mathbb(R)\n\nbb(R)\n\nbb(ABCDEFGHIJKLMNOPQRSTUVWXYZ)\n\nbb("
         "abcdefghijklmnopqrstuvwxyz)\n\nbb(0123456789)"},
        {"100-continued-fraction", "psi = 1 + 1/(1+1/(1+1/(1+1/(1+...))))"},
    };
  }

  std::string Translate(const std::string& input,
                        const std::string& options_string) final {
    auto options = SerializeOption(options_string);
    Style style;
    if (options["style"] == "ASCII") {
      style.divide = L'-';
      style.multiply = L'.';
      style.greater_or_equal = L">=";
      style.lower_or_equal = L"<=";
      style.left_parenthesis_0 = L'(';
      style.left_parenthesis_1 = L'/';
      style.left_parenthesis_2 = L'|';
      style.left_parenthesis_3 = L'\\';
      style.right_parenthesis_0 = L')';
      style.right_parenthesis_1 = L'\\';
      style.right_parenthesis_2 = L'|';
      style.right_parenthesis_3 = L'/';

      style.sqrt_0 = L'\\';
      style.sqrt_1 = L'/';
      style.sqrt_2 = L'_';

      style.summation_top = L'=';
      style.summation_bottom = L'=';
      style.summation_diagonal_top = L'\\';
      style.summation_diagonal_bottom = L'/';

      style.mult_top = L'_';
      style.mult_bottom = L'|';
      style.mult_intersection = L'_';

      style.integral_top = {L' ', L'.', L'-'};
      style.integral_middle = {L' ', L'|', L' '};
      style.integral_bottom = {L'-', L'\'', L' '};
      style.integral_min_height = 3;
    } else {
      style.divide = L'─';
      // style.multiply = L'×';
      style.multiply = L'⋅';
      style.greater_or_equal = L"≥";
      style.lower_or_equal = L"≤";

      style.left_parenthesis_0 = L'(';
      style.left_parenthesis_1 = L'⎛';
      style.left_parenthesis_2 = L'⎜';
      style.left_parenthesis_3 = L'⎝';
      style.right_parenthesis_0 = L')';
      style.right_parenthesis_1 = L'⎞';
      style.right_parenthesis_2 = L'⎟';
      style.right_parenthesis_3 = L'⎠';

      style.sqrt_0 = L'╲';
      style.sqrt_1 = L'╱';
      style.sqrt_2 = L'_';

      style.summation_top = L'_';
      style.summation_bottom = L'‾';
      style.summation_diagonal_top = L'╲';
      style.summation_diagonal_bottom = L'╱';

      style.mult_top = L'━';
      style.mult_bottom = L'┃';
      style.mult_intersection = L'┳';

      style.integral_top = {L'⌠'};
      style.integral_middle = {L'⎮'};
      style.integral_bottom = {L'⌡'};
      style.integral_min_height = 2;
    }

    if (options["style"] == "Latex") {
      if (options["transform_math_letters"] != "false") {
        style.variable_transform = {
            // Greek alphabet
            {L"...", L"\\ldots"},
            {L"Alpha", L"\\Alpha"},
            {L"alpha", L"\\alpha"},
            {L"Digamma", L"\\Digamma"},
            {L"digamma", L"\\digamma"},
            {L"Kappa", L"\\Kappa"},
            {L"kappa", L"\\kappa"},
            {L"Omicron", L"\\Omicron"},
            {L"omicron", L"\\omicron"},
            {L"Upsilon", L"\\Upsilon"},
            {L"upsilon", L"\\upsilon"},
            {L"Beta", L"\\Beta"},
            {L"beta", L"\\beta"},
            {L"Zeta", L"\\Zeta"},
            {L"zeta", L"\\zeta"},
            {L"Lambda", L"\\Lambda"},
            {L"lambda", L"\\lambda"},
            {L"Pi", L"\\Pi"},
            {L"pi", L"\\pi"},
            {L"Phi", L"\\Phi"},
            {L"phi", L"\\phi"},
            {L"Gamma", L"\\Gamma"},
            {L"gamma", L"\\gamma"},
            {L"Eta", L"\\Eta"},
            {L"eta", L"\\eta"},
            {L"Mu", L"\\Mu"},
            {L"mu", L"\\mu"},
            {L"Rho", L"\\Rho"},
            {L"rho", L"\\rho"},
            {L"Chi", L"\\Chi"},
            {L"chi", L"\\chi"},
            {L"Delta", L"\\Delta"},
            {L"delta", L"\\delta"},
            {L"Theta", L"\\Theta"},
            {L"theta", L"\\theta"},
            {L"Nu", L"\\Nu"},
            {L"nu", L"\\nu"},
            {L"Sigma", L"\\Sigma"},
            {L"sigma", L"\\sigma"},
            {L"Psi", L"\\Psi"},
            {L"psi", L"\\psi"},
            {L"Epsilon", L"\\Epsilon"},
            {L"epsilon", L"\\epsilon"},
            {L"Iota", L"\\Iota"},
            {L"iota", L"\\iota"},
            {L"Xi", L"\\Xi"},
            {L"xi", L"\\xi"},
            {L"Tau", L"\\Tau"},
            {L"tau", L"\\tau"},
            {L"Omega", L"\\Omega"},
            {L"omega", L"\\omega"},

            // Symbols
            {L"infty", L"\\infty"},
            {L"infinity", L"\\infty"},
        };
      }

      style.variable_transform[L"..."] = L"\\ldots";
    } else if (options["transform_math_letters"] != "false") {
      style.variable_transform = {
          // Greek alphabet
          {L"Alpha", L"Α"},
          {L"alpha", L"α"},
          {L"Digamma", L"Ϝ"},
          {L"digamma", L"ϝ"},
          {L"Kappa", L"Κ"},
          {L"kappa", L"ϰ"},
          {L"Omicron", L"Ο"},
          {L"omicron", L"ο"},
          {L"Upsilon", L"Υ"},
          {L"upsilon", L"υ"},
          {L"Beta", L"Β"},
          {L"beta", L"β"},
          {L"Zeta", L"Ζ"},
          {L"zeta", L"ζ"},
          {L"Lambda", L"Λ"},
          {L"lambda", L"λ"},
          {L"Pi", L"Π"},
          {L"pi", L"π"},
          {L"Phi", L"ϕ"},
          {L"phi", L"φ"},
          {L"Gamma", L"Γ"},
          {L"gamma", L"γ"},
          {L"Eta", L"Η"},
          {L"eta", L"η"},
          {L"Mu", L"Μ"},
          {L"mu", L"μ"},
          {L"Rho", L"ρ"},
          {L"rho", L"ϱ"},
          {L"Chi", L"Χ"},
          {L"chi", L"χ"},
          {L"Delta", L"Δ"},
          {L"delta", L"δ"},
          {L"Theta", L"θ"},
          {L"theta", L"ϑ"},
          {L"Nu", L"Ν"},
          {L"nu", L"ν"},
          {L"Sigma", L"σ"},
          {L"sigma", L"ς"},
          {L"Psi", L"Ψ"},
          {L"psi", L"ψ"},
          {L"Epsilon", L"ϵ"},
          {L"epsilon", L"ε"},
          {L"Iota", L"Ι"},
          {L"iota", L"ι"},
          {L"Xi", L"Ξ"},
          {L"xi", L"ξ"},
          {L"Tau", L"Τ"},
          {L"tau", L"τ"},
          {L"Omega", L"Ω"},
          {L"omega", L"ω"},

          // Symbols
          {L"infty", L"∞"},
          {L"infinity", L"∞"},
      };
    }

    //
    antlr4::ANTLRInputStream input_stream(input);

    // Lexer.
    MathLexer lexer(&input_stream);
    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    // Parser.
    MathParser parser(&tokens);
    auto* content = parser.multilineEquation();

    if (options["style"] == "Latex")
      return to_string(ParseLatex(content, &style)) + '\n';

    // Print th
    return to_string(Parse(content, &style));
  }
};

std::unique_ptr<Translator> MathTranslator() {
  return std::make_unique<Math>();
}
