# FindPIO.cmake
#
# Finds the Parallel IO (PIO) library.
#
# Variables defined:
#   PIO_FOUND
#   PIO_INCLUDE_DIRS
#   PIO_LIBRARIES
#
# Targets defined:
#   PIO::PIO

if(TARGET PIO::PIO)
    set(PIO_FOUND TRUE)
    return()
endif()

find_package(PIO CONFIG QUIET)

if(PIO_FOUND)
  if(NOT TARGET PIO::PIO)
    add_library(PIO::PIO INTERFACE IMPORTED)
    if(DEFINED PIO_INCLUDE_DIRS)
      target_include_directories(PIO::PIO INTERFACE ${PIO_INCLUDE_DIRS})
    endif()
    if(DEFINED PIO_LIBRARIES)
      target_link_libraries(PIO::PIO INTERFACE ${PIO_LIBRARIES})
    endif()
  endif()
else()
  # Basic fallback for common locations or discovered via ESMF
  find_path(PIO_INCLUDE_DIR NAMES pio.h PATHS /opt/software/pio/include /usr/local/include)
  find_library(PIO_LIBRARY NAMES pio PATHS /opt/software/pio/lib /usr/local/lib)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PIO DEFAULT_MSG PIO_LIBRARY PIO_INCLUDE_DIR)

  if(PIO_FOUND)
    set(PIO_INCLUDE_DIRS ${PIO_INCLUDE_DIR})
    set(PIO_LIBRARIES ${PIO_LIBRARY})
    if(NOT TARGET PIO::PIO)
      add_library(PIO::PIO INTERFACE IMPORTED)
      target_include_directories(PIO::PIO INTERFACE ${PIO_INCLUDE_DIRS})
      target_link_libraries(PIO::PIO INTERFACE ${PIO_LIBRARIES})
    endif()
  endif()
endif()
