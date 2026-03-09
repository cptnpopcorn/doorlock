idf_component_register(SRCS "hw_timer.cpp" "event_handle.cpp" "console_input.cpp"
    INCLUDE_DIRS "."
    REQUIRES error esp_event esp_timer)