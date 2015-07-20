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

void lcd_init(void) {

	// LCD Control and Status Register B
	LCDCRB = (1<<LCDCS)|(0<<LCD2B)|(0<<LCDMUX1)|(1<<LCDMUX0)|(1<<LCDPM2)|(0<<LCDPM1)|(1<<LCDPM0);
	// set contrast
	LCDCCR = (1<<LCDCC2)|(1<<LCDCC1)|(1<<LCDCC0) + 7;
	// frame rate
	LCDFRR = ((0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)|(0<<LCDCD2)|(1<<LCDCD1)|(0<<LCDCD0));

	// low power
	LCDCRA = (1<<LCDEN) | (1<<LCDAB);

	// enable LCD frame start interrupt
	//LCDCRA |= (1<<LCDIE);
}

ISR(LCD_vect) {
	asm volatile("nop");
}

