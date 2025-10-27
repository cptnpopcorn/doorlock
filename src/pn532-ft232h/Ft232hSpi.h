#ifndef D6C9BD54_FF74_4E1C_A6E6_70741AB348D3
#define D6C9BD54_FF74_4E1C_A6E6_70741AB348D3

#include <ftd2xx.h>
#include <chrono>
#include <span>

class Ft232hSpi final
{
public:
	Ft232hSpi() noexcept;
	Ft232hSpi(FT_HANDLE handle, std::chrono::milliseconds wakeupLatency, uint8_t gpioMask) noexcept;
	Ft232hSpi(Ft232hSpi&&) noexcept;

	Ft232hSpi& operator =(Ft232hSpi&&) noexcept;

	size_t Read(const std::span<uint8_t>& read, bool assertChipSelect = true, bool deassertChipSelect = true);
	size_t Write(const std::span<uint8_t const>& write, bool assertChipSelect = true, bool deassertChipSelect = true);
	void DeassertChipSelect();
	void WriteGPIO(uint8_t bits) const;
	void Flush();

private:
	void Swap(Ft232hSpi&) noexcept;
	void AssertChipSelect();

	FT_HANDLE handle;
	uint8_t gpioMask;
	std::chrono::milliseconds wakeupLatency;
};

#endif /* D6C9BD54_FF74_4E1C_A6E6_70741AB348D3 */
