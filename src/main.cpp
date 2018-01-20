#include <iostream>

//#include "antlr4-runtime.h"
#include "MyLexer.h"
#include "MyParser.h"
#include "analysis.h"

// using namespace antlrcpptest;
// using namespace antlr4;

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

int main(int, const char **) { return 0; }

extern "C" {

void translate(const char *input) {
  antlr4::ANTLRInputStream input_stream(input);

  // Apply Lexer.
  MyLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  // Apply Parser.
  MyParser parser(&tokens);
  MyParser::ExpressionContext *expression = parser.expression();

  // Print the tree.
  analysis::DisplayTree tree = analysis::display_tree(expression);

  std::string command;
  command += "output.value='";
  for (const auto &line : tree.content) {
    command += line + "\\n";
  }
  command += "'";
  emscripten_run_script(command.c_str());
}
}

#else

int main(int, const char **) {
  // Ask the user for an input.
  std::string text_input;
  std::cout << "Please enter an expression (for instance (1+2)*(3-4)/(5-6))"
            << std::endl;
  std::cout << "input = ";
  std::cin >> text_input;
  antlr4::ANTLRInputStream input(text_input);

  // Apply Lexer.
  MyLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  // Apply Parser.
  MyParser parser(&tokens);
  MyParser::ExpressionContext *expression = parser.expression();

  // Evaluate the result.
  std::cout << "Result = " << analysis::evaluate(expression) << std::endl;

  // Print the tree.
  std::cout << "Tree = " << std::endl;
  analysis::DisplayTree tree = analysis::display_tree(expression);
  for (const auto &line : tree.content)
    std::cout << line << std::endl;

  return 0;
}

#endif
