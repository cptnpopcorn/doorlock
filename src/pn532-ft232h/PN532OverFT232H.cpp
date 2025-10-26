#include "PN532OverFT232H.h"
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

PN532OverFT232H::PN532OverFT232H(
	void* ftHandle,
	uint8_t i2cAddr,
	int gpioResetPinIndex,
	const std::chrono::milliseconds& timeout) noexcept :
	ft232h{ftHandle, i2cAddr, static_cast<uint8_t>(1 << gpioResetPinIndex), timeout},
	formatter{},
	timeout{timeout}
{
}

void PN532OverFT232H::AssertResetAndPowerDown()
{
	ft232h.WriteGPIO(0b11111111);
}

void PN532OverFT232H::DeassertResetAndPowerUp()
{
	ft232h.WriteGPIO(0b00000000);
}

void PN532OverFT232H::StartDataTransport()
{
}

ControllerFrameWriter &PN532OverFT232H::WriteFrame()
{
	return *this;
}

void PN532OverFT232H::Ack()
{	
	ft232h.WriteI2C(formatter.GetAck(), I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK | I2C_TRANSFER_OPTIONS_STOP_BIT);
}

void PN532OverFT232H::Nack()
{
	ft232h.WriteI2C(formatter.GetNack(), I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK | I2C_TRANSFER_OPTIONS_STOP_BIT);
}

void PN532OverFT232H::DataFromHost(const std::span<uint8_t const> &data)
{
	const auto header = formatter.GetDataHeader(data);
	if (ft232h.WriteI2C(header, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK) != header.size())
	{
		throw runtime_error {"data header NACKed"}; // TODO: try again a couple of times ? but how bad could our link be that retry will suddenly work again ?
	}

	if (ft232h.WriteI2C(data, I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK) != data.size())
	{
		throw runtime_error {"payload NACked"};
	}

	const auto tail = formatter.GetDataTail(data);
	if (ft232h.WriteI2C(tail, I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK | I2C_TRANSFER_OPTIONS_STOP_BIT) != tail.size())
	{
		throw runtime_error {"data trailer NACked"};
	}
}

class stop_read_guard final
{
public:
	stop_read_guard(Ft232h &ft232h) noexcept : ft232h{ft232h} {}
	stop_read_guard(const stop_read_guard&) = delete;
	stop_read_guard(stop_read_guard&&) = delete;
	stop_read_guard& operator =(const stop_read_guard&) = delete;
	stop_read_guard& operator =(stop_read_guard&&) = delete;

	~stop_read_guard()
	{
		array<uint8_t, 0> none;
		ft232h.ReadI2C(none, I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_STOP_BIT);
	}

private:
	Ft232h &ft232h;
};

void PN532OverFT232H::ReadFrame(TargetFrameWriter &writer)
{
	uint8_t rdy {0b0};
	Timer t {timeout};

	for (; t.IsRunning(); t.Update())
	{
		const auto read = ft232h.ReadI2C(span<uint8_t>(&rdy, 1), I2C_TRANSFER_OPTIONS_START_BIT);
		if (read == 1 && (rdy & 0b1) == 0b1) break;

		ft232h.ReadI2C(span<uint8_t>(&rdy, 0), I2C_TRANSFER_OPTIONS_NO_ADDRESS | I2C_TRANSFER_OPTIONS_STOP_BIT);
	}	

	stop_read_guard stop_read(ft232h);
	if (!t.IsRunning()) throw runtime_error {"FT232H read timeout (not ready)"};
	
	array<uint8_t, 128> buffer;
	span<uint8_t> bufferSpan {buffer};
	FrameParser parser {writer};

	while (true)
	{
		const auto length = min(parser.GetRequiredLength(), buffer.size());
		if (length == 0) return;

		auto read = bufferSpan.subspan(0, ft232h.ReadI2C(bufferSpan.subspan(0, length), I2C_TRANSFER_OPTIONS_NO_ADDRESS));
		if (read.size() != length) return;

		while (read.size() != 0)
		{
			read = read.subspan(parser.Parse(read));
		}
	}
}
