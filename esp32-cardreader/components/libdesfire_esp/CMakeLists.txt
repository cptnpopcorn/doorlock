idf_component_register(
	SRCS "libdesfire_esp.c"
	INCLUDE_DIRS "include")

target_link_libraries(${COMPONENT_TARGET} PUBLIC libdesfire)