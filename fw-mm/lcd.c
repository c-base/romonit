/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 mm@c-base.org
 *
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/version.h>

#include "board.h"

void lcd_off(void) {
	LCDCRA = 0;
	PRR |= (1<<PRLCD);
}

void lcd_on(void) {
	PRR &= ~(1<<PRLCD);
	// low power
	LCDCRA = (1<<LCDEN) | (1<<LCDAB);
}

void lcd_init(void) {

	PRR &= ~(1<<PRLCD);
	// LCD Control and Status Register B
	LCDCRB = (1<<LCDCS)|(0<<LCD2B)|(0<<LCDMUX1)|(1<<LCDMUX0)|(1<<LCDPM2)|(0<<LCDPM1)|(1<<LCDPM0);
	// set contrast
	LCDCCR = ((1<<LCDCC2)|(1<<LCDCC1)|(1<<LCDCC0)) + 9;
	// frame rate
	LCDFRR = ((0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)|(0<<LCDCD2)|(1<<LCDCD1)|(0<<LCDCD0));

	lcd_on();

	// enable LCD frame start interrupt
	//LCDCRA |= (1<<LCDIE);

	// switch segments on
	LCDDR0 = 0xff;
	LCDDR1 = 0x80;
	// 0x1 = C, 0x2 = bat low, 0x4 = bat med, 0x8 = %, 0x10 = window, 0x20 = heat, 0x40 = warn
	LCDDR2 = 0x7f;

	LCDDR5 = 0xff;
	LCDDR6 = 0x80;
	// 0x01 = REL,  0x02, bat high, 0x04 = bat border, 0x08 = seg3 _,  0x10 = ',', 0x20 = seg2 _, 0x40 = seg1 _
	LCDDR7 = 0x7f;
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

ISR(LCD_vect) {
	asm volatile("nop");
}

