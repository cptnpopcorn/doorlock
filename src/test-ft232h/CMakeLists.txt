set(CMAKE_CXX_STANDARD 23)

add_executable(test-ft232h-i2c EXCLUDE_FROM_ALL test-i2c.cpp)
target_link_libraries(test-ft232h-i2c PRIVATE pn532-ft232h)

add_executable(test-ft232h-spi EXCLUDE_FROM_ALL test-spi.cpp)
target_link_libraries(test-ft232h-spi PRIVATE pn532-ft232h)