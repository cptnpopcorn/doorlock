get_filename_component(_FTD2XX_PREFIX "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

set(FTD2XX_VERSION "2.12.36.20")

# Locations
set(FTD2XX_INCLUDE_DIR "${_FTD2XX_PREFIX}/include")
set(FTD2XX_LIBRARY_DIR "${_FTD2XX_PREFIX}/lib")

# Libraries
find_library(FTD2XX_LIBRARY
    NAMES ftd2xx ftd2xx.lib
    PATHS "${FTD2XX_LIBRARY_DIR}"
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    FTD2XX
    REQUIRED_VARS FTD2XX_INCLUDE_DIR FTD2XX_LIBRARY
    VERSION_VAR FTD2XX_VERSION
)

# Export variables
set(FTD2XX_INCLUDE_DIRS "${FTD2XX_INCLUDE_DIR}")
set(FTD2XX_LIBRARIES "${FTD2XX_LIBRARY}")

# Imported target
if(NOT TARGET FTD2XX::FTD2XX)
    add_library(FTD2XX::FTD2XX UNKNOWN IMPORTED)
    set_target_properties(FTD2XX::FTD2XX PROPERTIES
        IMPORTED_LOCATION "${FTD2XX_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FTD2XX_INCLUDE_DIR}"
    )
endif()