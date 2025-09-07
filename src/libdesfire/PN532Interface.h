#ifndef B7131B7B_411E_42C3_B18F_B4F3F629894E
#define B7131B7B_411E_42C3_B18F_B4F3F629894E

#include <span>
#include <cstdint>

class ControllerFrameWriter;
class TargetFrameWriter;

class PN532Interface
{
public:
	virtual void AssertResetAndPowerDown() = 0;
	virtual void DeassertResetAndPowerUp() = 0;

	virtual size_t Write(const std::span<uint8_t const>& data) = 0; // TODO: remove
	virtual size_t Read(const std::span<uint8_t>& data) = 0; // TODO: remove

	virtual void StartDataTransport() = 0;
	virtual ControllerFrameWriter& WriteFrame() = 0;
	virtual void ReadFrame(TargetFrameWriter &&writer) = 0;

protected:
	PN532Interface() = default;
};

#endif /* B7131B7B_411E_42C3_B18F_B4F3F629894E */
