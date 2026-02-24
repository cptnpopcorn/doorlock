#include "mqtt_topic.h"

using namespace std;

mqtt_topic::mqtt_topic(const string &topic_site, const string &topic_room, const string &topic_door) :
	topic_site{topic_site}, topic_room{topic_room}, topic_door{topic_door}
{
}

std::string mqtt_topic::card_reader_str() const
{
	return "site/" + topic_site + "/room/" + topic_room + "/door/" + topic_door + "/cardreader/card-presented";
}

std::string mqtt_topic::door_opener_str() const
{
	return "site/" + topic_site + "/room/" + topic_room + "/door/" + topic_door + "/doorcontroller/open";
}
