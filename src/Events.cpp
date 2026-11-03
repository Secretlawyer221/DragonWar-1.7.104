#include "Events.h"
#include "Settings.h"
#include "FormLoader.h"

namespace Events
{
    OnCombatEventHandler* OnCombatEventHandler::GetSingleton()
    {
        static OnCombatEventHandler singleton;
        return &singleton;
    }

    //Credit: ThirdEyeSqueegee. Event code based on previous code written by ThirdEyeSqueegee
    RE::BSEventNotifyControl OnCombatEventHandler::ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>* a_eventSource)
    {
        if (!a_event || !a_event->actor || !a_event->targetActor)
            return RE::BSEventNotifyControl::kContinue;

        if (a_event->newState != RE::ACTOR_COMBAT_STATE::kCombat)
            return RE::BSEventNotifyControl::kContinue;

        // Cheapest checks first: only care about combat against the player
        if (!a_event->targetActor->IsPlayerRef()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto actor = a_event->actor->As<RE::Actor>();
        if (actor && FormLoader::GetSingleton()->IsDragon(actor) && !actor->HasKeywordString("DW_ActorTypeDragonCSExclude"sv)) {
            SetRandomCombatStyles(actor);
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void OnCombatEventHandler::SetRandomCombatStyles(RE::Actor* actor) {

        auto settings = Settings::GetSingleton();
        const auto& stylePool = FormLoader::GetSingleton()->combatStylePool;
        if (stylePool.empty() || !actor->combatController) {
            return;
        }
        const auto override = settings->OverrideSliders;
        logger::debug("\tini override: {}", override);

        if (settings->currentStyleIndex < 0 || static_cast<std::size_t>(settings->currentStyleIndex) >= stylePool.size()) {
            settings->currentStyleIndex = 0;
        }
        logger::debug("\tStyle Index {}", settings->currentStyleIndex);
        auto* chosenStyle = stylePool[settings->currentStyleIndex];

        settings->currentStyleIndex = (settings->currentStyleIndex + 1) % static_cast<std::int32_t>(stylePool.size());

        //Randomize the style and then add it to the dragon

        // One engine, seeded once, instead of constructing a std::random_device per value
        static std::mt19937 rng{ std::random_device{}() };
        const auto roll = [](float a_min, float a_max) {
            return a_min < a_max ? std::uniform_real_distribution<float>(a_min, a_max)(rng) : a_min;
        };

        const auto dive_bomb_chance     = override ? settings->fDiveBombChance : roll(settings->fDiveBombChanceMin, settings->fDiveBombChanceMax);
        const auto flying_attack_chance = override ? settings->fFlyingAttackChance : roll(settings->fFlyingAttackChanceMin, settings->fFlyingAttackChanceMax);
        const auto ground_attack_chance = override ? settings->fGroundAttackChance : roll(settings->fGroundAttackChanceMin, settings->fGroundAttackChanceMax);
        const auto ground_attack_mult   = override ? settings->fGroundAttackMult : roll(settings->fGroundAttackMultMin, settings->fGroundAttackMultMax);
        const auto hover_chance         = override ? settings->fHoverChance : roll(settings->fHoverChanceMin, settings->fHoverChanceMax);
        const auto hover_mult           = override ? settings->fHoverMult : roll(settings->fHoverMultMin, settings->fHoverMultMax);
        const auto perch_attack_chance  = override ? settings->fPerchAttackChance : roll(settings->fPerchAttackChanceMin, settings->fPerchAttackChanceMax);
        const auto perch_attack_mult    = override ? settings->fPerchAttackMult : roll(settings->fPerchAttackMultMin, settings->fPerchAttackMultMax);
        const auto equip_unarmed_mult   = override ? settings->fEquipUnarmedMult : roll(settings->fEquipUnarmedMultMin, settings->fEquipUnarmedMultMax);
        const auto equip_shout_mult     = override ? settings->fEquipShoutMult : roll(settings->fEquipShoutMultMin, settings->fEquipShoutMultMax);

        chosenStyle->flightData.diveBombChance = dive_bomb_chance;
        logger::debug("\tSet dive bomb chance to {}", dive_bomb_chance);
        chosenStyle->flightData.flyingAttackChance = flying_attack_chance;
        logger::debug("\tSet flying attack chance to {}", flying_attack_chance);
        chosenStyle->flightData.groundAttackChance = ground_attack_chance;
        logger::debug("\tSet ground attack chance to {}", ground_attack_chance);
        chosenStyle->flightData.groundAttackTimeMult = ground_attack_mult;
        logger::debug("\tSet ground attack time mult to {}", ground_attack_mult);
        chosenStyle->flightData.hoverChance = hover_chance;
        logger::debug("\tSet hover chance to {}", hover_chance);
        chosenStyle->flightData.hoverTimeMult = hover_mult;
        logger::debug("\tSet hover time mult to {}", hover_mult);
        chosenStyle->flightData.perchAttackChance = perch_attack_chance;
        logger::debug("\tSet perch attack chance to {}", perch_attack_chance);
        chosenStyle->flightData.perchAttackTimeMult = perch_attack_mult;
        logger::debug("\tSet perch attack time mult to {}", perch_attack_mult);
        chosenStyle->generalData.unarmedScoreMult = equip_unarmed_mult;
        logger::debug("\tSet unarmed score mult to {}", equip_unarmed_mult);
        chosenStyle->generalData.shoutScoreMult = equip_shout_mult;
        logger::debug("\tSet shout score mult to {}", equip_shout_mult);

        // Arguments are evaluated even when debug logging is off, so only touch combatStyle if it is non-null
        if (const auto before = actor->combatController->combatStyle) {
            logger::debug("\tCombat style before: {:X}", before->GetFormID());
        }

        actor->combatController->combatStyle = chosenStyle;
        logger::debug("\tCombat style after: {:X}", chosenStyle->GetFormID());

        logger::debug("\tRandomized {}'s flight data and equip sliders", actor->GetName());
    }

    void OnCombatEventHandler::Register()
    {
        const auto holder = RE::ScriptEventSourceHolder::GetSingleton();
        holder->AddEventSink(GetSingleton());
        logger::info("Registered OnCombat event handler");
    }
}
