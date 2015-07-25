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
#include "adc.h"
#include "sht1x.h"

void sleep(void) {
	// powersave mode, will never return unless interrupt sources are available
	SMCR = (0<<SM2)|(1<<SM1)|(1<<SM0)|(1<<SE);
	asm volatile ("sei");
	asm volatile ("sleep");
	asm volatile ("nop");
}

int main (void) {
	uint32_t lcd_state = 1;

	_delay_ms(1);			// wait some time for things to settle

	// set clock to 4 Mhz
	//CLKPR = (1<<CLKPCE);	// prescaler change enable
	//CLKPR = (1<<CLKPS0);	// prescaler = 2 (internal RC runs @ 8MHz)

	// disable analog comparator (saves power)
	ACSR = (1<<ACD);

	// disable digital buffers on analog comparator
	DIDR1 = 0x03;

	// switch on pullups according to data sheet
	PORTA=0x0c;
	PORTB=0x01;
	////PORTC=0xff;
	////PORTD=0xff;
	PORTE=0x3f;
	PORTF=0xff;
	PORTG=0x36;

	// power reduction mode for timer1, SPI, UART and ADC
	PRR = (1<<PRLCD)|(1<<PRTIM1)|(1<<PRSPI)|(1<<PRUSART0)|(1<<PRADC);

	led_init();
	led_on(); _delay_ms(100); led_off();
	timer2_init();
	sht_init();
	bat_update();
	sei();
	lcd_init();
	sht_start();

	while (1) {
		sleep();
		if ( button() ) {
			if (lcd_state) {
				lcd_state = 0;
				lcd_off();
			} else {
				lcd_state = sec;
				bat_update();
				lcd_on();
			}
		}
		if ( (lcd_state + 900) < sec ) {
			lcd_off();
			lcd_state = 0;
		}
		// blink the LED for 10ms every wakeup
		//if ( (sec%10) == 0) led_on(); _delay_ms(1); led_off();
		if (lcd_state) {
			if (sec % 20 == 0) {
				sht_start();
				bat_update();
			}
			if ( sec % 4 == 2 ) {
				int16_t h = sht_get_hum();
				lcd.digits[0] = (h / 1000) % 10;
				lcd.digits[1] = (h /  100) % 10;
				lcd.digits[2] = (h /   10) % 10;
				lcd.comma = 1;
				lcd.percent = 1;
				lcd.rel = 1;
				lcd.degrees = 0;
				lcd_update();
			} 
			else if ( sec % 4 == 0 ) {
				int16_t t = sht_get_tmp();
				lcd.digits[0] = (t / 1000) % 10;
				lcd.digits[1] = (t /  100) % 10;
				lcd.digits[2] = (t /   10) % 10;
				lcd.comma = 1;
				lcd.degrees = 1;
				lcd.rel = 0;
				lcd.percent = 0;
				lcd_update();
			}
		}
	}
}

