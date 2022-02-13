ANTLR(GraphPlanar.g4)

find_package(Boost COMPONENTS graph)

add_library(translator_graph_planar STATIC
  GraphPlanarLexer.cpp
  GraphPlanarParser.cpp
  GraphPlanar.cpp
)
if (MSVC)
else()
  target_compile_options(translator_graph_planar PRIVATE "-Wno-attributes")
endif()

target_link_libraries(translator_graph_planar
  PRIVATE diagon_base
  PRIVATE antlr4_static
  PRIVATE Boost::graph)
target_set_common(translator_graph_planar)
