set(COMMON_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/config)
set(COMMON_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${COMMON_GENERATED_DIR}")

include(${COMMON_CONFIG_DIR}/CreateByteArray.cmake)
include(${COMMON_CONFIG_DIR}/secrets.cmake)

set(AES_KEY_DECLS "")

# Loop through all defined secret keys
foreach(KEY_NAME ${SECRET_KEYS})
    # Expand the variable name to get the hex string
    set(HEX_VALUE "${${KEY_NAME}}")

    # Create declaration snippet
    create_byte_array("${HEX_VALUE}" "${KEY_NAME}" DECL_SNIPPET)

    # Append to final block
    string(APPEND AES_KEY_DECLS "${DECL_SNIPPET}\n")
endforeach()

# Generate header
configure_file(
    ${COMMON_CONFIG_DIR}/secrets.h.in
    ${COMMON_GENERATED_DIR}/secrets.h
    @ONLY
)

set(SECRET_FILE "${COMMON_CONFIG_DIR}/secrets.cmake")

if(NOT EXISTS "${SECRET_FILE}")
    message(FATAL_ERROR
        "Missing secret key file: ${SECRET_FILE}\n"
        "Please copy config/secrets.cmake.in → config/secrets.cmake\n"
        "Then edit it to insert valid AES keys (16-byte hex).")
endif()