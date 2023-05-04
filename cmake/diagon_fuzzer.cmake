set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

add_executable(fuzzer src/fuzzer.cpp)
target_compile_options(fuzzer PRIVATE -fsanitize=fuzzer)
target_link_libraries(fuzzer PRIVATE -fsanitize=fuzzer)
target_link_libraries(fuzzer PRIVATE diagon_lib)
target_set_common(fuzzer)
