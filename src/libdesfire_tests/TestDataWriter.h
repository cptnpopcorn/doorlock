#ifndef C77AA232_ABEE_460A_B213_FA174970DC80
#define C77AA232_ABEE_460A_B213_FA174970DC80

#include <TargetDataWriter.h>
#include <TargetDataValidator.h>
#include <functional>

class TestDataWriter final : public TargetDataWriter, private TargetDataValidator
{
public:
	TestDataWriter(
		std::function<size_t(const std::span<uint8_t const> &)> write,
		std::function<void()> dcsValid,
		std::function<void()> dcIsnvalid) noexcept;

	TargetDataValidator &Validator() override;
	size_t Write(const std::span<uint8_t const> &data) override;

private:
	void DcsValid() override;
	void DcsInvalid() override;

	std::function<size_t(const std::span<uint8_t const> &)> write;
	std::function<void()> dcsValid;
	std::function<void()> dcsInvalid;
};

#endif /* C77AA232_ABEE_460A_B213_FA174970DC80 */
