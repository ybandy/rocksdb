find_path(URING_INCLUDE_DIRS
  NAMES liburing.h
  HINTS ${URING_ROOT_DIR}/src/include)

find_library(URING_LIBRARIES
  NAMES uring
  HINTS ${URING_ROOT_DIR}/src)

find_package_handle_standard_args(uring DEFAULT_MSG URING_LIBRARIES URING_INCLUDE_DIRS)

mark_as_advanced(
  URING_INCLUDE_DIRS
  URING_LIBRARIES)
