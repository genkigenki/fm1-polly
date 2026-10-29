/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 M. Geisler (genkigenki)
 * fm1-base's .pool section (PLAT_POOL: the big zero-initialised buffers) in the ESP32-S3's internal RAM, after .bss
 * and before the heap, as on the FM-1. Without this the linker places the orphan section in flash and the app's
 * first write to a pool buffer panics (Cache error). NOLOAD: cp_fm1base.c zeroes _cp_pool_start.._cp_pool_end at
 * boot. Linked with -Wl,-T$PROJECT_DIR/src/cp_pool.ld. */
SECTIONS
{
  .cp_pool (NOLOAD) : ALIGN(16)
  {
    _cp_pool_start = ABSOLUTE(.);
    *(.pool .pool.*)
    . = ALIGN(16);
    _cp_pool_end = ABSOLUTE(.);
  } > dram0_0_seg
}
INSERT AFTER .dram0.bss;
