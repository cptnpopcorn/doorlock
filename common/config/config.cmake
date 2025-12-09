set(COMMON_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${COMMON_GENERATED_DIR}")

include(${CMAKE_CURRENT_LIST_DIR}/CreateByteArray.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/CreateStringBlobArray.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/secrets.cmake)

set(AES_KEY_DECLS "")

foreach(KEY_NAME ${CARD_KEYS})
    set(HEX_VALUE "${${KEY_NAME}}")
    create_byte_array("${HEX_VALUE}" "${KEY_NAME}" DECL_SNIPPET)
    string(APPEND AES_KEY_DECLS "${DECL_SNIPPET}\n")
endforeach()

foreach(KEY_NAME ${MQTT_KEYS})
	set(BLOB_VALUE "${${KEY_NAME}}")
	create_string_blob_array("${BLOB_VALUE}" "${KEY_NAME}" DECL_SNIPPET)
	string(APPEND MQTT_KEY_DECLS "${DECL_SNIPPET}\n")
endforeach()

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/secrets.h.in
    ${COMMON_GENERATED_DIR}/secrets.h
    @ONLY
)

set(SECRET_FILE "${CMAKE_CURRENT_LIST_DIR}/secrets.cmake")

if(NOT EXISTS "${SECRET_FILE}")
    message(FATAL_ERROR
        "Missing secret key file: ${SECRET_FILE}\n"
        "Please copy config/secrets.cmake.in → config/secrets.cmake\n"
        "Then edit it to insert valid AES keys (16-byte hex).")
endif()