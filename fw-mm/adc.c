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
	ADMUX = 0;
	ADCSRA &= ~(1<<ADEN);
	// enable powersaving for ADC
	PRR |= (1<<PRADC);

	return( (uint16_t)((110*1024L)/adc) );
}

void bat_update(void) {
	uint16_t voltage;

	voltage = adc_voltage();

	// LCD contrast adjustment
	#if 0
	if ( voltage < 270 ) {
		// 2.65V: 300us, fr/1 => 8.1uA
		LCDFRR = ( 0<<LCDCD2 | 0<<LCDCD1 | 0<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 0<<LCDDC1 | 0<<LCDDC0 );
	} else if ( voltage < 290 ) {
		// 2.90V: 150us, fr/1 => 7.7uA
		LCDFRR = ( 0<<LCDCD2 | 0<<LCDCD1 | 0<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 1<<LCDDC1 | 0<<LCDDC0 );
	} else if ( voltage < 310 ) {
		// 3.10V: 150us, fr/2 => 6.9uA
		LCDFRR = ( 0<<LCDCD2 | 0<<LCDCD1 | 1<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 1<<LCDDC1 | 0<<LCDDC0 );
	} else if ( voltage < 330 ) {
		// 3.30V: 150us, fr/3 => 6.7uA
		LCDFRR = ( 0<<LCDCD2 | 1<<LCDCD1 | 0<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 1<<LCDDC1 | 0<<LCDDC0 );
	} else if ( voltage < 350 ) {
		// 3.40V: 150us, fr/4 => 6.6uA
		LCDFRR = ( 0<<LCDCD2 | 1<<LCDCD1 | 1<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 1<<LCDDC1 | 0<<LCDDC0 );
	} else {
		// 3.65V: 150us, fr/5 => 6.7uA
		LCDFRR = ( 1<<LCDCD2 | 0<<LCDCD1 | 0<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 1<<LCDDC1 | 0<<LCDDC0 );
	}
	#else
	if ( voltage < 310 ) {
		// 2.75V: 300us, fr/1 => 7.6uA
		// 3.00V: 300us, fr/1 => 7.9uA
		LCDFRR = ( 0<<LCDCD2 | 0<<LCDCD1 | 0<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 0<<LCDDC1 | 0<<LCDDC0 );
	} else if ( voltage < 330 ) {
		// 3.20V: 300us, fr/2 => 6.9uA
		LCDFRR = ( 0<<LCDCD2 | 0<<LCDCD1 | 1<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 0<<LCDDC1 | 0<<LCDDC0 );
	} else if ( voltage < 350 ) {
		// 3.40V: 300us, fr/3 => 6.5uA
		LCDFRR = ( 0<<LCDCD2 | 1<<LCDCD1 | 0<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 0<<LCDDC1 | 0<<LCDDC0 );
	} else {
		// 3.65V: 300us, fr/4 => 6.3uA
		LCDFRR = ( 0<<LCDCD2 | 1<<LCDCD1 | 1<<LCDCD0 );
		LCDCCR = ( 0<<LCDDC2 | 0<<LCDDC1 | 0<<LCDDC0 );
	}
	#endif

	if ( voltage > 355 ) {
		lcd[0].bat = 3;
		lcd[1].bat = 3;
	} else if ( voltage > 340 ) {
		lcd[0].bat = 2;
		lcd[1].bat = 2;
	} else if ( voltage > 300 ) {
		lcd[0].bat = 1;
		lcd[1].bat = 1;
	} else {
		lcd[0].bat = 0;
		lcd[1].bat = 0;
	}
	lcd[0].bat_frame = 1;
	lcd[1].bat_frame = 1;
}
