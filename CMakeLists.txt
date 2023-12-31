cmake_minimum_required(VERSION 3.22)

project(CoreCtrl LANGUAGES CXX)
set(PROJECT_VERSION 1.4.0-dev)

include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# add uninstall target
if(NOT TARGET uninstall)
 configure_file(
   "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
   IMMEDIATE @ONLY)

 add_custom_target(uninstall
   COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

set(CMAKE_CXX_STANDARD 20)

option (WITH_DEBUG_INFO "Add debug information (to Debug builds)" OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions("-D_DEBUG")
  add_compile_options("-W;-Wall;-Wextra")
  add_compile_options("-Wno-redundant-move;-Wno-sign-compare")

  if (WITH_DEBUG_INFO)
    add_compile_options("-ggdb;-fno-omit-frame-pointer")
  endif()
endif()

find_package(Qt5 5.15 COMPONENTS Core DBus Quick Charts Widgets Network Svg REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(QuaZip REQUIRED)

find_package(Botan QUIET)
if(NOT Botan_FOUND)
  message(FATAL_ERROR "Botan library not found")
endif()

find_package(fmt 5 QUIET)
if(NOT fmt_FOUND)
  message("Using bundled fmt library")
  list(APPEND 3RDPARTY_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/3rdparty/fmt)
  list(APPEND 3RDPARTY_SRC ${PROJECT_SOURCE_DIR}/3rdparty/fmt/src/format.cc)
endif()

find_package(pugixml 1.11 QUIET)
if(NOT pugixml_FOUND)
  message("Using bundled pugixml library")
  list(APPEND 3RDPARTY_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/3rdparty/pugixml)
  list(APPEND 3RDPARTY_SRC ${PROJECT_SOURCE_DIR}/3rdparty/pugixml/pugixml.cpp)
endif()

find_package(units 2.3.1 QUIET)
if(NOT units_FOUND)
  message("Using bundled units library")
  list(APPEND 3RDPARTY_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/3rdparty/units)
endif()
list(APPEND 3RDPARTY_DEFINITIONS
  UNIT_LIB_DISABLE_IOSTREAM
  DISABLE_PREDEFINED_UNITS
  ENABLE_PREDEFINED_DATA_UNITS
  ENABLE_PREDEFINED_ANGLE_UNITS
  ENABLE_PREDEFINED_TIME_UNITS
  ENABLE_PREDEFINED_ANGULAR_VELOCITY_UNITS
  ENABLE_PREDEFINED_FREQUENCY_UNITS
  ENABLE_PREDEFINED_VOLTAGE_UNITS
  ENABLE_PREDEFINED_POWER_UNITS
  ENABLE_PREDEFINED_TEMPERATURE_UNITS
  ENABLE_PREDEFINED_CONVERT_UNITS
  ENABLE_PREDEFINED_CONCENTRATION_UNITS
  ELPP_NO_LOG_TO_FILE
  ELPP_NO_DEFAULT_LOG_FILE
  ELPP_FORCE_USE_STD_THREAD
)

find_package(easyloggingpp 9.96.7 QUIET)
if(NOT easyloggingpp_FOUND)
  message("Using bundled easyloggingpp library")
  list(APPEND 3RDPARTY_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/3rdparty/easyloggingpp)
  list(APPEND 3RDPARTY_SRC ${PROJECT_SOURCE_DIR}/3rdparty/easyloggingpp/easylogging++.cc)
else()
  list(APPEND 3RDPARTY_SRC ${easyloggingpp_INCLUDE_DIR}/easylogging++.cc)
endif()

# Allow pci.ids path customization using -DWITH_PCI_IDS_PATH=<path-to-pci.ids-file> (#136)
set(WITH_PCI_IDS_PATH "/usr/share/hwdata/pci.ids" CACHE FILEPATH "Path to pci.ids file")

add_subdirectory(src)

# Tests
include(CTest)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

# Icons
string(TOLOWER "${PROJECT_NAME}" ICON_NAME)
foreach(ICON_SIZE 16 22 24 32 48 64 72 96 128 192 256)
    install(
        FILES resources/icon/app_${ICON_SIZE}.svg
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps
        RENAME ${ICON_NAME}.svg
    )
endforeach()

# .desktop file
install(
  FILES resources/launcher.desktop
  DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications
  RENAME org.corectrl.corectrl.desktop
)

# AppStream metadata
install(
  FILES resources/org.corectrl.corectrl.appdata.xml
  DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo
)
