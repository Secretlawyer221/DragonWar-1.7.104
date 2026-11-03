#pragma once
#include "FormLoader.h"

// Credit note: po3 for Offensive spell AI in po3 tweaks.
// Code isnt the same, but this hooks the same vtable function, and I used it as a reference to do so.
namespace Hooks
{
    template <class T>
    class CheckShouldEquip
    {
    public:
        static void Install(REL::ID a_vtable_id)
        {
            logger::info("Installing Check Equip: {} hook", typeid(T).name());

            REL::Relocation<std::uintptr_t> vtbl{ a_vtable_id };
            func = vtbl.write_vfunc(0xF, thunk);

            logger::info("Installed Check Equip: {} hook", typeid(T).name());
        }

    private:
        static bool thunk(RE::CombatInventoryItemMagicT<RE::CombatInventoryItemShout, T>* a_this, RE::CombatController* a_controller)
        {
            auto result = func(a_this, a_controller);

            if (result && a_controller && a_this->IsValid() && a_this->item) {
                const auto attacker = a_controller->handleCount ? a_controller->cachedAttacker : a_controller->attackerHandle.get();
                auto*      formLoader = FormLoader::GetSingleton();
                if (attacker && !attacker->IsPlayer() && formLoader->IsDragon(attacker.get())) {
                    logger::debug("-----------------------");
                    logger::debug("Check equip {}", attacker->GetName());
                    auto shout = a_this->item->As<RE::TESShout>();

                    auto variationSpell01 = shout ? shout->variations[0].spell : nullptr;

                    if (variationSpell01 && formLoader->noShoutKeyword && variationSpell01->HasKeyword(formLoader->noShoutKeyword)) {
                        return false;
                    }

                }
            }

            return result;
        }

        inline static REL::Relocation<decltype(thunk)> func;
    };

    class OnActorUpdate
    {

            //REL::Relocation<std::uintptr_t> EvaluateProcessHook{ REL::RelocationID(36407, 37401), REL::Relocate(0x44, 0x44) };

    public:
        static void Install()
        {
            logger::info("Installing Actor update");
            // The call site moved in 1.7.x (verified against the 1.7.104 exe: call at +0x683, was +0x674 on 1.6.x)
            const auto callOffset = REX::FModule::GetExecutingModule().GetFileVersion() >= SKSE::RUNTIME_SSE_1_7_99 ? 0x683 : 0x674;
            REL::Relocation<std::uintptr_t> actorUpdate{ REL::ID(37348), callOffset };

            auto& trampoline = REL::GetTrampoline();
            _Update          = trampoline.write_call<5>(actorUpdate.address(), Update);
            logger::info("Hook installed");
        }

    private:
        static void Update(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell)
        {
            auto* formLoader = FormLoader::GetSingleton();

            // Update runs on the main thread and is not re-entrant, so no guard flag is needed
            if (a_actor && formLoader->xMarker && !a_actor->IsPlayer() && formLoader->IsDragon(a_actor)) {
                logger::debug("-----------------------");
                logger::debug("Actor update {}", a_actor->GetName());

                const auto combatTarget = a_actor->currentCombatTarget.get();

                if (combatTarget) {
                    logger::debug("Combat target: {}", combatTarget->GetName());

                    if (const auto process = a_actor->currentProcess) {
                        if (const auto package = process->currentPackage.package) {
                            logger::debug("Current package {:X}", package->GetFormID());
                        }

                        const auto packageTarget = process->currentPackage.target.get();
                        if (packageTarget && packageTarget->GetFormID() == a_actor->GetFormID()) {
                            logger::debug("Package target was self, not swapping");
                        }
                        else {
                            if (packageTarget) {
                                logger::debug("Package target was {}", packageTarget->GetName());
                            }
                            formLoader->xMarker->MoveTo(combatTarget.get());
                        }
                    }
                }
                else {
                    logger::debug("No Combat target");
                }
            }

            return _Update(a_actor, a_zPos, a_cell);
        }

        inline static REL::Relocation<decltype(Update)> _Update;
    };

    inline void Install()
    {
        CheckShouldEquip<RE::CombatMagicCasterTargetEffect>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterTargetEffect_[0]);
        CheckShouldEquip<RE::CombatInventoryItemShout>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterOffensive_[0]);
        CheckShouldEquip<RE::CombatMagicCasterSummon>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterSummon_[0]);
        CheckShouldEquip<RE::CombatMagicCasterReanimate>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterReanimate_[0]);
        CheckShouldEquip<RE::CombatMagicCasterRestore>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterRestore_[0]);
        CheckShouldEquip<RE::CombatMagicCasterScript>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterScript_[0]);
        CheckShouldEquip<RE::CombatMagicCasterStagger>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterStagger_[0]);
        CheckShouldEquip<RE::CombatMagicCasterWard>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterWard_[0]);
        CheckShouldEquip<RE::CombatMagicCasterBoundItem>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterBoundItem_[0]);
        CheckShouldEquip<RE::CombatMagicCasterCloak>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterCloak_[0]);
        CheckShouldEquip<RE::CombatMagicCasterDisarm>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterDisarm_[0]);
        CheckShouldEquip<RE::CombatMagicCasterInvisibility>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterInvisibility_[0]);
        CheckShouldEquip<RE::CombatMagicCasterLight>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterLight_[0]);
        CheckShouldEquip<RE::CombatMagicCasterParalyze>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterParalyze_[0]);

        logger::info("\t\tInstalled magic caster vtable hooks"sv);

        OnActorUpdate::Install();
        logger::info("Installed Actor Update hook");
    }
}
