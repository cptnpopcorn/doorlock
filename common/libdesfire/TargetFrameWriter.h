#ifndef A75F1D7F_3E1F_4D22_8ACB_0415D0D46089
#define A75F1D7F_3E1F_4D22_8ACB_0415D0D46089

#include <cstdint>

class TargetDataWriter;
class TargetDataValidator;

class TargetFrameWriter
{
public:
	virtual void LcsInvalid() = 0;
	virtual void Ack() = 0;
	virtual void Nack() = 0;
	virtual void TfiInvalid(uint8_t tfi) = 0;
	virtual TargetDataWriter& DataToHost() = 0;
	virtual TargetDataValidator& Error(uint8_t err) = 0;
	virtual void FrameIncomplete() = 0;

protected:
	TargetFrameWriter() = default;
};

#endif /* A75F1D7F_3E1F_4D22_8ACB_0415D0D46089 */
