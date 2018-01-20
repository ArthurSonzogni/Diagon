#ifndef COMPUTE_EXPRESSION_H
#define COMPUTE_EXPRESSION_H

#include "MyParser.h"

namespace analysis {

float evaluate(MyParser::ExpressionContext* expression);

struct DisplayTree {
  int entrance;
  std::vector<std::string> content;
};
DisplayTree display_tree(MyParser::ExpressionContext* expression);

}  // namespace analysis

#endif /* end of include guard: COMPUTE_EXPRESSION_H */
