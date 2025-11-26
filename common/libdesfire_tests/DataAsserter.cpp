#include "DataAsserter.h"

#include <CppUTest/Utest.h>
#include <CppUTest/UtestMacros.h>

#include <algorithm>

using namespace std;

DataAsserter::DataAsserter(const span<uint8_t  const>& data) noexcept : refdata{data}
{
}

size_t DataAsserter::Write(const std::span<uint8_t const>& data)
{
	CHECK(data.size() <= refdata.size());
	CHECK(ranges::equal(data, refdata.subspan(0, data.size())));
	refdata = refdata.last(refdata.size() - data.size());
	return data.size();
}

DataAsserter::~DataAsserter()
{
	CHECK(refdata.empty());
}
