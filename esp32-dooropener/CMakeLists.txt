cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
	../common-esp32/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD OFF)

project(dooropener)

add_subdirectory(
	${CMAKE_CURRENT_LIST_DIR}/../common/rapidjson
	${CMAKE_BINARY_DIR}/rapidjson
)

add_subdirectory(
	${CMAKE_CURRENT_LIST_DIR}/../common/messages
	${CMAKE_BINARY_DIR}/messages
)
