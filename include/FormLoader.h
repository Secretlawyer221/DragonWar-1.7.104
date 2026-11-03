#pragma once

class FormLoader
{
public:
    static FormLoader* GetSingleton()
    {
        static FormLoader formLoader;
        return &formLoader;
    }

    RE::BGSKeyword*                  noShoutKeyword{ nullptr };
    RE::BGSKeyword*                  dragonKeyword{ nullptr };
    RE::TESObjectREFR*               xMarker{ nullptr };
    std::vector<RE::TESCombatStyle*> combatStylePool;

    // True only if every required form was found
    bool loaded{ false };

    // Fast keyword check using the cached form; falls back to the slow string lookup if the form was not found
    bool IsDragon(RE::Actor* a_actor) const
    {
        if (!a_actor) {
            return false;
        }
        return dragonKeyword ? a_actor->HasKeyword(dragonKeyword) : a_actor->HasKeywordString("ActorTypeDragon"sv);
    }

    bool LoadAllForms()
    {
        const auto dragonWarEspName = "DragonWar.esp";
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        logger::info("Loading forms");

        loaded = false;
        combatStylePool.clear();

        if (!dataHandler) {
            logger::error("TESDataHandler not available");
            return false;
        }

        bool ok = true;

        noShoutKeyword = Lookup<RE::BGSKeyword>(dataHandler, 0x877A0C, dragonWarEspName);
        ok &= noShoutKeyword != nullptr;

        // Vanilla ActorTypeDragon keyword. Optional: falls back to string lookup if missing
        dragonKeyword = Lookup<RE::BGSKeyword>(dataHandler, 0x35D59, "Skyrim.esm");

        // Load combat style pool
        const std::pair<RE::FormID, const char*> styles[] = {
            { 0x48C83, "Skyrim.esm" },
            { 0x8AF548, dragonWarEspName },
            { 0x8AF549, dragonWarEspName },
            { 0x8AF54A, dragonWarEspName },
            { 0x8AF54B, dragonWarEspName },
            { 0x8AF54C, dragonWarEspName },
            { 0x8AF54D, dragonWarEspName },
            { 0x8AF54E, dragonWarEspName },
            { 0x8AF54F, dragonWarEspName },
            { 0x8AF550, dragonWarEspName },
        };
        for (const auto& [id, file] : styles) {
            if (auto* style = Lookup<RE::TESCombatStyle>(dataHandler, id, file)) {
                combatStylePool.push_back(style);
            }
            else {
                ok = false;
            }
        }
        ok &= !combatStylePool.empty();

        xMarker = Lookup<RE::TESObjectREFR>(dataHandler, 0x97EFAB, dragonWarEspName);
        ok &= xMarker != nullptr;

        loaded = ok;
        if (ok) {
            logger::info("Forms loaded");
        }
        else {
            logger::error("Failed to load one or more forms. Is {} enabled? DragonWar features are disabled.", dragonWarEspName);
        }
        return ok;
    }

private:
    template <class T>
    static T* Lookup(RE::TESDataHandler* a_handler, RE::FormID a_id, const char* a_file)
    {
        const auto form = a_handler->LookupForm(a_id, a_file);
        const auto result = form ? form->As<T>() : nullptr;
        if (!result) {
            logger::error("Could not find form {:X} in {}", a_id, a_file);
        }
        return result;
    }
};
