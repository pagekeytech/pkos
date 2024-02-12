#include "vga.h"
#include "../kernel/kernel.h"
#include "../memory/memory.h"
#include "../screen/screen.h"
#include "reg_ac.h"
#include "reg_crtc.h"
#include "reg_ext.h"
#include "reg_gc.h"
#include "reg_seq.h"

#define COLOR_BLACK 0x0
#define COLOR_GREEN 0x2
#define COLOR_PURPLE 0xf


unsigned int vga_mode_var = 0;

u32 get_reg(u32 address, u32 data, u32 index) {
    /**
        Get the value of a register using ioports.
    */
    // Save the current value of the address register
    u32 saved_addr_reg = ioport_in(address);
    // Set the address register to indicate where we will read
	ioport_out(address, index);
    // Get the data from that address
	u32 result = ioport_in(data);
    // Restore the original value of the address register
	ioport_out(address, saved_addr_reg);
    // Return the result
	return result;
}

u32 set_reg(u32 address, u32 data, u32 index, u32 value) {
    /**
        Set the value of a register using ioports.
    */
    // Save the current value of the address register
	unsigned int saved_addr_reg = ioport_in(address);
    // Set the address to which we are writing
	ioport_out(address, index);
    // Set the value at that address by writing to the data port
	ioport_out(data, value);
    // Restore the original value of the address register
	ioport_out(address, saved_addr_reg);
}



void vga_info() {
	println("Attribute Controller:");
	struct AttributeController ac;
	get_ac(ac);
	// print_ac(ac);
	println("--------------------");
	println("External/General:");
	struct ExternalGeneral ext;
	get_ext(ext);
	// print_ext(ext);
	println("--------------------");
	println("CRT Controller:");
	u8 ioAddressSelect = ext.regMisc & 0b1;
	struct CathodeRayTubeController crtc;
	get_crtc(crtc, ioAddressSelect);
	// print_crtc(crtc, ioAddressSelect);
	println("--------------------");
	println("Graphics Controller:");
	struct GraphicsController gc;
	get_gc(gc);
	// print_gc(gc);
	println("--------------------");
	println("Sequencer:");
	struct Sequencer seq;
	get_seq(seq);
	print_seq(seq);
	println("--------------------");
}

void vga_enter() {
	if (vga_mode_var == 1) return;
	vga_mode_var = 1;
    println("Attempting to switch modes...");

	// Save video memory somewhere else
	// 0xb8000 to 0xbffff (32K)
	memcpy(0x0010b8000, 0xb8000, COLS*ROWS*2);

	struct AttributeController ac;
	get_ac(ac);
	ac.regPalettes[0] = 0;
	ac.regPalettes[1] = 1;
	ac.regPalettes[2] = 2;
	ac.regPalettes[3] = 3;
	ac.regPalettes[4] = 4;
	ac.regPalettes[5] = 5;
	ac.regPalettes[6] = 6;
	ac.regPalettes[7] = 7;
	ac.regPalettes[8] = 8;
	ac.regPalettes[9] = 9;
	ac.regPalettes[10] = 10;
	ac.regPalettes[11] = 11;
	ac.regPalettes[12] = 12;
	ac.regPalettes[13] = 13;
	ac.regPalettes[14] = 14;
	ac.regPalettes[15] = 15;
	ac.regAttributeMode = 0x41;
	ac.regOverscanColor = 0;
	ac.regColorPlane = 0xF;
	ac.regHorizPixel = 0;
	ac.regPixelShift = 0;
	set_ac(ac);

	struct ExternalGeneral ext;
	get_ext(ext);
	ext.regMisc = 0x63;
	set_ext(ext);

	struct GraphicsController gc;
	get_gc(gc);
	gc.regSetReset = 0;
	gc.regEnableSetReset = 0;
	gc.regColorCompare = 0;
	gc.regDataRotate = 0;
	gc.regReadMap = 0;
	gc.regGraphicsMode = 0x40;
	gc.regMisc = 0x05;
	gc.regColorDontCare = 0x0F;
	gc.regBitMask = 0xFF;
	set_gc(gc);

	u8 ioAddressSelect = ext.regMisc & 0b1;
	struct CathodeRayTubeController crtc;
	get_crtc(crtc, ioAddressSelect);
	crtc.regHorizTotal = 0x5F;
	crtc.regEndHorizDisplay = 0x4F;
	crtc.regStartHorizBlanking = 0x50;
	crtc.regEndHorizBlanking = 0x82;
	crtc.regStartHorizRetrace = 0x54;
	crtc.regEndHorizRetrace = 0x80;
	crtc.regVertTotal = 0xBF;
	crtc.regOverflow = 0x1F;
	crtc.regPresetRowScan = 0x00;
	crtc.regMaxScanLine = 0x41;
	crtc.regCursorStart = 0x00;
	crtc.regCursorEnd = 0x00;
	crtc.regStartAddressHigh = 0x00;
	crtc.regStartAddressLow = 0x00;
	crtc.regCursorLocationHigh = 0x00;
	crtc.regCursorLocationLow = 0x00;
	crtc.regVertRetraceStart = 0x9C;
	crtc.regVertRetraceEnd = 0x0E;
	crtc.regVertDisplayEnd = 0x8F;
	crtc.regOffset = 0x28;
	crtc.regUnderlineLocation = 0x40;
	crtc.regStartVertBlanking = 0x96;
	crtc.regEndVertBlanking = 0xB9;
	crtc.regModeControl = 0xA3;
	crtc.regLineCompare = 0xFF;
	set_crtc(crtc, ioAddressSelect);

	// struct Sequencer seq;
	// get_seq(seq);
	// // TODO set relevant fields here
	// set_seq(seq);

	memset(0xb8000, 0, 60);
	vga_clear_screen();
	vga_plot_pixel(0, 0, COLOR_GREEN);
	vga_plot_pixel(1, 0, COLOR_PURPLE);
    vga_plot_pixel(1,1,COLOR_GREEN);
    vga_plot_pixel(2,1,COLOR_GREEN);
    vga_plot_pixel(2,2,COLOR_GREEN);
	// // draw rectangle
	// draw_rectangle(150, 10, 100, 50);
	// // draw some faces
	// draw_happy_face(10,10);
	// draw_happy_face(100,100);
	// draw_happy_face(300,150);
	// // bounds
	// vga_plot_pixel(0, 0, 15);
	// vga_plot_pixel(319, 199, COLOR_PURPLE);
	// // see some colors
	// for (int i = 0; i < 15; i++) {
	// 	for (int j = 0; j < 100; j++) {
	// 		vga_plot_pixel(i, 50+j, i);
	// 	}
	// }
	
}

void vga_exit() {
	if (vga_mode_var == 0) return;
	// Go back to alphanumeric disable 0
	struct GraphicsController config;
	get_gc(config);
	config.regMisc &= 0;
	config.regMisc |= 0b10; // bit 1 is RAM enable, set it to 1
	config.regMisc |= 0b1100; // set mem map select to 11
	set_gc(config);

	// Restore text-mode video memory
	memcpy(0xb8000, 0x0010b8000, COLS*ROWS*2);

	vga_mode_var = 0;

	print_prompt();
}

void draw_rectangle(int x, int y, int width, int height) {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			vga_plot_pixel(x+i, y+j, COLOR_GREEN);
		}
	}
}

void draw_happy_face(int x, int y) {
	// eye
	vga_plot_pixel(x,y,COLOR_PURPLE);
	// eye
	vga_plot_pixel(x+10,y,COLOR_PURPLE);
	// mouth
	vga_plot_pixel(x,	y+8,COLOR_PURPLE);
	vga_plot_pixel(x+1,	y+9,COLOR_PURPLE);
	vga_plot_pixel(x+2,	y+10,COLOR_PURPLE);
	vga_plot_pixel(x+3,	y+10,COLOR_PURPLE);
	vga_plot_pixel(x+4,	y+10,COLOR_PURPLE);
	vga_plot_pixel(x+5,	y+10,COLOR_PURPLE);
	vga_plot_pixel(x+6,	y+10,COLOR_PURPLE);
	vga_plot_pixel(x+7,	y+10,COLOR_PURPLE);
	vga_plot_pixel(x+8,	y+10,COLOR_PURPLE);
	vga_plot_pixel(x+9,	y+9,COLOR_PURPLE);
	vga_plot_pixel(x+10,y+8,COLOR_PURPLE);
}

void vga_clear_screen() {
    // Note: "clear_screen" name conflicted with something in screen.h
    // Now I see why namespacing is a thing
    for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 200; j++) {
            vga_plot_pixel(i,j,COLOR_BLACK);
        }
    }
}

void vga_plot_pixel(int x, int y, unsigned short color) {
    unsigned short offset = x + 320 * y;
	unsigned char *PLANE0 = (unsigned char*) PLANE0_ADDRESS;
    PLANE0[offset] = color;
	unsigned char *PLANE1 = (unsigned char*) PLANE1_ADDRESS;
    PLANE1[offset] = color;
	unsigned char *PLANE2 = (unsigned char*) PLANE2_ADDRESS;
    PLANE2[offset] = color;
	unsigned char *PLANE3 = (unsigned char*) PLANE3_ADDRESS;
    PLANE3[offset] = color;
}
