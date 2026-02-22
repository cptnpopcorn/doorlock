idf_component_register(
	SRCS "mqtt_wrapper.cpp" "mqtt_config.cpp" "mqtt_topic.cpp" "mqtt_event_handle.cpp"
	INCLUDE_DIRS "."
	REQUIRES mqtt
	PRIV_REQUIRES error wifi
)