#include "Hooks.h"
#include "FormLoader.h"
#include "Settings.h"
#include "Events.h"

void InitListener(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
        Settings::LoadSettings();
        if (!FormLoader::GetSingleton()->LoadAllForms()) {
            logger::error("Required forms missing, DragonWar hooks and events not installed");
            break;
        }
        Hooks::Install();
        Events::OnCombatEventHandler::Register();
		break;
	}
}

SKSE_PLUGIN_VERSION = []() {
    SKSE::PluginVersionData v{};
    v.PluginVersion(REL::Version{ Version::MAJOR, Version::MINOR, Version::PATCH, 0 });
    v.PluginName("DragonWar"sv);
    v.AuthorName("colinswrath and ThirdEyeSqueegee"sv);
    v.UsesAddressLibrary();
    v.UsesUpdatedStructs();
    return v;
}();

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* skse)
{
    // Sets up logging (SKSE logs folder) and the trampoline
    SKSE::Init(skse, { .trampoline = true, .trampolineSize = 64 });

    logger::info("{} {} loading...", SKSE::GetPluginName(), SKSE::GetPluginVersion().string("."));

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener(InitListener)) {
        return false;
    }

    logger::info("DragonWar loaded successfully");
    return true;
}
