set(COMMON_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/config)
include(${COMMON_CONFIG_DIR}/config.cmake)

idf_component_register(
	SRCS
		"main.cpp"
		"open_message_builder.cpp"
		"setup.cpp"
	PRIV_REQUIRES
		interaction
		error
		util
		mqtt_roles
		mqtt_roles_setup
		wifi
		wifi_setup
		storage
		esp_event
		driver
		rapidjson
	INCLUDE_DIRS
		${COMMON_GENERATED_DIR}
		${COMMON_CONFIG_DIR}
)