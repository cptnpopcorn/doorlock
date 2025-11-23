set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

include(${CMAKE_CURRENT_LIST_DIR}/config/CreateByteArray.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config/secrets.cmake)

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
    ${CMAKE_CURRENT_LIST_DIR}/config/secrets.h.in
    ${GENERATED_DIR}/secrets.h
    @ONLY
)

set(SECRET_FILE "${CMAKE_CURRENT_LIST_DIR}/config/secrets.cmake")

if(NOT EXISTS "${SECRET_FILE}")
    message(FATAL_ERROR
        "Missing secret key file: ${SECRET_FILE}\n"
        "Please copy config/secrets.cmake.in → config/secrets.cmake\n"
        "Then edit it to insert valid AES keys (16-byte hex).")
endif()

add_library(card_secrets INTERFACE)
target_include_directories(card_secrets INTERFACE ${GENERATED_DIR})

add_library(card_layout INTERFACE)
target_include_directories(card_layout INTERFACE ${CMAKE_CURRENT_LIST_DIR}/config)

add_subdirectory(libdesfire)
add_subdirectory(libdesfire_tests)
add_subdirectory(pn532-ft232h)
add_subdirectory(test-ft232h)
add_subdirectory(cardwriter)