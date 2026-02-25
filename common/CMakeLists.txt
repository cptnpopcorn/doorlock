cmake_minimum_required(VERSION 3.16)
project(doorlock)

set(CPPUTEST_MEM_LEAK_DETECTION_DISABLED ON CACHE BOOL "Disable memory leak detection" FORCE)
include(FetchContent)

FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        master # or use release tag, eg. v4.0
    # FIND_PACKAGE_ARGS # Recent CMake versions can integrate FetchContent with find_package
    # https://cmake.org/cmake/help/latest/module/FetchContent.html#integrating-with-find-package
)

FetchContent_MakeAvailable(CppUTest)
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
add_subdirectory(rapidjson)
add_subdirectory(messages)
add_subdirectory(messages_tests)