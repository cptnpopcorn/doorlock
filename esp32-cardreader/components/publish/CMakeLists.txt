idf_component_register(
	SRCS "mqtt_event_handle.cpp" "publisher.cpp"
	INCLUDE_DIRS "."
	REQUIRES mqtt
	PRIV_REQUIRES error wifi
)