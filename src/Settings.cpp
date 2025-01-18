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

		MainConfig.Bind(bUseCharacterRB, false);

		MainConfig.Load();

		PrintSettingValue(enablePitchTranslationFix);
		PrintSettingValue(disablePlayerRotationMagnetism);
		PrintSettingValue(disablePlayerMovementMagnetism);
		PrintSettingValue(disableNpcMovementMagnetism);
		PrintSettingValue(bUseCharacterRB);
	}

	void AMFSettings::OverrideGameSettings()
	{
		if (!disableNpcMovementMagnetism) {
			return;
		}

		if (!bUseCharacterRB) {
			static constexpr auto conflictDLLName = "DisableFollowerCollision.dll";
			auto conflicPlugin = GetModuleHandleA(conflictDLLName);
			if (conflicPlugin) {
				ERROR("Incompatible Plugin Detected: \"{}\"! To resolve this conflict, set ini setting \"bUseCharacterRB\" to true.", conflictDLLName);
				return;
			}
		}

		if (auto setting = RE::INISettingCollection::GetSingleton()->GetSetting("bUseCharacterRB:HAVOK")) {
			setting->data.b = bUseCharacterRB;
		}
	}

}