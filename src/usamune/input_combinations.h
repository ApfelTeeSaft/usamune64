#ifndef USAMUNE_INPUT_COMBINATIONS_H
#define USAMUNE_INPUT_COMBINATIONS_H

#include "../../include/sm64.h"

// Default input combinations for Usamune features
// These can be customized by users

// Savestate combinations
#define USAMUNE_SAVESTATE_1_DEFAULT     (L_TRIG | R_TRIG | U_CBUTTONS)
#define USAMUNE_SAVESTATE_2_DEFAULT     (L_TRIG | R_TRIG | D_CBUTTONS)
#define USAMUNE_LOADSTATE_1_DEFAULT     (L_TRIG | R_TRIG | L_CBUTTONS)
#define USAMUNE_LOADSTATE_2_DEFAULT     (L_TRIG | R_TRIG | R_CBUTTONS)

// Practice tool combinations
#define USAMUNE_FREECAM_DEFAULT         (L_TRIG | R_TRIG | A_BUTTON)
#define USAMUNE_LEVEL_RESET_DEFAULT     (L_TRIG | R_TRIG | B_BUTTON)
#define USAMUNE_SOFT_RESET_DEFAULT      (A_BUTTON | B_BUTTON | Z_TRIG | START_BUTTON)

// Stage control combinations
#define USAMUNE_WDW_WATER_DEFAULT       (L_TRIG | U_JPAD)
#define USAMUNE_TTC_SPEED_DEFAULT       (L_TRIG | D_JPAD)
#define USAMUNE_MOAT_TOGGLE_DEFAULT     (L_TRIG | L_JPAD)
#define USAMUNE_DDD_SUB_DEFAULT         (L_TRIG | R_JPAD)

// HUD toggle combinations
#define USAMUNE_SPEED_TOGGLE_DEFAULT    (L_TRIG | R_TRIG | START_BUTTON)
#define USAMUNE_INPUT_TOGGLE_DEFAULT    (Z_TRIG | START_BUTTON)

// Menu combinations
#define USAMUNE_MENU_OPEN_DEFAULT       (L_TRIG | R_TRIG | Z_TRIG)

#endif // USAMUNE_INPUT_COMBINATIONS_H