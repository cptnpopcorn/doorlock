set(CMAKE_CXX_STANDARD 23)

add_library(messages STATIC open_message_builder.cpp const_stream.cpp)

target_link_libraries(messages PUBLIC rapidjson)

target_include_directories(messages PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
)