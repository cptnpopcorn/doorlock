set(COMMON_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common)
set(COMMON_CONFIG_DIR ${COMMON_DIR}/config)
include(${COMMON_CONFIG_DIR}/config.cmake)

idf_component_register(
	SRCS
		"main.cpp"
		"setup.cpp"
		"relay_controller.cpp"
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
	INCLUDE_DIRS
		${COMMON_GENERATED_DIR}
		${COMMON_CONFIG_DIR}
)

target_link_libraries(${COMPONENT_LIB} PRIVATE messages rapidjson)