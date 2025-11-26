#ifndef DAA2E6A9_1B97_4622_B987_DFB49A648013
#define DAA2E6A9_1B97_4622_B987_DFB49A648013

#include "Ft232hSpi.h"
#include <PN532Interface.h>
#include <ControllerFrameWriter.h>
#include <FrameFormatter.h>
#include <chrono>

class PN532OverFT232HSPI final : public PN532Interface, public ControllerFrameWriter
{
public:
	PN532OverFT232HSPI(
		void* ftHandle,
		int gpioResetPinIndex,
		const std::chrono::milliseconds& timeout) noexcept;

	void AssertResetAndPowerDown() override;
	void DeassertResetAndPowerUp() override;

	void StartDataTransport() override;
	ControllerFrameWriter& WriteFrame() override;
	bool ReadFrame(TargetFrameWriter &writer) override;

private:
	virtual void Ack();
	virtual void Nack();
	virtual void DataFromHost(const std::span<uint8_t const>& data);
	
	Ft232hSpi spi;
	FrameFormatter formatter;
	const std::chrono::milliseconds timeout;
};

#endif /* DAA2E6A9_1B97_4622_B987_DFB49A648013 */
