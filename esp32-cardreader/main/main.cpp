#include <Desfire.h>
#include <Pn532BeetleEsp32C6Spi.h>

#include <soc/spi_reg.h>
#include <driver/usb_serial_jtag.h>
#include <driver/usb_serial_jtag_vfs.h>
#include <esp_event.h>
#include <esp_sleep.h>

#include "app_storage.h"
#include "card_layout.h"
#include "error.h"
#include "interaction_loop.h"
#include "mqtt_config.h"
#include "nvs_access.h"
#include "publisher.h"
#include "secrets.h"
#include "setup.h"
#include "wifi_connection.h"
#include "wifi_station.h"

#include <array>
#include <atomic>
#include <chrono>
#include <exception>
#include <iostream>
#include <system_error>
#include <thread>
#include <utility>

using namespace std;
using namespace std::chrono;

void check(esp_err_t err) {
  if (err == ESP_OK) return;
  throw system_error((int)err, system_category());
}

void check(esp_err_t err, const string &what) {
  if (err == ESP_OK) return;
  throw system_error((int)err, system_category(), what);
}

void error(const std::string &what) { throw runtime_error(what); }

#ifdef CONFIG_MQTT_USE_TLS
extern "C"
{
    extern const uint8_t _binary_ca_crt_start[] asm("_binary_ca_crt_start");
    extern const uint8_t _binary_ca_crt_end[] asm("_binary_ca_crt_end");
}
span<const uint8_t> get_ca_crt()
{
    return {&_binary_ca_crt_start[0], &_binary_ca_crt_end[0]};
}
#else
span<const uint8_t> get_ca_crt()
{
    return {};
}
#endif

#ifdef CONFIG_MQTT_TLS_CLIENT_AUTH
extern "C"
{
    extern const uint8_t _binary_client_crt_start[] asm("_binary_client_crt_start");
    extern const uint8_t _binary_client_crt_end[] asm("_binary_client_crt_end");
    extern const uint8_t _binary_client_key_start[] asm("_binary_client_key_start");
    extern const uint8_t _binary_client_key_end[] asm("_binary_client_key_end");
    span<const uint8_t> get_client_crt()
    {
        return {&_binary_client_crt_start[0], &_binary_client_crt_end[0]};
    }
    span<const uint8_t> get_client_key()
    {
        return {&_binary_client_key_start[0], &_binary_client_key_end[0]};
    }
}
#else
span<const uint8_t> get_client_crt()
{
    return {};
}
span<const uint8_t> get_client_key()
{
    return {};
}
#endif

static inline void set_key(AES& aes, const array<uint8_t, 16>& key, uint8_t version)
{
	aes.SetKeyData(key.data(), key.size(), version);
}

extern "C" void app_main(void)
{
	try
	{
		cout << "setting up PN532 SPI communication.." << endl;
		Pn532BeetleEsp32C6Spi pn_spi{500ms};

		check(esp_event_loop_create_default());

		nvs_access nvs{"cardreader"};
		wifi_station wifi{nvs};

		const string mqtt_broker_hostname{CONFIG_MQTT_BROKER_HOSTNAME};
		const string mqtt_topic_root{CONFIG_MQTT_TOPIC_ROOT};
		const mqtt_config mqtt_config{mqtt_broker_hostname, mqtt_topic_root, get_ca_crt(), get_client_crt(), get_client_key()};
	
		if (usb_serial_jtag_is_connected())
		{
			cout << "USB console connected, entering interactive mode.." << endl;

			setvbuf(stdin, nullptr, _IONBF, 0);
			usb_serial_jtag_driver_config_t usb_serial_jtag_config{.tx_buffer_size = 1024, .rx_buffer_size = 1024};
			check(usb_serial_jtag_driver_install(&usb_serial_jtag_config), "JTAG driver install");
			usb_serial_jtag_vfs_use_driver();

			interaction_loop loop{};
			setup s{loop.stop(), wifi, mqtt_config, nvs, pn_spi};
			loop.set(s);
			loop.start();
		}

		cout << "connecting WiFi.." << endl;
		wifi_connection connection{wifi};
		connection.start();
		if (connection.is_up().wait_for(20s) != future_status::ready) throw runtime_error{"WiFi connection timeout"};

		cout << "connecting to MQTT broker.." << endl;
		const auto topic = mqtt_config.topic_root + '/' + nvs.get_str(app_storage::mqtt_topic_key);
		publisher publisher{mqtt_config.broker_host, topic, mqtt_config.ca_cert, mqtt_config.client_cert, mqtt_config.client_key};
		auto is_mqtt_connected = publisher.is_connected();
		if (is_mqtt_connected.wait_for(5s) != future_status::ready) throw runtime_error{"MQTT connection timeout"};

		cout << "setting up NFC chip.." << endl;

		AES piccMasterKey;
		AES doorlockMasterKey;
		AES doorlockWriteKey;
		AES doorlockReadKey;

		set_key(piccMasterKey, secrets::CARD_PICC_MASTER_AES_KEY, to_underlying(KeyVersionPicc::Master));
		set_key(doorlockMasterKey, secrets::CARD_DOORLOCK_MASTER_AES_KEY, to_underlying(KeyVersionDoorlock::Master));
		set_key(doorlockWriteKey, secrets::CARD_DOORLOCK_WRITE_AES_KEY, to_underlying(KeyVersionDoorlock::Write));
		set_key(doorlockReadKey, secrets::CARD_DOORLOCK_READ_AES_KEY, to_underlying(KeyNumberDoorlock::Read));

		Desfire des{pn_spi};
		des.SetDebugLevel(0);
		des.begin();

		if (!des.SetPassiveActivationRetries()) throw runtime_error{"error configuring card retries"};
		if (!des.SamConfig())  runtime_error{"could not configure card secure access"};

		cout << "waiting for card.." << endl;

		for (;;this_thread::sleep_for(500ms))
		{
			(cout << '.').flush();
			
			array<uint8_t, 8> uid;
			uint8_t uid_length {0};
			eCardType cardType {};

			if (!des.ReadPassiveTargetID(uid.data(), &uid_length, &cardType) || uid_length == 0) continue;

			if (!des.SelectApplication(to_underlying(ApplicationId::Picc)))
			{
				cout << "not a \"DESFire EV\" card" << endl;
				continue;
			}
			
			if (!des.Authenticate(to_underlying(KeyNumberPicc::Master), &piccMasterKey))
			{
				cout << "card protected with unknown master key" << endl;
				continue;
			}

			if (!des.SelectApplication(to_underlying(ApplicationId::Doorlock)))
			{
				cout << "no data present" << endl;
				continue;
			}

			if (!des.Authenticate(to_underlying(KeyNumberDoorlock::Read), &doorlockReadKey))
			{
				cout << "application protected with unknown key" << endl;
				continue;
			}

			array<uint8_t, static_cast<size_t>(FileSize::PublicuserId)> user_id;
			if (!des.ReadFileData(to_underlying(FileId::PublicUserId), 0, user_id.size(), user_id.data()))
			{
				cout << "could not read user ID" << endl;
				continue;
			}

			if (!publisher.publish(user_id)) throw runtime_error{"could not publish ID via MQTT"};
		}
	}
	catch(const exception &e)
	{
		cerr << e.what() << endl;
	}

	cout << "wait and reboot" << endl;
	esp_deep_sleep(4'000'000);
}
