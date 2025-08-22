#ifndef USAMUNE_SAVESTATES_H
#define USAMUNE_SAVESTATES_H

#include "../../include/types.h"
#include "../game/mario.h"
#include "../game/level_update.h"
#include "../game/save_file.h"

#define SAVESTATE_SINGLE 0
#define SAVESTATE_DOUBLE 1
#define MAX_SAVESTATES 2

// Object state storage for important interactive objects
#define MAX_SAVED_OBJECTS 128

struct UsamuneObjectState {
    uintptr_t behavior;         // Object behavior pointer (changed from u32)
    Vec3f pos;                  // Object position
    Vec3s angle;                // Object rotation
    u32 actionState;            // Object action/state
    u32 flags;                  // Object flags
    f32 health;                 // Object health (for enemies)
    u8 active;                  // Whether object is active
    u32 extraData[4];           // Additional behavior-specific data
};

struct UsamuneSavestateData {
    // Mario state
    struct MarioState marioState;
    Vec3f marioVel;             // Mario velocity backup
    Vec3s marioFaceAngle;       // Mario face angle backup
    u32 marioFlags;             // Mario flags backup
    u32 marioAction;            // Mario action backup
    u16 marioActionState;       // Mario action state backup
    u16 marioActionTimer;       // Mario action timer backup
    u32 marioActionArg;         // Mario action argument backup
    
    // Camera state
    Vec3f cameraPos;
    Vec3f cameraFocus;
    s16 cameraYaw;
    s16 cameraPitch;
    s16 cameraMode;
    
    // Level state
    u16 levelTimer;             // Level timer backup
    s16 coinsCollected;         // Coin count backup
    s16 livesRemaining;         // Lives backup
    s16 healthValue;            // Health backup
    u16 powerMeterValue;        // Power meter backup
    
    // Object states (simplified)
    struct UsamuneObjectState objects[MAX_SAVED_OBJECTS];
    u8 numObjects;
    
    // Environmental state
    s16 waterLevel;             // Current water level
    u8 environmentFlags;        // Environmental state flags
    
    // Audio state
    u16 musicTrack;             // Current music track
    u8 musicVolume;             // Music volume
    
    // Special level states
    union {
        struct {
            u8 ttcClockSetting;     // TTC clock speed
            u8 ttcClockFrame;       // TTC clock frame
        } ttc;
        struct {
            s16 waterHeight;        // WDW water level
            u8 waterState;          // WDW water state
        } wdw;
        struct {
            u8 subPosition;         // DDD submarine position
            u8 subState;            // DDD submarine state
        } ddd;
        struct {
            u8 mistEnabled;         // JRB mist state
        } jrb;
    } levelSpecific;
    
    // Validity and metadata
    u8 isValid;                 // Whether this savestate is valid
    u8 levelNum;                // Level this state was saved in
    u8 areaNum;                 // Area this state was saved in
    u32 frameCount;             // Frame when state was saved
    u32 checksum;               // Simple checksum for validation
};

struct UsamuneSavestateSystem {
    struct UsamuneSavestateData states[MAX_SAVESTATES];
    u8 quickSaveSlot;           // Currently selected quick save slot
    u8 autoSaveEnabled;         // Auto-save on star collection
    u8 saveObjectStates;        // Whether to save object states
    u8 saveCameraState;         // Whether to save camera state
};

// Function prototypes
void usamune_savestates_init(void);
void usamune_savestate_save(u8 slot);
void usamune_savestate_load(u8 slot);
u8 usamune_savestate_is_valid(u8 slot);
void usamune_savestate_clear(u8 slot);
void usamune_savestate_clear_all(void);
void usamune_savestate_import_from_file(u8 slot, const char* filename);

// Internal functions
static void usamune_backup_mario_state(struct UsamuneSavestateData *savedata);
static void usamune_restore_mario_state(struct UsamuneSavestateData *savedata);
static void usamune_backup_camera_state(struct UsamuneSavestateData *savedata);
static void usamune_restore_camera_state(struct UsamuneSavestateData *savedata);
static void usamune_backup_level_state(struct UsamuneSavestateData *savedata);
static void usamune_restore_level_state(struct UsamuneSavestateData *savedata);
static void usamune_backup_object_states(struct UsamuneSavestateData *savedata);
static void usamune_restore_object_states(struct UsamuneSavestateData *savedata);

// Auto-save functionality
void usamune_savestate_auto_save(void);
void usamune_savestate_on_star_collect(void);
void usamune_savestate_on_level_enter(void);

#endif // USAMUNE_SAVESTATES_H