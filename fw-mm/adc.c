/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 mm@c-base.org
 *
 */

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

/* 
 * The trick to measure the VCC voltage, is to use VCC as reference voltage and
 * measure the internal 1.1V reference. 
 * This gives us VCC = 1.1 * 1024 / ADC
 *
 * We return VCC * 100
*/

uint16_t adc_voltage(void) {
	// disable powersaving for the ADC
	PRR &= ~(1<<PRADC);

	// VCC as reference and the 1.1V band gap as input
	ADMUX = (0<<REFS1)|(1<<REFS0)|(1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1);

	// enable ADC, set prescaller to /8  1MHz/8 = 125KHz
	ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);

	// wait for voltage reference 
	_delay_ms(1);

	/*
	The first conversion after switching voltage source may be inaccurate, and
	the user is advised to discard this result.
	*/
	// start conversion 
	ADCSRA |= (1<<ADSC);
	// wait for conversion to finish
	while( ADCSRA & (1<<ADSC) ); uint16_t adc = ADC;
	// ignore the result

	// start another conversion
	ADCSRA |= (1<<ADSC);
	while( ADCSRA & (1<<ADSC) );
	adc = ADC;

	// disable ADC
	ADCSRA &= ~(1<<ADEN);
	// enable powersaving for ADC
	PRR |= (1<<PRADC);

	return( (uint16_t)((110*1024L)/adc) );
}

void bat_update(void) {
	uint16_t voltage;

	voltage = adc_voltage();

	if ( voltage > 360 ) {
		lcd.bat = 3;
		LCDFRR |=  (1<<LCDCD1);
		LCDFRR &= ~(1<<LCDCD0);
	} else if ( voltage > 330 ) {
		lcd.bat = 2;
		LCDFRR &= ~(1<<LCDCD1);
		LCDFRR |=  (1<<LCDCD0);
	} else if ( voltage > 300 ) {
		lcd.bat = 1;
		LCDFRR &= ~((1<<LCDCD1)|(1<<LCDCD0));
	} else
		lcd.bat = 0;
	lcd.bat_frame = 1;
}
