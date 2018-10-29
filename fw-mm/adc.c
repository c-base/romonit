/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 mm@c-base.org
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "lcd.h"

/* 
 * The trick to measure the VCC voltage, is to use VCC as reference voltage and
 * measure the internal 1.1V reference. 
 * This gives us VCC = 1.1 * 1024 / ADC
 *
 * We return VCC * 100
*/
uint16_t voltage;

void adc_sleep(void) {
    // powersave mode, will never return unless interrupt sources are available
	SMCR = (0<<SM2)|(0<<SM1)|(1<<SM0)|(1<<SE);
	asm volatile ("sei");
	asm volatile ("sleep");
	asm volatile ("nop");
}

ISR(ADC_vect) {
	   voltage = (uint16_t)((110*1024L)/ADC);
}

void adc_voltage(void) {
	// disable powersaving for the ADC
	PRR &= ~(1<<PRADC);

	// VCC as reference and the 1.1V band gap as input
	ADMUX = (0<<REFS1)|(1<<REFS0)|(1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1);

	// enable ADC, enable interrrupt, set prescaller to /8  1MHz/8 = 125KHz
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS1) | (1<<ADPS0);

	// wait for voltage reference 
	_delay_ms(1);

	/*
	The first conversion after switching voltage source may be inaccurate, and
	the user is advised to discard this result.
	*/
	// start conversion 
	ADCSRA |= (1<<ADSC);
	// wait for conversion to finish
	while( ADCSRA & (1<<ADSC) )
		adc_sleep();

	// ignore the result
	// start another conversion
	ADCSRA |= (1<<ADSC);
	while( ADCSRA & (1<<ADSC) )
		adc_sleep();

	// disable ADC
	ADMUX = 0;
	ADCSRA &= ~(1<<ADEN);
	// enable powersaving for ADC
	PRR |= (1<<PRADC);
}

void bat_update(void) {

	adc_voltage();

	if ( voltage >= 350 ) {
		lcd[0].bat  =  7;
	} else if ( voltage >= 340 ) {
		lcd[0].bat  =  6;
	} else if ( voltage >= 330 ) {
		lcd[0].bat  =  5;
	} else if ( voltage >= 320 ) {
		lcd[0].bat  =  4;
	} else if ( voltage >= 310 ) {
		lcd[0].bat  =  3;
	} else if ( voltage >= 300 ) {
		lcd[0].bat  =  2;
	} else if ( voltage >= 290 ) {
		lcd[0].bat  =  1;
	} else {
		lcd[0].bat  =  0;
	}

	lcd[0].bat_frame = 1;
	lcd[1].bat_frame = 1;
	lcd[1].bat  = lcd[0].bat;
}
