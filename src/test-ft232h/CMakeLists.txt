set(CMAKE_CXX_STANDARD 23)
add_executable(test-ft232h EXCLUDE_FROM_ALL app.cpp)
target_link_libraries(test-ft232h PRIVATE pn532-ft232h)