#include "lvgl.h"
#include "preview_home.h"
#include <SDL.h>
#include <emscripten.h>
#include <stdbool.h>
#include <stdint.h>
#define W 640
#define H 172
#define PIXELS (W * H)
static SDL_Window *win;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static uint32_t fb[PIXELS];
static int mouse_x, mouse_y;
static bool pressed;
static uint32_t last_tick;
static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
    const lv_color16_t *src = (const lv_color16_t *)pixels;
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            lv_color16_t v = *src++;
            uint8_t r = (v.red * 255) / 31, g = (v.green * 255) / 63, b = (v.blue * 255) / 31;
            fb[y * W + x] = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }
    }
    lv_display_flush_ready(display);
}
static void pointer(lv_indev_t *input, lv_indev_data_t *data) {
    (void)input;
    data->point.x = mouse_x;
    data->point.y = mouse_y;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
static void frame(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_MOUSEMOTION) { mouse_x = event.motion.x; mouse_y = event.motion.y; }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            mouse_x = event.button.x; mouse_y = event.button.y; pressed = true;
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            mouse_x = event.button.x; mouse_y = event.button.y; pressed = false;
        } else if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION || event.type == SDL_FINGERUP) {
            mouse_x = (int)(event.tfinger.x * W);
            mouse_y = (int)(event.tfinger.y * H);
            pressed = event.type != SDL_FINGERUP;
        }
    }
    uint32_t now = SDL_GetTicks();
    lv_tick_inc(now - last_tick); last_tick = now;
    lv_timer_handler();
    SDL_UpdateTexture(texture, NULL, fb, W * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    win = SDL_CreateWindow("Smart Terminal UI Preview", 0, 0, W, H, 0);
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, W, H);
    if (!win || !renderer || !texture) return 2;
    lv_init();
    lv_display_t *disp = lv_display_create(W, H);
    static uint8_t b1[W * 40 * 2], b2[W * 40 * 2];
    lv_display_set_buffers(disp, b1, b2, sizeof b1, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, flush);
    lv_indev_t *input = lv_indev_create();
    lv_indev_set_type(input, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(input, pointer);
    preview_home_create();
    last_tick = SDL_GetTicks();
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
