#include "translator/math/Math.h"

#include "screen/Screen.h"
#include "translator/math/MathLexer.h"
#include "translator/math/MathParser.h"

std::unique_ptr<Translator> MathTranslator() {
  return std::make_unique<Math>();
}

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

Draw ComposeDiagonal(const Draw& A, const Draw& B) {
  Draw composition;
  composition.Append(A, 0, B.dim_y);
  composition.Append(B, A.dim_x, 0);

  composition.center_x = composition.dim_x / 2;
  composition.center_y = A.center_y + B.dim_y;

  return composition;
}

Draw WrapWithParenthesis(const Draw& element, Style* style) {
  Draw draw;
  draw.Resize(element.dim_x + 2, element.dim_y);

  for (auto& line : draw.content) {
    line.front() = U'|';
    line.back() = U'|';
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

Draw Parse(MathParser::MultilineEquationContext* context, Style* style) {
  Draw draw;
  for (int i = 0; i < context->equation().size(); ++i) {
    draw = ComposeVertical(draw, Parse(context->equation(i), style), 0);
    if (i<context->newlines().size())
    draw = ComposeVertical(draw, Parse(context->newlines(i)), 0);
  }
  return draw;
}

Draw Parse(MathParser::NewlinesContext* context) {
  Draw draw;
  draw.Resize(0, context->EOL().size()-1);
  return draw;
}

Draw Parse(MathParser::EquationContext* context, Style* style) {

  Draw draw = Parse(context->expression(0), style);
  for (int i = 1; i < context->expression().size(); ++i) {
    auto op = context->relop(i - 1);
    std::wstring symbol;
    // clang-format off
    if (op->LT()) symbol += U'<';
    if (op->GT()) symbol += U'>';
    if (op->EQ()) symbol += U'=';

    if (symbol == L"<=") symbol = style->lower_or_equal;
    if (symbol == L">=") symbol = style->greater_or_equal;
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

Draw Parse(MathParser::ExpressionContext* context, Style* style) {
  Draw draw = Parse(context->term(0), style);
  for (int i = 1; i < context->term().size(); ++i) {
    int op_x = draw.dim_x + 1;
    draw = ComposeHorizontal(draw, Parse(context->term(i), style), 3);
    draw.content[draw.center_y][op_x] =
        context->addop(i - 1)->PLUS() ? U'+' : U'-';
  }
  return draw;
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

Draw Parse(MathParser::FactorContext* context,
           Style* style,
           bool suppress_parenthesis) {
  suppress_parenthesis &= (context->value().size() == 1);
  Draw draw = Parse(context->value(0), style, suppress_parenthesis);
  for (int i = 1; i < context->value().size(); ++i) {
    draw = ComposeDiagonal(draw, Parse(context->value(i), style, false));
  }
  return draw;
}

Draw Parse(MathParser::ValueContext* context,
           Style* style,
           bool suppress_parenthesis) {
  if (context->function()) {
    return Parse(context->function(), style);
  } else {
    return Parse(context->signedAtom(), style, suppress_parenthesis);
  }
}

Draw Parse(MathParser::FunctionContext* context, Style* style) {
  Draw content = Parse(context->expression(), style);
  std::string function_name = context->variable()->VARIABLE()->getText();
  if (function_name == "sqrt") {
    Draw draw;
    draw.Append(content, 1 + content.dim_y, 1);
    draw.content.back().front() = style->sqrt_0;
    for(int y = 0; y<draw.content.size(); ++y)
      draw.content[draw.content.size()-1-y][1+y] = style->sqrt_1;
    for(int x = draw.content.size(); x<draw.content[0].size(); ++x)
      draw.content[0][x] = style->sqrt_2;
    draw.center_y = content.center_y+1;
    return draw;
  }
  return ComposeHorizontal(Parse(context->variable(), style),
                           WrapWithParenthesis(content, style), 1);
}

Draw Parse(MathParser::SignedAtomContext* context,
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

Draw Parse(MathParser::AtomContext* context,
           Style* style,
           bool suppress_parenthesis) {
  if (context->variable()) {
    return Parse(context->variable(), style);
  }
  if (context->expression()) {
    Draw draw = Parse(context->expression(), style);
    if (suppress_parenthesis) {
      return draw;
    } else {
      return WrapWithParenthesis(draw, style);
    }
  }
  if (context->scientific()) {
    return Parse(context->scientific());
  }
  // XXX
  return Draw();
}

Draw Parse(MathParser::VariableContext* context, Style* style) {
  std::wstring label = to_wstring(context->VARIABLE()->getText());
  if (style->variable_transform.count(label)) {
    label = style->variable_transform.at(label);
  }
  return Draw(label);
}

Draw Parse(MathParser::ScientificContext* context) {
  return Draw(to_wstring(context->SCIENTIFIC_NUMBER()->getText()));
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

std::string Math::operator()(const std::string& input,
                             const std::string& options_string) {
  auto options = SerializeOption(options_string);
  Style style;
  if (options["ascii_only"] == "true") {
    style.divide = U'-';
    style.multiply = U'.';
    style.greater_or_equal = L">=";
    style.lower_or_equal = L"<=";
    style.left_parenthesis_0 = U'(';
    style.left_parenthesis_1 = U'/';
    style.left_parenthesis_2 = U'|';
    style.left_parenthesis_3 = U'\\';
    style.right_parenthesis_0 = U')';
    style.right_parenthesis_1 = U'\\';
    style.right_parenthesis_2 = U'|';
    style.right_parenthesis_3 = U'/';

    style.sqrt_0 = U'\\';
    style.sqrt_1 = U'/';
    style.sqrt_2 = U'_';
  } else {
    style.divide = U'─';
    style.multiply = U'×';
    style.greater_or_equal = L"≥";
    style.lower_or_equal = L"≤";

    style.left_parenthesis_0 = U'(';
    style.left_parenthesis_1 = U'⎛';
    style.left_parenthesis_2 = U'⎜';
    style.left_parenthesis_3 = U'⎝';
    style.right_parenthesis_0 = U')';
    style.right_parenthesis_1 = U'⎞';
    style.right_parenthesis_2 = U'⎟';
    style.right_parenthesis_3 = U'⎠';

    style.sqrt_0 = U'╲';
    style.sqrt_1 = U'╱';
    style.sqrt_2 = U'_';
  }

  if (options["transform_math_letters"] == "true") {
    style.variable_transform = {
        {L"Alpha", L"Α"},   {L"alpha", L"α"},   {L"Digamma", L"Ϝ"},
        {L"digamma", L"ϝ"}, {L"Kappa", L"Κ"},   {L"kappa", L"ϰ"},
        {L"Omicron", L"Ο"}, {L"omicron", L"ο"}, {L"Upsilon", L"Υ"},
        {L"upsilon", L"υ"}, {L"Beta", L"Β"},    {L"beta", L"β"},
        {L"Zeta", L"Ζ"},    {L"zeta", L"ζ"},    {L"Lambda", L"Λ"},
        {L"lambda", L"λ"},  {L"Pi", L"Π"},      {L"pi", L"π"},
        {L"Phi", L"ϕ"},     {L"phi", L"φ"},     {L"Gamma", L"Γ"},
        {L"gamma", L"γ"},   {L"Eta", L"Η"},     {L"eta", L"η"},
        {L"Mu", L"Μ"},      {L"mu", L"μ"},      {L"Rho", L"ρ"},
        {L"rho", L"ϱ"},     {L"Chi", L"Χ"},     {L"chi", L"χ"},
        {L"Delta", L"Δ"},   {L"delta", L"δ"},   {L"Theta", L"θ"},
        {L"theta", L"ϑ"},   {L"Nu", L"Ν"},      {L"nu", L"ν"},
        {L"Sigma", L"σ"},   {L"sigma", L"ς"},   {L"Psi", L"Ψ"},
        {L"psi", L"ψ"},     {L"Epsilon", L"ϵ"}, {L"epsilon", L"ε"},
        {L"Iota", L"Ι"},    {L"iota", L"ι"},    {L"Xi", L"Ξ"},
        {L"xi", L"ξ"},      {L"Tau", L"Τ"},     {L"tau", L"τ"},
        {L"Omega", L"Ω"},   {L"omega", L"ω"}};
  }


  //⌠
  //⎮
  //⌡

  //⎲
  //⧹
  //⧸
  //⎳

  //∑


  //
  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  MathLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  // Parser.
  MathParser parser(&tokens);

  // Print th
  auto content = parser.multilineEquation();
  return to_string(Parse(content, &style));
}
