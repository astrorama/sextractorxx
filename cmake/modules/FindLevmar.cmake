# - Locate the Levmar library
# Defines:
#
#  Levmar_FOUND
#  LEVMAR_INCLUDE_DIR
#  LEVMAR_INCLUDE_DIRS (not cached)
#  LEVMAR_LIBRARY
#  LEVMAR_LIBRARIES (not cached)


if(NOT Levmar_FOUND)

    find_path(LEVMAR_INCLUDE_DIR levmar.h
            HINTS ENV LEVMAR_ROOT_DIR LEVMAR_INSTALL_DIR
            PATH_SUFFIXES include)

    find_library(LEVMAR_LIBRARY levmar
            HINTS ENV LEVMAR_ROOT_DIR LEVMAR_INSTALL_DIR
            PATH_SUFFIXES lib)

    find_library(M_LIBRARY m)

    set(LEVMAR_LIBRARIES ${LEVMAR_LIBRARY})
    set(LEVMAR_INCLUDE_DIRS ${LEVMAR_INCLUDE_DIR})

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Levmar DEFAULT_MSG LEVMAR_INCLUDE_DIRS LEVMAR_LIBRARIES)

    mark_as_advanced(Levmar_FOUND LEVMAR_INCLUDE_DIRS LEVMAR_LIBRARIES)

    list(REMOVE_DUPLICATES LEVMAR_LIBRARIES)
    list(REMOVE_DUPLICATES LEVMAR_INCLUDE_DIRS)

endif()