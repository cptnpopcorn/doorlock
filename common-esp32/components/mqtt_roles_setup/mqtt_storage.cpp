#include "mqtt_storage.h"
#include <nvs_access.h>

const char *mqtt_storage::mqtt_site_key{"site"};
const char *mqtt_storage::mqtt_room_key{"room"};
const char *mqtt_storage::mqtt_door_key{"door"};

mqtt_topic mqtt_storage::read_topic(nvs_access& nvs)
{
	return {
		nvs.get_str(mqtt_site_key),
		nvs.get_str(mqtt_room_key),
		nvs.get_str(mqtt_door_key)};
}
