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

void set_display(void) {
	int16_t val;

	val = sht_get_tmp();
	if ( val > 0 ) {
		if ( val > 1000 )
			lcd[0].digits[0] = (val / 1000) % 10;
		else
			lcd[0].digits[0] = 16;
		lcd[0].digits[1] = (val / 100) % 10;
		lcd[0].digits[2] = (val /  10) % 10;
		lcd[0].comma = 1;
	} else {
		lcd[0].digits[0] = 17;
		val *= -1;
	 	if ( val > 999 ) {
			lcd[0].digits[1] = (val / 1000) % 10;
			lcd[0].digits[2] = (val / 100) % 10;
			lcd[0].comma = 0;
		} else {
			lcd[0].digits[1] = (val / 100) % 10;
			lcd[0].digits[2] = (val /  10) % 10;
			lcd[0].comma = 1;
		}
	}
	lcd[0].degrees = 1;
	lcd[0].rel = 0;
	lcd[0].percent = 0;

	val = sht_get_hum();
	if ( val > 1000 ) 
		lcd[1].digits[0] = (val / 1000) % 10;
	else
		lcd[1].digits[0] = 16;
	lcd[1].digits[1] = (val / 100) % 10;
	lcd[1].digits[2] = (val /  10) % 10;
	lcd[1].comma = 1;
	lcd[1].percent = 1;
	lcd[1].rel = 1;
	lcd[1].degrees = 0;
}

int main (void) {
	uint8_t lcd_toggle = 0;
	uint32_t last = 1;

	// wait some time for things to settle
	_delay_ms(10);

	// disable watchdog
	WDTCR |= (1<<WDCE) | (1<<WDE);
	WDTCR = 0x00;

	// disable analog comparator (saves power)
	ACSR = (1<<ACD);

	// disable JTAG  (needs to be written twice in 4 cycles)
	MCUCR |= (1<<JTD);
	MCUCR |= (1<<JTD);

	// disable digital buffers on analog comparator
	DIDR1 = 0x03;
	// disable digital buffers on adc except PF3 (for button)
	DIDR0 = 0xf7;

	// switch on pullups according to data sheet
	// PORTA (LCD)
	// 0: COM0  (to LCD)
	// 1: COM1  (to LCD)
	// 2: COM2  unused
	// 3: COM3  unused
	// 4: SEG00 (to LCD)
	// 5: SEG01 (to LCD)
	// 6: SEG02 (to LCD)
	// 7: SEG03 (to LCD)
	PORTA=0xff;

	// PORTB (SPI, OCxx)
	// 0: SS   (to AT45DB041D)
	// 1: SCK  (to AT45DB041D)
	// 2: MOSI (to AT45DB041D)
	// 3: MISO (to AT45DB041D)
	// 4: OC0A (8 uA if pullup on)
	// 5: OC1A (8 uA if pullup on)
	// 6: OC1B (8 uA if pullup on)
	// 7: OC2A (8 uA if pullup on)
	PORTB=0x07;

	// PORTC (LCD)
	// 0: SEG12 unused
	// 1: SEG11 unused
	// 2: SEG10 unused
	// 3: SEG09 unused
	// 4: SEG08 unused
	// 5: SEG07 (to LCD)
	// 6: SEG06 (to LCD)
	// 7: SEG05 (to LCD)
	PORTC=0xff;

	// PORTD (LCD)
	// 0: SEG22 (to LCD)
	// 1: SEG21 (to LCD)
	// 2: SEG20 (to LCD)
	// 3: SEG19 (to LCD)
	// 4: SEG18 (to LCD)
	// 5: SEG17 (to LCD)
	// 6: SEG16 (to LCD)
	// 7: SEG15 (to LCD)
	PORTD=0xff;

	// PORTE (TWI, SERIAL, analog Comperator)
	// 0: RXD (to shrouded header)
	// 1: TXD (to shrouded header)
	// 2: XCK (to shrouded header)
	// 3: AIN1 (to shrouded header)
	// 4: USCK (to shrouded header)
	// 5: SDA (to shrouded header)
	// 6: DO  (to SHT SCK)
	// 7: CLKO to SHT DATA)
	PORTE=0xff;

	// PORTF (0-7, ADC, JTAG)
	// 0: ADC0 to shrouded header
	// 1: ADC1 connected to LCDCAP, needs to be on (as output or as pullup)
	// 2: ADC2 somehow related to SW1 (8uA when pressed and PF2 is high)
	// 3: ADC3 to button SW1 ( 100uA when pressed and pullup is on)
	// 4: ADC4 TCK to edge connector
	// 5: ADC5 TMS to edge connector
	// 6: ADC6 TDO to edge connector
	// 7: ADC7 TDI to edge connector
	PORTF=0xf9;

	// PORTG (0-5)
	// 0: SEG14 unused
	// 1: SEG13 unused
	// 2: SEG4  unused
	// 3: SEG24 unused
	// 4: SEG23 LED
	// 5: RESET
	PORTG=0x3f;

	// power reduction mode for timer1, SPI, UART and ADC
	PRR = (0<<PRLCD)|(1<<PRTIM1)|(1<<PRSPI)|(1<<PRUSART0)|(1<<PRADC);

	//led_on(); _delay_ms(100); led_off();
	lcd_init();
	timer2_init();
	sei();
	if ( button() ) {
		lcd_toggle = 1;
		sleep();
	}
	sht_init();
	// the first measurement is invalid
	sht_start();
	sht_start();
	bat_update();
	set_display();
	lcd_update(0);

	while (1) {
		// blink the LED for 10ms every wakeup
		//led_on(); _delay_ms(10); led_off();
		if (last != sec) {
			last = sec;

			if (sec % 8 == 0) {
				bat_update();
				sht_start();
				set_display();
			}

			if ( lcd_toggle ) {
				if ( sec % 4 == 0 )
					lcd_update(0);
				else if ( sec % 4 == 2 )
					lcd_update(1);
				else
					lcd_off();
			} else {
				if ( sec % 2 == 1 )
					lcd_update(1);
				else
					lcd_update(0);
			}
		}
		sleep();
	}
}
