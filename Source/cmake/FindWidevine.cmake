#  WIDEVINE_INCLUDE_DIRS - the widevine include directories
#  WIDEVINE_LIBRARIES    - link these to use widevine.

find_package(PkgConfig)
pkg_check_modules(WIDEVINE widevine)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WIDEVINE DEFAULT_MSG WIDEVINE_LIBRARIES)

mark_as_advanced(WIDEVINE_INCLUDE_DIRS WIDEVINE_LIBRARIES)
