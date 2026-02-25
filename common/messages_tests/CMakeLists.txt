project(messages_tests)
set(CMAKE_CXX_STANDARD 23)

add_executable(test_open_message_builder EXCLUDE_FROM_ALL test_open_message_builder.cpp)
target_link_libraries(test_open_message_builder PRIVATE messages CppUTest::CppUTest)

include(CTest) # Enable testing
include(CppUTest)
cpputest_discover_tests(test_open_message_builder)