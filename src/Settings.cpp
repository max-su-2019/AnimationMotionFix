#include "Settings.h"
#include <SimpleIni.h>

namespace AMF
{
	void AMFSettings::LoadSettings()
	{
		const auto path = std::format("Data/SKSE/Plugins/{}.ini", Plugin::NAME);

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(path.c_str());

		enablePitchTranslationFix = ini.GetBoolValue("Fix", "EnablePitchTranslationFix");

		constexpr const char* section = "Tweak";
		disablePlayerRotationMagnetism = ini.GetBoolValue(section, "DisablePlayerRotationMagnetism");
		playerMovementMagnetismMode = MovMagMode(ini.GetLongValue(section, "PlayerMovementMagnetismMode"));
		npcMovementMagnetismMode = MovMagMode(ini.GetLongValue(section, "NpcMovementMagnetismMode"));
	}
}