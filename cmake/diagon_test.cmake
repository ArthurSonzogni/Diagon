add_executable(input_output_test src/input_output_test.cpp)
target_link_libraries(input_output_test diagon_lib)
target_set_common(input_output_test)

option(DIAGON_ASAN "Set to ON to enable address sanitizer" OFF)
option(DIAGON_LSAN "Set to ON to enable leak sanitizer" OFF)
option(DIAGON_MSAN "Set to ON to enable memory sanitizer" OFF)
option(DIAGON_TSAN "Set to ON to enable thread sanitizer" OFF)
option(DIAGON_UBSAN "Set to ON to enable undefined behavior sanitizer" OFF)
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  return()
endif()

# Add ASAN
if (DIAGON_ASAN)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()

# Add LSAN
if (DIAGON_LSAN)
  add_compile_options(-fsanitize=leak)
  add_link_options(-fsanitize=leak)
endif()

# Add MSAN
if (DIAGON_MSAN)
  add_compile_options(-fsanitize=memory)
  add_link_options(-fsanitize=memory)
endif()

# Add TSAN
if (DIAGON_TSAN)
  add_compile_options(-fsanitize=thread)
  add_link_options(-fsanitize=thread)
endif()

# Add UBSAN
if (DIAGON_UBSAN)
  add_compile_options(-fsanitize=undefined)
  add_link_options(-fsanitize=undefined)
endif()
