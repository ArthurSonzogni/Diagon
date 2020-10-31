# Include source files and the generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#-------------------------------------------------------------------------------
# Environment variable available in C++ source
#-------------------------------------------------------------------------------

set(test_directory ${CMAKE_SOURCE_DIR}/test) 
execute_process(
  COMMAND 
    git rev-list --count HEAD
  WORKING_DIRECTORY
    ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE
    git_version
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/environment.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/environment.h
)

#-------------------------------------------------------------------------------

add_subdirectory(screen)
add_subdirectory(translator/frame)
add_subdirectory(translator/grammar)
add_subdirectory(translator/math)
add_subdirectory(translator/planar_graph)
add_subdirectory(translator/sequence)
add_subdirectory(translator/table)
add_subdirectory(translator/tree)

if (Web)

  # Emscripten options
  string(APPEND CMAKE_CXX_FLAGS " -Os")
  string(APPEND CMAKE_CXX_FLAGS " -s WASM=1")

  string(APPEND CMAKE_CXX_FLAGS " -s DEMANGLE_SUPPORT=1")
  string(APPEND CMAKE_CXX_FLAGS " --js-opts 3")
  string(APPEND CMAKE_CXX_FLAGS " --llvm-lto 3")
  string(APPEND CMAKE_CXX_FLAGS " --llvm-opts 3")
  string(APPEND CMAKE_CXX_FLAGS " -s ASSERTIONS=2")
  string(APPEND CMAKE_CXX_FLAGS " --closure 1")
  string(APPEND CMAKE_CXX_FLAGS " --no-heap-copy")
  string(APPEND CMAKE_CXX_FLAGS " -s EXPORTED_FUNCTIONS='[\"_translate\", \"_main\"]'")
  string(APPEND CMAKE_CXX_FLAGS " -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'")

  option(ADD_GOOGLE_ANALYTICS "Build the static library" ON)
  if (ADD_GOOGLE_ANALYTICS)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/http/google-analytics.html google-analytics)
  endif(ADD_GOOGLE_ANALYTICS)
  
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/http/style.css
    ${CMAKE_CURRENT_BINARY_DIR}/style.css
  )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/http/index.html
    ${CMAKE_CURRENT_BINARY_DIR}/index.html
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/http/tools.json
    ${CMAKE_CURRENT_BINARY_DIR}/tools.json
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/webserver/run_web_server.sh
    ${CMAKE_CURRENT_BINARY_DIR}/run_web_server.sh
    COPYONLY)

  add_executable(main
    main_webassembly.cpp
    translator/Translator.cpp
    translator/Translator.h
    translator/Factory.h
  )
  target_set_common(main)
  target_link_libraries(main PUBLIC translator_frame)
  target_link_libraries(main PUBLIC translator_grammar)
  target_link_libraries(main PUBLIC translator_math)
  target_link_libraries(main PUBLIC translator_planar_graph)
  target_link_libraries(main PUBLIC translator_sequence)
  target_link_libraries(main PUBLIC translator_table)
  target_link_libraries(main PUBLIC translator_tree)
  target_link_libraries(main PUBLIC screen)
  target_link_libraries(main PUBLIC antlr4_static)
  target_link_libraries(main PUBLIC kgt::abnf)

else()

  add_executable(diagon
    main.cpp
    translator/Translator.cpp
    translator/Translator.h
    translator/Factory.h
  )
  target_set_common(diagon)
  target_link_libraries(diagon PUBLIC translator_frame)
  target_link_libraries(diagon PUBLIC translator_grammar)
  target_link_libraries(diagon PUBLIC translator_math)
  target_link_libraries(diagon PUBLIC translator_planar_graph)
  target_link_libraries(diagon PUBLIC translator_sequence)
  target_link_libraries(diagon PUBLIC translator_table)
  target_link_libraries(diagon PUBLIC translator_tree)
  target_link_libraries(diagon PUBLIC screen)
  target_link_libraries(diagon PUBLIC antlr4_static)
  target_link_libraries(diagon PUBLIC -static-libstdc++)
  install(TARGETS diagon RUNTIME DESTINATION "bin")

  add_executable(input_output_test
    translator/Translator.cpp
    input_output_test.cpp
  )
  target_set_common(input_output_test)
  target_link_libraries(input_output_test PUBLIC translator_frame)
  target_link_libraries(input_output_test PUBLIC translator_grammar)
  target_link_libraries(input_output_test PUBLIC translator_math)
  target_link_libraries(input_output_test PUBLIC translator_planar_graph)
  target_link_libraries(input_output_test PUBLIC translator_sequence)
  target_link_libraries(input_output_test PUBLIC translator_table)
  target_link_libraries(input_output_test PUBLIC translator_tree)
  target_link_libraries(input_output_test PUBLIC antlr4_static)
  target_link_libraries(input_output_test PUBLIC screen)
  target_link_libraries(input_output_test PUBLIC stdc++fs)

endif()
