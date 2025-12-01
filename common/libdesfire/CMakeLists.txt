add_library(libdesfire Utils.cpp PN532.cpp Desfire.cpp Classic.cpp AES128.cpp DES.cpp FrameParser.cpp NullTargetDataWriter.cpp NullTargetDataValidator.cpp FrameFormatter.cpp Timer.cpp)
target_include_directories(libdesfire INTERFACE ${CMAKE_CURRENT_LIST_DIR})
set_property(TARGET libdesfire PROPERTY CXX_STANDARD 23)