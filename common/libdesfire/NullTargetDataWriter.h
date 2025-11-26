#ifndef C8B8AAC6_23D3_4A70_8AB9_D724C94CD9DE
#define C8B8AAC6_23D3_4A70_8AB9_D724C94CD9DE

#include "TargetDataWriter.h"

class NullTargetDataWriter final : public TargetDataWriter
{
public:
	static NullTargetDataWriter &Default() noexcept;

	TargetDataValidator& Validator() override;
	size_t Write(const std::span<uint8_t const>& data) override;
};

#endif /* C8B8AAC6_23D3_4A70_8AB9_D724C94CD9DE */
