#ifndef D76EADC2_7364_4B71_B0BF_F7EBADFDE4FF
#define D76EADC2_7364_4B71_B0BF_F7EBADFDE4FF

#include <cstdint>
#include <span>

class ControllerFrameWriter
{
public:
	virtual void Ack() = 0;
	virtual void Nack() = 0;
	virtual void DataFromHost(const std::span<uint8_t const>& data) = 0;

protected:
	ControllerFrameWriter() = default;
};

#endif /* D76EADC2_7364_4B71_B0BF_F7EBADFDE4FF */
