ANTLR(PlanarGraph.g4)

add_library(translator_planar_graph STATIC
  PlanarGraphLexer.cpp
  PlanarGraphParser.cpp
  PlanarGraph.cpp
)
if (MSVC)
else()
  target_compile_options(translator_planar_graph PRIVATE "-Wno-attributes")
endif()
target_link_libraries(translator_planar_graph PRIVATE diagon_base)
