#include "app_storage.h"
#include <nvs_access.h>

const char *app_storage::mqtt_site_key{"site"};
const char *app_storage::mqtt_room_key{"room"};
const char *app_storage::mqtt_door_key{"door"};

mqtt_topic app_storage::read_topic(nvs_access& nvs)
{
	return {
		nvs.get_str(mqtt_site_key),
		nvs.get_str(mqtt_room_key),
		nvs.get_str(mqtt_door_key)};
}
