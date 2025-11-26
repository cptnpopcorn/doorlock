#ifndef E110A074_650E_44C4_924C_654277286EE4
#define E110A074_650E_44C4_924C_654277286EE4

#include "TargetDataWriter.h"

template<class WriteFunc, class ValidatorFunc> class TargetDataWriterFuncs final : public TargetDataWriter
{
public:
	TargetDataWriterFuncs(WriteFunc &&writeFunc, ValidatorFunc &&validatorFunc) :
		writeFunc{std::move(writeFunc)}, validatorFunc{std::move(validatorFunc)}
	{
	}

	size_t Write(const std::span<uint8_t const>& data) override { return writeFunc(data); }
	TargetDataValidator &Validator() override { return validatorFunc(); }

private:
	WriteFunc writeFunc;
	ValidatorFunc validatorFunc;
};

template<class WriteFunc, class ValidatorFunc> TargetDataWriterFuncs<WriteFunc, ValidatorFunc> make_target_data_writer(
	WriteFunc &&writeFunc,
	ValidatorFunc &&validatorFunc)
{
	return {std::move(writeFunc), std::move(validatorFunc)};
}

#endif /* E110A074_650E_44C4_924C_654277286EE4 */
