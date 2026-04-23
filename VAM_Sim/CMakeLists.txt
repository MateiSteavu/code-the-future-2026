cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(VAM_SIM LANGUAGES CXX)
add_executable(${CMAKE_PROJECT_NAME})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE main.cpp)

#Libraries
add_library(geometry
  ${PROJECT_SOURCE_DIR}/src/Geometry/Ray.cpp
  ${PROJECT_SOURCE_DIR}/src/Geometry/Vector3.cpp
  ${PROJECT_SOURCE_DIR}/src/Geometry/Cone.cpp
  ${PROJECT_SOURCE_DIR}/src/Geometry/Voxel.cpp
)
target_include_directories(geometry PUBLIC 
  ${PROJECT_SOURCE_DIR}/inc/Geometry
)

add_library(custom_math
  ${PROJECT_SOURCE_DIR}/src/Custom_Math.cpp
)
target_include_directories(geometry PUBLIC 
  ${PROJECT_SOURCE_DIR}/inc/
)




# unit tests cu Google Test
enable_testing()

# descarcare google test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#Ray test Code
add_executable(
  Ray_test
  ${PROJECT_SOURCE_DIR}/Tests/Ray_test.cpp
)
target_link_libraries(
  Ray_test PRIVATE geometry 
  GTest::gtest_main
)

#Vector3_test.cpp
add_executable(
  Vector3_test
  ${PROJECT_SOURCE_DIR}/Tests/Vector3_test.cpp
)
target_link_libraries(
  Vector3_test PRIVATE geometry 
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(Ray_test)
gtest_discover_tests(Vector3_test)