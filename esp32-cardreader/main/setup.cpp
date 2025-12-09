#include "setup.h"
#include "console_input.h"
#include "interaction_control.h"
#include "wifi_connection.h"
#include "wifi_station.h"

#include <iostream>

using namespace std;

setup::setup(interaction &quit, wifi_station& wifi) noexcept : quit{quit}, wifi{*this, wifi}
{
}

void setup::start(interaction_control &control)
{
	cout << "card reader setup.." << endl;
	cout << "w - wifi" << endl;
    cout << "q - quit" << endl;

    switch (cin.get())
    {
		case 'w':
			control.set(wifi);
			return;

		case 'q':
			control.set(quit);
			return;
	}
}

wifi_setup::wifi_setup(interaction &quit, wifi_station &wifi) noexcept : quit{quit}, wifi{wifi}
{
}

void wifi_setup::start(interaction_control &control)
{
	cout << "WiFi setup.." << endl;
	cout << "c - current config" << endl;
	cout << "q - quit" << endl;

	switch (cin.get())
    {
		case 'c':
			show_config();
			return;

		case 'q':
			control.set(quit);
			return;
	}
}

void wifi_setup::show_config()
{
	const auto config = wifi.get_config();
	cout << "current WiFi is [" << config.ssid << "] with password [" << config.passwd << ']' << endl;
}
