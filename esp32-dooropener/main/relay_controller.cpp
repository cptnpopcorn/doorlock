#include "relay_controller.h"
#include "bounce.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <system_error>
#include <iostream>

using namespace std;

RelayController::RelayController(TickType_t duration, gpio_num_t pin) : events{xEventGroupCreate()}, duration{duration}, pin{pin}
{
	if (xTaskCreate(BOUNCE(RelayController, WaitForClose), "relay controller", 4096, this, tskIDLE_PRIORITY, nullptr) != pdPASS)
	{
		throw runtime_error { "could not create relay controller task" };
	}

	gpio_config_t pin_conf {};
	pin_conf.pin_bit_mask = 1ULL << pin;
	pin_conf.mode = GPIO_MODE_OUTPUT;
	pin_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	pin_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	pin_conf.intr_type = GPIO_INTR_DISABLE;

	if (gpio_config(&pin_conf) != ESP_OK) throw runtime_error{"bad GPIO pin (OPEN)"};

	gpio_set_level(pin, 0);
}

static constexpr uint32_t RELAY_CONTROLLER_EVENT_CLOSE { 1 << 0 };
static constexpr uint32_t RELAY_CONTROLLER_EVENT_STOP { 1 << 1 };
static constexpr uint32_t RELAY_CONTROLLER_EVENT_STOPPED { 1 << 2 };

void RelayController::Close()
{
	xEventGroupSetBits(events, RELAY_CONTROLLER_EVENT_CLOSE);
}

RelayController::~RelayController()
{
	xEventGroupSetBits(events, RELAY_CONTROLLER_EVENT_STOP);
	xEventGroupWaitBits(events, RELAY_CONTROLLER_EVENT_STOPPED, pdTRUE, pdFALSE, portMAX_DELAY);
}

void RelayController::WaitForClose()
{
	for (;;)
	{
		const auto bits = xEventGroupWaitBits(
			events,
			RELAY_CONTROLLER_EVENT_CLOSE | RELAY_CONTROLLER_EVENT_STOP,
			pdTRUE,
			pdFALSE,
			portMAX_DELAY);

		if ((bits & RELAY_CONTROLLER_EVENT_STOP) != 0)
		{
			xEventGroupSetBits(events, RELAY_CONTROLLER_EVENT_STOPPED);
			vTaskDelete(nullptr);
			return;
		}

		if ((bits & RELAY_CONTROLLER_EVENT_CLOSE) != 0)
		{
			gpio_set_level(pin, 1);
			xEventGroupWaitBits(events, RELAY_CONTROLLER_EVENT_STOP, pdFALSE, pdFALSE, duration);
			gpio_set_level(pin, 0);
		}
	}
}