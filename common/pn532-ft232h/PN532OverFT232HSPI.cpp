#include "PN532OverFT232HSPI.h"
#include <FrameParser.h>
#include <Timer.h>
#include <span>
#include <thread>

using namespace std;
using namespace std::chrono;

PN532OverFT232HSPI::PN532OverFT232HSPI(
	void *ftHandle,
	int gpioResetPinIndex,
	const milliseconds& timeout) noexcept :
	spi{ftHandle, 2ms, static_cast<uint8_t>(1 << gpioResetPinIndex)},
	formatter{},
	timeout{timeout}
{
}

void PN532OverFT232HSPI::AssertResetAndPowerDown()
{
	spi.WriteGPIO(0b11111111);
}

void PN532OverFT232HSPI::DeassertResetAndPowerUp()
{
	spi.WriteGPIO(0b00000000);
}

void PN532OverFT232HSPI::StartDataTransport()
{
}

ControllerFrameWriter &PN532OverFT232HSPI::WriteFrame()
{
	return *this;
}

void PN532OverFT232HSPI::Ack()
{
	array<uint8_t, 1> dw {0b01};
	spi.Write(dw, true, false);
	spi.Write(formatter.GetAck(), false, true);
}

void PN532OverFT232HSPI::Nack()
{
	array<uint8_t, 1> dw {0b01};
	spi.Write(dw, true, false);
	spi.Write(formatter.GetNack(), false, true);
}

class chip_select_guard final
{
public:
	chip_select_guard(Ft232hSpi &spi) noexcept : spi{spi} {}
	chip_select_guard(const chip_select_guard&) = delete;
	chip_select_guard(chip_select_guard&&) = delete;
	chip_select_guard& operator =(const chip_select_guard&) = delete;
	chip_select_guard& operator =(chip_select_guard&&) = delete;

	void disarm() { is_disarmed = true; }

	~chip_select_guard()
	{
		if (!is_disarmed) spi.DeassertChipSelect();
	}

private:
	Ft232hSpi &spi;
	bool is_disarmed {false};
};

void PN532OverFT232HSPI::DataFromHost(const std::span<uint8_t const> &data)
{
	array<uint8_t, 1> dw {0b01};
	spi.Write(dw, true, false);

	const auto header = formatter.GetDataHeader(data);
	const auto header_written = spi.Write(header, false, false);
	if (header_written != header.size()) throw runtime_error {"data header write error"};

	chip_select_guard deselect {spi};

	const auto data_written = spi.Write(data, false, false);
	if (data_written != data.size()) throw runtime_error {"payload write error"};

	const auto tail = formatter.GetDataTail(data);
	const auto tail_written = spi.Write(tail, false, true);
	if (tail_written != tail.size()) throw runtime_error {"tail write error"};

	deselect.disarm();
}

bool PN532OverFT232HSPI::ReadFrame(TargetFrameWriter &writer)
{
	Timer t{timeout};
	for (; t.IsRunning(); t.Update())
	{
		const array<uint8_t, 3> sr {0b10};
		if (spi.Write(sr) != sr.size()) throw runtime_error {"SR write error"};

		array<uint8_t, 1> rdy {0};
		if (spi.Read(rdy) != rdy.size()) throw runtime_error {"RDY read error"};
		if ((rdy[0] & 0b1) == 0b1) break;

		this_thread::sleep_for(1ms);
	}

	if (!t.IsRunning()) return false;

	const array<uint8_t, 1> dw {0b11};
	if (spi.Write(dw, true, false) != dw.size()) throw runtime_error {"DW write error"};

	chip_select_guard deselect {spi};
	array<uint8_t, 128> buffer;
	span<uint8_t> bufferSpan {buffer};
	FrameParser parser {writer};

	while (true)
	{
		const auto length = min(parser.GetRequiredLength(), buffer.size());
		if (length == 0) return true;

		auto read = bufferSpan.subspan(0, spi.Read(bufferSpan.subspan(0, length), false, false));
		if (read.size() != length) return false;

		while (read.size() != 0)
		{
			read = read.subspan(parser.Parse(read));
		}
	}

	return false;
}
