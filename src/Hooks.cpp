#include "Hooks.h"
#include "Settings.h"

namespace AMF
{
	static bool IsValidActor(const RE::TESObjectREFRPtr& actor)
	{
		return actor && actor->IsActor() && actor->IsInitialized() && actor->Is3DLoaded();
	}

	bool FixPitchTransHandler::RevertPitchRotation(RE::Actor* a_actor, RE::NiPoint3& a_translation)
	{
		if (!AMFSettings::GetSingleton()->enablePitchTranslationFix || a_actor->IsPlayerRef())
			return false;

		const auto actorState = a_actor->AsActorState();
		if (!actorState || actorState->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal || actorState->IsFlying())
			return false;

		bool bIsSynced = false;
		if (a_actor->GetGraphVariableBool(RE::FixedStrings::GetSingleton()->bIsSynced, bIsSynced) && bIsSynced) {
			return false;
		}

		if (a_actor->IsMovementAnimationDriven() && (a_actor->IsAnimationDriven() || a_actor->IsRotationAllowed())) {
			auto pitchAngle = a_actor->data.angle.x;
			if (std::abs(pitchAngle) > 1.57079638f) {
				return false;  //Gimbal Lock Occured
			}

			auto nonPitchTranslationY = a_translation.y / std::cos(pitchAngle);
			auto nonPitchTranslationZ = a_translation.z - nonPitchTranslationY * std::sin(pitchAngle);
			a_translation.y = nonPitchTranslationY;
			a_translation.z = nonPitchTranslationZ;
			return true;
		}

		return false;
	}

	void FixPitchTransHandler::Hook_ConvertMoveDirToTranslation(const RE::NiPoint3& a_angle, RE::NiPoint3& a_outDirection, RE::Actor* a_actor)
	{
		ConvertMoveDirToTranslation(a_angle, a_outDirection);
		RevertPitchRotation(a_actor, a_outDirection);
	}

	bool AttackMagnetismHandler::ShouldDisableMovementMagnetism(RE::Actor* a_actor)
	{
		auto settings = AMFSettings::GetSingleton();
		bool enabled = a_actor->IsPlayerRef() ? settings->disablePlayerMovementMagnetism : settings->disableNpcMovementMagnetism;
		if (enabled) {
			bool bForceMoveMagnetism = false;
			bForceMoveMagnetism = a_actor->GetGraphVariableBool("AMF_bForceMoveMagnetism", bForceMoveMagnetism) && bForceMoveMagnetism;
			return !bForceMoveMagnetism;
		} else {
			bool bForbidMoveMagnetism = false;
			bForbidMoveMagnetism = a_actor->GetGraphVariableBool("AMF_bForbidMoveMagnetism", bForbidMoveMagnetism) && bForbidMoveMagnetism;
			return bForbidMoveMagnetism;
		}
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
		if (!a_pusher || !a_target)
			return false;

		const auto pusherHandle = a_pusher->GetActorRuntimeData().currentCombatTarget;
		auto combatTarg = pusherHandle ? pusherHandle.get() : nullptr;

		if (combatTarg && AttackMagnetismHandler::ShouldDisableMovementMagnetism(a_pusher) && a_pusher->IsAttacking() && a_pusher->IsMovementAnimationDriven()) {
			if (a_target == combatTarg.get() || a_target->GetMountedBy(combatTarg)) {
				return true;
			}
		}

		return false;
	}

	bool PushCharacterHandler::ShouldPreventAttackPushing(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target)
	{
		auto pusherActor = GetActor(a_pusher);
		if (pusherActor) {
			auto targetActor = GetActor(a_target);
			if (targetActor)
				return ShouldPreventAttackPushing(pusherActor->As<RE::Actor>(), targetActor->As<RE::Actor>());
		}

		return false;
	}

	RE::TESObjectREFRPtr PushCharacterHandler::GetActor(RE::bhkCharacterController* a_charCtrl)
	{
		auto rigidBody = a_charCtrl ? a_charCtrl->GetRigidBody() : nullptr;
		if (!rigidBody)
			return nullptr;

		const auto charCollisionFilterInfo = rigidBody->collidable.GetCollisionLayer();
		if (charCollisionFilterInfo != RE::COL_LAYER::kCharController)
			return nullptr;

		RE::TESObjectREFRPtr objRef(rigidBody ? rigidBody->GetUserData() : nullptr);
		if (!IsValidActor(objRef))
			return nullptr;

		return objRef;
	}

	void PushCharacterHandler::ProxyPushProxyHandler::Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint)
	{
		if (ShouldPreventAttackPushing(a_pusher, a_target)) {
			return;
		}

		func(a_pusher, a_target, a_contactPoint);
	}

	void PushCharacterHandler::ProxyPushRigidBodyHandler::Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint)
	{
		if (ShouldPreventAttackPushing(a_pusher, a_target)) {
			return;
		}
		func(a_pusher, a_target, a_contactPoint);
	}

	void PushCharacterHandler::RigidBodyPushProxyHandler::Hook_ProcessConstraintsCallback(RE::bhkCharProxyController* a_proxyCtrl, const RE::hkpCharacterProxy* a_proxy, const RE::hkArray<RE::hkpRootCdPoint>& a_manifold, RE::hkpSimplexSolverInput& a_input)
	{
		ProcessConstraintsCallback(a_proxyCtrl, a_proxy, a_manifold, a_input);

		for (std::int32_t i = 0; i < a_manifold.size(); i++) {
			const RE::hkpRootCdPoint& rootPoint = a_manifold[i];
			const RE::hkpCollidable* rootCollidableB = rootPoint.rootCollidableB;

			if (!rootCollidableB || rootCollidableB->GetCollisionLayer() != RE::COL_LAYER::kCharController)
				continue;

			if (static_cast<RE::hkpWorldObject::BroadPhaseType>(rootCollidableB->broadPhaseHandle.type) != RE::hkpWorldObject::BroadPhaseType::kEntity)
				continue;

			RE::TESObjectREFRPtr attackerRef(RE::TESHavokUtilities::FindCollidableRef(*rootCollidableB));
			RE::TESObjectREFRPtr proxyRef = GetActor(a_proxyCtrl);

			const auto attacker = IsValidActor(attackerRef) ? attackerRef->As<RE::Actor>() : nullptr;
			const auto target = IsValidActor(proxyRef) ? proxyRef->As<RE::Actor>() : nullptr;

			if (ShouldPreventAttackPushing(attacker, target)) {
				auto attackerCharCtrl = attacker->GetCharController();
				auto rigidBodyChar = attackerCharCtrl ? skyrim_cast<RE::bhkCharRigidBodyController*>(attackerCharCtrl) : nullptr;

				if (rigidBodyChar) {
					a_input.constraints[i].velocity = { 0 };

					{
						WriteLocker lock(charCtrlPlaneLock);
						charCtrlPlaneMap.emplace(rigidBodyChar, a_input.constraints[i].plane);
					}
				}
			}
		}
	}

	void PushCharacterHandler::RigidBodyPushProxyHandler::Hook_UpdateForAnimationAttack(RE::bhkCharacterController* a_charCtrl)
	{
		UpdateForAnimationAttack(a_charCtrl);

		auto rigidCharCtrl = a_charCtrl ? skyrim_cast<RE::bhkCharRigidBodyController*>(a_charCtrl) : nullptr;
		if (!rigidCharCtrl)
			return;

		{
			WriteLocker lock(charCtrlPlaneLock);
			auto it = charCtrlPlaneMap.find(rigidCharCtrl);
			if (it != charCtrlPlaneMap.end()) {
				const auto& normal = it->second;
				RE::hkVector4 currentVelocity;
				rigidCharCtrl->GetLinearVelocityImpl(currentVelocity);

				const auto velDotNormal = currentVelocity.Dot3(normal);
				if (velDotNormal > 0.0f) {
					const auto counterVel = normal * (-velDotNormal);
					currentVelocity = currentVelocity + counterVel;
					rigidCharCtrl->SetLinearVelocityImpl(currentVelocity);
				}

				charCtrlPlaneMap.erase(it);
			}
		}
	}

	void PushCharacterHandler::RigidBodyPushProxyHandler::Hook_DeleteThis(RE::bhkCharRigidBodyController* a_charCtrl)
	{
		{
			WriteLocker lock(charCtrlPlaneLock);
			auto it = charCtrlPlaneMap.find(a_charCtrl);
			if (it != charCtrlPlaneMap.end()) {
				charCtrlPlaneMap.erase(it);
			}
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
		const auto prop = a_event.contactPointProperties;
		if (!prop || prop->flags.any(RE::hkContactPointMaterial::Flag::kIsDisabled) || !a_event.contactMgr || !a_event.contactPoint)
			return;

		const auto rigidBodyA = a_event.bodies[0];
		const auto rigidBodyB = a_event.bodies[1];
		const auto islandA = rigidBodyA ? rigidBodyA->simulationIsland : nullptr;
		const auto islandB = rigidBodyB ? rigidBodyB->simulationIsland : nullptr;
		if (islandA && islandB) {
			RE::TESObjectREFRPtr APtr(rigidBodyA->GetUserData());
			RE::TESObjectREFRPtr BPtr(rigidBodyB->GetUserData());

			auto attacker = IsValidActor(APtr) ? APtr->As<RE::Actor>() : nullptr;
			auto target = IsValidActor(BPtr) ? BPtr->As<RE::Actor>() : nullptr;

			if (ShouldPreventAttackPushing(attacker, target)) {
				rigidBodyB->responseModifierFlags |= RE::hkpResponseModifier::Flags::kMassScaling;
				RE::hkpAddModifierUtil::SetInvMassScalingForContact(a_event, rigidBodyB, { 0 });

				if (ShouldPreventAttackPushing(target, attacker)) {
					rigidBodyA->responseModifierFlags |= RE::hkpResponseModifier::Flags::kMassScaling;
					RE::hkpAddModifierUtil::SetInvMassScalingForContact(a_event, rigidBodyA, { 0 });
				}
			}
		}

		ContactPointCallback(a_listener, a_event);
	}
}
