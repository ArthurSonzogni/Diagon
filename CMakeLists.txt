cmake_minimum_required (VERSION 3.15)

set(git_version 156) # Obtained with `git rev-list --count HEAD`
project(Diagon
  LANGUAGES C CXX
  VERSION 1.1.${git_version}
)

option(DIAGON_BUILD_TESTS "Set to ON to build tests" OFF)
option(DIAGON_BUILD_TESTS_FUZZER "Set to ON to enable fuzzing" OFF)
option(DIAGON_ASAN "Set to ON to enable address sanitizer" OFF)
option(DIAGON_LSAN "Set to ON to enable leak sanitizer" OFF)
option(DIAGON_MSAN "Set to ON to enable memory sanitizer" OFF)
option(DIAGON_TSAN "Set to ON to enable thread sanitizer" OFF)
option(DIAGON_UBSAN "Set to ON to enable undefined behavior sanitizer" OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#-------------------------------------------------------------------------------

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1
  GIT_PROGRESS TRUE
  EXCLUDE_FROM_ALL TRUE
)

FetchContent_MakeAvailable(json)

#-------------------------------------------------------------------------------

FetchContent_Declare(antlr
  #GIT_REPOSITORY https://github.com/antlr/antlr4
  GIT_REPOSITORY https://github.com/ArthurSonzogni/antlr4
  # GIT_TAG 1cb4669f84cea5b59661fd44b0f80509fdacd3f9
  GIT_TAG remove-pthread
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  EXCLUDE_FROM_ALL TRUE
)

FetchContent_GetProperties(antlr)
if(NOT antlr_POPULATED)
  FetchContent_Populate(antlr)
  SET(WITH_LIBCXX OFF CACHE BOOL "")
  SET(CMAKE_HOME_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_subdirectory(
    ${antlr_SOURCE_DIR}/runtime/Cpp
    ${antlr_BINARY_DIR}
    EXCLUDE_FROM_ALL)
  target_include_directories(antlr4_static
    PUBLIC ${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src
  )
endif()

#-------------------------------------------------------------------------------

if (MSVC)
  add_compile_options("/utf-8")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tools)

function(target_set_common target)
  set_target_properties(${target} PROPERTIES
    VERSION ${PROJECT_VERSION}
    CXX_STANDARD 17
  )
  if (EMSCRIPTEN)
    target_compile_options(${target} PRIVATE "-fwasm-exceptions")
    target_link_options(${target} PRIVATE "-fwasm-exceptions")
    # Nothing
  elseif (MSVC)
    set_property(TARGET ${target} PROPERTY
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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

#-------------------------------------------------------------------------------
# Environment variable available in C++ source
#-------------------------------------------------------------------------------

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/environment.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/environment.h
)

#-------------------------------------------------------------------------------

add_library(diagon_base STATIC
  src/translator/Translator.cpp
  src/translator/Translator.h
  src/translator/antlr_error_listener.cpp
  src/translator/antlr_error_listener.h
)
target_link_libraries(diagon_base
  PRIVATE antlr4_static
)
target_set_common(diagon_base)

set(Boost_USE_STATIC_LIBS   ON)
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
  src/api.cpp
  src/api.hpp
  src/translator/Factory.cpp
  src/translator/Factory.h
)
target_set_common(diagon_lib)

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
  PRIVATE nlohmann_json::nlohmann_json
)
target_set_common(diagon_lib)

add_executable(diagon src/main.cpp)
target_link_libraries(diagon PRIVATE diagon_lib)
target_set_common(diagon)

if (EMSCRIPTEN)
  # Emscripten options
  string(APPEND CMAKE_CXX_FLAGS " -Os")
  string(APPEND CMAKE_CXX_FLAGS " -s WASM=1")
  string(APPEND CMAKE_CXX_FLAGS " --closure 1")
  string(APPEND CMAKE_CXX_FLAGS " -fexceptions")
  string(APPEND CMAKE_CXX_FLAGS " -sUSE_BOOST_HEADERS")
  #string(APPEND CMAKE_CXX_FLAGS " -s STANDALONE_WASM")
  #string(APPEND CMAKE_CXX_FLAGS " -s WASM_ASYNC_COMPILATION=0")
  #string(APPEND CMAKE_CXX_FLAGS " -s SIDE_MODULE=1")
  #string(APPEND CMAKE_CXX_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
  string(APPEND CMAKE_CXX_FLAGS " -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'")

  option(ADD_GOOGLE_ANALYTICS "Build the static library" ON)
  if (ADD_GOOGLE_ANALYTICS)
    file(READ
      ${CMAKE_CURRENT_SOURCE_DIR}/src/google-analytics.html
      google-analytics)
  endif(ADD_GOOGLE_ANALYTICS)
  
  foreach(file
      "style.css"
      "index.html"
      "run_diagon.sh")
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/src/${file}
      ${CMAKE_CURRENT_BINARY_DIR}/${file}
    )
  endforeach(file)

  #set_target_properties(diagon PROPERTIES SUFFIX ".wasm")

else()
  install(TARGETS diagon RUNTIME DESTINATION "bin")
endif()

if (UNIX AND NOT APPLE)
  set(CPACK_GENERATOR "DEB;External;RPM;STGZ;TBZ2;TGZ;TXZ;TZ;TZST;ZIP")
elseif (UNIX AND APPLE)
  set(CPACK_GENERATOR "DragNDrop;NuGet;TGZ;ZIP")
elseif (WIN32)
  set(CPACK_GENERATOR "DEB;NuGet;TGZ;ZIP")
else()
  set(CPACK_GENERATOR "ZIP")
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Generate beautiful ASCII art diagrams. Diagon is a collection of interactive generators. Math formulas, sequences diagrams, graph, border, ... This is written in C++, available online with WebAssembly.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://arthursonzogni.com/Diagon/")
set(CPACK_PACKAGE_NAME "diagon")
set(CPACK_PACKAGE_VENDOR "Arthur Sonzogni")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_DEBIAN_PACKAGE_DEPENDS " ")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://arthursonzogni.com/Diagon/")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE_URL "https://github.com/ArthurSonzogni/FTXUI/")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Arthur Sonzogni")
set(CPACK_DEBIAN_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_RPM_PACKAGE_LICENSE MIT)

include(CPack)

if (DIAGON_BUILD_TESTS)
  include(cmake/diagon_test.cmake)
endif()

if (DIAGON_BUILD_TESTS_FUZZER)
  include(cmake/diagon_fuzzer.cmake)
endif()
