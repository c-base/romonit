/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 jaseg@c-base.org
 *	    mm@c-base.org
 *
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/version.h>

#include "board.h"

typedef struct {
    uint8_t digits[3], /* 0-9, 10->disabled */
            warning:1,  /* 022 */
            thermometer:1, /* 021 */
            window:1,   /* 020 */
            percent:1,  /* 019 */
            degrees:1,  /* 016 */
            comma:1,    /* 120 */
            rel:1,      /* 116 */
            bat:2, /* 0-3 */
            bat_frame:1;/* 118 */
} lcd_st;

lcd_st lcd;

void lcd_off(void) {
	// disable LCD
	LCDCRA = 0;
	// shutoff LCD subsytem
	PRR |= (1<<PRLCD);
}

void lcd_on(void) {
	// clear LCD from powersave register
	PRR &= ~(1<<PRLCD);
	// low power driving waveform
	LCDCRA = (1<<LCDEN) | (1<<LCDAB) | (0<<LCDIE) | (1<<LCDBD) | (1<<LCDCCD);
}

void lcd_init(void) {

	lcd_on();
	// LCD Control and Status Register B
	LCDCRB = (1<<LCDCS)|(1<<LCDMUX0)|(1<<LCDPM2)|(1<<LCDPM0);
	// frame rate, will get adjusted after battery is measured
	LCDFRR = (1<<LCDCD0);

	#if 1
	// switch segments on
	LCDDR0 = 0xff;
	LCDDR1 = 0x80;
	LCDDR2 = 0x7f;
	LCDDR5 = 0xff;
	LCDDR6 = 0x80;
	LCDDR7 = 0x7f;
	#endif
}

void lcd_seg(uint8_t seg) {
	uint8_t bit;

	LCDDR0 = 0; // bits 0-7: 8 bits
	LCDDR1 = 0; // bits   7: 1 bit
	LCDDR2 = 0; // bits 0-6: 7 bit

	LCDDR5 = 0; // bits 0-7: 8 bits
	LCDDR6 = 0; // bits   7: 1 bit
	LCDDR7 = 0; // bits 0-6: 7 bits

	bit = seg % 8;
	if (seg < 8)
		LCDDR0 = (1<<bit);
	else if (seg < 9)
		LCDDR1 = ( (1<<bit) << 7);
	else if (seg < 16)
		LCDDR2 = (1<<(bit-1));

	else if (seg < 24)
		LCDDR5 = (1<<bit);
	else if (seg < 25)
		LCDDR6 = ( (1<<bit) << 7);
	else if (seg < 32)
		LCDDR7 = (1<<(bit-1));

}

#define SEG(x) (1<<(x))
#define ALL    0x7F
uint8_t digit_patterns[18] = {
    /* segments numbered left to right, top to bottom:
     * /-0-\
     * 1   2
     * +-3-+
     * 4   5
     * \-6-/
     */
    [0] = ALL - SEG(3),
    [1] = SEG(2) + SEG(5),
    [2] = ALL - SEG(1) - SEG(5),
    [3] = ALL - SEG(1) - SEG(4),
    [4] = SEG(1) + SEG(2) + SEG(3) + SEG(5),
    [5] = ALL - SEG(2) - SEG(4),
    [6] = ALL - SEG(2),
    [7] = SEG(0) + SEG(2) + SEG(5),
    [8] = ALL,
    [9] = ALL - SEG(4),
    [10]= ALL - SEG(6),
    [11]= ALL - SEG(0) - SEG(2),
    [12]= ALL - SEG(2) - SEG(3) - SEG(5),
    [13]= ALL - SEG(0) - SEG(1),
    [14]= ALL - SEG(2) - SEG(5),
    [15]= ALL - SEG(2) - SEG(6), - SEG(6),
    [16]= 0,
	[17]= SEG(3),
};
#undef SEG
#undef ALL

#define REG(v, s) ((!!(v))<<(s))
void lcd_update(void) {
    union {
        struct {
            uint8_t s6:1,
                    s5:1,
                    s4:1,
                    s3:1,
                    s2:1,
                    s1:1,
                    s0:1;
        };
        uint8_t pat;
    } d0 = {.pat=digit_patterns[lcd.digits[0]]},
      d1 = {.pat=digit_patterns[lcd.digits[1]]},
      d2 = {.pat=digit_patterns[lcd.digits[2]]};
    LCDDR0 = REG(d0.s5, 0) | REG(d0.s6, 1) | REG(d0.s4, 2)
        | REG(d1.s5, 3) | REG(d1.s6, 4) | REG(d1.s4, 5)
        | REG(d2.s5, 6) | REG(d2.s6, 7);
	LCDDR1 = REG(d2.s4, 7);
    LCDDR2 = REG(lcd.degrees, 0)
        | REG(lcd.bat>0, 1)
        | REG(lcd.bat>1, 2)
        | REG(lcd.percent, 3)
        | REG(lcd.window, 4)
        | REG(lcd.thermometer, 5)
        | REG(lcd.warning, 6);
    LCDDR5 = REG(d0.s2, 0) | REG(d0.s3, 1) | REG(d0.s1, 2)
        | REG(d1.s2, 3) | REG(d1.s3, 4) | REG(d1.s1, 5)
        | REG(d2.s2, 6) | REG(d2.s3, 7);
	LCDDR6 = REG(d2.s1, 7);
    LCDDR7 = REG(d2.s0, 3) | REG(d1.s0, 5) | REG(d0.s0, 6)
        | REG(lcd.rel, 0)
        | REG(lcd.bat>2, 1)
        | REG(lcd.bat_frame, 2)
        | REG(lcd.comma, 4);
}
#undef REG

ISR(LCD_vect) {
	asm volatile ("nop");
	//lcd_update();
}


