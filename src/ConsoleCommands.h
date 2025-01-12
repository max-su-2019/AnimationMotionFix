#pragma once

class ConsoleCommands
{
public:
	static void Register();

private:
	ConsoleCommands() = delete;
	ConsoleCommands(const ConsoleCommands&) = delete;
	ConsoleCommands(ConsoleCommands&&) = delete;
	~ConsoleCommands() = delete;

	ConsoleCommands& operator=(const ConsoleCommands&) = delete;
	ConsoleCommands& operator=(ConsoleCommands&&) = delete;

	static bool Exec(const RE::SCRIPT_PARAMETER* a_paramInfo, RE::SCRIPT_FUNCTION::ScriptData* a_scriptData, RE::TESObjectREFR* a_thisObj, RE::TESObjectREFR* a_containingObj, RE::Script* a_scriptObj, RE::ScriptLocals* a_locals, double& a_result, std::uint32_t& a_opcodeOffsetPtr);
	static void CPrint(const char* a_fmt, ...);

	static constexpr char LONG_NAME[] = "OutputDebugText";
	static constexpr char SHORT_NAME[] = "ODT";
};
