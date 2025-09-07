#ifndef B7131B7B_411E_42C3_B18F_B4F3F629894E
#define B7131B7B_411E_42C3_B18F_B4F3F629894E

#include <span>
#include <cstdint>

class PN532Interface
{
public:
	virtual void AssertResetAndPowerDown() = 0;
	virtual void DeassertResetAndPowerUp() = 0;

	virtual void StartDataTransport() = 0;
	virtual void Write(const std::span<uint8_t> &data) = 0;
	virtual bool Read(const std::span<uint8_t> &data) = 0;

protected:
	PN532Interface() = default;
};

#endif /* B7131B7B_411E_42C3_B18F_B4F3F629894E */
