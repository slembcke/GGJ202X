#include <stdlib.h>
#include <string.h>

#include "pixler.h"
#include "common.h"

#define BG_COLOR 0x31
static const u8 PALETTE[] = {
	BG_COLOR, 0x00, 0x10, 0x20,
	BG_COLOR, 0x06, 0x16, 0x26,
	BG_COLOR, 0x09, 0x19, 0x29,
	BG_COLOR, 0x01, 0x11, 0x21,
	
	BG_COLOR, 0x00, 0x10, 0x20,
	BG_COLOR, 0x06, 0x16, 0x26,
	BG_COLOR, 0x09, 0x19, 0x29,
	BG_COLOR, 0x01, 0x11, 0x21,
};

Gamepad pad1, pad2;

void read_gamepads(void){
	pad1.prev = pad1.value;
	pad1.value = joy_read(0);
	pad1.press = pad1.value & (pad1.value ^ pad1.prev);
	pad1.release = pad1.prev & (pad1.value ^ pad1.prev);
	
	pad2.prev = pad2.value;
	pad2.value = joy_read(1);
	pad2.press = pad2.value & (pad2.value ^ pad2.prev);
	pad2.release = pad2.prev & (pad2.value ^ pad2.prev);
}

void wait_noinput(void){
	while(joy_read(0) || joy_read(1)) px_wait_nmi();
}

static void darken(register const u8* palette, u8 shift){
	for(idx = 0; idx < 32; idx++){
		ix = palette[idx];
		ix -= shift << 4;
		if(ix > 0x40 || ix == 0x0D) ix = 0x1D;
		px_buffer_set_color(idx, ix);
	}
}

void fade_from_black(const u8* palette, u8 delay){
	darken(palette, 4);
	px_wait_frames(delay);
	darken(palette, 3);
	px_wait_frames(delay);
	darken(palette, 2);
	px_wait_frames(delay);
	darken(palette, 1);
	px_wait_frames(delay);
	darken(palette, 0);
}

void meta_spr(u8 x, u8 y, u8 pal, const u8* data);
static const u8 META_R1[] = {
	-8, -16, 0xD0, 0,
	 0, -16, 0xD1, 0,
	-8, -8, 0xE0, 0,
	 0, -8, 0xE1, 0,
	-8,  0, 0xF0, 0,
	 0,  0, 0xF1, 0,
	128,
};
static const u8 META_R2[] = {
	-8, -16, 0xD0, 0,
	 0, -16, 0xD1, 0,
	 0, -8, 0xE0, PX_SPR_FLIPX,
	-8, -8, 0xE1, PX_SPR_FLIPX,
	 0,  0, 0xF0, PX_SPR_FLIPX,
	-8,  0, 0xF1, PX_SPR_FLIPX,
	128,
};

static const u8 META_L1[] = {
	 0, -16, 0xD0, PX_SPR_FLIPX,
	-8, -16, 0xD1, PX_SPR_FLIPX,
	 0, -8, 0xE0, PX_SPR_FLIPX,
	-8, -8, 0xE1, PX_SPR_FLIPX,
	 0,  0, 0xF0, PX_SPR_FLIPX,
	-8,  0, 0xF1, PX_SPR_FLIPX,
	128,
};
static const u8 META_L2[] = {
	 0, -16, 0xD0, PX_SPR_FLIPX,
	-8, -16, 0xD1, PX_SPR_FLIPX,
	-8, -8, 0xE0, 0,
	 0, -8, 0xE1, 0,
	-8,  0, 0xF0, 0,
	 0,  0, 0xF1, 0,
	128,
};

static const u8* anim_walk_right[] = {
	META_R1,
	META_R2
};

static const u8* anim_walk_left[] = {
	META_L1,
	META_L2,
};

static const u8 HAMMER_UP[] = {
	 0, 0, 0xA0, 0,
	128,
};
static const u8 PIE_UP[] = {
	 0, 0, 0xB0, 0,
	128,
};
static const u8 BANANA_UP[] = {
	 0, 0, 0xA2, 0,
	128,
};

static void splash_screen(void){
	register u8 x = 32, y = 32;
	register s16 sin = 0, cos = 0x3FFF;
	bool walking = false;
	bool walkRight = true;

	// u8* pickups[] = {};

	px_ppu_sync_disable();{
		// Load the splash tilemap into nametable 0.
		px_lz4_to_vram(NT_ADDR(0, 0, 0), MAP_SPLASH);
	} px_ppu_sync_enable();
	
	// music_play(0);
	
	fade_from_black(PALETTE, 4);
	
	while(true){
		read_gamepads();
		walking = false;
		if(JOY_LEFT (pad1.value)) { x -= 1; walking = true; walkRight = false; } 
		if(JOY_RIGHT(pad1.value)) { x += 1; walking = true; walkRight = true; }
		if(JOY_DOWN (pad1.value)) { y += 1; walking = true; }
		if(JOY_UP   (pad1.value)) { y -= 1; walking = true; }
		if(JOY_BTN_A(pad1.press)) sound_play(SOUND_JUMP);
		if(JOY_BTN_B(pad1.press)) sound_play(SOUND_JUMP);

		// Draw a sprite.
		if (walking) {
			if (walkRight) {
				meta_spr(x, y, 2, anim_walk_right[px_ticks/8%2]);
			}
			else {
			 	meta_spr(x, y, 2, anim_walk_left[px_ticks/8%2]);
			}
		}
		else {
			if (walkRight) {
				meta_spr(x, y, 2, META_R1);
			}
			else {
				meta_spr(x, y, 2, META_L1);
			}
		}
		
		meta_spr(16, 16, 2, HAMMER_UP);
		meta_spr(32, 16, 2, PIE_UP);
		meta_spr(48, 16, 2, BANANA_UP);

		PX.scroll_y = 480 + (sin >> 9);
		sin += cos >> 6;
		cos -= sin >> 6;
		
		px_spr_end();
		px_wait_nmi();
	}
	
	splash_screen();
}

void main(void){
	// Set up CC65 joystick driver.
	joy_install(nes_stdjoy_joy);
	
	// Set which tiles to use for the background and sprites.
	px_bg_table(0);
	px_spr_table(1);
	
	// Not using bank switching, but a good idea to set a reliable value at boot.
	px_uxrom_select(0);
	
	// Black out the palette.
	for(idx = 0; idx < 32; idx++) px_buffer_set_color(idx, 0x1D);
	px_wait_nmi();
	
	// Decompress the tileset into character memory.
	px_lz4_to_vram(CHR_ADDR(0, 0), CHR0);
	px_lz4_to_vram(CHR_ADDR(1, 0), CHRSM);

	sound_init(&SOUNDS);
	music_init(&MUSIC);
	
	// Jump to the splash screen state.
	splash_screen();
}
