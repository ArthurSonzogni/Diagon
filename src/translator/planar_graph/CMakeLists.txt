ANTLR(PlanarGraph.g4)

add_library(translator_planar_graph STATIC
  PlanarGraphLexer.cpp
  PlanarGraphParser.cpp
  PlanarGraph.cpp
)
