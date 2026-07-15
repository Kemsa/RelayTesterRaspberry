cmake_minimum_required(VERSION 3.16)

project(relayTesterUnitTests VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

include(CTest)
enable_testing()

find_package(Qt5 REQUIRED COMPONENTS Core Test)

# In standalone mode, avoid pulling dependency test suites by default.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(BUILD_TESTING OFF CACHE BOOL "Disable nested tests for dependencies" FORCE)
endif()

if(NOT TARGET wiringpi_wrapper)
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../libs/Wiringpi_Wrapper
    ${CMAKE_CURRENT_BINARY_DIR}/Wiringpi_Wrapper-build
  )
endif()

add_subdirectory(tests)
