#ifndef E472FD4F_0AF8_4CC2_A3FB_1964819624A8
#define E472FD4F_0AF8_4CC2_A3FB_1964819624A8

#include "TargetDataValidator.h"
#include <utility>

template<class DcsValidFunc, class DcsInvalidFunc> class TargetDataValidatorFuncs : public TargetDataValidator
{
public:
	TargetDataValidatorFuncs(DcsValidFunc &&dcsValidFunc, DcsInvalidFunc &&dcsInvalidFunc) :
		dcsValidFunc{std::move(dcsValidFunc)}, dcsInvalidFunc{std::move(dcsInvalidFunc)}
	{
	}

	void DcsValid() override { dcsValidFunc(); }
	void DcsInvalid() override { dcsInvalidFunc(); }

private:

	DcsValidFunc dcsValidFunc;
	DcsInvalidFunc dcsInvalidFunc;
};

template<class DcsValidFunc, class DcsInvalidFunc> TargetDataValidatorFuncs<DcsValidFunc, DcsInvalidFunc> make_target_data_validator(
	DcsValidFunc &&dcsValidFunc,
	DcsInvalidFunc &&dcsInvalidFunc)
{
	return {std::move(dcsValidFunc), std::move(dcsInvalidFunc)};
}

#endif /* E472FD4F_0AF8_4CC2_A3FB_1964819624A8 */
