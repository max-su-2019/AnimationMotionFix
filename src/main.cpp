#include "Hooks.h"
#include "Settings.h"

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME);
	v.PluginVersion(Plugin::VERSION);
	v.AuthorName("Maxsu and SkyHorizon"sv);
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

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	DKUtil::Logger::Init(Plugin::NAME, REL::Module::get().version().string());

	REL::Module::reset();
	SKSE::Init(a_skse, false);

	INFO("{} v{} loaded", Plugin::NAME, Plugin::VERSION);

	// do stuff
	AMF::AMFSettings::GetSingleton();

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
