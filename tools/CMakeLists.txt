# ┌─────────────────────────────────────────────────┐
# │ ANTLR                                           │
# └─────────────────────────────────────────────────┘
if(NOT EXISTS "${CMAKE_BINARY_DIR}/antlr.jar")
  execute_process(
    COMMAND curl http://www.antlr.org/download/antlr-4.11.1-complete.jar -L -o antlr.jar
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
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
    -jar ${CMAKE_BINARY_DIR}/antlr.jar
    -Dlanguage=Cpp
    -no-listener
    -no-visitor
    -o ${source_gen_dir}
    ${source_src_dir}/${source_filename}.g4
    MAIN_DEPENDENCY
    ${source_src_dir}/${source_filename}.g4
    )
endfunction()
