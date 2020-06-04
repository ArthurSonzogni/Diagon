cmake_minimum_required (VERSION 2.9)
project(Diagon)

# ┌─────────────────────────────────────────────────┐
# │ Print Information about the compilation system  │
# └─────────────────────────────────────────────────┘
if(CMAKE_CXX_COMPILER MATCHES "/em\\+\\+(-[a-zA-Z0-9.])?$")
  set(Web true)
else()
  set(Web false)
endif()
message("┌── Diagon  configuration ───────────────────────")
message("│ - Build type : " ${CMAKE_BUILD_TYPE})
message("│ - C compiler: ${CMAKE_C_COMPILER}")
message("│ - C++ compiler: ${CMAKE_CXX_COMPILER}")
message("│ - System: " ${CMAKE_SYSTEM_NAME} )
message("│ - Use Web : " ${Web})
message("└────────────────────────────────────────────────")

# ┌─────────────────────────────────────────────────┐
# │ ANTLR                                           │
# └─────────────────────────────────────────────────┘
execute_process(
  COMMAND bash download_and_patch.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/antlr
)

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

function(target_set_common target)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
  if (Web)
    # Nothing
  elseif (MSVC)
    target_compile_options(${target} PRIVATE "/wd4244")
    target_compile_options(${target} PRIVATE "/wd4267")
    target_compile_options(${target} PRIVATE "/wd4996")
    target_compile_options(${target} PRIVATE "/wd4305")
  else()
    target_compile_options(${target} PRIVATE "-Wall")
    #target_compile_options(${target} PRIVATE "-Werror")
    target_compile_options(${target} PRIVATE "-Wno-sign-compare")
    target_compile_options(${target} PRIVATE "-Wno-attributes")
  endif()
endfunction()

# Build the ANTLR runtime.
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(
  ${CMAKE_SOURCE_DIR}/tools/antlr/cpp_runtime/
  ${CMAKE_CURRENT_BINARY_DIR}/antlr
  EXCLUDE_FROM_ALL
)
set_property(TARGET antlr4_static PROPERTY CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR}/tools/antlr/cpp_runtime/runtime/src)

#-------------------------------------------------------------------------------

add_subdirectory(src)
