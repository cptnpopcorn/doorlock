#ifndef EF9F3D78_3B33_4790_896D_404560D40BBF
#define EF9F3D78_3B33_4790_896D_404560D40BBF

#include <ftd2xx.h>
#include <chrono>
#include <span>

class Ft232hI2c final
{
public:
	Ft232hI2c() noexcept;
	Ft232hI2c(FT_HANDLE handle, uint8_t i2cAddr, uint8_t gpioMask, const std::chrono::milliseconds& timeout) noexcept;
	Ft232hI2c(Ft232hI2c&&) noexcept;
	Ft232hI2c& operator =(Ft232hI2c&&) noexcept;

	size_t Write(const std::span<uint8_t const>& data, DWORD options) const;
	size_t Read(const std::span<uint8_t>& data, DWORD options) const;
	void WriteGPIO(uint8_t bits) const;

private:
	void Swap(Ft232hI2c&) noexcept;

	FT_HANDLE handle;
	uint8_t i2cAddr;
	uint8_t gpioMask;
	std::chrono::milliseconds timeout;
};

#endif /* EF9F3D78_3B33_4790_896D_404560D40BBF */
