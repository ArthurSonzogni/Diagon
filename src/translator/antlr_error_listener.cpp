#include "translator/antlr_error_listener.h"
#include <sstream>

void AntlrErrorListener::syntaxError(antlr4::Recognizer* recognizer,
                                     antlr4::Token* offendingSymbol,
                                     size_t line,
                                     size_t charPositionInLine,
                                     const std::string& msg,
                                     std::exception_ptr e) {
  std::stringstream s;
  s << "Line(" << line << ":" << charPositionInLine << ") Error(" << msg << ")";
  throw std::invalid_argument(s.str());
}
