#ifndef E6502A94_566A_4F2B_AA2C_80A08608E7FD
#define E6502A94_566A_4F2B_AA2C_80A08608E7FD

#include <TargetFrameWriter.h>
#include <functional>
#include <cstdint>

class TestFrameWriter final : public TargetFrameWriter
{
public:
	TestFrameWriter(
		std::function<void()> lcsInvalid,
		std::function<void()> ack,
		std::function<void()> nack,
		std::function<void(uint8_t)> tfiInvalid,
		std::function<TargetDataWriter&()> dataFromHost,
		std::function<TargetDataValidator&(uint8_t)> errorToHost,
		std::function<void()> incomplete) noexcept;

	void LcsInvalid() override;
	void Ack() override;
	void Nack() override;
	void TfiInvalid(uint8_t tfi) override;
	TargetDataWriter& DataToHost() override;
	TargetDataValidator& Error(uint8_t err) override;
	void FrameIncomplete() override;

private:
	std::function<void()> lcsInvalid;
	std::function<void()> ack;
	std::function<void()> nack;
	std::function<void(uint8_t)> tfiInvalid;
	std::function<TargetDataWriter&()> dataToHost;
	std::function<TargetDataValidator&(uint8_t)> error;
	std::function<void()> incomplete;
};

#endif /* E6502A94_566A_4F2B_AA2C_80A08608E7FD */
