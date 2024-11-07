find_package(Catch2 3.0 REQUIRED)
find_package(trompeloeil 40 REQUIRED)

# Compile definitions
list(APPEND TESTS_COMPILE_DEFINITIONS
  ${3RDPARTY_DEFINITIONS}
  CATCH_CONFIG_FAST_COMPILE
  SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
)

# include directories for all tests
list(APPEND TESTS_INCLUDE_DIRECTORIES
  ${3RDPARTY_INCLUDE_DIRECTORIES}
  ${PROJECT_SOURCE_DIR}/src
)

# catch main
add_library(catch_main OBJECT "src/main.cpp")
target_include_directories(catch_main PRIVATE ${TESTS_INCLUDE_DIRECTORIES})
target_compile_definitions(catch_main PRIVATE ${TESTS_COMPILE_DEFINITIONS})
target_link_libraries(catch_main PRIVATE
  spdlog::spdlog
  Catch2::Catch2
  trompeloeil::trompeloeil
  ${ATOMIC_LIB}
)

# test_* files
file(GLOB test_files "src/test_*.cpp")

# generate test_all executable
add_executable(test_all $<TARGET_OBJECTS:catch_main> ${test_files} ${3RDPARTY_SRC})
target_include_directories(test_all PRIVATE ${TESTS_INCLUDE_DIRECTORIES})
target_compile_definitions(test_all PRIVATE ${TESTS_COMPILE_DEFINITIONS})
target_link_libraries(test_all PRIVATE
  corectrl_lib
  Qt5::Core
  spdlog::spdlog
  $<$<BOOL:${units_FOUND}>:units::units>
  Catch2::Catch2
  trompeloeil::trompeloeil
  ${ATOMIC_LIB}
)

include(CTest)
include(Catch)
catch_discover_tests(test_all)
