#ifndef ABCF1141_BA26_445B_8FDC_33A64445EF1F
#define ABCF1141_BA26_445B_8FDC_33A64445EF1F

#include <PN532Interface.h>
#include <ControllerFrameWriter.h>
#include <FrameFormatter.h>
#include <chrono>
#include <cstdint>
#include <span>

typedef struct spi_device_t *spi_device_handle_t;

class Pn532BeetleEsp32C6Spi final : public PN532Interface, private ControllerFrameWriter
{
public:
	Pn532BeetleEsp32C6Spi(const std::chrono::milliseconds &timeout);

	void AssertResetAndPowerDown() override;
	void DeassertResetAndPowerUp() override;

	void StartDataTransport() override;
	ControllerFrameWriter& WriteFrame() override;
	bool ReadFrame(TargetFrameWriter &writer) override;

	void AssertChipSelect();
	void DeassertChipSelect();

	~Pn532BeetleEsp32C6Spi();

private:
	void Ack() override;
	void Nack() override;
	void DataFromHost(const std::span<uint8_t const>& data) override;

	void Write(const std::span<const uint8_t> &data);
	void Read(const std::span<uint8_t> &data);

	spi_device_handle_t handle;
	FrameFormatter formatter;
	const std::chrono::milliseconds timeout;
};

#endif /* ABCF1141_BA26_445B_8FDC_33A64445EF1F */
