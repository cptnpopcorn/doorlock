set(COMMON_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../common/config)
include(${COMMON_CONFIG_DIR}/config.cmake)

idf_component_register(
	SRCS
		"mqtt_setup.cpp"
		"mqtt_storage.cpp"
	INCLUDE_DIRS
		"."
	PRIV_INCLUDE_DIRS
		${COMMON_GENERATED_DIR}
		${COMMON_CONFIG_DIR}
	REQUIRES mqtt_roles interaction
	PRIV_REQUIRES storage wifi util
)