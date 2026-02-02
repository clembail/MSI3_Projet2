cmake_minimum_required(VERSION 3.15)

set(VERSION PoissonMetal)
project(${VERSION})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (${CMAKE_BINARY_DIR} STREQUAL "Debug")
  add_compile_options(-DDEBUG)
endif()

include_directories(
  common
  metal
  metal-cpp
  ${CMAKE_BINARY_DIR}
  )

# Frameworks Apple
find_library(METAL_LIB Metal REQUIRED)
find_library(FOUNDATION_LIB Foundation REQUIRED)

configure_file(version.hxx.in version.hxx @ONLY)

set(SRCS
    main.cxx
    parameters.cxx
    values.cxx
    scheme.cxx
    user.cxx
    common/arguments.cxx
    common/os.cxx
    common/pause.cxx
    common/timer.cxx
  )

add_executable(
  ${VERSION}
  ${SRCS}
  )

target_link_libraries(${VERSION} PRIVATE
    ${METAL_LIB} ${FOUNDATION_LIB}
)


install(TARGETS ${VERSION} DESTINATION .)
