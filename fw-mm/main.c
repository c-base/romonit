/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 mm@c-base.org
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "board.h"
#include "timer2.h"
#include "lcd.h"
#include "button.h"

int main (void) {
	uint8_t lcd = 0;

	_delay_ms(1);			// wait some time for things to settle

	// set clock to 4 Mhz
	//CLKPR = (1<<CLKPCE);	// prescaler change enable
	//CLKPR = (1<<CLKPS0);	// prescaler = 2 (internal RC runs @ 8MHz)

	// disable analog comparator (saves power)
	ACSR = (1<<ACD);

	// power reduction mode for timer1, SPI, UART and ADC
	PRR = (1<<PRLCD)|(1<<PRTIM1)|(1<<PRSPI)|(1<<PRUSART0)|(0<<PRADC);

	led_init();
	led_on(); _delay_ms(100); led_off();
	timer2_init();
	lcd_init();
	lcd_off();

	sei();

	while (1) {
		// powersave mode, will never return until interrupt sources are available
		// blink the LED for 10ms every wakeup
		SMCR = (0<<SM2)|(1<<SM1)|(1<<SM0)|(1<<SE);
		asm volatile ("sei");
		asm volatile ("sleep");
		asm volatile ("nop");
		if ( (sec%10) == 0) led_on(); _delay_ms(1); led_off();
		if ( button() == 1 ) {
			if (lcd == 1) {
				lcd_off();
				lcd = 0;
			} else {
				lcd_on();
				lcd = 1;
			}
		}
		lcd_seg(sec%32);
	}
}

