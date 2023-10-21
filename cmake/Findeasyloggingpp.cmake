# - Find easyloggingpp
#
# This file is in the public domain.

find_package(PkgConfig REQUIRED)
pkg_check_modules(EASYLOGGINGPP_PKG QUIET easyloggingpp)

set(easyloggingpp_FOUND ${EASYLOGGINGPP_PKG_FOUND})
set(easyloggingpp_VERSION ${EASYLOGGINGPP_PKG_VERSION})

find_path(easyloggingpp_INCLUDE_DIR
  NAMES easylogging++.h
  HINTS ${EASYLOGGINGPP_PKG_INCLUDEDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(easyloggingpp
  REQUIRED_VARS easyloggingpp_FOUND easyloggingpp_INCLUDE_DIR
  VERSION_VAR easyloggingpp_VERSION)

mark_as_advanced(easyloggingpp_FOUND easyloggingpp_VERSION easyloggingpp_INCLUDE_DIR)
