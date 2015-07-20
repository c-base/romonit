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

int main (void) {

	_delay_ms(1);			// wait some time for things to settle

	// set clock to 4 Mhz
	CLKPR = (1<<CLKPCE);	// prescaler change enable
	CLKPR = (1<<CLKPS0);	// prescaler = 2 (internal RC runs @ 8MHz)

	// disable analog comparator (saves power)
	ACSR = (1<<ACD);

	// power reduction mode for timer1, SPI, UART and ADC
	PRR = (1<<PRTIM1)|(1<<PRSPI)|(1<<PRUSART0)|(1<<PRADC);

	// disable Digital input on PF0-7 except PF3 (saves power)
	DIDR0 = 0xF7;
	PORTF = 0x08;

	// switch on the LED to test
	DDRG  = 0x10;
	PORTG = 0x00;
	_delay_ms(100);
	PORTG = 0x10;

	timer2_init();
	lcd_init();

	sei();

		LCDDR0 = 0xff;
		LCDDR1 = 0x80;
// 0x1 = C, 0x2 = bat low, 0x4 = bat med, 0x8 = %, 0x10 = window, 0x20 = heat, 0x40 = warn
		LCDDR2 = 0x7f;
		
		LCDDR5 = 0xff;
		LCDDR6 = 0x80;
// 0x01 = REL,  0x02, bat high, 0x04 = bat border, 0x08 = seg3 _,  0x10 = ',', 0x20 = seg2 _, 0x40 = seg1 _
		LCDDR7 = 0x7f;

	while (1) {
		// powersave mode, will never return until interrupt sources are available
		// blink the LED for 10ms every wakeup
		SMCR = (0<<SM2)|(1<<SM1)|(1<<SM0)|(1<<SE);
		asm volatile ("sei");
		asm volatile ("sleep");
		asm volatile ("nop");
		PORTG = 0x00; _delay_ms(1); PORTG = 0x10;
	}
}

