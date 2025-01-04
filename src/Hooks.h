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

		class PlayerHook
		{
		public:
			static void InstallHook()
			{
				SKSE::AllocTrampoline(1 << 5);
				auto& trampoline = SKSE::GetTrampoline();

				REL::Relocation<std::uintptr_t> Base{ REL::ID(39379) };  //1.5.97 14069f730
				trampoline.write_branch<5>(Base.address() + 0x67, ModifyMovementData);
				_UpdateMagnetism = trampoline.write_call<5>(Base.address() + 0x42, UpdateMagnetism);

				INFO("{} Done!", __FUNCTION__);
			}

		private:
			static void ModifyMovementData(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_arg3, RE::NiPoint3& a_arg4);

			static void UpdateMagnetism(RE::PlayerCharacter* a_player, float a_delta, RE::NiPoint3& a_translation, float& a_rotationZ);
			static inline REL::Relocation<decltype(UpdateMagnetism)> _UpdateMagnetism;
		};
		friend class PlayerHook;

	private:
		ModifyMovementDataHandler() = delete;
		~ModifyMovementDataHandler() = delete;
	};

}