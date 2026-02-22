#include "error.h"
#include "interaction_loop.h"
#include "mqtt_config.h"
#include "nvs_access.h"
#include "secrets.h"
#include "wifi_connection.h"
#include "wifi_station.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <system_error>
#include <thread>
#include <utility>

extern "C" void app_main(void)
{
	check(esp_event_loop_create_default());
}