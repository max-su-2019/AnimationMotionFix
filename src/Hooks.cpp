#include "Hooks.h"

namespace AMF
{
	bool ModifyMovementDataHandler::IsMovementAnimationDriven_1405E3250(RE::Actor* a_actor)
	{
		using func_t = decltype(&IsMovementAnimationDriven_1405E3250);
		REL::Relocation<func_t> func{ RELOCATION_ID(36487, 0) };
		return func(a_actor);
	}

	bool ModifyMovementDataHandler::RevertPitchRotation(RE::Actor* a_actor, RE::NiPoint3& a_translation, RE::NiPoint3& a_rotation)
	{
		if (!a_actor)
			return false;

		if (a_actor->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal)
			return false;

		if (a_actor->IsFlying()) {
			return false;
		}

		bool bIsSynced = false;
		if (a_actor->GetGraphVariableBool(RE::FixedStrings::GetSingleton()->bIsSynced, bIsSynced) && bIsSynced) {
			return false;
		}

		if (IsMovementAnimationDriven_1405E3250(a_actor) && (a_actor->IsAnimationDriven() || a_actor->IsAllowRotation())) {
			auto pitchAngle = a_actor->data.angle.x;
			auto nonPitchTranslationY = a_translation.y / cosf(pitchAngle);
			auto nonPitchTranslationZ = a_translation.z - nonPitchTranslationY * sinf(pitchAngle);
			a_translation.y = nonPitchTranslationY;
			a_translation.z = nonPitchTranslationZ;
			return true;
		}

		return false;
	}

	void ModifyMovementDataHandler::CharacterEx::Hook_ModifyMovementData(float a_delta, RE::NiPoint3& a_translation, RE::NiPoint3& a_rotation)
	{
		RevertPitchRotation(this, a_translation, a_rotation);
	}

	void ModifyMovementDataHandler::PlayerHook::ModifyMovementData(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_translation, RE::NiPoint3& a_rotation)
	{
		RevertPitchRotation(a_player, a_translation, a_rotation);
	}

	void ModifyMovementDataHandler::PlayerHook::UpdateMagnetism(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_translation, float& a_rotationZ)
	{
		return;
	}
}