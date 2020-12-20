set(DIAGON_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")

# ┌─────────────────────────────────────────────────┐
# │ ANTLR                                           │
# └─────────────────────────────────────────────────┘
execute_process(
  COMMAND bash download_and_patch.sh
  WORKING_DIRECTORY ${DIAGON_TOOLS_DIR}
)

FetchContent_Declare(antlr
  GIT_REPOSITORY https://github.com/antlr/antlr4
  GIT_TAG 4.8)

FetchContent_GetProperties(antlr)
if(NOT json_POPULATED)
  FetchContent_Populate(antlr)
  SET(WITH_LIBCXX OFF CACHE BOOL "")
  add_subdirectory(
    ${antlr_SOURCE_DIR}/runtime/Cpp
    ${antlr_BINARY_DIR}
    EXCLUDE_FROM_ALL)
  target_include_directories(antlr4_static
    PUBLIC ${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src
  )
endif()

# Function
#   ANTLR(<file.g4>)
#
# Description:
#   Take an ANTLR file and produce a CMake rule to generate the corresponding
#   C++ files.
#
# Notes:
#   The ANTLR file path must be relative to ${CMAKE_CURRENT_SOURCE_DIR}
#
# Example:
#   ANTLR(Grammar.g4)
function(ANTLR source)
  get_filename_component(source_filename ${CMAKE_CURRENT_SOURCE_DIR}/${source} NAME_WE)
  get_filename_component(source_src_dir  ${CMAKE_CURRENT_SOURCE_DIR}/${source} DIRECTORY)
  get_filename_component(source_gen_dir  ${CMAKE_CURRENT_BINARY_DIR}/${source} DIRECTORY)
  add_custom_command(
    OUTPUT
      ${source_gen_dir}/${source_filename}Lexer.cpp
      ${source_gen_dir}/${source_filename}Parser.cpp
      ${source_gen_dir}/${source_filename}Lexer.h
      ${source_gen_dir}/${source_filename}Parser.h
    COMMAND
      java
    ARGS
      -jar ${DIAGON_TOOLS_DIR}/antlr.jar
      -Dlanguage=Cpp
      -no-listener
      -no-visitor
      -o ${source_gen_dir}
      ${source_src_dir}/${source_filename}.g4
    MAIN_DEPENDENCY
      ${source_src_dir}/${source_filename}.g4
  )
endfunction()

