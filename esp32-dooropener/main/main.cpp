#include <driver/usb_serial_jtag.h>
#include <driver/usb_serial_jtag_vfs.h>
#include <esp_sleep.h>

#include "error.h"
#include "interaction_loop.h"
#include "mqtt_config.h"
#include "mqtt_storage.h"
#include "mqtt_wrapper.h"
#include "nvs_access.h"
#include "secrets.h"
#include "wifi_connection.h"
#include "wifi_station.h"
#include "const_stream.h"
#include "open_message_builder.h"
#include "setup.h"

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <system_error>
#include <thread>
#include <utility>

using namespace std;

#ifdef CONFIG_MQTT_USE_TLS
span<const uint8_t> get_ca_crt()
{
    return secrets::MQTT_CA_CERT;
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
    span<const uint8_t> get_client_crt()
    {
        return secrets::MQTT_CLIENT_CERT;
    }
    span<const uint8_t> get_client_key()
    {
        return secrets::MQTT_CLIENT_KEY;
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

		nvs_access nvs{"dooropener"};
		wifi_station wifi{nvs};

		const mqtt_config mqtt_config{
			CONFIG_MQTT_BROKER_HOSTNAME,
			get_ca_crt(),
			get_client_crt(),
			get_client_key()};

		if (usb_serial_jtag_is_connected())
		{
			this_thread::sleep_for(10s); // allows to quickly open a terminal, after plugging in USB
			cout << "USB console connected, entering interactive mode.." << endl;

			setvbuf(stdin, nullptr, _IONBF, 0);
			usb_serial_jtag_driver_config_t usb_serial_jtag_config{.tx_buffer_size = 1024, .rx_buffer_size = 1024};
			check(usb_serial_jtag_driver_install(&usb_serial_jtag_config), "JTAG driver install");
			usb_serial_jtag_vfs_use_driver();

			interaction_loop loop{};
			setup s{loop.stop(), wifi, mqtt_config, nvs, static_cast<gpio_num_t>(CONFIG_RELAY_GPIO_PIN)};
			loop.set(s);
			loop.start();
		}

		// GPIO relay operation
		cout << "starting relay controller.." << endl;
		RelayController relay {pdMS_TO_TICKS(2'000), static_cast<gpio_num_t>(CONFIG_RELAY_GPIO_PIN)};

		cout << "connecting WiFi.." << endl;
		wifi_connection connection{wifi};
		connection.start();
		if (!connection.wait_is_up(pdMS_TO_TICKS(20'000))) throw runtime_error{"WiFi connection timeout"};

		cout << "connecting to MQTT broker " << mqtt_config.broker_host << ".." << endl;
		const auto topic = mqtt_storage::read_topic(nvs);
		mqtt_wrapper listener{mqtt_config.broker_host, topic.door_opener_str(), mqtt_config.ca_cert, mqtt_config.client_cert, mqtt_config.client_key};
		if (!listener.wait_is_connected(pdMS_TO_TICKS(5'000))) throw runtime_error{"MQTT connection timeout"};

		cout << "subscribing to topic " << topic.door_opener_str() << endl;
		if (!listener.subscribe([&relay](const auto& data)
		{
			OpenMessageBuilder message{};
			ConstStream stream{data};
			try
			{
				rapidjson::Reader{}.Parse(stream, message);
				if (message.is_valid())
				{
					cout << (message.get_open() ? "open" : "do not open") << " for " << message.get_user() << endl;
					relay.Close();
				}
				else
				{
					cout << "invalid message format" << endl;
				}
			}
			catch (const exception& e)
			{
				cout << "malformatted open message" << endl << e.what() << endl;
			}
		}))
		{
			throw runtime_error {"MQTT subscription failed"};
		}

		cout << "waiting for messages.." << endl;
		listener.wait_is_disconnected(portMAX_DELAY);
		cout << "MQTT disconnected" << endl;
	}
	catch (const exception &e)
	{
		cerr << e.what() << endl;
	}

	cout << "wait and reboot" << endl;
	esp_deep_sleep(4'000'000);
}