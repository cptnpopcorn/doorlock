#include "TestFrameWriter.h"

using namespace std;

TestFrameWriter::TestFrameWriter(
	function<void()> lcsInvalid,
	function<void()> ack,
	function<void()> nack,
	function<void(uint8_t)> tfiInvalid,
	function<TargetDataWriter &()> dataToHost,
	function<TargetDataValidator&(uint8_t)> errorToHost,
	std::function<void()> incomplete) noexcept :
	lcsInvalid{lcsInvalid},
	ack{ack},
	nack{nack},
	tfiInvalid{tfiInvalid},
	dataToHost{dataToHost},
	error{errorToHost},
	incomplete{incomplete}
{
}

void TestFrameWriter::LcsInvalid()
{
	lcsInvalid();
}

void TestFrameWriter::Ack()
{
	ack();
}

void TestFrameWriter::Nack()
{
	nack();
}

void TestFrameWriter::TfiInvalid(uint8_t tfi)
{
	tfiInvalid(tfi);
}

TargetDataWriter &TestFrameWriter::DataToHost()
{
	return dataToHost();
}

TargetDataValidator &TestFrameWriter::Error(uint8_t err)
{
	return error(err);
}

void TestFrameWriter::FrameIncomplete()
{
	incomplete();
}
