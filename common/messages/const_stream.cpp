#include "const_stream.h"
#include <cassert>

using namespace std;

ConstStream::ConstStream(const span<const uint8_t> &chars) noexcept : i{0}, chars{chars} {}
ConstStream::Ch ConstStream::Peek() const { return i < chars.size() ? chars[i] : Ch{}; }
ConstStream::Ch ConstStream::Take(){ return i < chars.size() ? static_cast<char>(chars[i++]) : Ch{}; }
size_t ConstStream::Tell() const{ return chars.size(); }
ConstStream::Ch *ConstStream::PutBegin(){ assert(false); return 0; }
void ConstStream::Put(Ch) { assert(false); }
void ConstStream::Flush() { assert(false); }
size_t ConstStream::PutEnd(Ch *) { assert(false); return 0; }