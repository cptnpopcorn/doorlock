#ifndef A531B53C_D2D9_4579_BC7D_FD96F6E04A00
#define A531B53C_D2D9_4579_BC7D_FD96F6E04A00

#include "interaction.h"

class mqtt_config;
class nvs_access;
class wifi_station;

class mqtt_setup final : public interaction
{
public:
	mqtt_setup(
		interaction& quit,
		wifi_station& wifi,
		const mqtt_config& config,
		nvs_access& nvs) noexcept;

	void start(interaction_control&) override;

private:
	void show_config();
	void set_topic(const char* key);
	void test_publish();
	void test_subscribe();

	interaction& quit;
	wifi_station& wifi;
	const mqtt_config& config;
	nvs_access& nvs;
};

#endif /* A531B53C_D2D9_4579_BC7D_FD96F6E04A00 */
