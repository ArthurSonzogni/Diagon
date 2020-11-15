add_library(translator_frame STATIC
  Frame.cpp
)
target_link_libraries(translator_frame PRIVATE diagon_base)
target_set_common(translator_frame)
