#include "NullTargetDataWriter.h"
#include "NullTargetDataValidator.h"

using namespace std;

inline NullTargetDataWriter defaultNullWriter{};
NullTargetDataWriter &NullTargetDataWriter::Default() noexcept { return defaultNullWriter; }
TargetDataValidator &NullTargetDataWriter::Validator() { return NullTargetDataValidator::Default(); }
size_t NullTargetDataWriter::Write(const span<uint8_t const> &data) { return data.size(); }
