ANTLR(PlanarGraph.g4)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME ON)
#find_package(Boost)
#include_directories(${Boost_INCLUDE_DIRS}/boost/graph)
#include_directories(${Boost_INCLUDE_DIRS})
#message(STATUS ${Boost_INCLUDE_DIRS})


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(translator_planar_graph STATIC
  PlanarGraphLexer.cpp
  PlanarGraphParser.cpp
  PlanarGraph.cpp
)
