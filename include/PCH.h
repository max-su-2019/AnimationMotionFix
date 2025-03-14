#pragma once

// c
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>

// cxx
#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <charconv>
#include <chrono>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <deque>
#include <exception>
#include <execution>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <string_view>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

// clib
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

// winnt
#include <ShlObj_core.h>

#include <xbyak/xbyak.h>

using namespace std::literals;
using namespace REL::literals;

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)

template <std::size_t N>
static constexpr std::size_t GetTrampolineBaseSize()
{
	static_assert(N == 5 || N == 6);

	// Reference: write_5branch() and write_6branch() of Trampoline.h
	if constexpr (N == 5) {
#pragma pack(push, 1)
		// FF /4
		// JMP r/m64
		struct TrampolineAssembly
		{
			// jmp [rip]
			std::uint8_t jmp;    // 0 - 0xFF
			std::uint8_t modrm;  // 1 - 0x25
			std::int32_t disp;   // 2 - 0x00000000
			std::uint64_t addr;  // 6 - [rip]
		};
#pragma pack(pop)

		return sizeof(TrampolineAssembly);
	}

	return sizeof(std::uintptr_t);
}

template <std::size_t N>
static constexpr void AllocExactSizeTrampoline(Xbyak::CodeGenerator& a_hookCode)
{
	a_hookCode.ready();

	//SKSE::AllocTrampoline(GetTrampolineBaseSize<N>() + a_hookCode.getSize());
}

template <std::size_t N>
static constexpr std::uintptr_t WriteBranchTrampoline(std::uintptr_t a_location, const Xbyak::CodeGenerator& a_hookCode)
{
	auto& hookCode = const_cast<Xbyak::CodeGenerator&>(a_hookCode);

	AllocExactSizeTrampoline<N>(hookCode);

	return SKSE::GetTrampoline().write_branch<N>(a_location, SKSE::GetTrampoline().allocate(hookCode));
}

// Plugin
#include "Plugin.h"

// DKUtil
#include "DKUtil/Logger.hpp"
