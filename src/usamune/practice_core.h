#ifndef USAMUNE_PRACTICE_CORE_H
#define USAMUNE_PRACTICE_CORE_H

#include "../../include/types.h"
#include "../game/mario.h"
#include "../game/level_update.h"
#include "../game/save_file.h"

// Usamune version info
#define USAMUNE_VERSION_MAJOR 1
#define USAMUNE_VERSION_MINOR 92
#define USAMUNE_VERSION_STRING "Usamune v1.92"

// Practice file types
#define PRACTICE_FILE_120_STAR     0
#define PRACTICE_FILE_UPSTAIRS_RTA 1  // 74/38/39 stars
#define PRACTICE_FILE_TIPPY_RTA    2  // 103/58 stars

// Timer types
#define TIMER_IGT        0
#define TIMER_SECTION    1
#define TIMER_STAR_GRAB  2
#define TIMER_ATTEMPT    3

// Savestate slots
#define SAVESTATE_SINGLE 0
#define SAVESTATE_DOUBLE 1

// Display positions (can be customized)
#define SPEED_DISPLAY_X 16
#define SPEED_DISPLAY_Y 212
#define INPUT_DISPLAY_X 280
#define INPUT_DISPLAY_Y 200
#define TIMER_DISPLAY_X 16
#define TIMER_DISPLAY_Y 32

struct UsamuneTimers {
    u32 igtFrames;              // In-game timer (includes castle)
    u32 sectionTimer;           // Section timer
    u32 starGrabTimer;          // Timer since last star grab
    u32 attemptCounter;         // Number of attempts
    u32 lagFrameCounter;        // Lag frame counter
    u8 timerRunning;            // Whether timers are active
    u8 showCentiseconds;        // Display format toggle
};

struct UsamuneSavestate {
    // Mario state backup
    struct MarioState marioState;
    Vec3f cameraPos;
    Vec3f cameraFocus;
    s16 cameraYaw;
    s16 cameraPitch;
    
    // Level state
    u16 levelTimer;
    s16 coinsCollected;
    u32 objectStates[64];       // Simplified object state storage
    
    // Flags
    u8 isValid;
    u8 hasWater;
    s16 waterLevel;
};

struct UsamuneConfig {
    // Timer settings
    u8 showIGT;
    u8 showSectionTimer;
    u8 showAttemptCounter;
    u8 showStarGrabTimer;
    u8 showLagCounter;
    u8 timerFormat;             // 0 = frames, 1 = centiseconds
    
    // HUD settings
    u8 showSpeedDisplay;
    u8 showInputDisplay;
    u8 showWallkickTimer;
    u8 showMemoryViewer;
    u8 speedDisplayFormat;      // 0 = XZ speed, 1 = total speed
    
    // Practice settings
    u8 freecamEnabled;
    u8 nonStopStars;
    u8 noMusicMode;
    u8 infiniteLives;
    u8 skipIntro;
    
    // Stage overrides
    u8 wdwWaterLevel;          // 0-3 for different water levels
    u8 ttcSpeed;               // 0-5 for TTC speeds
    u8 moatDrained;
    u8 dddSubSpawned;
    u8 mipsAlwaysSpawn;
    u8 toadsAlwaysHaveStars;
    u8 switchesUnpressed;
    u8 preventFatPenguinRace;
    u8 jrbMistEnabled;
    
    // Input mapping (button combinations)
    u16 savestateButton1;
    u16 savestateButton2;
    u16 loadstateButton1;
    u16 loadstateButton2;
    u16 freecamButton;
    u16 levelResetButton;
    u16 softResetButton;
};

struct UsamuneState {
    struct UsamuneTimers timers;
    struct UsamuneSavestate savestates[2];  // Single and double savestate
    struct UsamuneConfig config;
    
    // Runtime state
    u8 wallkickTimer;           // Frames since wallkick
    f32 currentSpeed;           // Mario's current speed
    s16 currentYaw;             // Mario's current facing angle
    
    // Menu state
    u8 menuOpen;
    u8 selectedOption;
    
    // Debug
    u8 memoryViewerPage;
    u32 memoryViewerAddress;
};

extern struct UsamuneState gUsamuneState;

// Core functions
void usamune_init(void);
void usamune_update(void);
void usamune_render(void);
void usamune_process_inputs(struct Controller *controller);

// Timer functions
void usamune_timers_init(void);
void usamune_timers_update(void);
void usamune_timers_render(void);
void usamune_timers_reset_section(void);
void usamune_timers_increment_attempts(void);

// Savestate functions
void usamune_savestate_save(u8 slot);
void usamune_savestate_load(u8 slot);
u8 usamune_savestate_is_valid(u8 slot);

// Practice tools
void usamune_toggle_freecam(void);
void usamune_warp_to_star(u8 courseNum, u8 starNum);
void usamune_reset_level(void);
void usamune_soft_reset(void);

// Stage controls
void usamune_apply_stage_overrides(void);
void usamune_toggle_wdw_water(void);
void usamune_toggle_ttc_speed(void);
void usamune_toggle_moat_water(void);
void usamune_toggle_ddd_sub(void);

// HUD extensions
void usamune_render_speed_display(void);
void usamune_render_input_display(void);
void usamune_render_wallkick_timer(void);
void usamune_render_memory_viewer(void);

// Practice files
void usamune_load_practice_file(u8 fileType);
void usamune_generate_120_star_file(void);
void usamune_save_preset(u8 slot);
void usamune_load_preset(u8 slot);

// Utility functions
void usamune_format_time(u32 frames, char *buffer, u8 showCentiseconds);
u8 usamune_check_button_combo(struct Controller *controller, u16 combo);

#endif // USAMUNE_PRACTICE_CORE_H