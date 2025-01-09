#pragma once

namespace AMF
{
	class ModifyMovementDataHandler
	{
		static bool IsMovementAnimationDriven_1405E3250(RE::Actor* a_actor);
		static bool RevertPitchRotation(RE::Actor* a_actor, RE::NiPoint3& a_translation, RE::NiPoint3& a_rotation);

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

	private:
		AttackMagnetismHandler() = delete;
		~AttackMagnetismHandler() = delete;
	};
}