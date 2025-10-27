#include "PN532OverFT232HI2C.h"
#include "Timer.h"

#include <FrameParser.h>
#include <NullTargetDataWriter.h>
#include <TargetFrameWriter.h>
#include <ftd2xx.h>
#include <libmpsse_i2c.h>

#include <algorithm>
#include <array>
#include <thread>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std;
using namespace std::chrono;

PN532OverFT232HI2C::PN532OverFT232HI2C(
	void* ftHandle,
	uint8_t i2cAddr,
	int gpioResetPinIndex,
	const std::chrono::milliseconds& timeout) noexcept :
	i2c{ftHandle, i2cAddr, static_cast<uint8_t>(1 << gpioResetPinIndex), timeout},
	formatter{},
	timeout{timeout}
{
}

void PN532OverFT232HI2C::AssertResetAndPowerDown()
{
	i2c.WriteGPIO(0b11111111);
}

void PN532OverFT232HI2C::DeassertResetAndPowerUp()
{
	i2c.WriteGPIO(0b00000000);
}

void PN532OverFT232HI2C::StartDataTransport()
{
}

ControllerFrameWriter &PN532OverFT232HI2C::WriteFrame()
{
	return *this;
}

void PN532OverFT232HI2C::Ack()
{
	i2c.Write(formatter.GetAck(), I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK | I2C_TRANSFER_OPTIONS_STOP_BIT);
}

void PN532OverFT232HI2C::Nack()
{
	i2c.Write(formatter.GetNack(), I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK | I2C_TRANSFER_OPTIONS_STOP_BIT);
}

void PN532OverFT232HI2C::DataFromHost(const std::span<uint8_t const> &data)
{
	const auto header = formatter.GetDataHeader(data);
	const auto header_written = i2c.Write(header, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
	if (header_written != header.size())
	{
		throw runtime_error {"data header NACKed"}; // TODO: try again a couple of times ? but how bad could our link be that retry will suddenly work again ?
	}

	const auto data_written = i2c.Write(data, I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
	if (data_written != data.size())
	{
		i2c.Write({}, I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_STOP_BIT);
		throw runtime_error {"payload NACked"};
	}

	const auto tail = formatter.GetDataTail(data);
	const auto tail_written = i2c.Write(tail, I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK | I2C_TRANSFER_OPTIONS_STOP_BIT);
	if (tail_written != tail.size())
	{
		throw runtime_error {"data trailer NACked"};
	}
}

class chip_deselect_guard final
{
public:
	chip_deselect_guard(Ft232hI2c &i2c) noexcept : i2c{i2c} {}
	chip_deselect_guard(const chip_deselect_guard&) = delete;
	chip_deselect_guard(chip_deselect_guard&&) = delete;
	chip_deselect_guard& operator =(const chip_deselect_guard&) = delete;
	chip_deselect_guard& operator =(chip_deselect_guard&&) = delete;

	~chip_deselect_guard()
	{
		uint8_t dummy {};
		i2c.Read(span{&dummy, 1}, I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_STOP_BIT);
	}

private:
	Ft232hI2c &i2c;
};

bool PN532OverFT232HI2C::ReadFrame(TargetFrameWriter &writer)
{
	uint8_t rdy {0b0};
	Timer t {timeout};

	for (; t.IsRunning(); t.Update())
	{
		const auto read = i2c.Read(span<uint8_t>(&rdy, 1), I2C_TRANSFER_OPTIONS_START_BIT);
		if (read == 1 && (rdy & 0b1) == 0b1) break;

		i2c.Read(span<uint8_t>(&rdy, 0), I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_STOP_BIT);
		this_thread::sleep_for(5ms);
	}	

	chip_deselect_guard stop_read(i2c);
	if (!t.IsRunning()) return false;

	this_thread::sleep_for(2ms);
	
	array<uint8_t, 128> buffer;
	span<uint8_t> bufferSpan {buffer};
	FrameParser parser {writer};

	while (true)
	{
		const auto length = min(parser.GetRequiredLength(), buffer.size());
		if (length == 0) return true;

		auto read = bufferSpan.subspan(0, i2c.Read(bufferSpan.subspan(0, length), I2C_TRANSFER_OPTIONS_NO_ADDRESS));
		if (read.size() != length) return false;

		while (read.size() != 0)
		{
			read = read.subspan(parser.Parse(read));
		}
	}

	return true;
}
