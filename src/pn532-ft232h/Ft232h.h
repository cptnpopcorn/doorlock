#ifndef EF9F3D78_3B33_4790_896D_404560D40BBF
#define EF9F3D78_3B33_4790_896D_404560D40BBF

#include <ftd2xx.h>
#include <chrono>
#include <span>

class Ft232h final
{
public:
	Ft232h() noexcept;
	Ft232h(FT_HANDLE handle, uint8_t i2cAddr, uint8_t gpioMask, const std::chrono::milliseconds& timeout) noexcept;
	Ft232h(Ft232h&&);
	Ft232h& operator =(Ft232h&&);

	size_t WriteI2C(const std::span<uint8_t const>& data, DWORD options) const;
	size_t ReadI2C(const std::span<uint8_t>& data, DWORD options) const;
	void WriteGPIO(uint8_t bits) const;

	~Ft232h();

private:
	void Swap(Ft232h&) noexcept;

	bool isValid;
	FT_HANDLE handle;
	uint8_t i2cAddr;
	uint8_t gpioMask;
	std::chrono::milliseconds timeout;
};

#endif /* EF9F3D78_3B33_4790_896D_404560D40BBF */
