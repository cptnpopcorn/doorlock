set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
  cli11_proj
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.6.1  # pin a release/tag you trust
)
FetchContent_MakeAvailable(cli11_proj)

add_executable(cardwriter EXCLUDE_FROM_ALL app.cpp card_operations.cpp)
target_link_libraries(cardwriter PRIVATE pn532-ft232h CLI11::CLI11 card_secrets card_layout libdesfire)