cmake_minimum_required(VERSION 3.31)
project(doorlock)

include(config/config.cmake)

add_library(card_secrets INTERFACE)
target_include_directories(card_secrets INTERFACE ${COMMON_GENERATED_DIR})

add_library(card_layout INTERFACE)
target_include_directories(card_layout INTERFACE "config")

add_subdirectory(libdesfire)
add_subdirectory(libdesfire_tests)
add_subdirectory(pn532-ft232h)
add_subdirectory(test-ft232h)
add_subdirectory(cardwriter)