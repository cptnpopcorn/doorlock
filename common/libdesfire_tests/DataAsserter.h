#ifndef FAB5E7A3_DD0A_4794_A4C7_F359B71567AF
#define FAB5E7A3_DD0A_4794_A4C7_F359B71567AF

#include <span>
#include <cstdint>

class DataAsserter final
{
public:
	DataAsserter(const std::span<uint8_t const>& data) noexcept;
	size_t Write(const std::span<uint8_t const>& data);
	~DataAsserter();

private:
	std::span<uint8_t const> refdata;
};

#endif /* FAB5E7A3_DD0A_4794_A4C7_F359B71567AF */
