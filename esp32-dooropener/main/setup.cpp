#include "setup.h"
#include "interaction_control.h"
#include <esp_sleep.h>
#include <iostream>

using namespace std;

setup::setup(
	interaction &quit,
	wifi_station& wifi,
	const mqtt_config& mqtt_config,
	nvs_access& nvs,
	gpio_num_t relayPin) noexcept :
	quit{quit},
	wifi{*this, wifi},
	mqtt{*this, wifi, mqtt_config, nvs},
	relay{pdMS_TO_TICKS(1'000), relayPin}
{
}

void setup::start(interaction_control& control)
{
	cout << "door lock setup.." << endl;
	cout << "w - wifi" << endl;
	cout << "m - mqtt" << endl;
	cout << "l - close relay for 1 second" << endl;
	cout << "r - restart after 4 seconds" << endl;
    cout << "q - quit" << endl;

    switch (cin.get())
    {
		case 'w': control.set(wifi); return;
		case 'm': control.set(mqtt); return;
		case 'l': relay.Close(); return;
		case 'r': restart(); return;
		case 'q': control.set(quit); return;
	}
}

void setup::restart()
{
	cout << "going into deep sleep.." << endl;
	esp_deep_sleep(4'000'000);
}