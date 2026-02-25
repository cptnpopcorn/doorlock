project(libdesfire_tests)
set(CMAKE_CXX_STANDARD 23)

add_executable(test_frame_parser EXCLUDE_FROM_ALL test_frame_parser.cpp TestDataWriter.cpp TestFrameWriter.cpp DataAsserter.cpp)
target_link_libraries(test_frame_parser PRIVATE CppUTest::CppUTest libdesfire)

add_executable(test_frame_formatter EXCLUDE_FROM_ALL test_frame_formatter.cpp)
target_link_libraries(test_frame_formatter PRIVATE CppUTest::CppUTest libdesfire)

include(CTest) # Enable testing

# automatically discover CppUTest tests
include(CppUTest)
cpputest_discover_tests(test_frame_parser)
cpputest_discover_tests(test_frame_formatter)