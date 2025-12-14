#include "mqtt_config.h"

using namespace std;

mqtt_config::mqtt_config(
	const string &broker_host,
	const string &topic_root,
	const cert_t &ca_cert,
	const cert_t &client_cert,
	const cert_t &client_key) noexcept
: broker_host{broker_host}, topic_root{topic_root}, ca_cert{ca_cert}, client_cert{client_cert}, client_key{client_key}
{
}
