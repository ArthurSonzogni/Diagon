#ifndef TRANSLATOR_MATH_H
#define TRANSLATOR_MATH_H

#include <string>
#include <vector>

#include "translator/Translator.h"
#include "translator/math/Math.h"
#include "translator/math/MathLexer.h"
#include "translator/math/MathParser.h"

class Screen;

class Math : public Translator {
 public:
  virtual ~Math() = default;
  std::string Translate(const std::string& input,
                        const std::string& options) override;

  const char* Name() override;
  const char* Description() override;
  std::vector<OptionDescription> Options() override;
  std::vector<Example> Examples() override;
};

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

#endif /* end of include guard: TRANSLATOR_MATH_H */
