ANTLR(Sequence.g4)

add_library(translator_sequence STATIC
  SequenceLexer.cpp
  SequenceParser.cpp
  Sequence.cpp
  Sequence.hpp
  Graph.cpp
  Graph.hpp
)
target_set_common(translator_sequence)
target_link_libraries(translator_sequence PRIVATE diagon_base)
