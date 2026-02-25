#include "mqtt_setup.h"
#include "card_layout.h"
#include "console_input.h"
#include "interaction_control.h"
#include "mqtt_storage.h"
#include "mqtt_config.h"
#include "mqtt_storage.h"
#include "mqtt_topic.h"
#include "mqtt_wrapper.h"
#include "nvs_access.h"
#include "wifi_connection.h"
#include "wifi_station.h"

#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

mqtt_setup::mqtt_setup(
	interaction &quit,
	wifi_station &wifi,
	const mqtt_config& config,
	nvs_access& nvs) noexcept :
	quit{quit}, wifi{wifi}, config{config}, nvs{nvs}
{
}

void mqtt_setup::start(interaction_control &control)
{
	cout << "MQTT setup.." << endl;
    cout << "c - current config" << endl;
    cout << "s - set site" << endl;
    cout << "r - set room" << endl;
    cout << "d - set door" << endl;
	cout << "p - test publishing card ID" << endl;
	cout << "l - test subscription to door open" << endl;
	cout << "q - quit" << endl;

	switch (cin.get())
	{
		case 'c': show_config(); return;
		case 's': set_topic(mqtt_storage::mqtt_site_key); return;
		case 'r': set_topic(mqtt_storage::mqtt_room_key); return;
		case 'd': set_topic(mqtt_storage::mqtt_door_key); return;
		case 'p': test_publish(); return;
		case 'l': test_subscribe(); return;
		case 'q': control.set(quit); return;
	}
}

void mqtt_setup::show_config()
{
	cout << "broker URI: " << config.broker_host << endl;
	cout << "card reader topic: " << mqtt_storage::read_topic(nvs).card_reader_str() << endl;
	cout << "door opener topic: " << mqtt_storage::read_topic(nvs).door_opener_str() << endl;
}

void mqtt_setup::set_topic(const char* key)
{
	cout << "enter " << key << ": ";
	const auto new_topic = console_read_line();
	cout << endl;
	if (new_topic.length() == 0)
	{
		cout << "empty input -> no change" << endl;
		return;
	}
	if (new_topic == nvs.get_str(key))
	{
		cout << "topic identical -> no change" << endl;
		return;
	}

	nvs.set_str(key, new_topic);
}

void mqtt_setup::test_publish()
{
	cout << "connecting WiFi.." << endl;
    wifi_connection connection{wifi};
	connection.start();

	if (!connection.wait_is_up(pdMS_TO_TICKS(5'000)))
	{
		cout << "WiFi connection timeout" << endl;
		return;
	}

	mqtt_wrapper publisher{config.broker_host, mqtt_storage::read_topic(nvs).card_reader_str(), config.ca_cert, config.client_cert, config.client_key};

	cout << "connecting to MQTT broker.." << endl;
	auto is_mqtt_connected = publisher.is_connected();
	if (is_mqtt_connected.wait_for(5s) != future_status::ready)
	{
		cout << "MQTT connection timeout" << endl;
		return;
	}

	cout << "publishing all-zero ID.." << endl;
	array<uint8_t, to_underlying(FileSize::PublicuserId)> dummy_id{};
	if (!publisher.publish(dummy_id))
	{
		cout << "MQTT publish failed" << endl;
		return;
	}
}

void mqtt_setup::test_subscribe()
{
	cout << "connecting WiFi.." << endl;
    wifi_connection connection{wifi};
	connection.start();

	if (!connection.wait_is_up(pdMS_TO_TICKS(5'000)))
	{
		cout << "WiFi connection timeout" << endl;
		return;
	}

	mqtt_wrapper subscriber{config.broker_host, mqtt_storage::read_topic(nvs).door_opener_str(), config.ca_cert, config.client_cert, config.client_key};

	cout << "connecting to MQTT broker.." << endl;
	auto is_mqtt_connected = subscriber.is_connected();
	if (is_mqtt_connected.wait_for(5s) != future_status::ready)
	{
		cout << "MQTT connection timeout" << endl;
		return;
	}

	cout << "subscribing to topic.." << endl;

	if (!subscriber.subscribe([](const auto &data){
		string_view text(reinterpret_cast<const char*>(data.data()), data.size());
		cout << "received message: " << text << endl;
	}))
	{
		cout << "subscription failed" << endl;
		return;
	}

	cout << "waiting for messages (8s).." << endl;
	this_thread::sleep_for(8s);
}
