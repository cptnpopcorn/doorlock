#include "error.h"
#include "interaction_loop.h"
#include "mqtt_config.h"
#include "nvs_access.h"
#include "secrets.h"
#include "wifi_connection.h"
#include "wifi_station.h"

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <system_error>
#include <thread>
#include <utility>

class OpenMessageBuilder final : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, OpenMessageBuilder>
{
public:

	bool Key(const char* str, rapidjson::SizeType length, bool)
	{
		// TODO
		return true;
	}

	bool String(const char* str, rapidjson::SizeType length, bool)
	{
		// TODO
		return true;
	}

	bool Bool(bool b)
	{
		// TODO
		return true;
	}
};

extern "C" void app_main(void)
{
	check(esp_event_loop_create_default());

	// TODO
	OpenMessageBuilder message {};
}