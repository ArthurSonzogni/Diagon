
# Include source files and the generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ANTLR(INPUT MyLexer.g4)
ANTLR(INPUT MyParser.g4 DEPENDENCIES MyLexer.cpp)

add_executable(main
  main.cpp
  MyLexer.cpp
  MyParser.cpp
  analysis.cpp
)
target_link_libraries(main ${ANTLR_LIBRARIES})
