#include "Hooks.h"
#include "Settings.h"

DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	DKUtil::Logger::Init(Plugin::NAME, REL::Module::get().version().string());

	REL::Module::reset();
	SKSE::Init(a_skse);

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	// do stuff
	AMF::AMFSettings::GetSingleton();

	AMF::FixPitchTransHandler::InstallHook();

	AMF::AttackMagnetismHandler::PlayerRotateMagnetismHook::InstallHook();
	AMF::AttackMagnetismHandler::MovementMagnetismHook::InstallHook();

	AMF::PushCharacterHandler::ProxyPushProxyHandler::InstallHook();
	AMF::PushCharacterHandler::ProxyPushRigidBodyHandler::InstallHook();
	AMF::PushCharacterHandler::RigidBodyPushProxyHandler::InstallHook();
	AMF::PushCharacterHandler::RigidBodyPushRigidBodyHandler::InstallHook();

	return true;
}
