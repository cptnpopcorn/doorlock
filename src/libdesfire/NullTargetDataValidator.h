#ifndef BAEE90CB_95AA_4F1C_9558_85F1E934066F
#define BAEE90CB_95AA_4F1C_9558_85F1E934066F

#include "TargetDataValidator.h"

class NullTargetDataValidator final : public TargetDataValidator
{
public:
	static NullTargetDataValidator &Default() noexcept;

	void DcsValid() override;
	void DcsInvalid() override;
};

#endif /* BAEE90CB_95AA_4F1C_9558_85F1E934066F */
