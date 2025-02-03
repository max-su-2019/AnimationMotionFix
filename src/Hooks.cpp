#include "Hooks.h"
#include "Settings.h"

namespace AMF
{
	inline bool IsMovementAnimationDriven_1405E3250(RE::Actor* a_actor)
	{
		using func_t = decltype(&IsMovementAnimationDriven_1405E3250);
		REL::Relocation<func_t> func{ RELOCATION_ID(36487, 0) };
		return func(a_actor);
	}

	inline RE::hkContactPoint* GetContactPoint_140A9ED70(RE::hkpSimpleConstraintContactMgr* a_mgr, uint16_t a_contactPointIds)
	{
		using func_t = decltype(&GetContactPoint_140A9ED70);
		REL::Relocation<func_t> func{ RELOCATION_ID(61252, 0) };
		return func(a_mgr, a_contactPointIds);
	}

	inline void SetInvMassScalingForContact_140AA8740(RE::hkpSimpleConstraintContactMgr* a_mgr, RE::hkpRigidBody* a_body, RE::hkpConstraintOwner& a_constraintOwner, const RE::hkVector4& a_factor)
	{
		using func_t = decltype(&SetInvMassScalingForContact_140AA8740);
		REL::Relocation<func_t> func{ RELOCATION_ID(61388, 0) };
		return func(a_mgr, a_body, a_constraintOwner, a_factor);
	}

	bool FixPitchTransHandler::RevertPitchRotation(RE::Actor* a_actor, RE::NiPoint3& a_translation)
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
			if (std::abs(pitchAngle) > 1.57079638f) {
				ERROR("Gimbal Lock Occured When Revert Pitch Rotation!");
				return false;
			}

			auto nonPitchTranslationY = a_translation.y / cosf(pitchAngle);
			auto nonPitchTranslationZ = a_translation.z - nonPitchTranslationY * sinf(pitchAngle);
			a_translation.y = nonPitchTranslationY;
			a_translation.z = nonPitchTranslationZ;
			return true;
		}

		return false;
	}

	void FixPitchTransHandler::Hook_ConvertMoveDirToTranslation(RE::NiPoint3& a_movementDirection, RE::NiPoint3& a_translationData, RE::Actor* a_actor)
	{
		ConvertMoveDirToTranslation(a_movementDirection, a_translationData);
		if (!a_actor->IsPlayerRef() && AMFSettings::GetSingleton()->enablePitchTranslationFix)
			RevertPitchRotation(a_actor, a_translationData);
	}

	bool AttackMagnetismHandler::ShouldDisableMovementMagnetism(RE::Actor* a_actor)
	{
		auto settings = AMFSettings::GetSingleton();
		return (a_actor->IsPlayerRef() && settings->disablePlayerMovementMagnetism) || (!a_actor->IsPlayerRef() && settings->disableNpcMovementMagnetism);
	}

	void AttackMagnetismHandler::PlayerRotateMagnetismHook::UpdateMagnetism(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_translation, float& a_rotationZ)
	{
		if (!AMFSettings::GetSingleton()->disablePlayerRotationMagnetism) {
			return func(a_player, a_delta, a_translation, a_rotationZ);
		}
	}

	bool AttackMagnetismHandler::MovementMagnetismHook::Hook_IsStartingMeleeAttack(RE::Actor* a_actor)
	{
		if (ShouldDisableMovementMagnetism(a_actor)) {
			return false;
		}

		return func(a_actor);
	}

	bool PushCharacterHandler::ShouldPreventAttackPushing(RE::Actor* a_pusher, RE::Actor* a_target)
	{
		if (a_pusher && AttackMagnetismHandler::ShouldDisableMovementMagnetism(a_pusher) &&
			a_pusher->IsAttacking() && IsMovementAnimationDriven_1405E3250(a_pusher) && a_pusher->currentCombatTarget) {
			auto combatTarg = a_pusher->currentCombatTarget.get();
			if (a_target && (a_target == combatTarg.get() || a_target->GetMountedBy(combatTarg))) {
				return true;
			}
		}

		return false;
	}

	bool PushCharacterHandler::ShouldPreventAttackPushing(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target)
	{
		auto pusherActor = GetActor(a_pusher);
		if (pusherActor) {
			auto targActor = GetActor(a_target);
			if (targActor)
				return ShouldPreventAttackPushing(pusherActor, targActor);
		}

		return false;
	}

	RE::Actor* PushCharacterHandler::GetActor(RE::bhkCharacterController* a_charCtrl)
	{
		return a_charCtrl ? GetActor(a_charCtrl->GetRigidBody()) : nullptr;
	}

	RE::Actor* PushCharacterHandler::GetActor(RE::hkpRigidBody* a_rigidBody)
	{
		if (!a_rigidBody)
			return nullptr;

		uint8_t charCollisionFilterInfo = a_rigidBody->collidable.broadPhaseHandle.collisionFilterInfo & 0x7F;
		if (charCollisionFilterInfo != 0x1E)
			return nullptr;

		auto objRef = a_rigidBody ? a_rigidBody->GetUserData() : nullptr;
		return objRef ? objRef->As<RE::Actor>() : nullptr;
	}

	void PushCharacterHandler::ProxyPushProxyHandler::Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint)
	{
		if (ShouldPreventAttackPushing(a_pusher, a_target))
			return;

		func(a_pusher, a_target, a_contactPoint);
	}

	void PushCharacterHandler::ProxyPushRigidBodyHandler::Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint)
	{
		if (ShouldPreventAttackPushing(a_pusher, a_target))
			return;

		func(a_pusher, a_target, a_contactPoint);
	}

	void PushCharacterHandler::RigidBodyPushProxyHandler::Hook_ProcessConstraintsCallback(RE::bhkCharProxyController* a_proxyCtrl, const RE::hkpCharacterProxy* a_proxy, const RE::hkArray<RE::hkpRootCdPoint>& a_manifold, RE::hkpSimplexSolverInput& a_input)
	{
		ProcessConstraintsCallback(a_proxyCtrl, a_proxy, a_manifold, a_input);

		for (int i = 0; i < a_manifold.size(); i++) {
			const RE::hkpRootCdPoint& rootPoint = a_manifold[i];
			const RE::hkpCollidable* rootCollidableB = rootPoint.rootCollidableB;
			uint8_t charCollisionFilterInfo = rootCollidableB->broadPhaseHandle.collisionFilterInfo & 0x7F;
			if (charCollisionFilterInfo == 0x1E &&
				static_cast<RE::hkpWorldObject::BroadPhaseType>(rootCollidableB->broadPhaseHandle.type) == RE::hkpWorldObject::BroadPhaseType::kEntity) {
				auto attackerRef = RE::TESHavokUtilities::FindCollidableRef(*rootCollidableB);
				auto attacker = attackerRef ? attackerRef->As<RE::Actor>() : nullptr;
				if (attacker && ShouldPreventAttackPushing(attacker, GetActor(a_proxyCtrl))) {
					auto attackerCharCtrl = attacker->GetCharController();
					auto rigidBodyChar = attackerCharCtrl ? skyrim_cast<RE::bhkCharRigidBodyController*>(attackerCharCtrl) : nullptr;
					if (rigidBodyChar) {
						a_input.constraints[i].velocity = { 0 };
						WriteLocker(charCtrlContactPointsLock);
						charCtrlContactPointsMap.emplace(rigidBodyChar, a_input.constraints[i].plane);
						return;
					}
				}
			}
		}
	}

	void PushCharacterHandler::RigidBodyPushProxyHandler::Hook_UpdateForAnimationAttack(RE::bhkCharacterController* a_charCtrl)
	{
		UpdateForAnimationAttack(a_charCtrl);

		auto rigidCharCtrl = a_charCtrl ? skyrim_cast<RE::bhkCharRigidBodyController*>(a_charCtrl) : nullptr;
		if (rigidCharCtrl) {
			WriteLocker(charCtrlContactPointsLock);
			auto it = charCtrlContactPointsMap.find(rigidCharCtrl);
			if (it != charCtrlContactPointsMap.end()) {
				auto normal = it->second;
				RE::hkVector4 currentVelocity;
				rigidCharCtrl->GetLinearVelocityImpl(currentVelocity);
				auto velDotNormal = currentVelocity.Dot3(normal);
				if (velDotNormal > 0.f) {
					auto counterVel = normal * (-velDotNormal);
					currentVelocity = currentVelocity + counterVel;
					rigidCharCtrl->SetLinearVelocityImpl(currentVelocity);
				}

				charCtrlContactPointsMap.erase(it);
			}
		}
	}

	void PushCharacterHandler::RigidBodyPushProxyHandler::Hook_DeleteThis(RE::bhkCharRigidBodyController* a_charCtrl)
	{
		WriteLocker(charCtrlContactPointsLock);
		auto it = charCtrlContactPointsMap.find(a_charCtrl);
		if (it != charCtrlContactPointsMap.end()) {
			charCtrlContactPointsMap.erase(it);
		}

		DeleteThis(a_charCtrl);
	}

	void PushCharacterHandler::RigidBodyPushRigidBodyHandler::Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint)
	{
		if (ShouldPreventAttackPushing(a_pusher, a_target))
			return;

		PushTargetCharacter(a_pusher, a_target, a_contactPoint);
	}

	void PushCharacterHandler::RigidBodyPushRigidBodyHandler::Hook_ContactPointCallback(RE::FOCollisionListener* a_listener, const RE::hkpContactPointEvent& a_event)
	{
		if (a_event.firingCallbacksForFullManifold && a_event.separatingVelocity) {
			auto attacker = GetActor(a_event.bodies[0]);
			if (attacker) {
				auto target = GetActor(a_event.bodies[1]);
				if (target && ShouldPreventAttackPushing(attacker, target) && a_event.contactMgr && a_event.bodies[1]->simulationIsland) {
					SetInvMassScalingForContact_140AA8740(a_event.contactMgr, a_event.bodies[1], *a_event.bodies[1]->simulationIsland, { 0 });
				}
			}
		}

		ContactPointCallback(a_listener, a_event);
	}

}
