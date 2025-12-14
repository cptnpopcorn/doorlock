#include "setup.h"
#include "console_input.h"
#include "interaction_control.h"
#include "app_storage.h"
#include "mqtt_config.h"
#include "nvs_access.h"
#include "wifi_connection.h"
#include "wifi_station.h"
#include "ap_selection.h"

#include <iostream>

using namespace std;

setup::setup(interaction &quit, wifi_station& wifi, const mqtt_config& mqtt_config, nvs_access& nvs) noexcept :
	quit{quit}, wifi{*this, wifi}, mqtt{*this, wifi, mqtt_config, nvs}
{
}

void setup::start(interaction_control &control)
{
	cout << "card reader setup.." << endl;
	cout << "w - wifi" << endl;
	cout << "m - mqtt" << endl;
    cout << "q - quit" << endl;

    switch (cin.get())
    {
		case 'w': control.set(wifi); return;
		case 'm': control.set(mqtt); return;
		case 'q': control.set(quit); return;
	}
}

wifi_setup::wifi_setup(interaction &quit, wifi_station &wifi) noexcept : quit{quit}, wifi{wifi}
{
}

void wifi_setup::start(interaction_control &control)
{
	cout << "WiFi setup.." << endl;
	cout << "c - current config" << endl;
	cout << "s - select access point" << endl;
	cout << "t - test wifi connection" << endl;
	cout << "q - quit" << endl;

	switch (cin.get())
    {
		case 'c': show_config(); return;
		case 's': select_ap(); return;
		case 't': test_connect(); return;
		case 'q': control.set(quit); return;
	}
}

void wifi_setup::show_config()
{
	const auto config = wifi.get_config();
	cout << "current WiFi is [" << config.ssid << "] with password [" << config.passwd << ']' << endl;
}

void wifi_setup::select_ap()
{
  cout << "scanning for access points..." << endl;
  ap_selection selection{};
  wifi.scan(selection);
  selection.flush();

  auto ssid = selection.get_selected_ssid();
  if (ssid.empty()) {
    cout << "no access point was selected" << endl;
    return;
  }

  cout << "WiFi password: ";
  auto config = wifi.get_config();
  config.ssid = ssid;
  config.passwd = console_read_line();
  cout << endl;

  wifi.set_config(config);
}

void wifi_setup::test_connect()
{
  cout << "connecting WiFi.." << endl;
  wifi_connection connection{wifi};

  cout << "getting IP address";
  auto is_up = connection.is_up();
  for ([[maybe_unused]] auto i = 10; i != 0; --i) {
    if (is_up.wait_for(1s) == future_status::ready) {
      cout << endl << "address sucessfully assigned" << endl;
      return;
    }
    cout << '.';
    cout.flush();
  }
  cout << endl << "timeout reached" << endl;
}

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
    cout << "t - set mqtt topic" << endl;
	cout << "q - quit" << endl;

	switch (cin.get())
	{
		case 'c': show_config(); return;
		case 't': set_topic(); return;
		case 'q': control.set(quit); return;
	}
}

void mqtt_setup::show_config()
{
	cout << "broker URI: " << config.broker_host << endl;
	cout << "topic root: " << config.topic_root << endl;
	cout << "topic: " << nvs.get_str(app_storage::mqtt_topic_key) << endl;
}

void mqtt_setup::set_topic()
{
	cout << "enter new topic: ";
	const auto new_topic = console_read_line();
	cout << endl;
	if (new_topic.length() == 0)
	{
		cout << "empty input -> no change" << endl;
		return;
	}
	if (new_topic == nvs.get_str(app_storage::mqtt_topic_key))
	{
		cout << "topic identical -> no change" << endl;
		return;
	}

	nvs.set_str(app_storage::mqtt_topic_key, new_topic);
}
