#ifndef C594DC41_0F5E_4B5B_B0B4_094AA0EAEFAB
#define C594DC41_0F5E_4B5B_B0B4_094AA0EAEFAB

#include <string>

class mqtt_topic final
{
public:
	mqtt_topic(
		const std::string &topic_site,
		const std::string &topic_room,
		const std::string &topic_door);

	std::string str() const;

	const std::string topic_site;
	const std::string topic_room;
	const std::string topic_door;
};

#endif /* C594DC41_0F5E_4B5B_B0B4_094AA0EAEFAB */
