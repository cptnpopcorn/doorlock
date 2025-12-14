#include <Desfire.h>
#include <Pn532BeetleEsp32C6Spi.h>

#include <soc/spi_reg.h>
#include <driver/usb_serial_jtag.h>
#include <driver/usb_serial_jtag_vfs.h>
#include <esp_event.h>

#include "card_layout.h"
#include "error.h"
#include "interaction_loop.h"
#include "mqtt_config.h"
#include "nvs_access.h"
#include "secrets.h"
#include "setup.h"
#include "wifi_station.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <system_error>
#include <thread>

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

extern "C" void app_main(void)
{
	try
	{
		check(esp_event_loop_create_default());

		nvs_access nvs{"cardreader"};
		wifi_station wifi{nvs};
		
		if (usb_serial_jtag_is_connected())
		{
			cout << "USB console connected, entering interactive mode.." << endl;

			setvbuf(stdin, nullptr, _IONBF, 0);
			usb_serial_jtag_driver_config_t usb_serial_jtag_config{.tx_buffer_size = 1024, .rx_buffer_size = 1024};
			check(usb_serial_jtag_driver_install(&usb_serial_jtag_config), "JTAG driver install");
			usb_serial_jtag_vfs_use_driver();

			const string mqtt_broker_hostname{CONFIG_MQTT_BROKER_HOSTNAME};
			const string mqtt_topic_root{CONFIG_MQTT_TOPIC_ROOT};
			const mqtt_config mqtt_config{mqtt_broker_hostname, mqtt_topic_root, get_ca_crt(), get_client_crt(), get_client_key()};

			interaction_loop loop{};
			setup s{loop.stop(), wifi, mqtt_config, nvs};
			loop.set(s);
			loop.start();
		}

		cout << "setting up PN532 SPI communication.." << endl;
		Pn532BeetleEsp32C6Spi pn_spi{20ms};

		while (true)
		{
			Desfire des{pn_spi};
			des.begin();

			cout << "waiting for 5 s" << endl;
			this_thread::sleep_for(5s);
			cout << "go" << endl;

			try
			{
				uint8_t icType {};
				uint8_t versionHi{};
				uint8_t versionLo{};
				uint8_t flags;
				if (des.GetFirmwareVersion(&icType, &versionHi, &versionLo, &flags))
				{
					cout << "IC Type " << (int)icType << endl;
					cout << "Version maj " << static_cast<int>(versionHi) << endl;
					cout << "Version min " << static_cast<int>(versionLo) << endl;
					cout << "Flags " << flags << endl;
				}
				else
				{
					cout << "error reading FW version" << endl;
				}
			}
			catch(const exception& e)
			{
				cout << e.what() << endl;
				return;
			}

			cout << "releasing" << endl;
			this_thread::sleep_for(1s);
		}
	}
	catch(const exception &e)
	{
		cerr << e.what() << endl;
	}

	cout << "ended" << endl;
}
