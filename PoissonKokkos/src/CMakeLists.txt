cmake_minimum_required(VERSION 3.22)

set(code PoissonKokkos_${VERSION})
project(${code})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Kokkos REQUIRED)

include_directories(common)

configure_file(version.hxx.in version.hxx @ONLY)

add_executable(
  ${code}
  main.cxx
  parameters.cxx
  values.cxx
  scheme.cxx
  common/os.cxx
  common/timer.cxx
  common/arguments.cxx
  )

if (FLOAT_TYPE)
  target_compile_definitions(${code} PUBLIC FLOAT_TYPE)
endif()

target_link_libraries(${code} Kokkos::kokkos)

install(TARGETS ${code} DESTINATION .)


