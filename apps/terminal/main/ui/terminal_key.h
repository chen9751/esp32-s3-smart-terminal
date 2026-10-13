#pragma once
#include <stdbool.h>
void terminal_key_init(void);
void terminal_key_poll(bool *short_press, bool *long_press);
