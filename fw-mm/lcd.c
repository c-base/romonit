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
#include <util/delay.h>

#include "board.h"

extern uint16_t voltage;

typedef struct {
    uint8_t  digits[3], /* 0-9, 10->disabled */
             warning:1,  /* 022 */
             thermometer:1, /* 021 */
             window:1,   /* 020 */
             percent:1,  /* 019 */
             degrees:1,  /* 016 */
             comma:1,    /* 120 */
             rel:1,      /* 116 */
             bat_frame:1,/* 118 */
             bat:3;      /* 0-7 */
} lcd_st;

lcd_st lcd[2];

void lcd_off(void) {

	// poweroff LCDCAP
	DDRF  &= ~(0x02);
	PORTF &= ~(0x02);
	// clear interrupt flag
	LCDCRA |= (1<<LCDIF);
	while ( !(LCDCRA & (1<<LCDIF)) );
	// blank display
	LCDCRA |= (1<<LCDBL);
	while ( !(LCDCRA & (1<<LCDIF)) );
	// shutoff display
	LCDCRA &= ~((1<<LCDEN)|(1<<LCDBL));
	PRR |= (1<<PRLCD);
}

void lcd_on(void) {
	PRR &= ~(1<<PRLCD);
	LCDCRA |= (1<<LCDEN);
	DDRF  |= 0x02;
	PORTF |= 0x02;
}

void lcd_contrast(void) {

	if ( voltage > 460) {
		// 150us drive time, 2.6V
		LCDCCR = (0<<LCDDC2)|(1<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// lowest framerate without flickering, clk: 256 Hz
		LCDFRR = (1<<LCDPS1) | (0<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (0<<LCDCD0);
		// 1/3 bias
		LCDCRB &= ~(1<<LCD2B);
		// lowpower waveform on
		LCDCRA |= (1<<LCDAB);
	} else if ( voltage > 420 ) {
		// 150us drive time, 2.6V
		LCDCCR = (0<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// lowest framerate without flickering, clk: 256 Hz
		LCDFRR = (1<<LCDPS1) | (0<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (0<<LCDCD0);
		// 1/2 bias
		LCDCRB |= (1<<LCD2B);
		// lowpower waveform on
		LCDCRA |= (1<<LCDAB);
	} else if ( voltage > 390 ) {
		// 150us drive time, 2.6V
		LCDCCR = (0<<LCDDC2)|(1<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// lowest framerate without flickering, clk: 256 Hz
		LCDFRR = (1<<LCDPS1) | (0<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (0<<LCDCD0);
		// 1/3 bias
		LCDCRB &= ~(1<<LCD2B);
		// lowpower waveform on
		LCDCRA &= ~(1<<LCDAB);
	} else if ( voltage > 360 ) { // 3.6 - 3.9 V
		// 150us drive time, 2.6V
		LCDCCR = (0<<LCDDC2)|(1<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// lowest framerate without flickering, clk: 256 Hz
		LCDFRR = (1<<LCDPS1) | (0<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (0<<LCDCD0);
		// 1/2 bias
		LCDCRB |= (1<<LCD2B);
		// lowpower waveform off
		LCDCRA &= ~(1<<LCDAB);
	} else if ( voltage > 320 ) { // 3.2 - 3.6 V
		// 300us drive time, 2.6V
		LCDCCR = (0<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// lowest framerate without flickering, clk: 256 Hz
		LCDFRR = (1<<LCDPS1) | (0<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (0<<LCDCD0);
		// 1/2 bias
		LCDCRB |= (1<<LCD2B);
		// lowpower waveform off
		LCDCRA &= ~(1<<LCDAB);
	} else if ( voltage > 290 ) { // 2.9V - 3.2V
		// 300us drive time, 2.6V
		LCDCCR = (0<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// increase framerate, clk: 512 Hz
		LCDFRR = (0<<LCDPS1) | (1<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (0<<LCDCD0);
		// 1/2 bias
		LCDCRB |= (1<<LCD2B);
		// lowpower waveform off
		LCDCRA &= ~(1<<LCDAB);
	} else if ( voltage > 270 ) { // 2.7V - 2.9V
		// 300us drive time, 2.6V
		LCDCCR = (0<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// increase framerate, clk: 1024 Hz
		LCDFRR = (0<<LCDPS1) | (0<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (1<<LCDCD0);
		// 1/2 bias
		LCDCRB |= (1<<LCD2B);
		// lowpower waveform off
		LCDCRA &= ~(1<<LCDAB);
	} else { // 0 - 2.7V
		// below 2.6V we would need contrast control, using 2.8V works fine down to 2.2V supply
		// but we would need to reinitialize the LCD, this is not implemented

		// 300us drive time, 2.6V
		LCDCCR = (1<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(0<<LCDCC3)|(0<<LCDCC2)|(0<<LCDCC1)|(0<<LCDCC0);
		// increase framerate, clk: 2048 Hz
		LCDFRR = (0<<LCDPS1) | (0<<LCDPS0) | (0<<LCDCD2) | (0<<LCDCD1) | (0<<LCDCD0);
		// 1/2 bias
		LCDCRB |= (1<<LCD2B);
		// lowpower waveform off
		LCDCRA &= ~(1<<LCDAB);
	}

}

void lcd_init(void) {

	// clear LCD from powersave register
	PRR &= ~(1<<PRLCD);

	// Contrast control 150us, 3.6V
	LCDCCR = ( 0<<LCDDC2 | 1<<LCDDC1 | 0<<LCDDC0 | 1<<LCDCC3 | 0<<LCDCC2 | 0<<LCDCC1 | 0<<LCDCC0 );

	// frame rate, will get adjusted after battery is measured
	LCDFRR = ( 1<<LCDPS0 | 0<<LCDCD2 | 0<<LCDCD1 | 1<<LCDCD0 );

	// LCD Control and Status Register B
	LCDCRB = (1<<LCDCS)|(1<<LCD2B)|(1<<LCDMUX0)|(1<<LCDPM2)|(1<<LCDPM0);

	// disable contrast control and buffer, low power driving waveform doesn't look good
	LCDCRA = (1<<LCDEN)|(0<<LCDAB)|(1<<LCDIF)|(0<<LCDIE)|(1<<LCDBD)|(1<<LCDCCD);

	// provide power to LCDCAP
	DDRF  |= 0x02;
	PORTF |= 0x02;
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
void lcd_update(uint8_t plane) {

	lcd_on();
	lcd_contrast();

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
    } d0 = {.pat=digit_patterns[lcd[plane].digits[0]]},
      d1 = {.pat=digit_patterns[lcd[plane].digits[1]]},
      d2 = {.pat=digit_patterns[lcd[plane].digits[2]]};
    LCDDR0 = REG(d0.s5, 0) | REG(d0.s6, 1) | REG(d0.s4, 2)
        | REG(d1.s5, 3) | REG(d1.s6, 4) | REG(d1.s4, 5)
        | REG(d2.s5, 6) | REG(d2.s6, 7);
	LCDDR1 = REG(d2.s4, 7);
    LCDDR2 = REG(lcd[plane].degrees, 0)
        | REG(lcd[plane].bat & 0x01, 1)
        | REG(lcd[plane].bat & 0x02, 2)
        | REG(lcd[plane].percent, 3)
        | REG(lcd[plane].window, 4)
        | REG(lcd[plane].thermometer, 5)
        | REG(lcd[plane].warning, 6);
    LCDDR5 = REG(d0.s2, 0) | REG(d0.s3, 1) | REG(d0.s1, 2)
        | REG(d1.s2, 3) | REG(d1.s3, 4) | REG(d1.s1, 5)
        | REG(d2.s2, 6) | REG(d2.s3, 7);
	LCDDR6 = REG(d2.s1, 7);
    LCDDR7 = REG(d2.s0, 3) | REG(d1.s0, 5) | REG(d0.s0, 6)
        | REG(lcd[plane].rel, 0)
        | REG(lcd[plane].bat & 0x04, 1)
        | REG(lcd[plane].bat_frame, 2)
        | REG(lcd[plane].comma, 4);
}
#undef REG

//ISR(LCD_vect) {
//	asm volatile ("nop");
	//lcd_update();
//}

