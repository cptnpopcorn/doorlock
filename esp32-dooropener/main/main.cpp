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

class ConstStream final
{
public:
	ConstStream(const span<const uint8_t>& chars) noexcept : i{0}, chars{chars} {}

	typedef char Ch;
	Ch Peek() const { return static_cast<char>(chars[i]); }
	Ch Take() { return static_cast<char>(chars[i++]); }
	size_t Tell() const { return chars.size(); }
	Ch* PutBegin() { assert(false); return 0; }
 	void Put(Ch) { assert(false); }
 	void Flush() { assert(false); }
 	size_t PutEnd(Ch*) { assert(false); return 0; }

private:
	size_t i;
	const span<const uint8_t>& chars;
};

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
			setup s{loop.stop(), wifi, mqtt_config, nvs};
			loop.set(s);
			loop.start();
		}

		cout << "connecting WiFi.." << endl;
		wifi_connection connection{wifi};
		connection.start();
		if (!connection.wait_is_up(pdMS_TO_TICKS(20'000))) throw runtime_error{"WiFi connection timeout"};

		cout << "connecting to MQTT broker " << mqtt_config.broker_host << ".." << endl;
		const auto topic = mqtt_storage::read_topic(nvs);
		mqtt_wrapper listener{mqtt_config.broker_host, topic.door_opener_str(), mqtt_config.ca_cert, mqtt_config.client_cert, mqtt_config.client_key};
		auto is_mqtt_connected = listener.is_connected();
		if (is_mqtt_connected.wait_for(5s) != future_status::ready) throw runtime_error{"MQTT connection timeout"};

		cout << "subscribing to topic " << topic.door_opener_str() << endl;
		if (!listener.subscribe([](const auto& data)
		{
			// TODO:
			string_view text(reinterpret_cast<const char*>(data.data()), data.size());
			cout << "received message: " << text << endl;

			OpenMessageBuilder message{};
			ConstStream stream{data};
			try
			{
				rapidjson::Reader{}.Parse(stream, message);
				if (message.is_valid())
				{
					// TODO: pure debug, need to notify timer task that operates GPIO
					cout << (message.get_open() ? "open" : "do not open") << " for " << message.get_user() << endl;
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

		cout << "waiting for messages" << endl;

		while (true) this_thread::sleep_for(10ms);
		//auto is_mqtt_disconnected = listener.is_connected();
		//is_mqtt_disconnected.wait();

		cout << "MQTT disconnected" << endl;
	}
	catch (const exception &e)
	{
		cerr << e.what() << endl;
	}

	cout << "wait and reboot" << endl;
	esp_deep_sleep(4'000'000);
}