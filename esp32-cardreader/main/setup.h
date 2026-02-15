#ifndef DC589AC8_033F_41DE_97DC_25696F1C71F7
#define DC589AC8_033F_41DE_97DC_25696F1C71F7

#include "interaction.h"
#include "mqtt_setup.h"
#include "wifi_setup.h"

class PN532Interface;

class setup final : public interaction
{
public:
	setup(
		interaction& quit,
		wifi_station& wifi,
		const mqtt_config& mqtt_config,
		nvs_access& nvs,
		PN532Interface& card_interface) noexcept;

	void start(interaction_control&) override;

private:
	void read_card();
	void restart();

	interaction &quit;
	wifi_setup wifi;
	mqtt_setup mqtt;
	PN532Interface& card_interface;
};

#endif /* DC589AC8_033F_41DE_97DC_25696F1C71F7 */
