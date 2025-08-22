#include "stage_controls.h"
#include "practice_core.h"
#include "../game/level_update.h"
#include "../game/save_file.h"
#include "../game/mario.h"
#include "../game/area.h"
#include "../game/object_list_processor.h"
#include "../include/behavior_data.h"
#include "../game/object_helpers.h"
#include "../audio/external.h"
#include "../levels/wdw/header.h"
#include "../levels/ttc/header.h"
#include "../levels/ddd/header.h"
#include "../levels/ccm/header.h"
#include "../levels/ttm/header.h"
#include "../levels/jrb/header.h"
#include "../levels/castle_grounds/header.h"
#include "../include/object_fields.h"
#include "dialog_ids.h"
#include "../../include/object_constants.h"
#include <math.h>

#define TOAD_STAR_1_DIALOG DIALOG_082
#define TOAD_STAR_2_DIALOG DIALOG_076
#define TOAD_STAR_3_DIALOG DIALOG_083

#define TOAD_STAR_1_DIALOG_AFTER DIALOG_154
#define TOAD_STAR_2_DIALOG_AFTER DIALOG_155
#define TOAD_STAR_3_DIALOG_AFTER DIALOG_156

extern struct UsamuneState gUsamuneState;

// Global stage override state
static struct UsamuneStageOverrides sStageOverrides;

void usamune_stage_controls_init(void) {
    // Initialize all overrides to disabled/default state
    bzero(&sStageOverrides, sizeof(sStageOverrides));
    
    // Set default values
    sStageOverrides.wdwWaterLevelOverride = WDW_WATER_LOWEST;
    sStageOverrides.ttcSpeedOverride = TTC_SPEED_STOPPED;
    sStageOverrides.dddSubStateOverride = DDD_SUB_INITIAL;
    sStageOverrides.jrbMistEnabled = TRUE; // Mist on by default
    
    // Load configuration from main Usamune state
    usamune_load_stage_config();
}

void usamune_apply_stage_overrides(void) {
    // Apply overrides based on current level
    switch (gCurrLevelNum) {
        case LEVEL_WDW:
            usamune_apply_wdw_overrides();
            break;
        case LEVEL_TTC:
            usamune_apply_ttc_overrides();
            break;
        case LEVEL_DDD:
            usamune_apply_ddd_overrides();
            break;
        case LEVEL_CCM:
            usamune_apply_ccm_overrides();
            break;
        case LEVEL_TTM:
            usamune_apply_ttm_overrides();
            break;
        case LEVEL_JRB:
            usamune_apply_jrb_overrides();
            break;
        case LEVEL_CASTLE_GROUNDS:
        case LEVEL_CASTLE_COURTYARD:
            usamune_apply_moat_overrides();
            break;
    }
    
    // Apply global spawn overrides
    usamune_apply_spawn_overrides();
    
    // Apply red coin overrides
    usamune_apply_red_coin_overrides();
}

void usamune_apply_wdw_overrides(void) {
    if (!sStageOverrides.wdwWaterLevelEnabled) {
        return;
    }
    
    // Set water level based on override
    if (gEnvironmentRegions != NULL) {
        switch (sStageOverrides.wdwWaterLevelOverride) {
            case WDW_WATER_LOWEST:
                gEnvironmentLevels[0] = -1800;
                break;
            case WDW_WATER_LOW:
                gEnvironmentLevels[0] = -1000;
                break;
            case WDW_WATER_HIGH:
                gEnvironmentLevels[0] = 0;
                break;
            case WDW_WATER_HIGHEST:
                gEnvironmentLevels[0] = 1024;
                break;
        }
        
        // Stop any water level changing
        gWDWWaterLevelChanging = FALSE;
    }
}

void usamune_apply_ttc_overrides(void) {
    if (!sStageOverrides.ttcSpeedEnabled) {
        return;
    }
    
    // Override TTC clock speed
    gTTCSpeedSetting = sStageOverrides.ttcSpeedOverride;
}

void usamune_apply_jrb_overrides(void) {
    if (!sStageOverrides.jrbMistEnabled) {
        
        struct ObjectNode *listHead;
        struct Object *obj;
        
        for (s32 list = 0; list < NUM_OBJ_LISTS; list++) {
            listHead = &gObjectLists[list];
            obj = (struct Object *) listHead->next;
            
            while (obj != (struct Object *) listHead) {
                if (obj->activeFlags & ACTIVE_FLAG_ACTIVE) {
                    // Check if this is a mist-related object
                    if (obj->rawData.asF32[O_POS_INDEX + 1] > -1000 && obj->rawData.asF32[O_POS_INDEX + 1] < 1000 && 
                        fabsf(obj->rawData.asF32[O_POS_INDEX + 0]) < 8000 && fabsf(obj->rawData.asF32[O_POS_INDEX + 2]) < 8000) {
                        // This might be a mist particle - disable it
                        obj->activeFlags = ACTIVE_FLAG_DEACTIVATED;
                    }
                }
                obj = (struct Object *) obj->header.next;
            }
        }
    }
}

void usamune_apply_moat_overrides(void) {
    if (sStageOverrides.moatDrainedEnabled && sStageOverrides.moatDrainedOverride) {
        // Set castle grounds moat as drained
        if (gEnvironmentRegions != NULL && gCurrLevelNum == LEVEL_CASTLE_GROUNDS) {
            gEnvironmentLevels[0] = -10000; // Very low water level
        }
    }
}

void usamune_toggle_wdw_water(void) {
    sStageOverrides.wdwWaterLevelEnabled = !sStageOverrides.wdwWaterLevelEnabled;
}

void usamune_toggle_ttc_speed(void) {
    sStageOverrides.ttcSpeedEnabled = !sStageOverrides.ttcSpeedEnabled;
}

void usamune_toggle_moat_water(void) {
    sStageOverrides.moatDrainedEnabled = !sStageOverrides.moatDrainedEnabled;
}

void usamune_toggle_ddd_sub(void) {
    sStageOverrides.dddSubStateEnabled = !sStageOverrides.dddSubStateEnabled;
}

void usamune_apply_ddd_overrides(void) {
    // DDD submarine override implementation
}

void usamune_apply_ccm_overrides(void) {
    // CCM override implementation  
}

void usamune_apply_ttm_overrides(void) {
    // TTM override implementation
}

void usamune_apply_red_coin_overrides(void) {
    // Red coin override implementation
}

void usamune_load_stage_config(void) {
    // Load config from gUsamuneState
}

void usamune_save_stage_config(void) {
    // Save config to gUsamuneState  
}


void usamune_apply_spawn_overrides(void) {
    // MIPS spawn override
    if (sStageOverrides.mipsAlwaysSpawn) {
        // Find MIPS objects and force them to be active using object list iteration
        struct ObjectNode *listHead;
        struct Object *obj;
        
        for (s32 list = 0; list < NUM_OBJ_LISTS; list++) {
            listHead = &gObjectLists[list];
            obj = (struct Object *) listHead->next;
            
            while (obj != (struct Object *) listHead) {
                if (obj->behavior == bhvMips && obj->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
                    obj->activeFlags = ACTIVE_FLAG_ACTIVE;
                    obj->rawData.asS32[0x2F] = 0; // oBhvParams2ndByte - Force 15-star version
                    
                    // Also set other MIPS properties from bhv_mips_init
                    obj->rawData.asU32[0x27] = 0; // oInteractionSubtype = INT_SUBTYPE_HOLDABLE_NPC
#ifndef VERSION_JP
                    obj->rawData.asF32[0x49] = 40.0f; // oMipsForwardVelocity = 40.0f
#endif
                    obj->rawData.asF32[0x17] = 15.0f; // oGravity = 15.0f (or 2.5f for JP)
                    obj->rawData.asF32[0x3A] = 0.89f; // oFriction = 0.89f
                    obj->rawData.asF32[0x3B] = 1.2f;  // oBuoyancy = 1.2f
                }
                obj = (struct Object *) obj->header.next;
            }
        }
    }
    
    // Toad star override
    if (sStageOverrides.toadsAlwaysHaveStars) {
        // Find Toad message objects and modify their dialog using object list iteration
        struct ObjectNode *listHead;
        struct Object *obj;
        
        for (s32 list = 0; list < NUM_OBJ_LISTS; list++) {
            listHead = &gObjectLists[list];
            obj = (struct Object *) listHead->next;
            
            while (obj != (struct Object *) listHead) {
                if (obj->behavior == bhvToadMessage) {
                    // Force Toad to have star dialog regardless of collection status
                    s32 dialogID = (obj->rawData.asS32[0x40] >> 24) & 0xFF; // oBhvParams
                    switch (dialogID) {
                        case TOAD_STAR_1_DIALOG_AFTER:
                            obj->rawData.asU32[0x20] = TOAD_STAR_1_DIALOG; // oToadMessageDialogID
                            break;
                        case TOAD_STAR_2_DIALOG_AFTER:
                            obj->rawData.asU32[0x20] = TOAD_STAR_2_DIALOG;
                            break;
                        case TOAD_STAR_3_DIALOG_AFTER:
                            obj->rawData.asU32[0x20] = TOAD_STAR_3_DIALOG;
                            break;
                    }
                }
                obj = (struct Object *) obj->header.next;
            }
        }
    }
}

void usamune_stage_controls_on_level_init(void) {
    // Called when a level is initialized
    // Some overrides need to be applied at level init time
    usamune_apply_stage_overrides();
}