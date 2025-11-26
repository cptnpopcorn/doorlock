set(CMAKE_CXX_STANDARD 23)

find_package(FTD2XX 2.12.36.20 REQUIRED)
find_package(MPSSE 1.0.8 REQUIRED)

add_library(pn532-ft232h EXCLUDE_FROM_ALL PN532OverFT232HI2C.cpp Ft232hI2c.cpp PN532OverFT232HSPI.cpp Ft232hSpi.cpp)
target_compile_definitions(pn532-ft232h PUBLIC FTDIMPSSE_STATIC)
target_include_directories(pn532-ft232h INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(pn532-ft232h PUBLIC FTD2XX::FTD2XX MPSSE::MPSSE PRIVATE libdesfire)