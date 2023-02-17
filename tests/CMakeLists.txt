find_package(Catch2 2.6 QUIET)
if(NOT Catch2_FOUND)
  message("Using bundled Catch2 library")
  list(APPEND TESTS_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/catch2>)
endif()

# Compile definitions
list(APPEND TESTS_COMPILE_DEFINITIONS
  # https://github.com/catchorg/Catch2/blob/master/docs/configuration.md
  CATCH_CONFIG_FAST_COMPILE

  # https://github.com/zuhd-org/easyloggingpp/blob/master/README.md
  ELPP_DISABLE_LOGS
  ELPP_NO_LOG_TO_FILE
  ELPP_NO_DEFAULT_LOG_FILE

  ${3RDPARTY_DEFINITIONS}
)

# include directories for all tests
list(APPEND TESTS_INCLUDE_DIRECTORIES
  ${3RDPARTY_INCLUDE_DIRECTORIES}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/trompeloeil>
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
)

add_test(NAME "test_all"
         COMMAND test_all
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
