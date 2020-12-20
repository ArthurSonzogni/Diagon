cmake_minimum_required (VERSION 3.11)
project(Diagon LANGUAGES C CXX)

include(FetchContent)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tools)

function(target_set_common target)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
  if (EMSCRIPTEN)
    # Nothing
  elseif (MSVC)
    target_compile_options(${target} PRIVATE "/wd4244")
    target_compile_options(${target} PRIVATE "/wd4267")
    target_compile_options(${target} PRIVATE "/wd4996")
    target_compile_options(${target} PRIVATE "/wd4305")
  else()
    target_compile_options(${target} PRIVATE "-Wall")
    target_compile_options(${target} PRIVATE "-Wno-sign-compare")
    target_compile_options(${target} PRIVATE "-Wno-attributes")
  endif()
endfunction()

set(test_directory ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Include source files and the generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(test_directory ${CMAKE_CURRENT_SOURCE_DIR}/test)

#-------------------------------------------------------------------------------
# Environment variable available in C++ source
#-------------------------------------------------------------------------------

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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/environment.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/environment.h
)

#-------------------------------------------------------------------------------

add_library(diagon_base STATIC
  src/translator/Translator.cpp
  src/translator/Translator.h
)

add_subdirectory(src/screen)
add_subdirectory(src/translator/frame)
add_subdirectory(src/translator/grammar)
add_subdirectory(src/translator/graph_dag)
add_subdirectory(src/translator/graph_planar)
add_subdirectory(src/translator/math)
add_subdirectory(src/translator/flowchart)
add_subdirectory(src/translator/sequence)
add_subdirectory(src/translator/table)
add_subdirectory(src/translator/tree)

add_library(diagon_lib STATIC
  src/translator/Factory.cpp
  src/translator/Factory.h
)

target_link_libraries(diagon_lib
  PRIVATE diagon_base
  PRIVATE translator_frame
  PRIVATE translator_grammar
  PRIVATE translator_graph_dag
  PRIVATE translator_graph_planar
  PRIVATE translator_math
  PRIVATE translator_flowchart
  PRIVATE translator_sequence
  PRIVATE translator_table
  PRIVATE translator_tree
  PRIVATE antlr4_static
  PRIVATE screen
)
target_set_common(diagon_lib)

if (EMSCRIPTEN)
  # Emscripten options
  string(APPEND CMAKE_CXX_FLAGS " -Os")
  string(APPEND CMAKE_CXX_FLAGS " -s WASM=1")
  string(APPEND CMAKE_CXX_FLAGS " --closure 1")
  string(APPEND CMAKE_CXX_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
  string(APPEND CMAKE_CXX_FLAGS " -s EXPORTED_FUNCTIONS='[\"_translate\", \"_main\", \"_API\"]'")
  string(APPEND CMAKE_CXX_FLAGS " -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'")

  option(ADD_GOOGLE_ANALYTICS "Build the static library" ON)
  if (ADD_GOOGLE_ANALYTICS)
    file(READ
      ${CMAKE_CURRENT_SOURCE_DIR}/src/http/google-analytics.html
      google-analytics)
  endif(ADD_GOOGLE_ANALYTICS)
  
  foreach(file
      "style.css"
      "index.html"
      "run_web_server.sh")
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/src/http/${file}
      ${CMAKE_CURRENT_BINARY_DIR}/${file}
    )
  endforeach(file)

  add_executable(diagon src/main_webassembly.cpp)
  target_link_libraries(diagon PRIVATE diagon_lib)
  target_set_common(diagon)

  FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.9.1)

  FetchContent_GetProperties(json)
  if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()

  target_link_libraries(diagon PRIVATE nlohmann_json::nlohmann_json)

else()
  target_link_libraries(diagon_lib PRIVATE -static-libstdc++)
  target_link_libraries(diagon_lib PRIVATE stdc++fs)

  add_executable(diagon src/main.cpp)
  target_link_libraries(diagon PRIVATE diagon_lib)
  install(TARGETS diagon RUNTIME DESTINATION "bin")
  target_set_common(diagon)

  add_executable(input_output_test src/input_output_test.cpp)
  target_link_libraries(input_output_test diagon_lib)
  target_set_common(input_output_test)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "diagon")
set(CPACK_PACKAGE_VENDOR "Arthur Sonzogni")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Arthur Sonzogni")
set(CPACK_PACKAGE_VERSION_MAJOR ${git_version})
set(CPACK_PACKAGE_VERSION_MINOR "")
set(CPACK_PACKAGE_VERSION_PATCH "")
set(CPACK_DEBIAN_PACKAGE_VERSION ${git_version})
set(CPACK_PACKAGE_HOMEPAGE_URL "https://arthursonzogni.com/Diagon/")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "
  Generate beautiful ASCII art diagrams.
  Diagon is a collection of interactive generators. Math formulas, sequences diagrams, graph,
  border, ... This is written in C++, available online with WebAssembly.
")
set(CPACK_DEBIAN_PACKAGE_DEPENDS " ")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://arthursonzogni.com/Diagon/")

include(CPack)
