#include "Pn532BeetleEsp32C6Spi.h"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <FrameParser.h>
#include <Timer.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <utility>

using namespace std;
using namespace std::chrono;

namespace Pins
{
	constexpr gpio_num_t Cs = GPIO_NUM_16;
	constexpr gpio_num_t Rstpd = GPIO_NUM_17;
	constexpr gpio_num_t Miso = GPIO_NUM_21;
	constexpr gpio_num_t Mosi = GPIO_NUM_22;
	constexpr gpio_num_t Sck = GPIO_NUM_23;
};

Pn532BeetleEsp32C6Spi::Pn532BeetleEsp32C6Spi(const chrono::milliseconds &timeout) :
	handle{nullptr},
	formatter{},
	timeout{timeout}
{
	gpio_config_t rst_conf {};
	rst_conf.pin_bit_mask = 1ULL << Pins::Rstpd;
	rst_conf.mode = GPIO_MODE_OUTPUT;
	rst_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	rst_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	rst_conf.intr_type = GPIO_INTR_DISABLE;

	if (gpio_config(&rst_conf) != ESP_OK) throw runtime_error{"bad GPIO parameters (RSTPD)"};

	gpio_set_level(Pins::Rstpd, 1);

	gpio_config_t chipselect_conf {};
	chipselect_conf.pin_bit_mask = 1ULL << Pins::Cs;
	chipselect_conf.mode = GPIO_MODE_OUTPUT;
	chipselect_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	chipselect_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	chipselect_conf.intr_type = GPIO_INTR_DISABLE;

	if (gpio_config(&chipselect_conf) != ESP_OK) throw runtime_error{"bad GPIO parameters (CS)"};

	gpio_set_level(Pins::Cs, 1);

	spi_bus_config_t spi_bus_config {};
	spi_bus_config.mosi_io_num = Pins::Mosi;
	spi_bus_config.miso_io_num = Pins::Miso;
	spi_bus_config.sclk_io_num = Pins::Sck;
	spi_bus_config.quadwp_io_num = -1;
	spi_bus_config.quadhd_io_num = -1;

	// DMA mode does not work for sending data from flash memory, and we have a couple of const octets we will send here and there
	if (spi_bus_initialize(SPI2_HOST, &spi_bus_config, SPI_DMA_CH_AUTO) != ESP_OK) throw runtime_error{"error configuring SPI"};

	spi_device_interface_config_t spi_dev_config {};
	spi_dev_config.mode = 0;
	spi_dev_config.clock_speed_hz = 100'000;
	spi_dev_config.spics_io_num = -1; // we will handle the chip select outselves, due to PN532 requirements for read / write and its latency
	spi_dev_config.queue_size = 4;
	spi_dev_config.flags = SPI_DEVICE_HALFDUPLEX;

	if (spi_bus_add_device(SPI2_HOST, &spi_dev_config, &handle) != ESP_OK) throw runtime_error{"could not add SPI device"};
}

void Pn532BeetleEsp32C6Spi::AssertResetAndPowerDown()
{
	gpio_set_level(Pins::Rstpd, 0);
}

void Pn532BeetleEsp32C6Spi::DeassertResetAndPowerUp()
{
	gpio_set_level(Pins::Rstpd, 1);
}

void Pn532BeetleEsp32C6Spi::StartDataTransport()
{
}

ControllerFrameWriter &Pn532BeetleEsp32C6Spi::WriteFrame()
{
	return *this;
}

class chip_select_guard final
{
public:
	chip_select_guard(Pn532BeetleEsp32C6Spi& spi) : spi{spi} { spi.AssertChipSelect(); }
	chip_select_guard(const chip_select_guard&) = delete;
	chip_select_guard(chip_select_guard&&) = delete;
	chip_select_guard& operator =(const chip_select_guard&) = delete;
	chip_select_guard& operator =(chip_select_guard&&) = delete;
	~chip_select_guard() { spi.DeassertChipSelect(); }

private:
	Pn532BeetleEsp32C6Spi& spi;
};

void Pn532BeetleEsp32C6Spi::Ack()
{
	chip_select_guard cs{*this};
	array<uint8_t, 1> dw {0b01};
	Write(dw);
	Write(formatter.GetAck());
}

void Pn532BeetleEsp32C6Spi::Nack()
{
	chip_select_guard cs{*this};
	array<uint8_t, 1> dw {0b01};
	Write(dw);
	Write(formatter.GetNack());
}

void Pn532BeetleEsp32C6Spi::DataFromHost(const std::span<uint8_t const> &data)
{
	chip_select_guard cs{*this};
	array<uint8_t, 1> dw {0b01};
	Write(dw);
	Write(formatter.GetDataHeader(data));
	Write(data);
	Write(formatter.GetDataTail(data));
}

bool Pn532BeetleEsp32C6Spi::ReadFrame(TargetFrameWriter &writer)
{
	Timer t{timeout};
	for (; t.IsRunning(); t.Update())
	{
		{
			chip_select_guard cs {*this};
			const array<uint8_t, 3> sr {0b10};
			Write(sr);
		}

		{
			array<uint8_t, 1> rdy {0};
			Read(rdy);
			if ((rdy[0] & 0b1) == 0b1) break;
		}

		this_thread::sleep_for(1ms);
	}

	if (!t.IsRunning()) return false;

	chip_select_guard cs {*this};
	const array<uint8_t, 1> dw {0b11};
	Write(dw);

	array<uint8_t, 128> buffer;
	span<uint8_t> bufferSpan {buffer};
	FrameParser parser {writer};

	while (true)
	{
		const auto length = min(parser.GetRequiredLength(), buffer.size());
		if (length == 0) return true;

		auto read = bufferSpan.subspan(0, length);
		Read(read);

		while (read.size() != 0)
		{
			read = read.subspan(parser.Parse(read));
		}
	}

	return false;
}

void Pn532BeetleEsp32C6Spi::AssertChipSelect()
{
	gpio_set_level(Pins::Cs, 0);
	this_thread::sleep_for(2ms);
}

void Pn532BeetleEsp32C6Spi::DeassertChipSelect()
{
	gpio_set_level(Pins::Cs, 1);
}

// SPI lib will use GDMA and not reorder bits, so we need to do it ourselves (BUG in ESP-IDF for ESP32-C6)

constexpr uint8_t reflect_bits(const uint8_t& b)
{
	const auto b1 = ((b << 1) & 0b10101010) | ((b >> 1) & 0b01010101);
	const auto b2 = ((b1 << 2) & 0b11001100) | ((b1 >> 2) & 0b00110011);
	return ((b2 << 4) & 0b11110000) | (b2 >> 4);
}

void reflect_bits(const span<const uint8_t> &src, const span<uint8_t> &dst)
{
	ranges::transform(src, dst.begin(), [](auto bits) { return reflect_bits(bits); });
}

void Pn532BeetleEsp32C6Spi::Write(const std::span<const uint8_t> &data)
{
	auto to_convert = data;
	array<uint8_t, 32> buffer;

	while (!to_convert.empty())
	{
		const auto slice = to_convert.subspan(0, min(to_convert.size(), buffer.size()));
		reflect_bits(slice, buffer);
		to_convert = to_convert.subspan(slice.size());

		spi_transaction_t description {};
		description.length = slice.size() * 8;
		description.tx_buffer = buffer.data();

		if (spi_device_transmit(handle, &description) != ESP_OK) throw runtime_error{"SPI write error"};
	}
}

void Pn532BeetleEsp32C6Spi::Read(const std::span<uint8_t> &data)
{
	auto to_convert = data;
	array<uint8_t, 32> buffer;

	while (!to_convert.empty())
	{
		const span<uint8_t> slice{buffer.begin(), min(to_convert.size(), buffer.size())};
		spi_transaction_t description {};
		description.rxlength = slice.size() * 8;
		description.rx_buffer = slice.data();

		if (spi_device_transmit(handle, &description) != ESP_OK) throw runtime_error{"SPI read error"};

		reflect_bits(slice, to_convert);
		to_convert = to_convert.subspan(0, slice.size());
	}
}

Pn532BeetleEsp32C6Spi::~Pn532BeetleEsp32C6Spi()
{
	// doesn't really help to handle errors here
	if (handle != nullptr) spi_bus_remove_device(handle);
	spi_bus_free(SPI2_HOST);
}
