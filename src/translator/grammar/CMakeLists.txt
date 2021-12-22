add_library(translator_grammar STATIC Grammar.cpp)
set_property(TARGET translator_grammar PROPERTY CXX_STANDARD 17)
target_link_libraries(translator_grammar PRIVATE diagon_base)
target_set_common(translator_grammar)

if (MSVC)
else()
  include(FetchContent)
  set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
  FetchContent_Declare(kgt
    GIT_REPOSITORY https://github.com/ArthurSonzogni/kgt
    GIT_TAG c2af649438583e0ec5a97b83b5be6f1824b21a2a
    )
  FetchContent_GetProperties(kgt)
  FetchContent_Populate(kgt)
  add_subdirectory(${kgt_SOURCE_DIR} ${kgt_BINARY_DIR} EXCLUDE_FROM_ALL)
  target_link_libraries(translator_grammar PRIVATE kgt::kgt)
endif()
