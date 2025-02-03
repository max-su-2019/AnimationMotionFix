#include "Settings.h"

namespace AMF
{
	static auto MainConfig = COMPILE_PROXY("AnimationMotionFix.ini"sv);

	AMFSettings::AMFSettings()
	{
		MainConfig.Bind(enablePitchTranslationFix, true);

		MainConfig.Bind(disablePlayerRotationMagnetism, true);
		MainConfig.Bind(disablePlayerMovementMagnetism, true);
		MainConfig.Bind(disableNpcMovementMagnetism, true);

		MainConfig.Load();

		PrintSettingValue(enablePitchTranslationFix);
		PrintSettingValue(disablePlayerRotationMagnetism);
		PrintSettingValue(disablePlayerMovementMagnetism);
		PrintSettingValue(disableNpcMovementMagnetism);
	}
}