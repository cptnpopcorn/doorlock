#include "wifi_setup.h"
#include "console_input.h"
#include "interaction_control.h"
#include "nvs_access.h"
#include "wifi_connection.h"
#include "wifi_station.h"
#include "ap_selection.h"

#include <iostream>

using namespace std;

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
  connection.start();

  cout << "getting IP address";
  for ([[maybe_unused]] auto i = 10; i != 0; --i) {
    if (connection.wait_is_up(pdMS_TO_TICKS(1'000))) {
      cout << endl << "address sucessfully assigned" << endl;
      return;
    }
    cout << '.';
    cout.flush();
  }
  cout << endl << "timeout reached" << endl;
}