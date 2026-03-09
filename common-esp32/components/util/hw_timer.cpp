#include "hw_timer.h"
#include <algorithm>
#include <system_error>

using namespace std;

HwTimer::HwTimer() : taskHandle{xTaskGetCurrentTaskHandle()}, timerHandle{nullptr}
{
	esp_timer_create_args_t args{};
	args.callback = &HwTimer::callback;
	args.arg = this;
	args.name = "util_timer";

	if (esp_timer_create(&args, &timerHandle) != ESP_OK) throw runtime_error{"could not create timer"};
}

HwTimer::HwTimer(HwTimer&& other) noexcept
{
	swap(other);
}

HwTimer& HwTimer::operator=(HwTimer &&other) noexcept
{
	if (this != &other)
	{
		release();
		swap(other);
	}

	return *this;
}

void HwTimer::Start(const std::chrono::microseconds &duration)
{
	if (esp_timer_start_once(timerHandle, duration.count()) != ESP_OK) throw runtime_error{"could not start timer"};
}

void HwTimer::Wait()
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

HwTimer::~HwTimer()
{
	release();
}

void HwTimer::swap(HwTimer &other)
{
	std::swap(taskHandle, other.taskHandle);
	std::swap(timerHandle, other.timerHandle);
}

void HwTimer::callback(void *arg)
{
	auto self = static_cast<HwTimer*>(arg);
	BaseType_t woken = pdFALSE;
	vTaskNotifyGiveFromISR(self->taskHandle, &woken);
	portYIELD_FROM_ISR();
}

void HwTimer::release()
{
	if (timerHandle == nullptr) return;

	esp_timer_stop(timerHandle);
	esp_timer_delete(timerHandle);
	timerHandle = nullptr;
}
