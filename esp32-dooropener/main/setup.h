#ifndef B15E82B3_0118_4B54_AB70_45B5D0DA522A
#define B15E82B3_0118_4B54_AB70_45B5D0DA522A

#include "interaction.h"
#include "mqtt_setup.h"
#include "wifi_setup.h"

class setup final : public interaction
{
public:
	setup(
		interaction& quit,
		wifi_station& wifi,
		const mqtt_config& mqtt_config,
		nvs_access& nvs) noexcept;

	void start(interaction_control&) override;

private:
	void restart();
	
	interaction &quit;
	wifi_setup wifi;
	mqtt_setup mqtt;
};

#endif /* B15E82B3_0118_4B54_AB70_45B5D0DA522A */
