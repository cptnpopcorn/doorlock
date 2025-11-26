#ifndef CBDBEF10_F5D9_4D42_B247_958EA53D2A79
#define CBDBEF10_F5D9_4D42_B247_958EA53D2A79

#include <array>
#include <cstdint>
#include <span>

class TargetFrameWriter;
class TargetDataWriter;
class TargetDataValidator;

class FrameParser final
{
public:
	FrameParser(TargetFrameWriter& writer) noexcept;
	size_t GetRequiredLength() const noexcept;
	size_t Parse(const std::span<uint8_t const>& data);
	~FrameParser();

private:
	typedef size_t (FrameParser::*LengthFunc)() const noexcept;
	typedef size_t (FrameParser::*ParseFunc)(const std::span<uint8_t const>& data);
	struct State final { LengthFunc Length; ParseFunc Parse; };

	size_t StartLength() const noexcept;
	size_t ParseStartCode(const std::span<uint8_t const>& data);

	void ContinueWithLength() noexcept;
	size_t LengthLength() const noexcept;
	size_t ParseLength(const std::span<uint8_t const>& data);

	void ContinueWithExtendedLength() noexcept;
	size_t ExtendedLengthLength() const noexcept;
	size_t ParseExtendedLength(const std::span<uint8_t const>& data);

	void ContinueWithError() noexcept;
	size_t ErrorLength() const noexcept;
	size_t ParseError(const std::span<uint8_t const>& data);

	void ContinueWithTfi(size_t dataLength) noexcept;
	size_t TfiLength() const noexcept;
	size_t ParseTfi(const std::span<uint8_t const>& data);

	void ContinueWithData(TargetDataWriter& dataWriter) noexcept;
	size_t DataLength() const noexcept;
	size_t ParseData(const std::span<uint8_t const>& data);

	void ContinueWithDcs(TargetDataValidator &dataValidator) noexcept;
	size_t DcsLength() const noexcept;
	size_t ParseDcs(const std::span<uint8_t const>& data);

	void ContinueWithPostamble() noexcept;
	size_t PostambleLength() const noexcept;
	size_t ParsePostamble(const std::span<uint8_t const>& data);

	void Done();
	size_t DoneLength() const noexcept;
	size_t ParseDone(const std::span<uint8_t const>& data);

	size_t numLastOctets;
	std::array<uint8_t, 2> lastOctets;

	size_t dataLength;
	uint8_t dataSum;

	State state;
	TargetFrameWriter& writer;
	TargetDataWriter* dataWriter;
	TargetDataValidator* dataValidator;
};

#endif /* CBDBEF10_F5D9_4D42_B247_958EA53D2A79 */
