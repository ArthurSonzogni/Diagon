ANTLR(Math.g4)

add_library(translator_math STATIC
  MathLexer.cpp
  MathParser.cpp
  Math.cpp
)
set_property(TARGET translator_math PROPERTY CXX_STANDARD 17)
#target_add_warning(translator_math)
