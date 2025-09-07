#ifndef C5287DFD_F0FF_4146_81EB_73CC120750F8
#define C5287DFD_F0FF_4146_81EB_73CC120750F8

#include <cstdint>
#include <span>

class TargetDataValidator;

class TargetDataWriter
{
public:
	virtual TargetDataValidator& Validator() = 0;
	virtual size_t Write(const std::span<uint8_t const>& data) = 0;

protected:
	TargetDataWriter() = default;
};

#endif /* C5287DFD_F0FF_4146_81EB_73CC120750F8 */
