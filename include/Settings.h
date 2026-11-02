#pragma once

namespace AMF
{
	class AMFSettings : public REX::Singleton<AMFSettings>
	{
	public:
		enum MovMagMode : long
		{
			kGlobalEnable = 0,
			kDynamic,
			kGlobalDisable
		};

		void LoadSettings();

		bool enablePitchTranslationFix{ true };
		bool disablePlayerRotationMagnetism{ true };

		MovMagMode playerMovementMagnetismMode{ kDynamic };
		MovMagMode npcMovementMagnetismMode{ kDynamic };
	};
	using MovMagMode = AMFSettings::MovMagMode;

}