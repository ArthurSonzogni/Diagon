#include "translator/example/Example.h"
#include "translator/example/ExampleLexer.h"
#include "translator/example/ExampleParser.h"

std::unique_ptr<Translator> ExampleTranslator() {
  return std::make_unique<Example>();
}

void Example::Process(const std::string& input) {
  antlr4::ANTLRInputStream input_stream(input);
  ExampleLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  for (auto token : tokens.getTokens()) {
    std::cout << token->toString() << std::endl;
  }

  // Parser.
  ExampleParser parser(&tokens);

  // Print the tree.
  auto program = parser.program();
  std::cout << program->toStringTree(&parser) << std::endl << std::endl;

  output_ = input;
}
std::string Example::Output() {
  return output_;
}
