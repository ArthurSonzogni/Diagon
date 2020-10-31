include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(kgt
  GIT_REPOSITORY https://github.com/ArthurSonzogni/kgt
  GIT_TAG origin/main
  #GIT_TAG e79328f7c1ef61f66a841672ef7a2abf6404f231
)
FetchContent_GetProperties(kgt)
FetchContent_Populate(kgt)
add_subdirectory(${kgt_SOURCE_DIR} ${kgt_BINARY_DIR} EXCLUDE_FROM_ALL)

add_library(translator_grammar STATIC
  Grammar.hpp
  Grammar.cpp
)
set_property(TARGET translator_grammar PROPERTY CXX_STANDARD 17)

target_link_libraries(translator_grammar PUBLIC kgt::kgt)
