set(CMAKE_CXX_STANDARD 23)
add_executable(cardwriter EXCLUDE_FROM_ALL app.cpp)
target_link_libraries(cardwriter PRIVATE pn532-ft232h)