#include "NullTargetDataValidator.h"

inline NullTargetDataValidator defaultNullTargetValidator {};

NullTargetDataValidator &NullTargetDataValidator::Default() noexcept
{
	return defaultNullTargetValidator;
}

void NullTargetDataValidator::DcsValid()
{
}

void NullTargetDataValidator::DcsInvalid()
{
}
