#ifndef CF853CBD_A326_4944_A548_06D3FD87669A
#define CF853CBD_A326_4944_A548_06D3FD87669A

#include "Ft232hI2c.h"
#include <PN532Interface.h>
#include <ControllerFrameWriter.h>
#include <FrameFormatter.h>
#include <chrono>

class PN532OverFT232HI2C final : public PN532Interface, private ControllerFrameWriter
{
public:
	PN532OverFT232HI2C(
		void* ftHandle,
		uint8_t i2cAddr,
		int gpioResetPinIndex,
		const std::chrono::milliseconds& timeout) noexcept;

	void AssertResetAndPowerDown() override;
	void DeassertResetAndPowerUp() override;

	void StartDataTransport() override;
	ControllerFrameWriter& WriteFrame() override;
	bool ReadFrame(TargetFrameWriter &writer) override;

private:

	void Ack() override;
	void Nack() override;
	void DataFromHost(const std::span<uint8_t const>& data) override;

	Ft232hI2c i2c;
	FrameFormatter formatter;
	const std::chrono::milliseconds timeout;
};

#endif /* CF853CBD_A326_4944_A548_06D3FD87669A */
