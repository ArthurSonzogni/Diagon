ANTLR(Flowchart.g4)

add_library(translator_flowchart STATIC
  FlowchartLexer.cpp
  FlowchartParser.cpp
  Flowchart.cpp
)

target_link_libraries(translator_flowchart PRIVATE diagon_base)
target_link_libraries(translator_flowchart PRIVATE antlr4_static)
target_set_common(translator_flowchart)
