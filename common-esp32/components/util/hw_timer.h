#ifndef DA024529_66C8_432E_A3C0_9992D39060DB
#define DA024529_66C8_432E_A3C0_9992D39060DB

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <chrono>
#include <utility>

class HwTimer final
{
public:
	HwTimer();
	HwTimer(const HwTimer&) = delete;
	HwTimer(HwTimer&&) noexcept;

	HwTimer& operator =(const HwTimer&) = delete;
	HwTimer& operator =(HwTimer&&) noexcept;

	void Start(const std::chrono::microseconds& duration);
	void Wait();

	~HwTimer();

private:
	void swap(HwTimer&);
	static void callback(void*);
	void release();
	TaskHandle_t taskHandle;
    esp_timer_handle_t timerHandle;
};

#endif /* DA024529_66C8_432E_A3C0_9992D39060DB */
