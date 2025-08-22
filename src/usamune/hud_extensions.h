#ifndef USAMUNE_HUD_EXTENSIONS_H
#define USAMUNE_HUD_EXTENSIONS_H

#include "../../include/types.h"
#include "../game/mario.h"

// HUD element positions (customizable)
#define SPEED_DISPLAY_X         16
#define SPEED_DISPLAY_Y         212
#define INPUT_DISPLAY_X         280
#define INPUT_DISPLAY_Y         200
#define WALLKICK_DISPLAY_X      16
#define WALLKICK_DISPLAY_Y      180
#define MEMORY_VIEWER_X         200
#define MEMORY_VIEWER_Y         32

// Speed display formats
#define SPEED_FORMAT_XZ         0   // Horizontal speed only
#define SPEED_FORMAT_TOTAL      1   // Total 3D speed
#define SPEED_FORMAT_FORWARD    2   // Forward velocity
#define SPEED_FORMAT_DETAILED   3   // X, Y, Z components

// Input display styles
#define INPUT_STYLE_BUTTONS     0   // Show button states
#define INPUT_STYLE_ANALOG      1   // Show analog stick
#define INPUT_STYLE_BOTH        2   // Show buttons and analog
#define INPUT_STYLE_MINIMAL     3   // Minimal display

// Memory viewer types
#define MEMORY_TYPE_MARIO       0
#define MEMORY_TYPE_OBJECTS     1
#define MEMORY_TYPE_LEVEL       2
#define MEMORY_TYPE_CUSTOM      3

// Colors for HUD elements
#define HUD_COLOR_WHITE         0xFFFFFFFF
#define HUD_COLOR_RED           0xFF0000FF
#define HUD_COLOR_GREEN         0x00FF00FF
#define HUD_COLOR_BLUE          0x0000FFFF
#define HUD_COLOR_YELLOW        0xFFFF00FF
#define HUD_COLOR_CYAN          0x00FFFFFF
#define HUD_COLOR_MAGENTA       0xFF00FFFF
#define HUD_COLOR_GRAY          0x808080FF

struct UsamuneSpeedDisplay {
    u8 enabled;
    u8 format;                  // Speed format type
    u8 showDecimals;            // Show decimal places
    u8 showDirection;           // Show movement direction
    s16 posX, posY;             // Screen position
    u32 color;                  // Display color
    f32 currentSpeed;           // Current calculated speed
    f32 maxSpeed;               // Maximum speed reached
    f32 averageSpeed;           // Rolling average speed
};

struct UsamuneInputDisplay {
    u8 enabled;
    u8 style;                   // Display style
    u8 showPrevious;            // Show previous frame inputs
    u8 showAnalogValues;        // Show raw analog values
    s16 posX, posY;             // Screen position
    u32 colorPressed;           // Color for pressed buttons
    u32 colorReleased;          // Color for released buttons
    
    // Input history for display
    u16 buttonHistory[8];       // Last 8 frames of button inputs
    s8 stickXHistory[8];        // Last 8 frames of stick X
    s8 stickYHistory[8];        // Last 8 frames of stick Y
    u8 historyIndex;            // Current history index
};

struct UsamuneWallkickDisplay {
    u8 enabled;
    u8 showFrame;               // Show frame of wallkick
    u8 showAngle;               // Show wallkick angle
    s16 posX, posY;             // Screen position
    u32 color;                  // Display color
    u8 wallkickFrame;           // Current wallkick frame
    s16 wallkickAngle;          // Wallkick angle
    u8 displayTimer;            // How long to show display
};

struct UsamuneMemoryViewer {
    u8 enabled;
    u8 memoryType;              // Type of memory to view
    u8 displayFormat;           // Hex, decimal, etc.
    u8 bytesPerLine;            // Bytes to show per line
    s16 posX, posY;             // Screen position
    u32 baseAddress;            // Base memory address
    u32 currentAddress;         // Current viewing address
    u8 numLines;                // Number of lines to display
    u32 highlightAddress;       // Address to highlight
    u32 colorNormal;            // Normal text color
    u32 colorHighlight;         // Highlight color
};

struct UsamuneDebugDisplay {
    u8 enabled;
    u8 showClassic;             // Show classic debug info
    u8 showAdvanced;            // Show advanced debug info
    u8 showMarioState;          // Show Mario state details
    u8 showPhysics;             // Show physics information
    s16 posX, posY;             // Screen position
    u32 color;                  // Display color
};

// Function prototypes
void usamune_hud_extensions_init(void);
void usamune_hud_extensions_update(void);
void usamune_hud_extensions_render(void);

// Speed display functions
void usamune_speed_display_init(void);
void usamune_speed_display_update(void);
void usamune_render_speed_display(void);
void usamune_speed_display_set_format(u8 format);
void usamune_speed_display_set_position(s16 x, s16 y);
void usamune_speed_display_toggle(void);

// Input display functions
void usamune_input_display_init(void);
void usamune_input_display_update(void);
void usamune_render_input_display(void);
void usamune_input_display_set_style(u8 style);
void usamune_input_display_set_position(s16 x, s16 y);
void usamune_input_display_toggle(void);

// Wallkick display functions
void usamune_wallkick_display_init(void);
void usamune_wallkick_display_update(void);
void usamune_render_wallkick_timer(void);
void usamune_wallkick_display_set_position(s16 x, s16 y);
void usamune_wallkick_display_toggle(void);

// Memory viewer functions
void usamune_memory_viewer_init(void);
void usamune_memory_viewer_update(void);
void usamune_render_memory_viewer(void);
void usamune_memory_viewer_set_address(u32 address);
void usamune_memory_viewer_set_type(u8 memoryType);
void usamune_memory_viewer_scroll_up(void);
void usamune_memory_viewer_scroll_down(void);
void usamune_memory_viewer_toggle(void);

// Debug display functions
void usamune_debug_display_init(void);
void usamune_debug_display_update(void);
void usamune_render_debug_display(void);
void usamune_debug_display_toggle_classic(void);
void usamune_debug_display_toggle_advanced(void);

// Utility functions
void usamune_render_text_with_color(s16 x, s16 y, const char *text, u32 color);
void usamune_render_button_state(s16 x, s16 y, u16 buttonMask, u16 currentButtons, const char *label);
void usamune_format_hex_bytes(char *buffer, u8 *data, u8 numBytes);
f32 usamune_calculate_speed(struct MarioState *m, u8 format);

#endif // USAMUNE_HUD_EXTENSIONS_H