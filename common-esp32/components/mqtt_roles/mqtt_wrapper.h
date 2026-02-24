#ifndef AA38B310_1E68_4268_8387_AA6266CDEAFC
#define AA38B310_1E68_4268_8387_AA6266CDEAFC

#include <mqtt_event_handle.h>

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <span>
#include <string>

class mqtt_wrapper final {
public:
	mqtt_wrapper(
		const std::string& broker_host,
		const std::string& topic,
		const std::span<const uint8_t>& ca_cert,
		const std::span<const uint8_t>& client_cert,
		const std::span<const uint8_t>& client_key);

	std::future<void> is_connected() noexcept;
	bool publish(const std::span<const uint8_t>&);
	bool subscribe(std::function<void(const std::span<const uint8_t>&)> receive);
	std::future<void> is_disconnected() noexcept;

	~mqtt_wrapper();

private:
	void handle_event(esp_event_base_t base, int32_t id, void* data);

	std::promise<void> connected;
	std::promise<void> disconnected;

	esp_mqtt_client_handle_t client;
	mqtt_event_handle event_handle;
	std::string topic;
	std::mutex tx_lock;
	std::condition_variable tx_cond;
	int publish_id;
	std::function<void(const std::span<const uint8_t>&)> receive;
	int subscribe_id;
};

#endif /* AA38B310_1E68_4268_8387_AA6266CDEAFC */
