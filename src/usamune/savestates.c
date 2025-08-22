#include "savestates.h"
#include "practice_core.h"
#include "../game/mario.h"
#include "../game/level_update.h"
#include "../game/save_file.h"
#include "../game/camera.h"
#include "../game/object_list_processor.h"
#include "../engine/math_util.h"
#include "../../include/behavior_data.h"
#include "../game/object_helpers.h"
#include "../../include/sm64.h"
#include "../audio/external.h"
#include <math.h>

extern struct UsamuneState gUsamuneState;

static u32 usamune_calculate_savestate_checksum(struct UsamuneSavestateData *savedata);
static void usamune_backup_mario_state(struct UsamuneSavestateData *savedata);
static void usamune_restore_mario_state(struct UsamuneSavestateData *savedata);
static void usamune_backup_camera_state(struct UsamuneSavestateData *savedata);
static void usamune_restore_camera_state(struct UsamuneSavestateData *savedata);
static void usamune_backup_level_state(struct UsamuneSavestateData *savedata);
static void usamune_restore_level_state(struct UsamuneSavestateData *savedata);
static void usamune_backup_object_states(struct UsamuneSavestateData *savedata);
static void usamune_restore_object_states(struct UsamuneSavestateData *savedata);

// Global savestate system
static struct UsamuneSavestateSystem sSavestateSystem;

void usamune_savestates_init(void) {
    // Clear all savestate data
    bzero(&sSavestateSystem, sizeof(sSavestateSystem));
    
    // Set default configuration
    sSavestateSystem.quickSaveSlot = SAVESTATE_SINGLE;
    sSavestateSystem.autoSaveEnabled = FALSE;
    sSavestateSystem.saveObjectStates = TRUE;
    sSavestateSystem.saveCameraState = TRUE;
}

void usamune_savestate_save(u8 slot) {
    if (slot >= MAX_SAVESTATES) {
        return; // Invalid slot
    }
    
    // Can't save during certain states
    if (gMarioState == NULL || 
        gMarioState->action == ACT_UNINITIALIZED ||
        gCurrLevelNum == LEVEL_NONE) {
        return;
    }
    
    struct UsamuneSavestateData *savedata = &sSavestateSystem.states[slot];
    
    // Clear previous data
    bzero(savedata, sizeof(struct UsamuneSavestateData));
    
    // Backup Mario state
    usamune_backup_mario_state(savedata);
    
    // Backup camera state
    if (sSavestateSystem.saveCameraState) {
        usamune_backup_camera_state(savedata);
    }
    
    // Backup level state
    usamune_backup_level_state(savedata);
    
    // Backup object states
    if (sSavestateSystem.saveObjectStates) {
        usamune_backup_object_states(savedata);
    }
    
    // Set metadata
    savedata->isValid = TRUE;
    savedata->levelNum = gCurrLevelNum;
    savedata->areaNum = gCurrAreaIndex;
    savedata->frameCount = gUsamuneState.timers.igtFrames;
    
    // Calculate checksum
    savedata->checksum = usamune_calculate_savestate_checksum(savedata);
    
    // Play confirmation sound
    play_sound(SOUND_MENU_CLICK_FILE_SELECT, gGlobalSoundSource);
}

static void usamune_restore_object_states(struct UsamuneSavestateData *savedata) {
    // Iterate through all object lists to find objects to restore
    struct ObjectNode *listHead;
    struct Object *obj;
    
    for (s32 list = 0; list < NUM_OBJ_LISTS; list++) {
        listHead = &gObjectLists[list];
        obj = (struct Object *) listHead->next;
        
        while (obj != (struct Object *) listHead) {
            if (!(obj->activeFlags & ACTIVE_FLAG_ACTIVE) || obj->behavior == NULL) {
                obj = (struct Object *) obj->header.next;
                continue;
            }
            
            // Find matching saved object
            for (s32 j = 0; j < savedata->numObjects; j++) {
                struct UsamuneObjectState *objState = &savedata->objects[j];
                
                if (objState->behavior == (uintptr_t)obj->behavior &&
                    fabsf(objState->pos[0] - obj->rawData.asF32[0x37]) < 100.0f &&  // oHomeX
                    fabsf(objState->pos[2] - obj->rawData.asF32[0x39]) < 100.0f) {  // oHomeZ
                    
                    // Restore object state
                    obj->rawData.asF32[O_POS_INDEX + 0] = objState->pos[0];     // oPosX
                    obj->rawData.asF32[O_POS_INDEX + 1] = objState->pos[1];     // oPosY
                    obj->rawData.asF32[O_POS_INDEX + 2] = objState->pos[2];     // oPosZ
                    obj->rawData.asS32[O_MOVE_ANGLE_PITCH_INDEX] = objState->angle[0];  // oMoveAnglePitch
                    obj->rawData.asS32[O_MOVE_ANGLE_YAW_INDEX] = objState->angle[1];    // oMoveAngleYaw
                    obj->rawData.asS32[O_MOVE_ANGLE_ROLL_INDEX] = objState->angle[2];   // oMoveAngleRoll
                    obj->rawData.asS32[0x31] = objState->actionState;           // oAction
                    obj->rawData.asS32[0x3F] = objState->health;                // oHealth
                    
                    objState->active = FALSE; // Mark as restored
                    break;
                }
            }
            
            obj = (struct Object *) obj->header.next;
        }
    }
}

void usamune_savestate_load(u8 slot) {
    if (slot >= MAX_SAVESTATES) {
        return; // Invalid slot
    }
    
    struct UsamuneSavestateData *savedata = &sSavestateSystem.states[slot];
    
    // Check if savestate is valid
    if (!savedata->isValid) {
        return; // No savestate in this slot
    }
    
    // Verify checksum
    if (savedata->checksum != usamune_calculate_savestate_checksum(savedata)) {
        // Savestate corrupted
        savedata->isValid = FALSE;
        return;
    }
    
    // Can't load if not in the same level
    if (savedata->levelNum != gCurrLevelNum || 
        savedata->areaNum != gCurrAreaIndex) {
        return; // Wrong level/area
    }
    
    // Restore Mario state
    usamune_restore_mario_state(savedata);
    
    // Restore camera state
    if (sSavestateSystem.saveCameraState) {
        usamune_restore_camera_state(savedata);
    }
    
    // Restore level state
    usamune_restore_level_state(savedata);
    
    // Restore object states
    if (sSavestateSystem.saveObjectStates) {
        usamune_restore_object_states(savedata);
    }
    
    // Play confirmation sound
    play_sound(SOUND_MENU_CLICK_CHANGE_VIEW, gGlobalSoundSource);
}

u8 usamune_savestate_is_valid(u8 slot) {
    if (slot >= MAX_SAVESTATES) {
        return FALSE;
    }
    
    struct UsamuneSavestateData *savedata = &sSavestateSystem.states[slot];
    return savedata->isValid && 
           savedata->levelNum == gCurrLevelNum && 
           savedata->areaNum == gCurrAreaIndex;
}

void usamune_savestate_clear(u8 slot) {
    if (slot >= MAX_SAVESTATES) {
        return;
    }
    
    sSavestateSystem.states[slot].isValid = FALSE;
}

void usamune_savestate_clear_all(void) {
    for (u8 i = 0; i < MAX_SAVESTATES; i++) {
        usamune_savestate_clear(i);
    }
}

static void usamune_backup_mario_state(struct UsamuneSavestateData *savedata) {
    if (gMarioState == NULL) {
        return;
    }
    
    // Copy Mario state structure
    bcopy(gMarioState, &savedata->marioState, sizeof(struct MarioState));
    
    // Backup additional Mario data
    vec3f_copy(savedata->marioVel, gMarioState->vel);
    vec3s_copy(savedata->marioFaceAngle, gMarioState->faceAngle);
    savedata->marioFlags = gMarioState->flags;
    savedata->marioAction = gMarioState->action;
    savedata->marioActionState = gMarioState->actionState;
    savedata->marioActionTimer = gMarioState->actionTimer;
    savedata->marioActionArg = gMarioState->actionArg;
}

static void usamune_restore_mario_state(struct UsamuneSavestateData *savedata) {
    if (gMarioState == NULL) {
        return;
    }
    
    // Restore Mario state structure
    bcopy(&savedata->marioState, gMarioState, sizeof(struct MarioState));
    
    // Restore additional Mario data
    vec3f_copy(gMarioState->vel, savedata->marioVel);
    vec3s_copy(gMarioState->faceAngle, savedata->marioFaceAngle);
    gMarioState->flags = savedata->marioFlags;
    gMarioState->action = savedata->marioAction;
    gMarioState->actionState = savedata->marioActionState;
    gMarioState->actionTimer = savedata->marioActionTimer;
    gMarioState->actionArg = savedata->marioActionArg;
    
    // Update Mario's object position
    if (gMarioState->marioObj != NULL) {
        vec3f_copy(gMarioState->marioObj->header.gfx.pos, gMarioState->pos);
        vec3s_copy(gMarioState->marioObj->header.gfx.angle, gMarioState->faceAngle);
    }
}

static void usamune_backup_camera_state(struct UsamuneSavestateData *savedata) {
    // Backup camera position and focus
    vec3f_copy(savedata->cameraPos, gCamera->pos);
    vec3f_copy(savedata->cameraFocus, gCamera->focus);
    
    // Compute yaw and pitch from pos and focus
    f32 dist, yaw, pitch;
    vec3f_get_dist_and_angle(gCamera->focus, gCamera->pos, &dist, &yaw, &pitch);
    
    savedata->cameraYaw = (s16)yaw;
    savedata->cameraPitch = (s16)pitch;
    savedata->cameraMode = gCamera->mode;
}


static void usamune_restore_camera_state(struct UsamuneSavestateData *savedata) {
    // Restore camera position and focus  
    vec3f_copy(gCamera->pos, savedata->cameraPos);
    vec3f_copy(gCamera->focus, savedata->cameraFocus);
    gCamera->yaw = savedata->cameraYaw;
    f32 dist, yaw, pitch;
    vec3f_get_dist_and_angle(gCamera->focus, gCamera->pos, &dist, &yaw, &pitch);
    gCamera->mode = savedata->cameraMode;
}

static void usamune_backup_level_state(struct UsamuneSavestateData *savedata) {
    // Backup HUD display values
    savedata->levelTimer = gHudDisplay.timer;
    savedata->coinsCollected = gHudDisplay.coins;
    savedata->livesRemaining = gHudDisplay.lives;
    
    // Backup Mario's health
    if (gMarioState != NULL) {
        savedata->healthValue = gMarioState->health;
    }
    
    // Backup level-specific states
    switch (gCurrLevelNum) {
        case LEVEL_TTC:
            savedata->levelSpecific.ttc.ttcClockSetting = getTTCClockSetting();
            savedata->levelSpecific.ttc.ttcClockFrame = getTTCClockFrame();
            break;
        case LEVEL_WDW:
            savedata->levelSpecific.wdw.waterHeight = getWDWWaterHeight();
            savedata->levelSpecific.wdw.waterState = getWDWWaterState();
            break;
        case LEVEL_DDD:
            savedata->levelSpecific.ddd.subPosition = getDDDSubPosition();
            savedata->levelSpecific.ddd.subState = getDDDSubState();
            break;
        case LEVEL_JRB:
            savedata->levelSpecific.jrb.mistEnabled = getJRBMistState();
            break;
    }
}

static void usamune_restore_level_state(struct UsamuneSavestateData *savedata) {
    // Restore HUD display values
    gHudDisplay.timer = savedata->levelTimer;
    gHudDisplay.coins = savedata->coinsCollected;
    gHudDisplay.lives = savedata->livesRemaining;
    
    // Restore Mario's health
    if (gMarioState != NULL) {
        gMarioState->health = savedata->healthValue;
    }
    
    // Restore level-specific states
    switch (gCurrLevelNum) {
        case LEVEL_TTC:
            setTTCClockSetting(savedata->levelSpecific.ttc.ttcClockSetting);
            setTTCClockFrame(savedata->levelSpecific.ttc.ttcClockFrame);
            break;
        case LEVEL_WDW:
            setWDWWaterHeight(savedata->levelSpecific.wdw.waterHeight);
            setWDWWaterState(savedata->levelSpecific.wdw.waterState);
            break;
        case LEVEL_DDD:
            setDDDSubPosition(savedata->levelSpecific.ddd.subPosition);
            setDDDSubState(savedata->levelSpecific.ddd.subState);
            break;
        case LEVEL_JRB:
            setJRBMistState(savedata->levelSpecific.jrb.mistEnabled);
            break;
    }
}

void usamune_backup_object_states(struct UsamuneSavestateData *savedata) {
    savedata->numObjects = 0;
    
    // Iterate through all object lists instead of gObjectPool directly
    struct ObjectNode *listHead;
    struct Object *obj;
    
    for (s32 list = 0; list < NUM_OBJ_LISTS; list++) {
        listHead = &gObjectLists[list];
        obj = (struct Object *) listHead->next;
        
        while (obj != (struct Object *) listHead) {
            if (savedata->numObjects >= MAX_SAVED_OBJECTS) break;
            
            // Only save active objects with behavior
            if (!(obj->activeFlags & ACTIVE_FLAG_ACTIVE) || obj->behavior == NULL) {
                obj = (struct Object *) obj->header.next;
                continue;
            }
            
            // Skip certain object types
            if (obj->behavior == bhvMario || 
                obj->behavior == bhvCameraLakitu ||
                obj->behavior == bhvStaticObject) {
                obj = (struct Object *) obj->header.next;
                continue;
            }
            
            // Save object state
            struct UsamuneObjectState *objState = &savedata->objects[savedata->numObjects];
            objState->behavior = (uintptr_t)obj->behavior;
            objState->pos[0] = obj->rawData.asF32[O_POS_INDEX + 0];
            objState->pos[1] = obj->rawData.asF32[O_POS_INDEX + 1];
            objState->pos[2] = obj->rawData.asF32[O_POS_INDEX + 2];
            objState->angle[0] = obj->rawData.asS32[O_MOVE_ANGLE_PITCH_INDEX];
            objState->angle[1] = obj->rawData.asS32[O_MOVE_ANGLE_YAW_INDEX];
            objState->angle[2] = obj->rawData.asS32[O_MOVE_ANGLE_ROLL_INDEX];
            objState->actionState = obj->rawData.asS32[0x31];
            objState->flags = obj->activeFlags;
            objState->health = obj->rawData.asS32[0x3F];
            objState->active = TRUE;
            
            savedata->numObjects++;
            obj = (struct Object *) obj->header.next;
        }
    }
}

static u32 usamune_calculate_savestate_checksum(struct UsamuneSavestateData *savedata) {
    // Simple checksum calculation for savestate validation
    u32 checksum = 0;
    u8 *data = (u8*)savedata;
    u32 size = sizeof(struct UsamuneSavestateData) - sizeof(u32); // Exclude checksum field
    
    for (u32 i = 0; i < size; i++) {
        checksum += data[i];
        checksum = (checksum << 1) | (checksum >> 31); // Rotate left
    }
    
    return checksum;
}

void usamune_savestate_auto_save(void) {
    if (sSavestateSystem.autoSaveEnabled) {
        usamune_savestate_save(SAVESTATE_SINGLE);
    }
}

void usamune_savestate_on_star_collect(void) {
    if (sSavestateSystem.autoSaveEnabled) {
        // Auto-save before star collection
        usamune_savestate_save(SAVESTATE_SINGLE);
    }
}

void usamune_savestate_on_level_enter(void) {
    // Clear savestates when entering a new level
    usamune_savestate_clear_all();
}

void usamune_savestate_set_auto_save(u8 enabled) {
    sSavestateSystem.autoSaveEnabled = enabled;
}

u8 usamune_savestate_get_auto_save(void) {
    return sSavestateSystem.autoSaveEnabled;
}

void usamune_savestate_set_save_objects(u8 enabled) {
    sSavestateSystem.saveObjectStates = enabled;
}

void usamune_savestate_set_save_camera(u8 enabled) {
    sSavestateSystem.saveCameraState = enabled;
}

u8 usamune_savestate_get_slot_level(u8 slot) {
    if (slot >= MAX_SAVESTATES) {
        return LEVEL_NONE;
    }
    
    return sSavestateSystem.states[slot].levelNum;
}

u32 usamune_savestate_get_slot_frame(u8 slot) {
    if (slot >= MAX_SAVESTATES) {
        return 0;
    }
    
    return sSavestateSystem.states[slot].frameCount;
}

// Debug functions for savestate system

void usamune_savestate_debug_info(u8 slot) {
    if (slot >= MAX_SAVESTATES) {
        return;
    }
    
    struct UsamuneSavestateData *savedata = &sSavestateSystem.states[slot];
    
    if (!savedata->isValid) {
        // Print "Slot X: Empty"
        return;
    }
    
    // Print savestate info
    // printf("Slot %d: Level %d, Area %d, Frame %d\n", 
    //        slot, savedata->levelNum, savedata->areaNum, savedata->frameCount);
    // printf("Mario Pos: (%.2f, %.2f, %.2f)\n", 
    //        savedata->marioState.pos[0], savedata->marioState.pos[1], savedata->marioState.pos[2]);
    // printf("Mario Action: 0x%08X, Health: %d\n", 
    //        savedata->marioAction, savedata->healthValue);
}


void usamune_savestate_quick_save(void) {
    usamune_savestate_save(sSavestateSystem.quickSaveSlot);
}

void usamune_savestate_quick_load(void) {
    usamune_savestate_load(sSavestateSystem.quickSaveSlot);
}

void usamune_savestate_cycle_slot(void) {
    sSavestateSystem.quickSaveSlot = (sSavestateSystem.quickSaveSlot + 1) % MAX_SAVESTATES;
}


void usamune_savestate_save_with_screenshot(u8 slot) {
    // Save normal savestate
    usamune_savestate_save(slot);
    
    // TODO: Capture and save a small screenshot for the savestate
    // This would require framebuffer access and image compression
    // Only possible on PC iirc
}

void usamune_savestate_export_to_file(u8 slot, const char* filename) {
    // TODO: Export savestate to external file
    // This would allow sharing savestates between players
    if (slot >= MAX_SAVESTATES) {
        return;
    }
    
    struct UsamuneSavestateData *savedata = &sSavestateSystem.states[slot];
    if (!savedata->isValid) {
        return;
    }
    
    // Write savestate data to file
    // FILE *file = fopen(filename, "wb");
    // if (file) {
    //     fwrite(savedata, sizeof(struct UsamuneSavestateData), 1, file);
    //     fclose(file);
    // }
}

void usamune_savestate_import_from_file(u8 slot, const char* filename) {
    // TODO: Import savestate from external file
    if (slot >= MAX_SAVESTATES) {
        return;
    }
    
    // Read savestate data from file
    // FILE *file = fopen(filename, "rb");
    // if (file) {
    //     struct UsamuneSavestateData *savedata = &sSavestateSystem.states[slot];
    //     fread(savedata, sizeof(struct UsamuneSavestateData), 1, file);
    //     fclose(file);
    //     
    //     // Verify checksum
    //     if (savedata->checksum != usamune_calculate_savestate_checksum(savedata)) {
    //         savedata->isValid = FALSE;
    //     }
    // }
}