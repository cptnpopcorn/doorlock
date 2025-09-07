set(CMAKE_CXX_STANDARD 23)
add_library(libdesfire Utils.cpp PN532.cpp Desfire.cpp Classic.cpp AES128.cpp DES.cpp FrameParser.cpp NullTargetDataWriter.cpp NullTargetDataValidator.cpp FrameFormatter.cpp)
target_include_directories(libdesfire INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})