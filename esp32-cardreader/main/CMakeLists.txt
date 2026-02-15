set(COMMON_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/config)
include(${COMMON_CONFIG_DIR}/config.cmake)

idf_component_register(
	SRCS
		"main.cpp"
		"setup.cpp"
		"ap_selection.cpp"
	PRIV_REQUIRES
		pn532_beetle_esp32c6
		interaction
		error
		util
		mqtt_roles
		mqtt_roles_setup
		wifi
		storage
		esp_event
		driver
	INCLUDE_DIRS
		${COMMON_GENERATED_DIR}
		${COMMON_CONFIG_DIR}
)