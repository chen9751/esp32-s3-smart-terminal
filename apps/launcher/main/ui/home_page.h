#pragma once
// Layout contract: 3 large mode cards + 1 small USB Storage utility entry.
// Later LVGL layer calls launcher_policy_select_mode(MODE_*) for large cards,
// and launcher_storage_open() for small USB entry.
void home_page_show(void);
