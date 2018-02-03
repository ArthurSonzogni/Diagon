cmake_minimum_required (VERSION 2.9)
project(Diagon)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
# ┌─────────────────────────────────────────────────┐
# │ Print Information about the compilation system  │
# └─────────────────────────────────────────────────┘
if(CMAKE_CXX_COMPILER MATCHES "/em\\+\\+(-[a-zA-Z0-9.])?$")
  set(Web true)
else()
  set(Web false)
endif()
message(" * Build type : " ${CMAKE_BUILD_TYPE})
message(" * C compiler: ${CMAKE_C_COMPILER}")
message(" * C++ compiler: ${CMAKE_CXX_COMPILER}")
message(" * System: " ${CMAKE_SYSTEM_NAME} )
message(" * Use Web : " ${Web})

# ┌─────────────────────────────────────────────────┐
# │ ANTLR                                           │
# └─────────────────────────────────────────────────┘

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
      -jar ${CMAKE_SOURCE_DIR}/tools/antlr/antlr.jar
      -Dlanguage=Cpp
      -no-listener
      -no-visitor
      -o ${source_gen_dir}
      ${source_src_dir}/${source_filename}.g4
    MAIN_DEPENDENCY
      ${source_src_dir}/${source_filename}.g4
  )
endfunction()

# Build the ANTLR runtime.
add_subdirectory(${CMAKE_SOURCE_DIR}/tools/antlr/cpp_runtime/ EXCLUDE_FROM_ALL)
include_directories(${CMAKE_SOURCE_DIR}/tools/antlr/cpp_runtime/runtime/src)

#-------------------------------------------------------------------------------

add_subdirectory(src)
