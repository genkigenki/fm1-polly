/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 M. Geisler (genkigenki)
 * fm1-base's platform (include/plat.h, by Charles Vestal) on the M5Stack Cardputer, over the mvave-to-cardputer board
 * layer: any app built on fm1-base runs on the Cardputer with this file, a main (cp_fm1base_main.cpp) and its own
 * sources unchanged. As fm1-base's host simulator (host/plat_host.c) does, this unit includes the app (app.c).
 * Not on the Cardputer (yet): MIDI (USB, BLE, TRS), Wi-Fi, Bluetooth audio, the data area, the second core.
 * MPL-2.0 like fm1-base, so an app of any licence can use it. */
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "cp_audio.h"
#include "cp_console.h"
#include "cp_display.h"
#include "cp_keys.h"
#include "cp_overlay.h"
#include "cp_store.h"
#include "cp_sys.h"

/* the FM-1's .pool section and other attributes: as the host build, dropped from here on (only fm1-base's and the
 * app's sources follow, all C, none needing an attribute on the ESP32) */
#define __attribute__(x)

static void lcd_sync(void) {}
static void lcd_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t c) { cp_lcd_fill(x, y, w, h, c); }
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *px) { cp_lcd_blit(x, y, w, h, px); }

#include "plat.h"
#include "app.h"                        /* before gfx.c: the app may size the canvas (APP_CANVAS_PX) */
#include "gfx.c"
#ifdef APP_TOUCH
#include "touch.h"
#endif
#include "fm1b_config.h"

/* ------------------------------------------------------------- the panel --- */
static uint32_t lit_keys, lit_dim, lit_btn;
static int play_red, glow_on = 1;

uint32_t plat_ms(void) { return cp_ms(); }
uint32_t plat_buttons(void) { return cp_keys_buttons(); }
uint32_t plat_keys(void) { return cp_keys_notes() & ((1u << NKEYS) - 1u); }
int32_t plat_enc(int role) { return role >= 0 && role < NE ? cp_enc_take((uint32_t)role) : 0; }
uint32_t plat_master(void) { return cp_master_q12(); }
int plat_battery(void) { return cp_battery_bars(); }
static void show(void)
{
    cp_overlay_leds(lit_keys, lit_dim & ~lit_keys, lit_btn, play_red);
    cp_overlay_knob(cp_enc_active());
}
void plat_lights(uint32_t buttons, uint32_t keys, uint32_t dim_buttons, uint32_t dim_keys, int glow)
{
    (void)dim_buttons, (void)glow;      /* the strip has no glow level for the buttons */
    lit_btn = buttons, lit_keys = keys, lit_dim = dim_keys;
    show();
}
void plat_lights_mid(uint32_t buttons, uint32_t keys) { (void)buttons, lit_dim |= keys, show(); }
void plat_glow(int on) { glow_on = on; }
void plat_play_red(int red) { play_red = red != 0; }
void plat_leds(uint32_t buttons, uint32_t keys) { plat_lights(buttons, keys, glow_on ? ~buttons : 0u, 0, glow_on ? 2 : 0); }
void plat_sense(int on) { (void)on; }
uint32_t plat_analog_mask(void) { return 0; }

/* -------------------------------------------------- MIDI, USB host, radio --- */
int plat_midi_in(uint32_t *pkt, int *port) { (void)pkt, (void)port; return 0; }
uint32_t plat_midi_in_from(void) { return 0; }
void plat_midi_send(uint32_t pkt, uint32_t to) { (void)pkt, (void)to; }
void plat_midi_out(uint32_t pkt) { (void)pkt; }
uint32_t plat_midi_out_room(uint32_t to) { (void)to; return 64; }
uint32_t plat_midi_out_link(void) { return 0; }
uint32_t plat_midi_count(int port, int out) { (void)port, (void)out; return 0; }
uint32_t plat_midi_ago(int port, int out) { (void)port, (void)out; return 0xFFFFFFFFu; }
void plat_usb_host(int on) { (void)on; }
int plat_usb_host_state(void) { return USBH_OFF; }
int plat_usb_host_msg(void) { return USBH_MSG_NONE; }
int plat_usb_lost(void) { return 0; }
int plat_ble_state(void) { return BLE_OFF; }
int plat_ble_connected(void) { return 0; }
int plat_ble_linked(void) { return 0; }
void plat_ble_start(void) {}
uint32_t plat_ble_log_total(void) { return 0; }
const char *plat_ble_log_line(int back) { (void)back; return ""; }
void plat_ble_scan(int on) { (void)on; }
int plat_ble_found(int i, char *name, int *rssi) { (void)i, (void)name, (void)rssi; return 0; }
int plat_ble_connect(int i) { (void)i; return -1; }
void plat_ble_disconnect(void) {}
int plat_ble_device(void) { return BLE_DEV_OFF; }
int plat_ble_passkey(void) { return -1; }
void plat_ble_enable(int on) { (void)on; }
void plat_bta_on(int on) { (void)on; }
void plat_bta_scan(int on) { (void)on; }
int plat_bta_found(int i, char *name) { (void)i, (void)name; return 0; }
int plat_bta_known(int i, char *name) { (void)i, (void)name; return 0; }
void plat_bta_pick(int i) { (void)i; }
int plat_bta_reconnect(int i) { (void)i; return -1; }
void plat_bta_forget(int i) { (void)i; }
void plat_bta_hangup(void) {}
int plat_bta_state(void) { return PLAT_BTA_OFF; }
void plat_wifi_on(int on) { (void)on; }
int plat_wifi_state(void) { return WIFI_OFF; }
const char *plat_wifi_why(void) { return ""; }
const char *plat_wifi_ssid(void) { return ""; }
uint32_t plat_wifi_ip(void) { return 0; }
int plat_wifi_rssi(void) { return 0; }
int plat_wifi_scan(void) { return -1; }
int plat_wifi_found(int i, char *ssid, int *rssi, int *secured) { (void)i, (void)ssid, (void)rssi, (void)secured; return 0; }
int plat_wifi_found_channel(int i) { (void)i; return 0; }
int plat_wifi_join(const char *ssid, const char *pass) { (void)ssid, (void)pass; return -1; }
int plat_wifi_saved(int i, char *ssid) { (void)i, (void)ssid; return 0; }
void plat_wifi_forget(int i) { (void)i; }
int plat_wifi_kept(const char *ssid) { (void)ssid; return 0; }
void plat_wifi_rx(uint32_t *frames, int *rssi) { if (frames) *frames = 0; if (rssi) *rssi = 0; }
uint32_t plat_wifi_scans(void) { return 0; }
int plat_net_task(void (*fn)(void *), void *arg, const char *name) { (void)fn, (void)arg, (void)name; return -1; }
void plat_sysex_send(const uint8_t *frame, uint32_t n) { (void)frame, (void)n; }
int plat_host_read(uint32_t addr, uint8_t *dst, uint32_t n) { (void)addr, (void)dst, (void)n; return -1; }
void plat_host_patience(uint32_t try_ms, uint32_t total_ms) { (void)try_ms, (void)total_ms; }
void plat_usb_audio_full(int on) { (void)on; }

/* ----------------------------------------------------- threads and logs --- */
/* the render is a task on this core at a higher priority (as the FM-1's audio interrupt): a lock keeps it out */
static portMUX_TYPE plat_mux = portMUX_INITIALIZER_UNLOCKED;
uint32_t plat_lock(void) { taskENTER_CRITICAL(&plat_mux); return 1; }
void plat_unlock(uint32_t f) { (void)f; taskEXIT_CRITICAL(&plat_mux); }
void plat_yield(void) { taskYIELD(); }
void plat_log(const char *line) { cp_console_printf("%s\n", line); }

/* ------------------------------------------------------ storage and data --- */
static uint8_t store_tmp[PLAT_STORE_MAX];
int plat_store_load(uint32_t obj, void *dst, uint32_t max)
{
    int n = cp_store_load(obj, store_tmp, sizeof store_tmp);
    if (n < 0)
        return -1;
    if ((uint32_t)n > max)
        n = (int)max;
    memcpy(dst, store_tmp, (size_t)n);
    return n;
}
int plat_store_head(uint32_t obj, void *dst, uint32_t n)
{
    int got = cp_store_load(obj, store_tmp, sizeof store_tmp);
    if (got < 0 || (uint32_t)got < n)
        return -1;
    memcpy(dst, store_tmp, n);
    return (int)n;
}
int plat_store_size(uint32_t obj) { int n = cp_store_load(obj, store_tmp, sizeof store_tmp); return n <= 0 ? -1 : n; }
int plat_store_save(uint32_t obj, const void *src, uint32_t len) { return cp_store_save(obj, src, len) ? -1 : 0; }
const uint8_t *plat_data(uint32_t off) { (void)off; return 0; }   /* no data area on the Cardputer (yet) */
int plat_data_erase(uint32_t off) { (void)off; return -1; }
int plat_data_write(uint32_t off, const void *src, uint32_t n) { (void)off, (void)src, (void)n; return -1; }
const uint8_t *plat_chip_uid(void)
{
    static uint8_t uid[16];
    esp_efuse_mac_get_default(uid);
    return uid;
}

/* ------------------------------------------------------------ the cores --- */
uint32_t plat_cpu_pct(void) { return cp_audio_cpu_pct(); }
uint32_t plat_xruns(void) { return cp_audio_xruns(); }
uint32_t plat_ticks24(void) { return cp_ticks24(); }
int plat_cpu2_set(int on) { return on ? -1 : 0; }
int plat_cpu2_on(void) { return 0; }
int plat_cpu2_run(void (*fn)(uint32_t), uint32_t arg) { (void)fn, (void)arg; return 0; }
int plat_cpu2_wait(void) { return 0; }
void plat_cpu2_sync(void) {}
int plat_cpu2_busy(void) { return 0; }
int plat_cpu2_gave_up(void) { return 3; }
uint32_t plat_cpu2_bad(void) { return 0; }
int plat_cpu2_barred(void) { return 1; }
uint32_t plat_cpu2_stack(void) { return 0; }

/* -------------------------------------------------------- the app itself --- */
#include "ann_words.h"
static int ann_wire_send(const uint8_t *p, uint32_t n) { (void)p, (void)n; return 0; }   /* no MIDI: no reader */
#include "announce.c"
#ifdef APP_TOUCH
#include "touch.c"
#endif
#ifdef APP_BTDEV
#include "btdev.c"
#endif
#include "app.c"

#undef __attribute__

/* ------------------------------------------- what the main (C++) calls --- */
void fm1b_boot(void)
{
    palette_set(APP_PALETTE);
    app_init();
    lcd_fill(0, 0, 240, 240, 0);
#ifdef APP_BOOT
    app_boot();
#endif
    app_ui_init();
}

int fm1b_tick(uint32_t now_ms)          /* one millisecond of fm1-base's main loop: 1 = a UI frame was drawn */
{
    static uint32_t last_frame;
#ifdef APP_TICK
    app_tick();
#endif
#ifdef APP_LOOP
    app_loop();
#endif
    if (now_ms - last_frame >= 16u) {
        last_frame = now_ms;
        app_frame();
        return 1;
    }
    app_input_only();
    return 0;
}

void fm1b_render(int32_t *o, uint32_t n) { app_render(o, n, APP_GAIN(plat_master())); }
uint32_t fm1b_block(void) { return APP_BLOCK; }
const char *fm1b_name(void) { return APP_NAME; }
