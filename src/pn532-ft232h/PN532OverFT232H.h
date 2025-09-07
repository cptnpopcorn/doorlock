#ifndef CF853CBD_A326_4944_A548_06D3FD87669A
#define CF853CBD_A326_4944_A548_06D3FD87669A

#include "Ft232h.h"
#include <PN532Interface.h>
#include <ControllerFrameWriter.h>
#include <FrameFormatter.h>
#include <chrono>

class PN532OverFT232H final : public PN532Interface, private ControllerFrameWriter
{
public:
	PN532OverFT232H(
		void* ftHandle,
		uint8_t i2cAddr,
		int gpioResetPinIndex,
		const std::chrono::milliseconds& timeout) noexcept;

	void AssertResetAndPowerDown() override;
	void DeassertResetAndPowerUp() override;

	void StartDataTransport() override;
	ControllerFrameWriter& WriteFrame() override;
	void ReadFrame(TargetFrameWriter &&writer) override;

	size_t Write(const std::span<uint8_t const>& data) override; // TODO: remove
	size_t Read(const std::span<uint8_t>& data) override; // TODO: remove

private:

	void Ack() override;
	void Nack() override;
	void DataFromHost(const std::span<uint8_t const>& data) override;

	Ft232h ft232h;
	FrameFormatter formatter;
	const std::chrono::milliseconds timeout;
};

#endif /* CF853CBD_A326_4944_A548_06D3FD87669A */
