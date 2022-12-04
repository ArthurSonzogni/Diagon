find_package(Boost COMPONENTS graph)

if (NOT APPLE AND (Boost_FOUND OR EMSCRIPTEN))
  ANTLR(GraphPlanar.g4)
  add_library(translator_graph_planar STATIC
    GraphPlanarLexer.cpp
    GraphPlanarParser.cpp
    GraphPlanar.cpp
  )

  target_link_libraries(translator_graph_planar
    PRIVATE diagon_base
    PRIVATE antlr4_static
  )

  if (Boost_FOUND)
    target_link_libraries(translator_graph_planar PRIVATE Boost::graph)
  else()
    message(WARNING "GraphPlanar: Boost::graph not found")
  endif()
else()
  add_library(translator_graph_planar STATIC
    GraphPlanarEmpty.cpp
  )
endif()

if (NOT MSVC)
  target_compile_options(translator_graph_planar PRIVATE "-Wno-attributes")
endif()

target_set_common(translator_graph_planar)
