#ifndef E4065226_BD93_4A94_B8B1_58AF6074727E
#define E4065226_BD93_4A94_B8B1_58AF6074727E

#include <mqtt_topic.h>

class nvs_access;

class app_storage final
{
public:

	static mqtt_topic read_topic(nvs_access& nvs);

	static const char *mqtt_site_key;
	static const char *mqtt_room_key;
	static const char *mqtt_door_key;
};

#endif /* E4065226_BD93_4A94_B8B1_58AF6074727E */
