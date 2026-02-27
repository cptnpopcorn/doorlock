#ifndef E5647301_C632_40BE_9C6F_3499693820FE
#define E5647301_C632_40BE_9C6F_3499693820FE

#include <esp_event.h>
#include <driver/gpio.h>

class RelayController final
{
public:
	RelayController(TickType_t duration, gpio_num_t pin);
	void Close();
	~RelayController();

private:
	void WaitForClose();

	EventGroupHandle_t events;
	TickType_t duration;
	gpio_num_t pin;
};

#endif /* E5647301_C632_40BE_9C6F_3499693820FE */
