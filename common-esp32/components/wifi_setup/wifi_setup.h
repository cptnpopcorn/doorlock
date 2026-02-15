#ifndef FD00F87D_DE7E_406F_ACE9_0EF257888BD7
#define FD00F87D_DE7E_406F_ACE9_0EF257888BD7

#include "interaction.h"

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


#endif /* FD00F87D_DE7E_406F_ACE9_0EF257888BD7 */
