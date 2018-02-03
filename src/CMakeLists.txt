# Include source files and the generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#-------------------------------------------------------------------------------
# Environment variable available in C++ source
#-------------------------------------------------------------------------------

set(testing_directory ${CMAKE_SOURCE_DIR}/testing)

execute_process(
  COMMAND 
    git rev-list --count HEAD
  WORKING_DIRECTORY
    ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE
    git_version
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/environment.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/environment.h
)

#-------------------------------------------------------------------------------

add_subdirectory(translator/example)
add_subdirectory(translator/sequence)
add_subdirectory(screen)

if (Web)

  # Emscripten options
  string(APPEND CMAKE_CXX_FLAGS " -Os")
  string(APPEND CMAKE_CXX_FLAGS " -Os")
  string(APPEND CMAKE_CXX_FLAGS " -s WASM=1")

  string(APPEND CMAKE_CXX_FLAGS " --js-opts 3")
  string(APPEND CMAKE_CXX_FLAGS " --llvm-lto 3")
  string(APPEND CMAKE_CXX_FLAGS " --llvm-opts 3")
  string(APPEND CMAKE_CXX_FLAGS " -s ASSERTIONS=0")
  string(APPEND CMAKE_CXX_FLAGS " --closure 1")
  string(APPEND CMAKE_CXX_FLAGS " --no-heap-copy")
  string(APPEND CMAKE_CXX_FLAGS " -s EXPORTED_FUNCTIONS='[\"_translate\"]'")
  string(APPEND CMAKE_CXX_FLAGS " -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\" cwrap\"]'")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/index.html
    ${CMAKE_CURRENT_BINARY_DIR}/index.html
    COPYONLY)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/style.css
    ${CMAKE_CURRENT_BINARY_DIR}/style.css
    COPYONLY)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/webserver/run_web_server.sh
    ${CMAKE_CURRENT_BINARY_DIR}/run_web_server.sh
    COPYONLY)

  add_executable(main.js main_webassembly.cpp)
  target_link_libraries(main.js
    translator_example
    translator_sequence
    screen
    antlr4_static
  )

else()

  add_executable(main main.cpp)
  target_link_libraries(main
    translator_example
    translator_sequence
    screen
    antlr4_static
  )

  add_executable(input_output_test input_output_test.cpp)
  target_link_libraries(input_output_test
    translator_example
    translator_sequence
    screen
    antlr4_static
    stdc++fs
  )

endif()
