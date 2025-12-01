set(LIBDES_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../common/libdesfire)

set(LIBDES_FILES
	AES128.cpp 
	Classic.cpp
	DES.cpp
	Desfire.cpp
	FrameFormatter.cpp
	FrameParser.cpp
	NullTargetDataValidator.cpp
	NullTargetDataWriter.cpp
	PN532.cpp
	Timer.cpp
	Utils.cpp
)

set(LIBDES_PATHS)
foreach(file IN LISTS LIBDES_FILES)
	list(APPEND LIBDES_PATHS "${LIBDES_DIR}/${file}")
endforeach()

idf_component_register(
	SRCS "${LIBDES_PATHS}"
	INCLUDE_DIRS "${LIBDES_DIR}")

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 23)