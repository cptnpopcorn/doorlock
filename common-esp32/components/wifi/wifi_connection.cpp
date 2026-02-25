#include "wifi_connection.h"

#include <error.h>
#include <esp_wifi.h>

using namespace std;

wifi_connection::wifi_connection(wifi_station& sta) : events{xEventGroupCreate()}, sta{sta}
{
	wifi_event_handle = register_event(
		WIFI_EVENT,
		ESP_EVENT_ANY_ID,
		this,
		BOUNCE(wifi_connection, handle_wifi_event));

	ip_event_handle = register_event(
		IP_EVENT,
		IP_EVENT_STA_GOT_IP,
		this,
		BOUNCE(wifi_connection, handle_ip_event));
}

void wifi_connection::start()
{
	check(esp_wifi_start(), "WiFi start");
}

void wifi_connection::handle_wifi_event(esp_event_base_t base, int32_t id, void* data)
{
	switch (id)
	{
		case WIFI_EVENT_STA_START:
			check(esp_wifi_connect(), "WiFi connect");
			return;
	}
}

static constexpr uint32_t WIFI_CONN_EVENT_UP { 1 << 0 };

void wifi_connection::handle_ip_event(esp_event_base_t base, int32_t id, void* data)
{
	xEventGroupSetBits(events, WIFI_CONN_EVENT_UP);
}

bool wifi_connection::wait_is_up(TickType_t timeout)
{
	return xEventGroupWaitBits(events, WIFI_CONN_EVENT_UP, pdTRUE, pdFALSE, timeout) == WIFI_CONN_EVENT_UP;
}

wifi_connection::~wifi_connection()
{
	check(esp_wifi_stop(), "WiFi stop");
	vEventGroupDelete(events);
}
