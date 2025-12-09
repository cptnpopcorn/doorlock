#ifndef DC589AC8_033F_41DE_97DC_25696F1C71F7
#define DC589AC8_033F_41DE_97DC_25696F1C71F7

#include <interaction.h>

class setup final : public interaction
{
public:
	setup(interaction& quit) noexcept;
	void start(interaction_control&) override;

private:
    interaction &quit;
};

#endif /* DC589AC8_033F_41DE_97DC_25696F1C71F7 */
