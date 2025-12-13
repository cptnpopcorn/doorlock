#ifndef DC589AC8_033F_41DE_97DC_25696F1C71F7
#define DC589AC8_033F_41DE_97DC_25696F1C71F7

#include <interaction.h>

class wifi_station;

class wifi_setup final : public interaction
{
public:
	wifi_setup(interaction& quit, wifi_station& wifi) noexcept;
	void start(interaction_control&) override;

private:
	void show_config();
	void select_ap();
	void test_connect();

	interaction& quit;
	wifi_station& wifi;
};

class setup final : public interaction
{
public:
	setup(interaction& quit, wifi_station& wifi) noexcept;
	void start(interaction_control&) override;

private:
	interaction &quit;
	wifi_setup wifi;
};

#endif /* DC589AC8_033F_41DE_97DC_25696F1C71F7 */
