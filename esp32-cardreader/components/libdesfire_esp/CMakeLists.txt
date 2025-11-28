idf_component_register(
	SRCS "libdesfire_esp.cpp"
	INCLUDE_DIRS "")

target_link_libraries(${COMPONENT_TARGET} PUBLIC libdesfire)