/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: GO_Scripts
SD%Complete: 100
SDComment: Quest support: 1920, 1960, 5097, 5098
SDCategory: Game Objects
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "Entities/Object.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
/* ContentData
go_andorhal_tower
go_containment_coffer
EndContentData */

/*######
## go_andorhal_tower
######*/

enum
{
    QUEST_ALL_ALONG_THE_WATCHTOWERS_ALLIANCE = 5097,
    QUEST_ALL_ALONG_THE_WATCHTOWERS_HORDE    = 5098,
    NPC_ANDORHAL_TOWER_1                     = 10902,
    NPC_ANDORHAL_TOWER_2                     = 10903,
    NPC_ANDORHAL_TOWER_3                     = 10904,
    NPC_ANDORHAL_TOWER_4                     = 10905,
    GO_ANDORHAL_TOWER_1                      = 176094,
    GO_ANDORHAL_TOWER_2                      = 176095,
    GO_ANDORHAL_TOWER_3                      = 176096,
    GO_ANDORHAL_TOWER_4                      = 176097
};

bool GOUse_go_andorhal_tower(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_ALL_ALONG_THE_WATCHTOWERS_ALLIANCE) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(QUEST_ALL_ALONG_THE_WATCHTOWERS_HORDE) == QUEST_STATUS_INCOMPLETE)
    {
        uint32 uiKillCredit = 0;
        switch (pGo->GetEntry())
        {
            case GO_ANDORHAL_TOWER_1:   uiKillCredit = NPC_ANDORHAL_TOWER_1;   break;
            case GO_ANDORHAL_TOWER_2:   uiKillCredit = NPC_ANDORHAL_TOWER_2;   break;
            case GO_ANDORHAL_TOWER_3:   uiKillCredit = NPC_ANDORHAL_TOWER_3;   break;
            case GO_ANDORHAL_TOWER_4:   uiKillCredit = NPC_ANDORHAL_TOWER_4;   break;
        }
        if (uiKillCredit)
            pPlayer->KilledMonsterCredit(uiKillCredit);
    }
    return true;
}

/*####
## go_bells
####*/

enum
{
    // Bells
    EVENT_ID_BELLS = 1024
};

enum BellHourlySoundFX
{
    BELLTOLLTRIBAL = 6595, // Horde
    BELLTOLLHORDE = 6675,
    BELLTOLLALLIANCE = 6594, // Alliance
    BELLTOLLNIGHTELF = 6674,
    BELLTOLLDWARFGNOME = 7234
};

enum BellHourlySoundAreas
{
    // Local areas
    TARREN_MILL_AREA = 272,
    KARAZHAN_MAPID   = 532,
    IRONFORGE_1_AREA = 809,
    BRILL_AREA       = 2118,

    // Global areas (both zone and area)
    UNDERCITY_AREA   = 1497,
    STORMWIND_AREA   = 1519,
    IRONFORGE_2_AREA = 1537,
    DARNASSUS_AREA   = 1657,
};

enum BellHourlyObjects
{
    GO_HORDE_BELL    = 175885,
    GO_ALLIANCE_BELL = 176573
};

struct go_ai_bell : public GameObjectAI
{
    go_ai_bell(GameObject* go) : GameObjectAI(go), m_uiBellTolls(0), m_uiBellSound(GetBellSound(go)), m_uiBellTimer(0), m_playTo(GetBellZoneOrArea(go))
    {
        m_go->SetNotifyOnEventState(true);
        m_go->SetActiveObjectState(true);
    }

    uint32 m_uiBellTolls;
    uint32 m_uiBellSound;
    uint32 m_uiBellTimer;
    PlayPacketSettings m_playTo;

    uint32 GetBellSound(GameObject* pGo) const
    {
        uint32 soundId = 0;
        switch (pGo->GetEntry())
        {
            case GO_HORDE_BELL:
                switch (pGo->GetAreaId())
                {
                    case UNDERCITY_AREA:
                    case BRILL_AREA:
                    case TARREN_MILL_AREA:
                        soundId = BELLTOLLTRIBAL;
                        break;
                    default:
                        soundId = BELLTOLLHORDE;
                        break;
                }
                break;
            case GO_ALLIANCE_BELL:
            {
                switch (pGo->GetAreaId())
                {
                    case IRONFORGE_1_AREA:
                    case IRONFORGE_2_AREA:
                        soundId = BELLTOLLDWARFGNOME;
                        break;
                    case DARNASSUS_AREA:
                        soundId = BELLTOLLNIGHTELF;
                        break;
                    default:
                        soundId = BELLTOLLALLIANCE;
                        break;
                }
                break;
            }
        }
        return soundId;
    }

    PlayPacketSettings GetBellZoneOrArea(GameObject* pGo) const
    {
        PlayPacketSettings playTo = PLAY_AREA;
        switch (pGo->GetEntry())
        {
            case GO_HORDE_BELL:
                switch (pGo->GetAreaId())
                {
                    case UNDERCITY_AREA:
                        playTo = PLAY_ZONE;
                        break;
                }
                break;
            case GO_ALLIANCE_BELL:
            {
                switch (pGo->GetAreaId())
                {
                    case DARNASSUS_AREA:
                    case IRONFORGE_2_AREA:
                        playTo = PLAY_ZONE;
                        break;
                }
                break;
            }
        }
        return playTo;
    }

    void OnEventHappened(uint16 event_id, bool activate, bool resume) override
    {
        if (event_id == EVENT_ID_BELLS && activate && !resume)
        {
            time_t curTime = time(nullptr);
            tm localTm = *localtime(&curTime);
            m_uiBellTolls = (localTm.tm_hour + 11) % 12;

            if (m_uiBellTolls)
                m_uiBellTimer = 3000;

            m_go->GetMap()->PlayDirectSoundToMap(m_uiBellSound, m_go->GetAreaId());
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiBellTimer)
        {
            if (m_uiBellTimer <= uiDiff)
            {
                m_go->PlayDirectSound(m_uiBellSound, PlayPacketParameters(PLAY_AREA, m_go->GetAreaId()));

                m_uiBellTolls--;
                if (m_uiBellTolls)
                    m_uiBellTimer = 3000;
                else
                    m_uiBellTimer = 0;
            }
            else
                m_uiBellTimer -= uiDiff;
        }
    }
};

GameObjectAI* GetAI_go_bells(GameObject* go)
{
    return new go_ai_bell(go);
}

/*####
## go_darkmoon_faire_music
####*/

enum
{
    MUSIC_DARKMOON_FAIRE_MUSIC = 8440
};

struct go_ai_dmf_music : public GameObjectAI
{
    go_ai_dmf_music(GameObject* go) : GameObjectAI(go)
    {
        m_uiMusicTimer = 5000;
    }

    uint32 m_uiMusicTimer;

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiMusicTimer <= uiDiff)
        {
            m_go->PlayMusic(MUSIC_DARKMOON_FAIRE_MUSIC);
            m_uiMusicTimer = 5000;
        }
        else
            m_uiMusicTimer -= uiDiff;
    }
};

GameObjectAI* GetAI_go_darkmoon_faire_music(GameObject* go)
{
    return new go_ai_dmf_music(go);
}

enum
{
    ITEM_GOBLIN_TRANSPONDER = 9173,
};

bool TrapTargetSearch(Unit* unit)
{
    if (unit->GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = static_cast<Player*>(unit);
        if (player->HasItemCount(ITEM_GOBLIN_TRANSPONDER, 1))
            return true;
    }

    return false;
}

/*##################
## go_elemental_rift
##################*/

enum
{
    // Elemental invasions
    NPC_WHIRLING_INVADER        = 14455,
    NPC_WATERY_INVADER          = 14458,
    NPC_BLAZING_INVADER         = 14460,
    NPC_THUNDERING_INVADER      = 14462,

    GO_EARTH_ELEMENTAL_RIFT     = 179664,
    GO_WATER_ELEMENTAL_RIFT     = 179665,
    GO_FIRE_ELEMENTAL_RIFT      = 179666,
    GO_AIR_ELEMENTAL_RIFT       = 179667,
};

struct go_elemental_rift : public GameObjectAI
{
    go_elemental_rift(GameObject* go) : GameObjectAI(go), m_uiElementalTimer(urand(0, 30 * IN_MILLISECONDS)) {}

    uint32 m_uiElementalTimer;

    void DoRespawnElementalsIfCan()
    {
        uint32 elementalEntry;
        switch (m_go->GetEntry())
        {
            case GO_EARTH_ELEMENTAL_RIFT:
                elementalEntry = NPC_THUNDERING_INVADER;
                break;
            case GO_WATER_ELEMENTAL_RIFT:
                elementalEntry = NPC_WATERY_INVADER;
                break;
            case GO_AIR_ELEMENTAL_RIFT:
                elementalEntry = NPC_WHIRLING_INVADER;
                break;
            case GO_FIRE_ELEMENTAL_RIFT:
                elementalEntry = NPC_BLAZING_INVADER;
                break;
            default:
                return;
        }

        CreatureList lElementalList;
        GetCreatureListWithEntryInGrid(lElementalList, m_go, elementalEntry, 35.0f);
        // Do nothing if at least three elementals are found nearby
        if (lElementalList.size() >= 3)
            return;

        // Spawn an elemental at a random point
        float fX, fY, fZ;
        m_go->GetRandomPoint(m_go->GetPositionX(), m_go->GetPositionY(), m_go->GetPositionZ(), 25.0f, fX, fY, fZ);
        m_go->SummonCreature(elementalEntry, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Do nothing if not spawned
        if (!m_go->IsSpawned())
            return;

        if (m_uiElementalTimer <= uiDiff)
        {
            DoRespawnElementalsIfCan();
            m_uiElementalTimer = 30 * IN_MILLISECONDS;
        }
        else
            m_uiElementalTimer -= uiDiff;
    }
};

GameObjectAI* GetAI_go_elemental_rift(GameObject* go)
{
    return new go_elemental_rift(go);
}

std::function<bool(Unit*)> function = &TrapTargetSearch;

enum
{
    SPELL_RALLYING_CRY_OF_THE_DRAGONSLAYER = 22888,
    
    NPC_OVERLORD_RUNTHAK            = 14392,
    NPC_MAJOR_MATTINGLY             = 14394,
    NPC_HIGH_OVERLORD_SAURFANG      = 14720,
    NPC_FIELD_MARSHAL_AFRASIABI     = 14721,

    GO_ONYXIA_H                     = 179556,
    GO_ONYXIA_A                     = 179558,
    GO_NEFARIAN_H                   = 179881,
    GO_NEFARIAN_A                   = 179882,
};

struct go_dragon_head : public GameObjectAI
{
    go_dragon_head(GameObject* go) : GameObjectAI(go) {}

    void JustSpawned() override
    {
        uint32 npcEntry = 0;
        switch (m_go->GetEntry())
        {
            case GO_ONYXIA_H: npcEntry = NPC_OVERLORD_RUNTHAK; break;
            case GO_ONYXIA_A: npcEntry = NPC_MAJOR_MATTINGLY; break;
            case GO_NEFARIAN_H: npcEntry = NPC_HIGH_OVERLORD_SAURFANG; break;
            case GO_NEFARIAN_A: npcEntry = NPC_FIELD_MARSHAL_AFRASIABI; break;
        }

        Unit* caster = GetClosestCreatureWithEntry(m_go, npcEntry, 30.f);
        if (caster)
            caster->CastSpell(nullptr, SPELL_RALLYING_CRY_OF_THE_DRAGONSLAYER, TRIGGERED_OLD_TRIGGERED);
    }
};

GameObjectAI* GetAI_go_dragon_head(GameObject* go)
{
    return new go_dragon_head(go);
}

enum class GoBubblyFissure
{
    SPELL_BUBBLY_FISSURE = 17775,
};

struct go_bubbly_fissure_caster : public GameObjectAI, public TimerManager
{
    go_bubbly_fissure_caster(GameObject* go) : GameObjectAI(go)
    {
        AddCustomAction(1, 2000u, [&]()
        {
            m_go->CastSpell(nullptr, nullptr, (uint32)GoBubblyFissure::SPELL_BUBBLY_FISSURE, TRIGGERED_OLD_TRIGGERED);
            ResetTimer(1, 2000);
        });
    }

    void UpdateAI(const uint32 diff)
    {
        UpdateTimers(diff);
    }
};

enum
{
    SPELL_WARCHIEFS_BLESSING = 16609,

    NPC_THRALL = 4949,
};

struct go_unadorned_spike : public GameObjectAI
{
    go_unadorned_spike(GameObject* go) : GameObjectAI(go) {}

    void OnLootStateChange(Unit* /*user*/) override
    {
        if (m_go->GetLootState() != GO_ACTIVATED)
            return;

        if (Creature* thrall = GetClosestCreatureWithEntry(m_go, NPC_THRALL, 30.f))
            thrall->CastSpell(nullptr, SPELL_WARCHIEFS_BLESSING, TRIGGERED_OLD_TRIGGERED);
    }
};

GameObjectAI* GetAI_go_unadorned_spike(GameObject* go)
{
    return new go_unadorned_spike(go);
}

/*######################
## go_containment_coffer
######################*/

enum
{
    NPC_RIFT_SPAWN = 6492
};

struct go_containment : public GameObjectAI
{
    go_containment(GameObject* go) : GameObjectAI(go), m_activated(false), m_startTimer(2000) {}

    bool m_activated;
    uint32 m_startTimer;

    void UpdateAI(const uint32 diff) override
    {
        if (!m_activated)
        {
            if (m_startTimer < diff)
            {
                // Nearest Rift Spawn NPC must activate this GO_TYPE_BUTTON in order to trigger the linked trap
                if (Creature* riftSpawn = GetClosestCreatureWithEntry(m_go, NPC_RIFT_SPAWN, 5.0f))
                {
                    m_go->Use(riftSpawn);
                    m_activated = true;
                    m_startTimer = 0;
                }
            }
            else
                m_startTimer -= diff;
        }
    }
};

GameObjectAI* GetAI_go_containment(GameObject* go)
{
    return new go_containment(go);
}

struct go_ectoplasmic_distiller_trap : public GameObjectAI
{
    go_ectoplasmic_distiller_trap(GameObject* go) : GameObjectAI(go), m_castTimer(1000) {}

    uint32 m_castTimer;

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_castTimer <= uiDiff)
        {
            m_go->CastSpell(nullptr, nullptr, m_go->GetGOInfo()->trap.spellId, TRIGGERED_OLD_TRIGGERED);
            m_castTimer = 2 * IN_MILLISECONDS;
        }
        else
            m_castTimer -= uiDiff;
    }
};

GameObjectAI* GetAI_go_ectoplasmic_distiller_trap(GameObject* go)
{
    return new go_ectoplasmic_distiller_trap(go);
}

void AddSC_go_scripts()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "go_andorhal_tower";
    pNewScript->pGOUse =          &GOUse_go_andorhal_tower;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_bells";
    pNewScript->GetGameObjectAI = &GetAI_go_bells;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_darkmoon_faire_music";
    pNewScript->GetGameObjectAI = &GetAI_go_darkmoon_faire_music;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_transpolyporter_bb";
    pNewScript->pTrapSearching = &function;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_elemental_rift";
    pNewScript->GetGameObjectAI = &GetAI_go_elemental_rift;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_dragon_head";
    pNewScript->GetGameObjectAI = &GetAI_go_dragon_head;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_bubbly_fissure";
    pNewScript->GetGameObjectAI = &GetNewAIInstance<go_bubbly_fissure_caster>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_unadorned_spike";
    pNewScript->GetGameObjectAI = &GetAI_go_unadorned_spike;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_containment_coffer";
    pNewScript->GetGameObjectAI = &GetAI_go_containment;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_ectoplasmic_distiller_trap";
    pNewScript->GetGameObjectAI = &GetAI_go_ectoplasmic_distiller_trap;
    pNewScript->RegisterSelf();
}
