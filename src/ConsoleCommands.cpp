#include "ConsoleCommands.h"

bool ConsoleCommands::Exec(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData* a_scriptData, RE::TESObjectREFR* a_thisObj, RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, double&, std::uint32_t&)
{
	if (a_thisObj) {
		auto actor = a_thisObj->As<RE::Actor>();
		auto charController = actor ? actor->GetCharController() : nullptr;
		if (charController) {
			auto priority = charController->priority;
			if (actor->IsPlayerRef() && priority == 0) {
				//charController->flags.set(RE::CHARACTER_FLAGS::kNotPushable);
				charController->priority = 2;
			}
			INFO("Priority Is {}", priority);
		}
	}

	return true;
}

void ConsoleCommands::Register()
{
	using Type = RE::SCRIPT_PARAM_TYPE;

	auto info = RE::SCRIPT_FUNCTION::LocateConsoleCommand("ToggleDebugText");  // Unused
	if (info) {
		info->referenceFunction = true;
		info->executeFunction = Exec;
		info->conditionFunction = nullptr;
		info->editorFilter = false;
		info->invalidatesCellList = false;
		info->functionName = LONG_NAME;
		info->shortName = SHORT_NAME;

		DEBUG("Replace ToggleDebugText Console Command Successfully!");
	} else {
		ERROR("Fail to Locate ToggleDebugText Console Command!");
	}
}

void ConsoleCommands::CPrint(const char* a_fmt, ...)
{
	auto console = RE::ConsoleLog::GetSingleton();
	if (console && console->IsConsoleMode()) {
		std::va_list args;
		va_start(args, a_fmt);
		console->VPrint(a_fmt, args);
		va_end(args);
	}
}
