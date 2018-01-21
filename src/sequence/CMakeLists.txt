ANTLR(INPUT SequenceLexer.g4)
ANTLR(INPUT SequenceParser.g4 DEPENDENCIES SequenceLexer.cpp)

add_library(sequence STATIC
  SequenceLexer.cpp
  SequenceParser.cpp
  Sequence.cpp
  SequenceImpl.cpp
)
