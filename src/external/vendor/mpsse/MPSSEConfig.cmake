get_filename_component(_MPSSE_PREFIX "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

set(MPSSE_VERSION "1.0.8")

# Locations
set(MPSSE_INCLUDE_DIR "${_MPSSE_PREFIX}/include")
set(MPSSE_LIBRARY_DIR "${_MPSSE_PREFIX}/lib")

# Library
find_library(MPSSE_LIBRARY
    NAMES libmpsse libmpsse.lib
    PATHS "${MPSSE_LIBRARY_DIR}"
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    MPSSE
    REQUIRED_VARS MPSSE_INCLUDE_DIR MPSSE_LIBRARY
    VERSION_VAR MPSSE_VERSION
)

# Export variables
set(MPSSE_INCLUDE_DIRS "${MPSSE_INCLUDE_DIR}")
set(MPSSE_LIBRARIES "${MPSSE_LIBRARY}")

# Imported target
if(NOT TARGET MPSSE::MPSSE)
    add_library(MPSSE::MPSSE UNKNOWN IMPORTED)
    set_target_properties(MPSSE::MPSSE PROPERTIES
        IMPORTED_LOCATION "${MPSSE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MPSSE_INCLUDE_DIR}"
    )
endif()