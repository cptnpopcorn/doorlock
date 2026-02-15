#include "setup.h"
#include "console_input.h"
#include "interaction_control.h"
#include "card_layout.h"
#include "nvs_access.h"
#include "wifi_connection.h"
#include "wifi_station.h"
#include "ap_selection.h"

#include <esp_sleep.h>
#include <AES128.h>
#include <card_layout.h>
#include <Desfire.h>
#include <secrets.h>
#include <Timer.h>

#include <array>
#include <iostream>
#include <utility>

using namespace std;
using namespace std::chrono;

setup::setup(
	interaction &quit,
	wifi_station& wifi,
	const mqtt_config& mqtt_config,
	nvs_access& nvs,
	PN532Interface& card_interface) noexcept :
	quit{quit}, wifi{*this, wifi}, mqtt{*this, wifi, mqtt_config, nvs}, card_interface{card_interface}
{
}

void setup::start(interaction_control &control)
{
	cout << "card reader setup.." << endl;
	cout << "c - read card" << endl;
	cout << "w - wifi" << endl;
	cout << "m - mqtt" << endl;
	cout << "r - restart after 4 seconds" << endl;
    cout << "q - quit" << endl;

    switch (cin.get())
    {
		case 'c': read_card(); return;
		case 'w': control.set(wifi); return;
		case 'm': control.set(mqtt); return;
		case 'r': restart(); return;
		case 'q': control.set(quit); return;
	}
}

static inline void set_key(AES& aes, const array<uint8_t, 16>& key, uint8_t version)
{
	aes.SetKeyData(key.data(), key.size(), version);
}

void setup::read_card()
{
	AES piccMasterKey;
	AES doorlockMasterKey;
	AES doorlockWriteKey;
	AES doorlockReadKey;

	set_key(piccMasterKey, secrets::CARD_PICC_MASTER_AES_KEY, to_underlying(KeyVersionPicc::Master));
	set_key(doorlockMasterKey, secrets::CARD_DOORLOCK_MASTER_AES_KEY, to_underlying(KeyVersionDoorlock::Master));
	set_key(doorlockWriteKey, secrets::CARD_DOORLOCK_WRITE_AES_KEY, to_underlying(KeyVersionDoorlock::Write));
	set_key(doorlockReadKey, secrets::CARD_DOORLOCK_READ_AES_KEY, to_underlying(KeyNumberDoorlock::Read));

	Desfire des{card_interface};
	des.SetDebugLevel(0);
	des.begin();

	if (!des.SetPassiveActivationRetries())
	{
		cout << "error configuring retries" << endl;
		return;
	}

	if (!des.SamConfig())
	{
		cout << "could not configure secure access" << endl;
		return;
	}

	cout << "present card within 10s";
	cout.flush();

	for (Timer t {10s}; t.IsRunning(); t.Update())
	{
		array<uint8_t, 8> uid;
		uint8_t uid_length {0};
		eCardType cardType {};

		if (!des.ReadPassiveTargetID(uid.data(), &uid_length, &cardType) || uid_length == 0)
		{
			cout << '.';
			cout.flush();
			this_thread::sleep_for(500ms);
			continue;
		}

		cout << endl << "card found" << endl;
		
		if (!des.SelectApplication(to_underlying(ApplicationId::Picc)))
		{
			cout << "this is not a \"DESFire EV\" card, cannot be used" << endl;
			return;
		}

		if (!des.Authenticate(to_underlying(KeyNumberPicc::Master), &piccMasterKey))
		{
			cout << "card was not initialized by the door lock application, not usable" << endl;
			return;
		}

		if (!des.SelectApplication(to_underlying(ApplicationId::Doorlock)))
		{
			cout << "no doorlock application data present / no user ID written" << endl;
			return;
		}

		cout << "doorlock application found" << endl;

		if (!des.Authenticate(to_underlying(KeyNumberDoorlock::Read), &doorlockReadKey))
		{
			cout << "could not access doorlock data for reading, security key not matching" << endl;
			return;
		}

		array<uint8_t, static_cast<size_t>(FileSize::PublicuserId)> user_id;
		if (!des.ReadFileData(to_underlying(FileId::PublicUserId), 0, user_id.size(), user_id.data()))
		{
			cout << "could not read doorlock user ID file" << endl;
			return;
		}

		cout << "found user ID ";
		const auto flags = cout.flags();
		cout << hex << uppercase << setfill('0');
		for (const auto &c : user_id) cout << setw(2) << static_cast<int>(c);
		cout << endl;
		cout.setf(flags);

		return;
	}

	cout << endl << "no card found" << endl;
}

void setup::restart()
{
	cout << "going into deep sleep.." << endl;
	esp_deep_sleep(4'000'000);
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
  connection.start();

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