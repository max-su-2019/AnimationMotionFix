#pragma once
#include "DKUtil/Config.hpp"
#include "DKUtil/Utility.hpp"

namespace AMF
{
	using namespace DKUtil::Alias;

	class AMFSettings : public DKUtil::model::Singleton<AMFSettings>
	{
		friend DKUtil::model::Singleton<AMFSettings>;

	private:
		AMFSettings();

		template <class T>
		static void PrintSettingValue(const T& a_setting)
		{
			INFO("Setting:\"{}\" is {}"sv, a_setting.get_key(), a_setting.get_data());
		}

	public:
		void OverrideGameSettings();

		Boolean enablePitchTranslationFix{ "EnablePitchTranslationFix", "Fix" };

		Boolean disablePlayerRotationMagnetism{ "DisablePlayerRotationMagnetism", "Tweak" };
		Boolean disablePlayerMovementMagnetism{ "DisablePlayerMovementMagnetism", "Tweak" };
		Boolean disableNpcMovementMagnetism{ "DisableNpcMovementMagnetism", "Tweak" };

		Boolean bUseCharacterRB{ "bUseCharacterRB", "GameSettings" };
	};
}