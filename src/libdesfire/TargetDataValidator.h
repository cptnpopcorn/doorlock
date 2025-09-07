#ifndef C6B16CDA_1E11_4B41_99BC_10590038FD6E
#define C6B16CDA_1E11_4B41_99BC_10590038FD6E

class TargetDataValidator
{
public:
	virtual void DcsValid() = 0;
	virtual void DcsInvalid() = 0;

protected:
	TargetDataValidator() = default;
};

#endif /* C6B16CDA_1E11_4B41_99BC_10590038FD6E */
