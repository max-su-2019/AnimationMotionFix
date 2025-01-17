#pragma once

namespace AMF
{
	static bool IsMovementAnimationDriven_1405E3250(RE::Actor* a_actor);

	class ConvertMovementDirectionHook : public Xbyak::CodeGenerator
	{
		static bool RevertPitchRotation(RE::Actor* a_actor, RE::NiPoint3& a_translation);

		struct Hook : Xbyak::CodeGenerator
		{
			Hook()
			{
				Xbyak::Label hookLabel;
				Xbyak::Label retnLabel;

				mov(r8, rdi);  // rdi = Actor*
				call(ptr[rip + hookLabel]);

				jmp(ptr[rip + retnLabel]);

				L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&Hook_ConvertMoveDirToTranslation));
				L(retnLabel), dq(hookedAddress + 5);
			}
		};

	public:
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> Base{ REL::ID(36365) };  //1.5.97 1405D87F0
			hookedAddress = Base.address() + 0x365;
			ConvertMoveDirToTranslation = WriteBranchTrampoline<5>(hookedAddress, Hook());
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		static inline std::uintptr_t hookedAddress;
		static void Hook_ConvertMoveDirToTranslation(RE::NiPoint3& a_movementDirection, RE::NiPoint3& a_translationData, RE::Actor* a_actor);
		static inline REL::Relocation<void (*)(RE::NiPoint3& a_movementDirection, RE::NiPoint3& a_translationData)> ConvertMoveDirToTranslation;
	};

	class ModifyMovementDataHandler
	{
	public:
		class CharacterEx : public RE::Character
		{
		public:
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> CharacterVtbl{ VTABLE[0] };
				func = CharacterVtbl.write_vfunc(0x11A, &Hook_ModifyMovementData);
				INFO("{} Done!", __FUNCTION__);
			}

		private:
			void Hook_ModifyMovementData(float a_delta, RE::NiPoint3& a_translation, RE::NiPoint3& a_rotation);

			static inline REL::Relocation<decltype(&RE::Character::ModifyMovementData)> func;
		};
		friend class CharacterEx;

	private:
		ModifyMovementDataHandler() = delete;
		~ModifyMovementDataHandler() = delete;
	};

	class AttackMagnetismHandler
	{
	public:
		struct PlayerRotateMagnetismHook
		{
		public:
			static void InstallHook()
			{
				SKSE::AllocTrampoline(1 << 5);
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::ID(39379) };  //1.5.97 14069f730
				func = trampoline.write_call<5>(Base.address() + 0x42, UpdateMagnetism);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void UpdateMagnetism(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_translation, float& a_rotationZ);
			static inline REL::Relocation<decltype(UpdateMagnetism)> func;
		};

		struct MovementMagnetismHook
		{
		public:
			static void InstallHook()
			{
				SKSE::AllocTrampoline(1 << 5);
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::ID(36357) };  //1.5.97 1405D6FB0
				func = trampoline.write_call<5>(Base.address() + 0x222, IsStartingMeleeAttack);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static bool IsStartingMeleeAttack(RE::Actor* a_actor);
			static inline REL::Relocation<decltype(IsStartingMeleeAttack)> func;
		};

		struct PushCharacterHook
		{
		public:
			static void InstallHook()
			{
				SKSE::AllocTrampoline(1 << 5);
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::ID(77248) };  //1.5.97 140DEFBA0
				func = trampoline.write_call<5>(Base.address() + 0x3F, Hook_PushTargetCharacter);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void Hook_PushTargetCharacter(RE::bhkCharacterController* a_pusher, RE::bhkCharacterController* a_target, RE::hkContactPoint* a_contactPoint);
			static inline REL::Relocation<decltype(Hook_PushTargetCharacter)> func;
		};

	private:
		AttackMagnetismHandler() = delete;
		~AttackMagnetismHandler() = delete;
	};

}