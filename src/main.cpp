#include "Hooks.h"
#include "Settings.h"

#define DLLEXPORT __declspec(dllexport)

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME);
	v.AuthorName("Maxsu and SkyHorizon"sv);
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
	/*
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif
*/

	SKSE::Init(skse, true);

	spdlog::set_pattern("[%H:%M:%S:%e] [%l] %v"s);
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::trace);
	spdlog::flush_on(spdlog::level::trace);
#else
	spdlog::set_level(spdlog::level::info);
	spdlog::flush_on(spdlog::level::info);
#endif

	SKSE::log::info("Game version: {}", skse->RuntimeVersion());

	AMF::AMFSettings::GetSingleton()->LoadSettings();

	SKSE::AllocTrampoline(129);

	AMF::FixPitchTransHandler::InstallHook();

	AMF::AttackMagnetismHandler::PlayerRotateMagnetismHook::InstallHook();
	AMF::AttackMagnetismHandler::MovementMagnetismHook::InstallHook();

	AMF::PushCharacterHandler::ProxyPushProxyHandler::InstallHook();
	AMF::PushCharacterHandler::ProxyPushRigidBodyHandler::InstallHook();
	AMF::PushCharacterHandler::RigidBodyPushProxyHandler::InstallHook();
	AMF::PushCharacterHandler::RigidBodyPushRigidBodyHandler::InstallHook();

	return true;
}
