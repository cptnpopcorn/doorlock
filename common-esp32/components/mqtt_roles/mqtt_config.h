#ifndef B8BEEF90_CEAA_4480_A3C6_5D47DDBDD88D
#define B8BEEF90_CEAA_4480_A3C6_5D47DDBDD88D

#include <cstdint>
#include <span>
#include <string>

class mqtt_config final
{
public:
	using cert_t = std::span<const uint8_t>;

	mqtt_config(
		const std::string &broker_host,
		const cert_t &ca_cert,
		const cert_t &client_cert,
		const cert_t &client_key) noexcept;

	const std::string broker_host;
	const cert_t ca_cert;
	// those are either both present or both not present, to use client auth
	const cert_t client_cert;
	const cert_t client_key;
};

#endif /* B8BEEF90_CEAA_4480_A3C6_5D47DDBDD88D */