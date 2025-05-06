#pragma once
#include "DKUtil/Config.hpp"
#include "DKUtil/Utility.hpp"
#include "RE/bhkCharRigidBodyController.h"
#include "RE/bhkCharacterRigidBody.h"
#include "RE/hkpCharacterRigidBodyListener.h"

namespace AMF
{
	// Fix the game engine's translation reduction bug when npcs has pitch angle offset
	class FixPitchTransHandler
	{
		static bool RevertPitchRotation(RE::Actor* a_actor, RE::NiPoint3& a_translation);

		struct Hook : Xbyak::CodeGenerator
		{
			Hook()
			{
				Xbyak::Label hookLabel;
				Xbyak::Label retnLabel;

				mov(r8, rdi);  // rdi = Actor*, same on AE, VR
				call(ptr[rip + hookLabel]);

				jmp(ptr[rip + retnLabel]);

				L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&Hook_ConvertMoveDirToTranslation));
				L(retnLabel), dq(hookedAddress + 5);
			}
		};

	public:
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> Base{ REL::VariantID(36365, 37356, 0x5E0E20) };  //1.5.97 1405D87F0 - 1.6.640 14060FAE0
			hookedAddress = Base.address() + REL::Relocate(0x365, 0x3FE, 0x3B5);
			ConvertMoveDirToTranslation = WriteBranchTrampoline<5>(hookedAddress, Hook());
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		static inline std::uintptr_t hookedAddress;
		static void Hook_ConvertMoveDirToTranslation(RE::NiPoint3& a_movementDirection, RE::NiPoint3& a_translationData, RE::Actor* a_actor);
		static inline REL::Relocation<void (*)(RE::NiPoint3& a_movementDirection, RE::NiPoint3& a_translationData)> ConvertMoveDirToTranslation;

		FixPitchTransHandler() = delete;
		~FixPitchTransHandler() = delete;
	};

	class AttackMagnetismHandler
	{
	public:
		static bool ShouldDisableMovementMagnetism(RE::Actor* a_actor);

		//Disable player 's rotation magnetism effect in combat.
		class PlayerRotateMagnetismHook
		{
		public:
			static void InstallHook()
			{
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::VariantID(39379, 40451, 0x6BFF10) };  //1.5.97 14069f730
				func = trampoline.write_call<5>(Base.address() + 0x42, UpdateMagnetism);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void UpdateMagnetism(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_translation, float& a_rotationZ);
			static inline REL::Relocation<decltype(UpdateMagnetism)> func;
		};

		//Disable actor's movement magnetism effect when performing animation driven attacking.
		class MovementMagnetismHook
		{
		public:
			static void InstallHook()
			{
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::VariantID(36357, 37348, 0x5DF5E0) };  //1.5.97 1405D6FB0
				func = trampoline.write_call<5>(Base.address() + REL::Relocate(0x222, 0x1FB), Hook_IsStartingMeleeAttack);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static bool Hook_IsStartingMeleeAttack(RE::Actor* a_actor);

			static inline REL::Relocation<decltype(Hook_IsStartingMeleeAttack)> func;
		};

	private:
		AttackMagnetismHandler() = delete;
		~AttackMagnetismHandler() = delete;
	};

	//After disable movement magnetism, npcs would pushing away their attack target during animation driven attacking, so we need this handler to prevent the pushing.
	class PushCharacterHandler
	{
	public:
		static bool ShouldPreventAttackPushing(RE::Actor* a_pusher, RE::Actor* a_target);
		static bool ShouldPreventAttackPushing(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target);

		static RE::Actor* GetActor(RE::bhkCharacterController* a_charCtrl);
		static RE::Actor* GetActor(RE::hkpWorldObject* a_rigidBody);

		// Prevent a proxy character controller pushing another proxy controller when performing animation dirven attacking.
		class ProxyPushProxyHandler
		{
		public:
			static void InstallHook()
			{
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::VariantID(77248, 79134, 0xE44BA0) };  //1.5.97 140DEFBA0
				func = trampoline.write_call<5>(Base.address() + REL::Relocate(0x3F, 0x37), Hook_PushTargetCharacter);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint);
			static inline REL::Relocation<decltype(Hook_PushTargetCharacter)> func;
		};

		// Prevent a proxy character controller pushing another rigid body controller when performing animation dirven attacking.
		class ProxyPushRigidBodyHandler
		{
		public:
			static void InstallHook()
			{
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::VariantID(77244, 79130, 0xE44090) };  //1.5.97 140DEF090
				func = trampoline.write_call<5>(Base.address() + REL::Relocate(0x3CD, 0x3B1), Hook_PushTargetCharacter);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint);
			static inline REL::Relocation<decltype(Hook_PushTargetCharacter)> func;
		};

		// Prevent a rigid body character controller pushing another proxy controller when performing animation dirven attacking.
		class RigidBodyPushProxyHandler
		{
			using Lock = std::shared_mutex;
			using ReadLocker = std::shared_lock<Lock>;
			using WriteLocker = std::unique_lock<Lock>;

			static inline Lock charCtrlPlaneLock;
			static inline std::unordered_map<RE::bhkCharRigidBodyController*, RE::hkVector4> charCtrlPlaneMap{};

		public:
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> Vtbl_ProxyCtrl{ RE::VTABLE_bhkCharProxyController[0] };
				ProcessConstraintsCallback = Vtbl_ProxyCtrl.write_vfunc(0x1, &Hook_ProcessConstraintsCallback);

				auto& trampoline = SKSE::GetTrampoline();
				REL::Relocation<std::uintptr_t> Base{ REL::VariantID(77321, 79201, 0xE47B30) };  //1.5.97 140DF2B30
				UpdateForAnimationAttack = trampoline.write_call<5>(Base.address() + REL::Relocate(0x159, 0x147), Hook_UpdateForAnimationAttack);

				REL::Relocation<std::uintptr_t> Vtbl_RigidBodyCtrl{ RE::VTABLE_bhkCharRigidBodyController[0] };
				DeleteThis = Vtbl_RigidBodyCtrl.write_vfunc(0x1, &Hook_DeleteThis);
				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void Hook_ProcessConstraintsCallback(RE::bhkCharProxyController* a_proxyCtrl, const RE::hkpCharacterProxy* a_proxy, const RE::hkArray<RE::hkpRootCdPoint>& a_manifold, RE::hkpSimplexSolverInput& a_input);
			static inline REL::Relocation<decltype(&RE::bhkCharProxyController::ProcessConstraintsCallback)> ProcessConstraintsCallback;

			static void Hook_UpdateForAnimationAttack(RE::bhkCharacterController* a_charCtrl);
			static inline REL::Relocation<decltype(Hook_UpdateForAnimationAttack)> UpdateForAnimationAttack;

			static void Hook_DeleteThis(RE::bhkCharRigidBodyController* a_charCtrl);
			static inline REL::Relocation<decltype(&RE::bhkCharRigidBodyController::DeleteThis)> DeleteThis;
		};

		// Prevent a rigid body character controller pushing another rigid body controller when performing animation dirven attacking.
		class RigidBodyPushRigidBodyHandler
		{
		public:
			class AMFContactListener : public RE::hkpContactListener, public DKUtil::model::Singleton<AMFContactListener>
			{
			public:
				friend DKUtil::model::Singleton<AMFContactListener>;

				void ContactPointCallback(const RE::hkpContactPointEvent& a_event) override;

			private:
				AMFContactListener() = default;
				~AMFContactListener() = default;
			};

			static void InstallHook()
			{
				auto& trampoline = SKSE::GetTrampoline();
				REL::Relocation<std::uintptr_t> Base1{ REL::VariantID(77306, 79186, 0xE466A0) };  //1.5.97 140DF16A0
				PushTargetCharacter = trampoline.write_call<5>(Base1.address() + REL::Relocate(0x29F, 0x2A9), Hook_PushTargetCharacter);

				REL::Relocation<std::uintptr_t> Base2{ REL::VariantID(18709, 19195, 0x282DA0) };  //1.5.97 140271780
				_AddContactListener = trampoline.write_call<5>(Base2.address() + REL::Relocate(0x183, 0x186), Hook_AddContactListener);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint);
			static inline REL::Relocation<decltype(Hook_PushTargetCharacter)> PushTargetCharacter;

			static void Hook_AddContactListener(RE::bhkWorld* a_world, RE::hkpContactListener* a_listener);
			static inline REL::Relocation<decltype(Hook_AddContactListener)> _AddContactListener;
		};

	private:
		PushCharacterHandler() = delete;
		~PushCharacterHandler() = delete;
	};
}