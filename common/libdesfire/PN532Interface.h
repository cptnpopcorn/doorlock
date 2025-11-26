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

	virtual void StartDataTransport() = 0;
	virtual ControllerFrameWriter& WriteFrame() = 0;
	virtual bool ReadFrame(TargetFrameWriter &writer) = 0; // false for timeout, other errors handled by writer

protected:
	PN532Interface() = default;
};

#endif /* B7131B7B_411E_42C3_B18F_B4F3F629894E */
