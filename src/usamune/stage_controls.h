#ifndef USAMUNE_STAGE_CONTROLS_H
#define USAMUNE_STAGE_CONTROLS_H

#include "../../include/types.h"
#include "../../include/level_table.h"
#include "../../include/course_table.h"

// WDW water level settings
#define WDW_WATER_LOWEST    0
#define WDW_WATER_LOW       1
#define WDW_WATER_HIGH      2
#define WDW_WATER_HIGHEST   3

// TTC speed settings
#define TTC_SPEED_STOPPED   0
#define TTC_SPEED_SLOW      1
#define TTC_SPEED_FAST      2
#define TTC_SPEED_RANDOM    3
#define TTC_SPEED_FASTEST   4
#define TTC_SPEED_SLOWEST   5

// DDD submarine states
#define DDD_SUB_INITIAL     0
#define DDD_SUB_MOVED       1
#define DDD_SUB_AFTER_BOWSER 2

// CCM/TTM red coin states
#define RED_COIN_STATE_DEFAULT      0
#define RED_COIN_STATE_ALL_SPAWNED  1
#define RED_COIN_STATE_EASY_PATTERN 2

struct UsamuneStageOverrides {
    // WDW (Wet Dry World)
    u8 wdwWaterLevelOverride;
    u8 wdwWaterLevelEnabled;
    s16 wdwCustomWaterHeight;
    
    // TTC (Tick Tock Clock)
    u8 ttcSpeedOverride;
    u8 ttcSpeedEnabled;
    u8 ttcClockFrame;
    
    // Moat water level
    u8 moatDrainedOverride;
    u8 moatDrainedEnabled;
    
    // DDD (Dire Dire Docks)
    u8 dddSubStateOverride;
    u8 dddSubStateEnabled;
    u8 dddSubAlwaysSpawned;
    
    // Spawn overrides
    u8 mipsAlwaysSpawn;
    u8 toadsAlwaysHaveStars;
    u8 switchesUnpressed;
    
    // CCM (Cool Cool Mountain)
    u8 preventFatPenguinRace;
    u8 ccmRedCoinOverride;
    
    // TTM (Tall Tall Mountain)
    u8 ttmRedCoinOverride;
    
    // JRB (Jolly Roger Bay)
    u8 jrbMistToggle;
    u8 jrbMistEnabled;
    
    // Global overrides
    u8 allStarsNonStop;
    u8 defaultRedCoinsEnabled;
    u8 defaultStarCursor;
};

// Function prototypes
void usamune_stage_controls_init(void);
void usamune_apply_stage_overrides(void);
void usamune_stage_controls_on_level_init(void);

// WDW functions
void usamune_toggle_wdw_water(void);
void usamune_set_wdw_water_level(u8 level);
void usamune_apply_wdw_overrides(void);

// TTC functions
void usamune_toggle_ttc_speed(void);
void usamune_set_ttc_speed(u8 speed);
void usamune_apply_ttc_overrides(void);

// Moat functions
void usamune_toggle_moat_water(void);
void usamune_apply_moat_overrides(void);

// DDD functions
void usamune_toggle_ddd_sub(void);
void usamune_set_ddd_sub_state(u8 state);
void usamune_apply_ddd_overrides(void);

// Spawn override functions
void usamune_apply_spawn_overrides(void);
void usamune_toggle_mips_spawn(void);
void usamune_toggle_toad_stars(void);
void usamune_toggle_switches_unpressed(void);

// Level-specific functions
void usamune_apply_ccm_overrides(void);
void usamune_apply_ttm_overrides(void);
void usamune_apply_jrb_overrides(void);
void usamune_toggle_jrb_mist(void);

// Red coin manipulation
void usamune_toggle_ccm_red_coins(void);
void usamune_toggle_ttm_red_coins(void);
void usamune_apply_red_coin_overrides(void);

// Utility functions
void usamune_reset_all_overrides(void);
void usamune_save_stage_config(void);
void usamune_load_stage_config(void);

#endif // USAMUNE_STAGE_CONTROLS_H