// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2026 M. Geisler (genkigenki)
// An fm1-base app on the M5Stack Cardputer: boot, the render task and the main loop (after fm1-base's host
// simulator). The render runs in a task on the same core as this loop at a higher priority: like the FM-1's audio
// interrupt it preempts the loop, and plat_lock keeps it out where the app asks.
#include <M5Cardputer.h>
#include "cp_audio.h"
#include "cp_console.h"
#include "cp_display.h"
#include "cp_keys.h"
#include "cp_keymap_x0x.h"
#include "cp_overlay.h"
#include "cp_store.h"
#include "cp_sys.h"

extern "C" {
void fm1b_boot(void);
int fm1b_tick(uint32_t now_ms);
void fm1b_render(int32_t *o, uint32_t n);
uint32_t fm1b_block(void);
const char *fm1b_name(void);
}

#ifndef CP_APP_SCREEN
#define CP_APP_SCREEN CP_SCREEN_SQUEEZE
#endif
#ifndef CP_APP_TRIM_Q12
#define CP_APP_TRIM_Q12 32768u                  // x8: the FM-1 is line level, this speaker needs gain; console: gain N
#endif
#ifndef CP_APP_STORE_NS
#define CP_APP_STORE_NS "fm1app"                // the NVS namespace: each app passes its own (-DCP_APP_STORE_NS=...)
#endif

static const char *const KNOBS[7] = {"SELECT", "ALGORITHM", "PRESETS", "KNOB 1", "KNOB 2", "KNOB 3", "KNOB 4"};
static uint32_t frame_sum, frames, frame_max, loops;

static void cmd_perf(const char *)
{
    cp_console_printf("%s: ui frames %lu avg %lu us max %lu us, loops %lu\n", fm1b_name(), (unsigned long)frames,
                      (unsigned long)(frames ? frame_sum / frames : 0), (unsigned long)frame_max, (unsigned long)loops);
    frame_sum = frames = frame_max = loops = 0;
}

void setup()
{
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    Serial.begin(115200);
    cp_console_init();
    cp_console_register("perf", cmd_perf);
    int safe_mode = cp_bootguard_begin() >= 2;  // two crashes in a row: no audio this time
    cp_display_init(CP_APP_SCREEN, CP_STRIP_H);
    cp_overlay_init(KNOBS, 7);
    cp_keys_init(&CP_KEYMAP_X0X);
    cp_store_init(CP_APP_STORE_NS);
    fm1b_boot();
    if (!safe_mode) {
        cp_audio_set_trim_q12(CP_APP_TRIM_Q12);
        cp_audio_start(fm1b_render, fm1b_block(), 44100);
    }
}

void loop()
{
    static uint32_t last_ms, healthy;
    uint32_t now = cp_ms();
    loops++;
    cp_keys_scan();
    if (now != last_ms) {
        last_ms = now;
        uint32_t t0 = micros();
        if (fm1b_tick(now)) {
            cp_lcd_sync();
            uint32_t us = micros() - t0;
            frame_sum += us, frames++;
            if (us > frame_max)
                frame_max = us;
        }
    }
    cp_console_poll();
    if (!healthy && now > 10000u) {
        healthy = 1;
        cp_bootguard_healthy();
    }
    vTaskDelay(1);
}
