#include "timers.h"
#include "practice_core.h"

extern struct UsamuneState gUsamuneState;

void usamune_timers_init(void) {
    gUsamuneState.timers.igtFrames = 0;
    gUsamuneState.timers.sectionTimer = 0;
    gUsamuneState.timers.starGrabTimer = 0;
    gUsamuneState.timers.attemptCounter = 0;
    gUsamuneState.timers.lagFrameCounter = 0;
    gUsamuneState.timers.timerRunning = TRUE;
    gUsamuneState.timers.showCentiseconds = TRUE;
}

void usamune_timers_update(void) {
    if (gUsamuneState.timers.timerRunning) {
        gUsamuneState.timers.igtFrames++;
        gUsamuneState.timers.sectionTimer++;
        gUsamuneState.timers.starGrabTimer++;
    }
}

void usamune_timers_render(void) {
    if (!gUsamuneState.config.showIGT) return;
    
    char timerBuffer[32];
    usamune_format_time(gUsamuneState.timers.igtFrames, timerBuffer, 
                       gUsamuneState.timers.showCentiseconds);
    print_generic_string(TIMER_DISPLAY_X, TIMER_DISPLAY_Y, (const u8*)timerBuffer);
}

void usamune_timers_increment_attempts(void) {
    gUsamuneState.timers.attemptCounter++;
}

void usamune_timers_reset_section(void) {
    gUsamuneState.timers.sectionTimer = 0;
}