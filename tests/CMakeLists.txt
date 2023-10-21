find_package(Catch2 2.6 QUIET)
if(NOT Catch2_FOUND)
  message("Using bundled Catch2 library")
  list(APPEND TESTS_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/catch2>)
endif()

find_package(trompeloeil 40 QUIET)
if(NOT trompeloeil_FOUND)
  message("Using bundled trompeloeil library")
  list(APPEND TESTS_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/trompeloeil>)
endif()

# Compile definitions
list(APPEND TESTS_COMPILE_DEFINITIONS
  ${3RDPARTY_DEFINITIONS}
  CATCH_CONFIG_FAST_COMPILE
  ELPP_DISABLE_LOGS
)

# include directories for all tests
list(APPEND TESTS_INCLUDE_DIRECTORIES
  ${3RDPARTY_INCLUDE_DIRECTORIES}
  ${PROJECT_SOURCE_DIR}/src
)

# catch main
add_library(catch_main OBJECT "src/main.cpp")
target_include_directories(catch_main PRIVATE ${TESTS_INCLUDE_DIRECTORIES})

# test_* files
file(GLOB test_files "src/test_*.cpp")

# generate test_all executable
add_executable(test_all $<TARGET_OBJECTS:catch_main> ${test_files} ${3RDPARTY_SRC})
target_include_directories(test_all PRIVATE ${TESTS_INCLUDE_DIRECTORIES})
target_compile_definitions(test_all PRIVATE ${TESTS_COMPILE_DEFINITIONS})
target_link_libraries(test_all PRIVATE
  corectrl_lib
  Qt5::Core
  $<$<BOOL:${units_FOUND}>:units::units>
  $<$<BOOL:${Catch2_FOUND}>:Catch2::Catch2>
  $<$<BOOL:${trompeloeil_FOUND}>:trompeloeil::trompeloeil>
)

add_test(NAME "test_all"
         COMMAND test_all
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
