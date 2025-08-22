#include "practice_core.h"
#include "../../include/sm64.h"
#include "../game/mario.h"
#include "../game/level_update.h"
#include "../game/save_file.h"
#include "../game/hud.h"
#include "../game/ingame_menu.h"
#include "../game/interaction.h"
#include "../engine/math_util.h"

struct UsamuneState gUsamuneState;

// Default button combinations
#define DEFAULT_SAVESTATE_1     (L_TRIG | R_TRIG | U_CBUTTONS)
#define DEFAULT_SAVESTATE_2     (L_TRIG | R_TRIG | D_CBUTTONS)
#define DEFAULT_LOADSTATE_1     (L_TRIG | R_TRIG | L_CBUTTONS)
#define DEFAULT_LOADSTATE_2     (L_TRIG | R_TRIG | R_CBUTTONS)
#define DEFAULT_FREECAM         (L_TRIG | R_TRIG | A_BUTTON)
#define DEFAULT_LEVEL_RESET     (L_TRIG | R_TRIG | B_BUTTON)
#define DEFAULT_SOFT_RESET      (A_BUTTON | B_BUTTON | Z_TRIG | START_BUTTON)

static struct {
    u8 enabled;
    Vec3f pos;
    Vec3f focus;
    s16 yaw;
    s16 pitch;
    f32 speed;
} sFreecam = { FALSE, {0,0,0}, {0,0,0}, 0, 0, 20.0f };

void usamune_init(void) {
    // Initialize Usamune state
    bzero(&gUsamuneState, sizeof(gUsamuneState));
    
    // Set default configuration
    struct UsamuneConfig *config = &gUsamuneState.config;
    
    // Timer defaults
    config->showIGT = TRUE;
    config->showSectionTimer = FALSE;
    config->showAttemptCounter = FALSE;
    config->showStarGrabTimer = FALSE;
    config->showLagCounter = FALSE;
    config->timerFormat = 1; // Centiseconds
    
    // HUD defaults
    config->showSpeedDisplay = FALSE;
    config->showInputDisplay = FALSE;
    config->showWallkickTimer = FALSE;
    config->showMemoryViewer = FALSE;
    config->speedDisplayFormat = 0; // XZ speed
    
    // Practice defaults
    config->freecamEnabled = FALSE;
    config->nonStopStars = FALSE;
    config->noMusicMode = FALSE;
    config->infiniteLives = FALSE;
    config->skipIntro = FALSE;
    
    // Stage override defaults (all disabled)
    config->wdwWaterLevel = 0;
    config->ttcSpeed = 0;
    config->moatDrained = FALSE;
    config->dddSubSpawned = FALSE;
    config->mipsAlwaysSpawn = FALSE;
    config->toadsAlwaysHaveStars = FALSE;
    config->switchesUnpressed = FALSE;
    config->preventFatPenguinRace = FALSE;
    config->jrbMistEnabled = TRUE;
    
    // Input mapping defaults
    config->savestateButton1 = DEFAULT_SAVESTATE_1;
    config->savestateButton2 = DEFAULT_SAVESTATE_2;
    config->loadstateButton1 = DEFAULT_LOADSTATE_1;
    config->loadstateButton2 = DEFAULT_LOADSTATE_2;
    config->freecamButton = DEFAULT_FREECAM;
    config->levelResetButton = DEFAULT_LEVEL_RESET;
    config->softResetButton = DEFAULT_SOFT_RESET;
    
    // Initialize timers
    usamune_timers_init();
}

void usamune_update(void) {
    // Update timers
    usamune_timers_update();

    if (gUsamuneState.config.freecamEnabled) {
    usamune_update_freecam(gMarioState->controller);
}
    
    // Update current speed for display
    if (gMarioState != NULL) {
        gUsamuneState.currentSpeed = sqrtf(
            gMarioState->vel[0] * gMarioState->vel[0] + 
            gMarioState->vel[2] * gMarioState->vel[2]
        );
        gUsamuneState.currentYaw = gMarioState->faceAngle[1];
        
        // Update wallkick timer
        if (gMarioState->action == ACT_WALL_KICK_AIR) {
            gUsamuneState.wallkickTimer = gMarioState->actionTimer;
        } else if (gUsamuneState.wallkickTimer > 0) {
            gUsamuneState.wallkickTimer--;
        }
    }
    
    // Apply stage overrides
    usamune_apply_stage_overrides();
}

void usamune_reset_level(void) {
    // Reset the current level by triggering a level exit
    if (gMarioState != NULL) {
        // Trigger level reset - you may need to adjust this based on SM64's level system
        level_trigger_warp(gMarioState, WARP_OP_DEATH);
    }
}

void usamune_render(void) {
    // Render timers
    usamune_timers_render();

    usamune_hud_extensions_render();
}

void usamune_process_inputs(struct Controller *controller) {
    struct UsamuneConfig *config = &gUsamuneState.config;
    
    // Check savestate inputs
    if (usamune_check_button_combo(controller, config->savestateButton1)) {
        usamune_savestate_save(SAVESTATE_SINGLE);
    }
    if (usamune_check_button_combo(controller, config->savestateButton2)) {
        usamune_savestate_save(SAVESTATE_DOUBLE);
    }

    if (usamune_check_button_combo(controller, config->freecamButton)) {
        usamune_toggle_freecam();
    }
    
    // Check loadstate inputs
    if (usamune_check_button_combo(controller, config->loadstateButton1)) {
        usamune_savestate_load(SAVESTATE_SINGLE);
    }
    if (usamune_check_button_combo(controller, config->loadstateButton2)) {
        usamune_savestate_load(SAVESTATE_DOUBLE);
    }
    
    // Check freecam toggle
    if (usamune_check_button_combo(controller, config->freecamButton)) {
        usamune_toggle_freecam();
    }
    
    // Check level reset
    if (usamune_check_button_combo(controller, config->levelResetButton)) {
        usamune_reset_level();
    }
    
    // Check soft reset
    if (usamune_check_button_combo(controller, config->softResetButton)) {
        usamune_soft_reset();
    }
    
    // Stage-specific toggles
    if (controller->buttonPressed & L_TRIG) {
        if (controller->buttonPressed & U_JPAD) {
            usamune_toggle_wdw_water();
        }
        if (controller->buttonPressed & D_JPAD) {
            usamune_toggle_ttc_speed();
        }
        if (controller->buttonPressed & L_JPAD) {
            usamune_toggle_moat_water();
        }
        if (controller->buttonPressed & R_JPAD) {
            usamune_toggle_ddd_sub();
        }
    }
}

void usamune_soft_reset(void) {
    // Soft reset to file select
    level_trigger_warp(gMarioState, WARP_OP_GAME_OVER);
}

void usamune_toggle_freecam(void) {
    if (!sFreecam.enabled) {
        // Enable freecam - save current camera state
        if (gCamera != NULL) {
            vec3f_copy(sFreecam.pos, gLakituState.pos);
            vec3f_copy(sFreecam.focus, gLakituState.focus);
            sFreecam.yaw = gCamera->yaw;
            calculate_angles(sFreecam.pos, sFreecam.focus, &sFreecam.pitch, &sFreecam.yaw);
        }
        sFreecam.enabled = TRUE;
        gUsamuneState.config.freecamEnabled = TRUE;
    } else {
        // Disable freecam
        sFreecam.enabled = FALSE;
        gUsamuneState.config.freecamEnabled = FALSE;
    }
}

void usamune_update_freecam(struct Controller *controller) {
    if (!sFreecam.enabled || gCamera == NULL) {
        return;
    }
    
    // Calculate movement vectors
    f32 forwardX = coss(sFreecam.yaw) * coss(sFreecam.pitch);
    f32 forwardY = sins(sFreecam.pitch);
    f32 forwardZ = sins(sFreecam.yaw) * coss(sFreecam.pitch);
    
    f32 rightX = coss(sFreecam.yaw + 0x4000);
    f32 rightZ = sins(sFreecam.yaw + 0x4000);
    
    f32 moveSpeed = sFreecam.speed;
    if (controller->buttonDown & Z_TRIG) moveSpeed *= 3.0f; // Speed boost
    if (controller->buttonDown & R_TRIG) moveSpeed *= 0.3f; // Slow mode
    
    // Movement with C buttons
    if (controller->buttonDown & U_CBUTTONS) {
        sFreecam.pos[0] += forwardX * moveSpeed;
        sFreecam.pos[1] += forwardY * moveSpeed;
        sFreecam.pos[2] += forwardZ * moveSpeed;
    }
    if (controller->buttonDown & D_CBUTTONS) {
        sFreecam.pos[0] -= forwardX * moveSpeed;
        sFreecam.pos[1] -= forwardY * moveSpeed;
        sFreecam.pos[2] -= forwardZ * moveSpeed;
    }
    if (controller->buttonDown & L_CBUTTONS) {
        sFreecam.pos[0] -= rightX * moveSpeed;
        sFreecam.pos[2] -= rightZ * moveSpeed;
    }
    if (controller->buttonDown & R_CBUTTONS) {
        sFreecam.pos[0] += rightX * moveSpeed;
        sFreecam.pos[2] += rightZ * moveSpeed;
    }
    
    // Vertical movement with A/B
    if (controller->buttonDown & A_BUTTON) {
        sFreecam.pos[1] += moveSpeed;
    }
    if (controller->buttonDown & B_BUTTON) {
        sFreecam.pos[1] -= moveSpeed;
    }
    
    // Rotation with analog stick
    sFreecam.yaw += controller->stickX * 32;
    sFreecam.pitch -= controller->stickY * 32;
    
    // Clamp pitch
    if (sFreecam.pitch > 0x3000) sFreecam.pitch = 0x3000;
    if (sFreecam.pitch < -0x3000) sFreecam.pitch = -0x3000;
    
    // Calculate focus point
    sFreecam.focus[0] = sFreecam.pos[0] + forwardX * 1000.0f;
    sFreecam.focus[1] = sFreecam.pos[1] + forwardY * 1000.0f;
    sFreecam.focus[2] = sFreecam.pos[2] + forwardZ * 1000.0f;
    
    // Override camera
    vec3f_copy(gLakituState.goalPos, sFreecam.pos);
    vec3f_copy(gLakituState.goalFocus, sFreecam.focus);
    vec3f_copy(gLakituState.pos, sFreecam.pos);
    vec3f_copy(gLakituState.focus, sFreecam.focus);
    gCamera->yaw = sFreecam.yaw;
    
    // Disable smooth movement for instant response
    gLakituState.posHSpeed = 1.0f;
    gLakituState.posVSpeed = 1.0f;
    gLakituState.focHSpeed = 1.0f;
    gLakituState.focVSpeed = 1.0f;
}

u8 usamune_check_button_combo(struct Controller *controller, u16 combo) {
    return (controller->buttonDown & combo) == combo && 
           (controller->buttonPressed & combo) != 0;
}

void usamune_format_time(u32 frames, char *buffer, u8 showCentiseconds) {
    if (showCentiseconds) {
        u32 centiseconds = (frames * 100) / 30; // Convert to centiseconds (assuming 30 FPS)
        u32 seconds = centiseconds / 100;
        u32 minutes = seconds / 60;
        
        centiseconds %= 100;
        seconds %= 60;
        
        sprintf(buffer, "%02d:%02d.%02d", minutes, seconds, centiseconds);
    } else {
        sprintf(buffer, "%d", frames);
    }
}