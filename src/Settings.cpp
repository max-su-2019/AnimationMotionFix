#include "Settings.h"
#include "AnimationMotionRevolutionAPI.h"
#include <SimpleIni.h>

namespace AMF
{
	AMR_API::IVAMR1* amr = nullptr;

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

		amr = static_cast<AMR_API::IVAMR1*>(AMR_API::RequestPluginAPI());
		if (!amr) {
			SKSE::log::warn("Not AMR API Found!");
		}
	}

	bool AMFSettings::HasAnimMotionData(const RE::hkbCharacter* a_hkbCharacter)
	{
		if (amr && amr->HasAnimMotionData(a_hkbCharacter)) {
			return true;
		}

		return false;
	}
}