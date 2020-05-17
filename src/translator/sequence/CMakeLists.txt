ANTLR(Sequence.g4)

add_library(translator_sequence STATIC
  SequenceLexer.cpp
  SequenceParser.cpp
  Sequence.cpp
)
target_add_warning(translator_sequence)
