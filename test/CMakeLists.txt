enable_testing()

add_executable(test_list
  test_list.c
  simpletest.c
)
target_link_libraries(test_list PRIVATE core)
add_test(NAME test_list COMMAND test_list)

add_executable(test_pipeline
  test_pipeline.cpp
)
target_compile_options(test_pipeline PRIVATE -g)
target_link_libraries(test_pipeline PRIVATE core GTest::gtest_main)
add_test(NAME test_pipeline COMMAND test_pipeline)
set_tests_properties(test_pipeline PROPERTIES TIMEOUT 10)

