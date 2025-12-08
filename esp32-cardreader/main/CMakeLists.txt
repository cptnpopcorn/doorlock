include(${CMAKE_CURRENT_LIST_DIR}/../../common/config.cmake)

idf_component_register(SRCS "main.cpp"
					   PRIV_REQUIRES pn532_beetle_esp32c6 driver
					   INCLUDE_DIRS ${COMMON_GENERATED_DIR} ${COMMON_CONFIG_DIR})
