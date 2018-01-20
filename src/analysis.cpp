#include "analysis.h"
#include "MyLexer.h"
#include "MyParser.h"

namespace analysis {

float evaluate(MyParser::ExpressionContext* e);
float evaluate(MyParser::MinusExpressionContext* e);
float evaluate(MyParser::PlusExpressionContext* e);
float evaluate(MyParser::DivideExpressionContext* e);
float evaluate(MyParser::MultiplyExpressionContext* e);
float evaluate(MyParser::ValueExpressionContext* e);

DisplayTree display_tree(MyParser::ExpressionContext* e);
DisplayTree display_tree(MyParser::MinusExpressionContext* e);
DisplayTree display_tree(MyParser::PlusExpressionContext* e);
DisplayTree display_tree(MyParser::DivideExpressionContext* e);
DisplayTree display_tree(MyParser::MultiplyExpressionContext* e);
DisplayTree display_tree(MyParser::ValueExpressionContext* e);

float evaluate(MyParser::ExpressionContext* e) {
  return evaluate(e->minusExpression());
}

float evaluate(MyParser::MinusExpressionContext* e) {
  float value = evaluate(e->plusExpression(0));
  for (int i = 1; i < e->plusExpression().size(); ++i) {
    value -= evaluate(e->plusExpression(i));
  }
  return value;
}

float evaluate(MyParser::PlusExpressionContext* e) {
  float value = evaluate(e->divideExpression(0));
  for (int i = 1; i < e->divideExpression().size(); ++i) {
    value += evaluate(e->divideExpression(i));
  }
  return value;
}

float evaluate(MyParser::DivideExpressionContext* e) {
  float value = evaluate(e->multiplyExpression(0));
  for (int i = 1; i < e->multiplyExpression().size(); ++i) {
    // TODO division by zero.
    value /= evaluate(e->multiplyExpression(i));
  }
  return value;
}

float evaluate(MyParser::MultiplyExpressionContext* e) {
  float value = evaluate(e->valueExpression(0));
  for (int i = 1; i < e->valueExpression().size(); ++i) {
    value *= evaluate(e->valueExpression(i));
  }
  return value;
}

float evaluate(MyParser::ValueExpressionContext* e) {
  if (e->Int()) {
    return std::stoi(e->Int()->getSymbol()->getText());
  } else {
    return evaluate(e->minusExpression());
  }
}

DisplayTree MergeDisplayTree(DisplayTree up,
                             DisplayTree down,
                             std::string symbol) {
  DisplayTree output;
  if ((up.content.size() + down.entrance - up.entrance) % 2 == 1) {
    up.content.push_back("");
  }

  int k = (up.content.size() + down.entrance - up.entrance) / 2;
  output.entrance = up.entrance + k;
  for (const auto& line : up.content)
    output.content.push_back(std::string(k, ' ') + line);
  for (const auto& line : down.content)
    output.content.push_back(std::string(k, ' ') + line);

  output.content[output.entrance].erase(0, 1);
  output.content[output.entrance].insert(0, symbol);
  for (int i = 1; i < k; ++i) {
    output.content[output.entrance - i].erase(i, 1);
    output.content[output.entrance - i].insert(i, "╱");

    output.content[output.entrance + i].erase(i, 1);
    output.content[output.entrance + i].insert(i, "╲");
  }

  return output;
}

DisplayTree display_tree(MyParser::ExpressionContext* e) {
  return display_tree(e->minusExpression());
}

DisplayTree display_tree(MyParser::MinusExpressionContext* e) {
  DisplayTree output = display_tree(e->plusExpression(0));
  for (int i = 1; i < e->plusExpression().size(); ++i) {
    output = MergeDisplayTree(output, display_tree(e->plusExpression(i)), "-");
  }
  return output;
}

DisplayTree display_tree(MyParser::PlusExpressionContext* e) {
  DisplayTree output = display_tree(e->divideExpression(0));
  for (int i = 1; i < e->divideExpression().size(); ++i) {
    output =
        MergeDisplayTree(output, display_tree(e->divideExpression(i)), "+");
  }
  return output;
}

DisplayTree display_tree(MyParser::DivideExpressionContext* e) {
  DisplayTree output = display_tree(e->multiplyExpression(0));
  for (int i = 1; i < e->multiplyExpression().size(); ++i) {
    output =
        MergeDisplayTree(output, display_tree(e->multiplyExpression(i)), "÷");
  }
  return output;
}

DisplayTree display_tree(MyParser::MultiplyExpressionContext* e) {
  DisplayTree output = display_tree(e->valueExpression(0));
  for (int i = 1; i < e->valueExpression().size(); ++i) {
    output = MergeDisplayTree(output, display_tree(e->valueExpression(i)), "x");
  }
  return output;
}

DisplayTree display_tree(MyParser::ValueExpressionContext* e) {
  if (e->Int()) {
    DisplayTree output;
    output.content.push_back("");
    output.content.push_back(e->Int()->getSymbol()->getText());
    output.content.push_back("");
    output.entrance = 1;
    return output;
  } else {
    return display_tree(e->minusExpression());
  }
}

};  // namespace analysis
