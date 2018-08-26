cmake_minimum_required(VERSION 2.8)
project(SerializationBenchmark)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
find_package(Boost REQUIRED serialization)

add_library(rfcbase64 base64.c)
add_library(base64_impl impl.cxx)
target_link_libraries(base64_impl PRIVATE rfcbase64)

include(CTest)
enable_testing()

add_executable(benchmark benchmark.cxx)
target_link_libraries(benchmark 
  PRIVATE 
  Boost::serialization 
  base64_impl 
  rfcbase64)

add_executable(unit-tests main.cxx)
target_link_libraries(unit-tests
  PRIVATE
  rfcbase64
  ${GMOCK_MAIN_LIBRARIES}
  Threads::Threads
  Boost::serialization
  base64_impl
  )
gtest_add_tests(unit-tests "" AUTO)
