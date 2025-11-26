#include "TestDataWriter.h"

using namespace std;

TestDataWriter::TestDataWriter(
	function<size_t(const span<uint8_t const> &data)> write,
	function<void()> dcsValid,
	function<void()> dcsInvalid) noexcept :
	write{write},
	dcsValid{dcsValid},
	dcsInvalid{dcsInvalid}
{
}

TargetDataValidator &TestDataWriter::Validator()
{
	return *this;
}

size_t TestDataWriter::Write(const span<uint8_t const> &data)
{
	return write(data);
}

void TestDataWriter::DcsValid()
{
	dcsValid();
}

void TestDataWriter::DcsInvalid()
{
	dcsInvalid();
}
