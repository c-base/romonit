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

void sleep(void) {
	// powersave mode, will never return until interrupt sources are available
	SMCR = (0<<SM2)|(1<<SM1)|(1<<SM0)|(1<<SE);
	asm volatile ("sei");
	asm volatile ("sleep");
	asm volatile ("nop");
}

int main (void) {
	uint8_t lcd_state = 0;

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
	lcd_state = 1;
	//lcd_off();

	sei();

	while (1) {
		sleep();
		// blink the LED for 10ms every wakeup
		if ( (sec%10) == 0) led_on(); _delay_ms(1); led_off();
		if ( button() ) {
			if (lcd_state) {
				lcd_off();
				lcd_state = 0;
			} else {
				lcd_on();
				lcd_state = 1;
			}
		}
		#if 1
		if (sec < 100 )
			lcd.digits[0] = 10;
		else
			lcd.digits[0] = (uint8_t)((sec/100) % 10);

		if (sec < 10 )
			lcd.digits[1] = 10;
		else
			lcd.digits[1] = (uint8_t)((sec/10) % 10);

		if ( sec == 0 )
			lcd.digits[2] = 10;
		else
			lcd.digits[2] = (uint8_t)( sec%10 );

		lcd.bat = sec % 4;

		if ( sec % 10 == 0 ) {
			lcd.bat_frame = 0;
			lcd.window = 0;
			lcd.thermometer = 0;
			lcd.warning = 0;
			lcd.percent = 0;
			lcd.rel = 0;
			lcd.comma = 0;
			lcd.degrees = 0;
		}

		if (sec % 10 == 1)
			lcd.bat_frame = 1;

		if (sec % 10 == 2)
			lcd.degrees = 1;

		if ( sec % 10 == 3)
			lcd.rel = 1;

		if ( sec % 10 == 4)
			lcd.percent = 1;

		if ( sec % 10 == 5)
			lcd.window = 1;

		if ( sec % 10 == 6)
			lcd.thermometer = 1;

		if ( sec % 10 == 7)
			lcd.warning = 1;

		if (sec % 10 == 8)
			lcd.comma = 1;

		#else
		lcd.digits[0] = 1; lcd.digits[1] = 2; lcd.digits[2] = 3;
		#endif

		lcd_update();
	}
}

