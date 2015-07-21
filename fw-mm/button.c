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

uint8_t button(void) {
	uint8_t ret;

	// enable digital input only during button read to save power
	DIDR0 &= ~(1<<SW1);
	// switch on pullup
	PORTF |= (1<<SW1);
	asm volatile ("nop");

	if (PINF & (1<<SW1))
		ret = 0;
	else
		ret = 1;

	// turn off pullup again
	//PORTF &= ~(1<<SW1);
	// turn off digital input
	DIDR0 |= (1<<SW1);

	return(ret);
}

