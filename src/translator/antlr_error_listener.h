#ifndef TRANSLATOR_ANTLR_ERROR_LISTENER_HPP
#define TRANSLATOR_ANTLR_ERROR_LISTENER_HPP

#include <antlr4-runtime.h>

class AntlrErrorListener : public antlr4::BaseErrorListener {
  void syntaxError(antlr4::Recognizer* recognizer,
                   antlr4::Token* offendingSymbol,
                   size_t line,
                   size_t charPositionInLine,
                   const std::string& msg,
                   std::exception_ptr e) final;
};

#endif  // TRANSLATOR_ANTLR_ERROR_LISTENER_HPP
