idf_component_register(SRCS "Pn532BeetleEsp32C6Spi.cpp"
					INCLUDE_DIRS "."
					REQUIRES libdesfire_esp
					PRIV_REQUIRES esp_driver_spi esp_driver_gpio)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 23)