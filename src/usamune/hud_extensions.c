#include "hud_extensions.h"
#include "practice_core.h"
#include "../game/ingame_menu.h"
#include "../game/mario.h"
#include "../game/mario_actions_airborne.h"
#include "../engine/math_util.h"
#include "../game/print.h"
#include <stdio.h>
#include "../../include/sm64.h"
#include "../game/object_list_processor.h"

extern struct UsamuneState gUsamuneState;

// HUD extension state
static struct UsamuneSpeedDisplay sSpeedDisplay;
static struct UsamuneInputDisplay sInputDisplay;
static struct UsamuneWallkickDisplay sWallkickDisplay;
static struct UsamuneMemoryViewer sMemoryViewer;
static struct UsamuneDebugDisplay sDebugDisplay;

void usamune_hud_extensions_init(void) {
    usamune_speed_display_init();
    usamune_input_display_init();
    usamune_wallkick_display_init();
    usamune_memory_viewer_init();
    usamune_debug_display_init();
}

void usamune_hud_extensions_update(void) {
    usamune_speed_display_update();
    usamune_input_display_update();
    usamune_wallkick_display_update();
    usamune_memory_viewer_update();
    usamune_debug_display_update();
}

void usamune_hud_extensions_render(void) {
    if (sSpeedDisplay.enabled) {
        usamune_render_speed_display();
    }
    
    if (sInputDisplay.enabled) {
        usamune_render_input_display();
    }
    
    if (sWallkickDisplay.enabled && sWallkickDisplay.displayTimer > 0) {
        usamune_render_wallkick_timer();
    }
    
    if (sMemoryViewer.enabled) {
        usamune_render_memory_viewer();
    }
    
    if (sDebugDisplay.enabled) {
        usamune_render_debug_display();
    }
}

// ============================================================================
// SPEED DISPLAY IMPLEMENTATION
// ============================================================================

void usamune_speed_display_init(void) {
    sSpeedDisplay.enabled = FALSE;
    sSpeedDisplay.format = SPEED_FORMAT_XZ;
    sSpeedDisplay.showDecimals = TRUE;
    sSpeedDisplay.showDirection = FALSE;
    sSpeedDisplay.posX = SPEED_DISPLAY_X;
    sSpeedDisplay.posY = SPEED_DISPLAY_Y;
    sSpeedDisplay.color = HUD_COLOR_WHITE;
    sSpeedDisplay.currentSpeed = 0.0f;
    sSpeedDisplay.maxSpeed = 0.0f;
    sSpeedDisplay.averageSpeed = 0.0f;
}

void usamune_speed_display_update(void) {
    if (!sSpeedDisplay.enabled || gMarioState == NULL) {
        return;
    }
    
    // Calculate current speed based on format
    sSpeedDisplay.currentSpeed = usamune_calculate_speed(gMarioState, sSpeedDisplay.format);
    
    // Update maximum speed
    if (sSpeedDisplay.currentSpeed > sSpeedDisplay.maxSpeed) {
        sSpeedDisplay.maxSpeed = sSpeedDisplay.currentSpeed;
    }
    
    // Update rolling average (simple exponential moving average)
    static f32 alpha = 0.1f; // Smoothing factor
    sSpeedDisplay.averageSpeed = alpha * sSpeedDisplay.currentSpeed + 
                                (1.0f - alpha) * sSpeedDisplay.averageSpeed;
}

void usamune_render_speed_display(void) {
    char speedBuffer[64];
    
    switch (sSpeedDisplay.format) {
        case SPEED_FORMAT_XZ:
            if (sSpeedDisplay.showDecimals) {
                sprintf(speedBuffer, "Speed: %.2f", sSpeedDisplay.currentSpeed);
            } else {
                sprintf(speedBuffer, "Speed: %.0f", sSpeedDisplay.currentSpeed);
            }
            break;
            
        case SPEED_FORMAT_TOTAL:
            if (sSpeedDisplay.showDecimals) {
                sprintf(speedBuffer, "3D Speed: %.2f", sSpeedDisplay.currentSpeed);
            } else {
                sprintf(speedBuffer, "3D Speed: %.0f", sSpeedDisplay.currentSpeed);
            }
            break;
            
        case SPEED_FORMAT_FORWARD:
            if (sSpeedDisplay.showDecimals) {
                sprintf(speedBuffer, "Forward: %.2f", gMarioState->forwardVel);
            } else {
                sprintf(speedBuffer, "Forward: %.0f", gMarioState->forwardVel);
            }
            break;
            
        case SPEED_FORMAT_DETAILED:
            sprintf(speedBuffer, "Vel: %.1f,%.1f,%.1f", 
                   gMarioState->vel[0], gMarioState->vel[1], gMarioState->vel[2]);
            break;
    }
    
    usamune_render_text_with_color(sSpeedDisplay.posX, sSpeedDisplay.posY, 
                                  speedBuffer, sSpeedDisplay.color);
    
    // Show max speed if enabled
    if (sSpeedDisplay.showDirection) {
        char maxBuffer[32];
        sprintf(maxBuffer, "Max: %.2f", sSpeedDisplay.maxSpeed);
        usamune_render_text_with_color(sSpeedDisplay.posX, sSpeedDisplay.posY + 16, 
                                      maxBuffer, HUD_COLOR_YELLOW);
    }
}

f32 usamune_calculate_speed(struct MarioState *m, u8 format) {
    switch (format) {
        case SPEED_FORMAT_XZ:
            return sqrtf(m->vel[0] * m->vel[0] + m->vel[2] * m->vel[2]);
            
        case SPEED_FORMAT_TOTAL:
            return sqrtf(m->vel[0] * m->vel[0] + m->vel[1] * m->vel[1] + m->vel[2] * m->vel[2]);
            
        case SPEED_FORMAT_FORWARD:
            return m->forwardVel;
            
        default:
            return 0.0f;
    }
}

void usamune_speed_display_toggle(void) {
    sSpeedDisplay.enabled = !sSpeedDisplay.enabled;
    if (sSpeedDisplay.enabled) {
        sSpeedDisplay.maxSpeed = 0.0f; // Reset max speed when enabling
    }
}

// ============================================================================
// INPUT DISPLAY IMPLEMENTATION
// ============================================================================

void usamune_input_display_init(void) {
    sInputDisplay.enabled = FALSE;
    sInputDisplay.style = INPUT_STYLE_BOTH;
    sInputDisplay.showPrevious = FALSE;
    sInputDisplay.showAnalogValues = FALSE;
    sInputDisplay.posX = INPUT_DISPLAY_X;
    sInputDisplay.posY = INPUT_DISPLAY_Y;
    sInputDisplay.colorPressed = HUD_COLOR_WHITE;
    sInputDisplay.colorReleased = HUD_COLOR_GRAY;
    sInputDisplay.historyIndex = 0;
    
    // Clear input history
    for (int i = 0; i < 8; i++) {
        sInputDisplay.buttonHistory[i] = 0;
        sInputDisplay.stickXHistory[i] = 0;
        sInputDisplay.stickYHistory[i] = 0;
    }
}

void usamune_input_display_update(void) {
    if (!sInputDisplay.enabled || gMarioState == NULL || gMarioState->controller == NULL) {
        return;
    }
    
    struct Controller *controller = gMarioState->controller;
    
    // Update input history
    sInputDisplay.buttonHistory[sInputDisplay.historyIndex] = controller->buttonDown;
    sInputDisplay.stickXHistory[sInputDisplay.historyIndex] = controller->rawStickX;
    sInputDisplay.stickYHistory[sInputDisplay.historyIndex] = controller->rawStickY;
    
    sInputDisplay.historyIndex = (sInputDisplay.historyIndex + 1) % 8;
}

void usamune_render_input_display(void) {
    if (gMarioState == NULL || gMarioState->controller == NULL) {
        return;
    }
    
    struct Controller *controller = gMarioState->controller;
    s16 x = sInputDisplay.posX;
    s16 y = sInputDisplay.posY;
    
    // Render button states
    if (sInputDisplay.style == INPUT_STYLE_BUTTONS || sInputDisplay.style == INPUT_STYLE_BOTH) {
        usamune_render_button_state(x, y, A_BUTTON, controller->buttonDown, "A");
        usamune_render_button_state(x + 20, y, B_BUTTON, controller->buttonDown, "B");
        usamune_render_button_state(x + 40, y, Z_TRIG, controller->buttonDown, "Z");
        usamune_render_button_state(x + 60, y, START_BUTTON, controller->buttonDown, "S");
        
        // C buttons
        usamune_render_button_state(x, y + 20, U_CBUTTONS, controller->buttonDown, "C^");
        usamune_render_button_state(x + 25, y + 20, D_CBUTTONS, controller->buttonDown, "Cv");
        usamune_render_button_state(x + 50, y + 20, L_CBUTTONS, controller->buttonDown, "C<");
        usamune_render_button_state(x + 75, y + 20, R_CBUTTONS, controller->buttonDown, "C>");
        
        // D-pad
        usamune_render_button_state(x, y + 40, U_JPAD, controller->buttonDown, "D^");
        usamune_render_button_state(x + 25, y + 40, D_JPAD, controller->buttonDown, "Dv");
        usamune_render_button_state(x + 50, y + 40, L_JPAD, controller->buttonDown, "D<");
        usamune_render_button_state(x + 75, y + 40, R_JPAD, controller->buttonDown, "D>");
        
        // Triggers
        usamune_render_button_state(x, y + 60, L_TRIG, controller->buttonDown, "L");
        usamune_render_button_state(x + 20, y + 60, R_TRIG, controller->buttonDown, "R");
        
        y += 80;
    }
    
    // Render analog stick
    if (sInputDisplay.style == INPUT_STYLE_ANALOG || sInputDisplay.style == INPUT_STYLE_BOTH) {
        char stickBuffer[32];
        
        if (sInputDisplay.showAnalogValues) {
            sprintf(stickBuffer, "Stick: %d,%d", controller->rawStickX, controller->rawStickY);
        } else {
            sprintf(stickBuffer, "Stick: %.1f,%.1f", controller->stickX, controller->stickY);
        }
        
        usamune_render_text_with_color(x, y, stickBuffer, sInputDisplay.colorPressed);
        
        // Show stick magnitude
        char magBuffer[16];
        sprintf(magBuffer, "Mag: %.1f", controller->stickMag);
        usamune_render_text_with_color(x, y + 16, magBuffer, sInputDisplay.colorPressed);
    }
}

void usamune_render_button_state(s16 x, s16 y, u16 buttonMask, u16 currentButtons, const char *label) {
    u32 color = (currentButtons & buttonMask) ? sInputDisplay.colorPressed : sInputDisplay.colorReleased;
    usamune_render_text_with_color(x, y, label, color);
}

void usamune_input_display_toggle(void) {
    sInputDisplay.enabled = !sInputDisplay.enabled;
}

// ============================================================================
// WALLKICK DISPLAY IMPLEMENTATION
// ============================================================================

void usamune_wallkick_display_init(void) {
    sWallkickDisplay.enabled = FALSE;
    sWallkickDisplay.showFrame = TRUE;
    sWallkickDisplay.showAngle = FALSE;
    sWallkickDisplay.posX = WALLKICK_DISPLAY_X;
    sWallkickDisplay.posY = WALLKICK_DISPLAY_Y;
    sWallkickDisplay.color = HUD_COLOR_CYAN;
    sWallkickDisplay.wallkickFrame = 0;
    sWallkickDisplay.wallkickAngle = 0;
    sWallkickDisplay.displayTimer = 0;
}

void usamune_wallkick_display_update(void) {
    if (!sWallkickDisplay.enabled || gMarioState == NULL) {
        return;
    }
    
    // Check if Mario is performing a wallkick
    if (gMarioState->action == ACT_WALL_KICK_AIR) {
        sWallkickDisplay.wallkickFrame = gMarioState->actionTimer;
        sWallkickDisplay.wallkickAngle = gMarioState->faceAngle[1];
        sWallkickDisplay.displayTimer = 60; // Show for 2 seconds
    } else if (sWallkickDisplay.displayTimer > 0) {
        sWallkickDisplay.displayTimer--;
    }
}

void usamune_render_wallkick_timer(void) {
    char wallkickBuffer[32];
    
    if (sWallkickDisplay.showFrame && sWallkickDisplay.showAngle) {
        sprintf(wallkickBuffer, "Wallkick: F%d A%d", 
               sWallkickDisplay.wallkickFrame, sWallkickDisplay.wallkickAngle);
    } else if (sWallkickDisplay.showFrame) {
        sprintf(wallkickBuffer, "Wallkick: Frame %d", sWallkickDisplay.wallkickFrame);
    } else if (sWallkickDisplay.showAngle) {
        sprintf(wallkickBuffer, "Wallkick: Angle %d", sWallkickDisplay.wallkickAngle);
    } else {
        sprintf(wallkickBuffer, "Wallkick");
    }
    
    usamune_render_text_with_color(sWallkickDisplay.posX, sWallkickDisplay.posY, 
                                  wallkickBuffer, sWallkickDisplay.color);
}

void usamune_wallkick_display_toggle(void) {
    sWallkickDisplay.enabled = !sWallkickDisplay.enabled;
}

// ============================================================================
// MEMORY VIEWER IMPLEMENTATION
// ============================================================================

void usamune_memory_viewer_init(void) {
    sMemoryViewer.enabled = FALSE;
    sMemoryViewer.memoryType = MEMORY_TYPE_MARIO;
    sMemoryViewer.displayFormat = 0; // Hex format
    sMemoryViewer.bytesPerLine = 16;
    sMemoryViewer.posX = MEMORY_VIEWER_X;
    sMemoryViewer.posY = MEMORY_VIEWER_Y;
    sMemoryViewer.baseAddress = 0x80000000;
    sMemoryViewer.currentAddress = 0x80000000;
    sMemoryViewer.numLines = 8;
    sMemoryViewer.highlightAddress = 0;
    sMemoryViewer.colorNormal = HUD_COLOR_WHITE;
    sMemoryViewer.colorHighlight = HUD_COLOR_YELLOW;
}

void usamune_memory_viewer_update(void) {
    if (!sMemoryViewer.enabled) {
        return;
    }
    
    // Update base address based on memory type
    switch (sMemoryViewer.memoryType) {
        case MEMORY_TYPE_MARIO:
            if (gMarioState != NULL) {
                sMemoryViewer.baseAddress = (uintptr_t)gMarioState;
            }
            break;
        case MEMORY_TYPE_OBJECTS:
            // Point to object pool if available
            sMemoryViewer.baseAddress = (u32)gObjectPool;
            break;
        case MEMORY_TYPE_LEVEL:
            // Point to level data
            // idk
            // sMemoryViewer.baseAddress = (u32);
            break;
        case MEMORY_TYPE_CUSTOM:
            // Use current address as set by user
            break;
    }
}

void usamune_render_memory_viewer(void) {
    char addressBuffer[32];
    char hexBuffer[64];
    char asciiBuffer[32];
    
    s16 x = sMemoryViewer.posX;
    s16 y = sMemoryViewer.posY;
    
    // Render title
    const char* typeNames[] = {"Mario", "Objects", "Level", "Custom"};
    sprintf(addressBuffer, "Memory: %s", typeNames[sMemoryViewer.memoryType]);
    usamune_render_text_with_color(x, y, addressBuffer, sMemoryViewer.colorHighlight);
    y += 16;
    
    // Render memory data
    for (u8 line = 0; line < sMemoryViewer.numLines; line++) {
        u32 lineAddress = sMemoryViewer.currentAddress + (line * sMemoryViewer.bytesPerLine);
        
        // Check if address is valid (simplified check)
        if (lineAddress < 0x80000000 || lineAddress > 0x807FFFFF) {
            continue; // Skip invalid addresses
        }
        
        // Format address
        sprintf(addressBuffer, "%08X:", lineAddress);
        usamune_render_text_with_color(x, y, addressBuffer, sMemoryViewer.colorNormal);
        
        // Format hex bytes
        char *hexPtr = hexBuffer;
        char *asciiPtr = asciiBuffer;
        
        for (u8 byte = 0; byte < sMemoryViewer.bytesPerLine && byte < 16; byte++) {
            u8 value = *(u8*)(uintptr_t)(lineAddress + byte);
            sprintf(hexPtr, "%02X ", value);
            hexPtr += 3;
            
            // ASCII representation
            if (value >= 32 && value < 127) {
                *asciiPtr = value;
            } else {
                *asciiPtr = '.';
            }
            asciiPtr++;
        }
        *asciiPtr = '\0';
        
        // Render hex data
        usamune_render_text_with_color(x + 80, y, hexBuffer, sMemoryViewer.colorNormal);
        
        // Render ASCII data
        usamune_render_text_with_color(x + 240, y, asciiBuffer, sMemoryViewer.colorNormal);
        
        y += 12; // Smaller line spacing for memory viewer
    }
}

void usamune_memory_viewer_set_address(u32 address) {
    sMemoryViewer.currentAddress = address;
}

void usamune_memory_viewer_scroll_up(void) {
    if (sMemoryViewer.currentAddress >= sMemoryViewer.bytesPerLine) {
        sMemoryViewer.currentAddress -= sMemoryViewer.bytesPerLine;
    }
}

void usamune_memory_viewer_scroll_down(void) {
    sMemoryViewer.currentAddress += sMemoryViewer.bytesPerLine;
}

void usamune_memory_viewer_toggle(void) {
    sMemoryViewer.enabled = !sMemoryViewer.enabled;
}

// ============================================================================
// DEBUG DISPLAY IMPLEMENTATION
// ============================================================================

void usamune_debug_display_init(void) {
    sDebugDisplay.enabled = FALSE;
    sDebugDisplay.showClassic = TRUE;
    sDebugDisplay.showAdvanced = FALSE;
    sDebugDisplay.showMarioState = FALSE;
    sDebugDisplay.showPhysics = FALSE;
    sDebugDisplay.posX = 16;
    sDebugDisplay.posY = 100;
    sDebugDisplay.color = HUD_COLOR_GREEN;
}

void usamune_debug_display_update(void) {
    // Debug display doesn't need regular updates
}

void usamune_render_debug_display(void) {
    if (gMarioState == NULL) {
        return;
    }
    
    char debugBuffer[64];
    s16 x = sDebugDisplay.posX;
    s16 y = sDebugDisplay.posY;
    
    if (sDebugDisplay.showClassic) {
        // Classic debug info (similar to original SM64 debug)
        sprintf(debugBuffer, "Pos: %.1f,%.1f,%.1f", 
               gMarioState->pos[0], gMarioState->pos[1], gMarioState->pos[2]);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
        
        sprintf(debugBuffer, "Angle: %d,%d,%d", 
               gMarioState->faceAngle[0], gMarioState->faceAngle[1], gMarioState->faceAngle[2]);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
        
        sprintf(debugBuffer, "Action: %08X", gMarioState->action);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
    }
    
    if (sDebugDisplay.showAdvanced) {
        sprintf(debugBuffer, "Health: %d Lives: %d", 
               gMarioState->health, gMarioState->numLives);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
        
        sprintf(debugBuffer, "Flags: %08X", gMarioState->flags);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
        
        sprintf(debugBuffer, "Timer: %d State: %d", 
               gMarioState->actionTimer, gMarioState->actionState);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
    }
    
    if (sDebugDisplay.showMarioState) {
        sprintf(debugBuffer, "InvincTimer: %d", gMarioState->invincTimer);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
        
        sprintf(debugBuffer, "WallKickTimer: %d", gMarioState->wallKickTimer);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
    }
    
    if (sDebugDisplay.showPhysics) {
        sprintf(debugBuffer, "ForwardVel: %.2f", gMarioState->forwardVel);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
        
        sprintf(debugBuffer, "SlideVel: %.2f,%.2f", 
               gMarioState->slideVelX, gMarioState->slideVelZ);
        usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
        y += 14;
        
        if (gMarioState->floor != NULL) {
            sprintf(debugBuffer, "FloorHeight: %.1f", gMarioState->floorHeight);
            usamune_render_text_with_color(x, y, debugBuffer, sDebugDisplay.color);
            y += 14;
        }
    }
}

void usamune_debug_display_toggle_classic(void) {
    sDebugDisplay.showClassic = !sDebugDisplay.showClassic;
}

void usamune_debug_display_toggle_advanced(void) {
    sDebugDisplay.showAdvanced = !sDebugDisplay.showAdvanced;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void usamune_render_text_with_color(s16 x, s16 y, const char *text, u32 color) {
    
    // Set color in display list (simplified)
    // gDPSetPrimColor(gDisplayListHead++, 0, 0, 
    //                (color >> 24) & 0xFF, (color >> 16) & 0xFF, 
    //                (color >> 8) & 0xFF, color & 0xFF);
    
    // Render the text
    print_generic_string(x, y, (const u8*)text);
}

void usamune_format_hex_bytes(char *buffer, u8 *data, u8 numBytes) {
    char *ptr = buffer;
    for (u8 i = 0; i < numBytes; i++) {
        sprintf(ptr, "%02X ", data[i]);
        ptr += 3;
    }
    if (numBytes > 0) {
        *(ptr - 1) = '\0'; // Remove trailing space
    }
}

// ============================================================================
// HUD ELEMENT CONFIGURATION FUNCTIONS
// ============================================================================

void usamune_speed_display_set_format(u8 format) {
    if (format < 4) { // Validate format
        sSpeedDisplay.format = format;
    }
}

void usamune_speed_display_set_position(s16 x, s16 y) {
    sSpeedDisplay.posX = x;
    sSpeedDisplay.posY = y;
}

void usamune_input_display_set_style(u8 style) {
    if (style < 4) { // Validate style
        sInputDisplay.style = style;
    }
}

void usamune_input_display_set_position(s16 x, s16 y) {
    sInputDisplay.posX = x;
    sInputDisplay.posY = y;
}

void usamune_wallkick_display_set_position(s16 x, s16 y) {
    sWallkickDisplay.posX = x;
    sWallkickDisplay.posY = y;
}

void usamune_memory_viewer_set_type(u8 memoryType) {
    if (memoryType < 4) { // Validate type
        sMemoryViewer.memoryType = memoryType;
    }
}

// Advanced HUD features

void usamune_hud_set_all_positions(s16 speedX, s16 speedY, s16 inputX, s16 inputY,
                                  s16 wallkickX, s16 wallkickY, s16 memoryX, s16 memoryY) {
    usamune_speed_display_set_position(speedX, speedY);
    usamune_input_display_set_position(inputX, inputY);
    usamune_wallkick_display_set_position(wallkickX, wallkickY);
    sMemoryViewer.posX = memoryX;
    sMemoryViewer.posY = memoryY;
}

void usamune_hud_toggle_all(void) {
    sSpeedDisplay.enabled = !sSpeedDisplay.enabled;
    sInputDisplay.enabled = !sInputDisplay.enabled;
    sWallkickDisplay.enabled = !sWallkickDisplay.enabled;
    sMemoryViewer.enabled = !sMemoryViewer.enabled;
    sDebugDisplay.enabled = !sDebugDisplay.enabled;
}

void usamune_hud_reset_to_defaults(void) {
    usamune_hud_extensions_init();
}

// Save/load HUD configuration
void usamune_hud_save_config(void) {
    // Save HUD configuration to Usamune state
    struct UsamuneConfig *config = &gUsamuneState.config;
    
    config->showSpeedDisplay = sSpeedDisplay.enabled;
    config->showInputDisplay = sInputDisplay.enabled;
    config->showWallkickTimer = sWallkickDisplay.enabled;
    config->showMemoryViewer = sMemoryViewer.enabled;
    config->speedDisplayFormat = sSpeedDisplay.format;
}

void usamune_hud_load_config(void) {
    // Load HUD configuration from Usamune state
    struct UsamuneConfig *config = &gUsamuneState.config;
    
    sSpeedDisplay.enabled = config->showSpeedDisplay;
    sInputDisplay.enabled = config->showInputDisplay;
    sWallkickDisplay.enabled = config->showWallkickTimer;
    sMemoryViewer.enabled = config->showMemoryViewer;
    sSpeedDisplay.format = config->speedDisplayFormat;
}