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

set(METAL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/metal/kernels.metal")
set(METAL_LIB "${CMAKE_CURRENT_BINARY_DIR}/default.metallib")

add_custom_command(
    OUTPUT ${METAL_LIB}
    COMMAND xcrun -sdk macosx metal -o ${METAL_LIB} ${METAL_SRC}
    DEPENDS ${METAL_SRC}
    COMMENT "Compiling Metal shaders to default.metallib"
)

set(SRCS
    main.cxx
    parameters.cxx
    values.cxx
    scheme.cxx
    #
    common/arguments.cxx
    common/os.cxx
    common/pause.cxx
    common/timer.cxx
    #
    metal/Context.cxx
    metal/dim_metal.cxx
    metal/iteration_metal.cxx
    metal/memmove_metal.cxx
    metal/metal_implementation.cxx
    metal/values_metal.cxx
    metal/variation_metal.cxx
  )

add_executable(
  ${VERSION}
  ${SRCS}
  ${METAL_LIB}
  )

target_link_libraries(${VERSION} PRIVATE
    ${METAL_LIB}
    ${FOUNDATION_LIB}
)


install(TARGETS ${VERSION} DESTINATION .)
install(FILES ${METAL_LIB} DESTINATION .)
