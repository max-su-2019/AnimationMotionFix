#pragma once

/*
 * For modders: Copy this file into your own project to use the AMR API.
 *
 * Example:
 *   auto* amr = static_cast<AMR_API::IVAMR1*>(AMR_API::RequestPluginAPI());
 *   if (amr && amr->HasAnimMotionData(hkbCharacter)) { ... }
 */

#include "RE/Skyrim.h"

#include <Windows.h>

namespace AMR_API
{
	constexpr const char* AMRPluginName = "AnimationMotionRevolution";

	// Available Animation Motion Revolution interface versions.
	enum class InterfaceVersion : std::uint8_t
	{
		V1
	};

	// Animation Motion Revolution's modder interface, version 1.
	class IVAMR1
	{
	public:
		/// Returns whether AMR has parsed and cached animation motion data for this hkb character.
		/// A null character pointer returns false.
		[[nodiscard]] virtual bool HasAnimMotionData(
			const RE::hkbCharacter* a_hkbCharacter) const noexcept = 0;
	};

	using _RequestPluginAPI = void* (*)(const InterfaceVersion);

	/// Requests an AMR API interface. Returns nullptr if AMR or the requested version is unavailable.
	[[nodiscard]] inline void* RequestPluginAPI(
		const InterfaceVersion a_interfaceVersion = InterfaceVersion::V1) noexcept
	{
		const auto module = GetModuleHandleA("AnimationMotionRevolution.dll");
		if (!module) {
			return nullptr;
		}

		const auto request = reinterpret_cast<_RequestPluginAPI>(
			GetProcAddress(module, "RequestPluginAPI"));
		return request ? request(a_interfaceVersion) : nullptr;
	}
}
