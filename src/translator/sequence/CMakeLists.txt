ANTLR(Sequence.g4)

add_library(translator_sequence STATIC
  SequenceLexer.cpp
  SequenceParser.cpp
  Sequence.cpp
)
target_set_common(translator_sequence)
