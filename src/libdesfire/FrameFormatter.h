#ifndef D84B0AA2_D978_4AF6_8CC7_65D102B7FCED
#define D84B0AA2_D978_4AF6_8CC7_65D102B7FCED

#include <array>
#include <span>

class FrameFormatter final
{
public:
	FrameFormatter() noexcept;
	std::span<uint8_t const> GetDataHeader(const std::span<uint8_t const> &data);
	std::span<uint8_t const> GetDataTail(const std::span<uint8_t const>& data) noexcept;
	std::span<uint8_t const> GetAck() const noexcept;
	std::span<uint8_t const> GetNack() const noexcept;

private:
	std::array<uint8_t, 9> header;
	std::array<uint8_t, 2> tail;
};

#endif /* D84B0AA2_D978_4AF6_8CC7_65D102B7FCED */
