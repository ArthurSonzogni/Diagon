# Include source files and the generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#-------------------------------------------------------------------------------
# Environment variable available in C++ source
#-------------------------------------------------------------------------------

set(test_directory ${CMAKE_SOURCE_DIR}/test) 
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

add_library(diagon_base STATIC
  translator/Translator.cpp
  translator/Translator.h
)

add_subdirectory(screen)
add_subdirectory(translator/directed_acyclic_graph)
add_subdirectory(translator/frame)
add_subdirectory(translator/grammar)
add_subdirectory(translator/math)
add_subdirectory(translator/planar_graph)
add_subdirectory(translator/sequence)
add_subdirectory(translator/table)
add_subdirectory(translator/tree)

add_library(diagon_lib STATIC
  translator/Factory.cpp
  translator/Factory.h
)

target_link_libraries(diagon_lib PRIVATE diagon_base)
target_link_libraries(diagon_lib PRIVATE translator_directed_acyclic_graph)
target_link_libraries(diagon_lib PRIVATE translator_frame)
target_link_libraries(diagon_lib PRIVATE translator_grammar)
target_link_libraries(diagon_lib PRIVATE translator_math)
target_link_libraries(diagon_lib PRIVATE translator_planar_graph)
target_link_libraries(diagon_lib PRIVATE translator_sequence)
target_link_libraries(diagon_lib PRIVATE translator_table)
target_link_libraries(diagon_lib PRIVATE translator_tree)
target_link_libraries(diagon_lib PRIVATE antlr4_static)
target_link_libraries(diagon_lib PRIVATE screen)
target_set_common(diagon_lib)

if (Web)
  # Emscripten options
  string(APPEND CMAKE_CXX_FLAGS " -Os")
  string(APPEND CMAKE_CXX_FLAGS " -s WASM=1")
  string(APPEND CMAKE_CXX_FLAGS " --closure 1")
  string(APPEND CMAKE_CXX_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
  string(APPEND CMAKE_CXX_FLAGS " -s EXPORTED_FUNCTIONS='[\"_translate\", \"_main\", \"_API\"]'")
  string(APPEND CMAKE_CXX_FLAGS " -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'")

  option(ADD_GOOGLE_ANALYTICS "Build the static library" ON)
  if (ADD_GOOGLE_ANALYTICS)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/http/google-analytics.html google-analytics)
  endif(ADD_GOOGLE_ANALYTICS)
  
  foreach(file
      "style.css"
      "index.html"
      "run_web_server.sh")
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/http/${file}
      ${CMAKE_CURRENT_BINARY_DIR}/${file}
    )
  endforeach(file)

  add_executable(diagon main_webassembly.cpp)
  target_link_libraries(diagon PRIVATE diagon_lib)
  target_set_common(diagon)

  include(FetchContent)

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

  add_executable(diagon main.cpp)
  target_link_libraries(diagon PRIVATE diagon_lib)
  install(TARGETS diagon RUNTIME DESTINATION "bin")
  target_set_common(diagon)

  add_executable(input_output_test input_output_test.cpp)
  target_link_libraries(input_output_test diagon_lib)
  target_set_common(input_output_test)
endif()
