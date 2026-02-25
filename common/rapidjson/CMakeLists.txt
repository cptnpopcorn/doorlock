add_library(rapidjson INTERFACE)

include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)

if(IS_BIG_ENDIAN)
    target_compile_definitions(rapidjson INTERFACE RAPIDJSON_ENDIAN=RAPIDJSON_BIGENDIAN)
else()
    target_compile_definitions(rapidjson INTERFACE RAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN)
endif()

target_include_directories(rapidjson INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)