# Include source files and the generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (Web)

  set(main "main.js")

  # Emscripten options
  string(APPEND CMAKE_CXX_FLAGS " -Os")
  string(APPEND CMAKE_CXX_FLAGS " -s WASM=1")

  string(APPEND CMAKE_CXX_FLAGS " --js-opts 3")
  string(APPEND CMAKE_CXX_FLAGS " --llvm-lto 3")
  string(APPEND CMAKE_CXX_FLAGS " --llvm-opts 3")
  string(APPEND CMAKE_CXX_FLAGS " -s ASSERTIONS=0")
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

else()

  set(main "main")

endif()

add_executable(${main}
  main.cpp
)

# Sequence
add_subdirectory(sequence)
target_link_libraries(${main} sequence)
target_link_libraries(${main} antlr4_static)

# ANTLR
target_link_libraries(${main} ${ANTLR_LIBRARIES})
