#include "Hooks.h"
#include "Settings.h"

static void EventCallback(SKSE::MessagingInterface::Message* msg)
{
	using MES = SKSE::MessagingInterface;
	switch (msg->type) {
	case MES::kDataLoaded:
		AMF::AMFSettings::GetSingleton()->OverrideGameSettings();
		break;
	}
}

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
	AMF::ConvertMovementDirectionHook::InstallHook();
	AMF::AttackMagnetismHandler::MovementMagnetismHook::InstallHook();
	AMF::AttackMagnetismHandler::PushCharacterHook::InstallHook();
	AMF::AttackMagnetismHandler::PlayerRotateMagnetismHook::InstallHook();

	auto g_message = SKSE::GetMessagingInterface();
	if (!g_message) {
		ERROR("Messaging Interface Not Found!");
		return false;
	}

	g_message->RegisterListener(EventCallback);

	return true;
}
