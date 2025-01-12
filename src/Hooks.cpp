#include "Hooks.h"

namespace AMF
{
	bool IsMovementAnimationDriven_1405E3250(RE::Actor* a_actor)
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
		if (!this->IsPlayerRef())
			RevertPitchRotation(this, a_translation, a_rotation);

		return func(this, a_delta, a_translation, a_rotation);
	}

	void AttackMagnetismHandler::PlayerRotateMagnetismHook::UpdateMagnetism(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_translation, float& a_rotationZ)
	{
		return;
	}

	bool AttackMagnetismHandler::MovementMagnetismHook::IsStartingMeleeAttack(RE::Actor* a_actor)
	{
		return false;
	}

	void AttackMagnetismHandler::PushCharacterHook::Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint)
	{
		auto GetActor = [](RE::bhkCharacterController* a_charCtrl) -> RE::Actor* {
			if (!a_charCtrl)
				return nullptr;

			auto rigidBoby = a_charCtrl->GetRigidBody();
			auto objRef = rigidBoby ? rigidBoby->GetUserData() : nullptr;
			return objRef ? objRef->As<RE::Actor>() : nullptr;
		};

		auto attacker = GetActor(a_pusher);
		if (attacker && attacker->IsAttacking() && IsMovementAnimationDriven_1405E3250(attacker) && attacker->currentCombatTarget) {
			auto targetActor = GetActor(a_target);
			if (targetActor && targetActor == attacker->currentCombatTarget.get().get()) {
				return;
			}
		}

		func(a_pusher, a_target, a_contactPoint);
	}

}