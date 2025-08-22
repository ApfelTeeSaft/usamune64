#ifndef USAMUNE_TIMERS_H
#define USAMUNE_TIMERS_H

#include "../../include/types.h"

void usamune_timers_init(void);
void usamune_timers_update(void);
void usamune_timers_render(void);
void usamune_timers_increment_attempts(void);
void usamune_timers_reset_section(void);

#endif // USAMUNE_TIMERS_H