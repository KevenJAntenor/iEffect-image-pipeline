cmake_minimum_required(VERSION 3.8)

project(inf3173-232-tp3 LANGUAGES C CXX VERSION 1)

# faire un build dans les sources, c'est MAL
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PreventInSourceBuilds)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BENCHMARK_ENABLE_TESTING FALSE)
add_subdirectory(3rdparty/googletest)
#add_subdirectory(3rdparty/benchmark)

option(ENABLE_TESTS "Enable tests" ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

option(ENABLE_THREAD_SANITIZER "Compile with -fsanitize=thread" OFF)
if(ENABLE_THREAD_SANITIZER)
  add_compile_options(-fsanitize=thread)
  add_link_options(-fsanitize=thread)
endif()

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/inf3173/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
configure_file(env.sh.in env.sh)

add_subdirectory(src)

if (ENABLE_TESTS)
  enable_testing()
  include(CTest)
  set(CTEST_OUTPUT_ON_FAILURE TRUE)
  add_subdirectory(test)
endif()

configure_file(env.sh.in env.sh)

# Package
string(TIMESTAMP TS "%Y%m%d%H%M%S")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${TS})
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES
	"/build.*/;/.git/;~$;CMakeLists.txt.user;/data/.*\\\\.png;/results/;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)


