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

volatile uint32_t sec;

void timer2_init(void) {

	// disable OCIE2A and TOIE2
	TIMSK2 &= ~(1<<TOIE2);
	// timer2 asynchronous operation
	ASSR = (1<<AS2);

	TCNT2 = 0;

	// 32.768 kHz / 1024 => 8 sec per overflow
	TCCR2A |= (1<<CS22)|(0<<CS21)|(1<<CS20);
	// 32.768 kHz / 128 => 1 sec per overflow
	//TCCR2A |= (1<<CS22)|(0<<CS21)|(1<<CS20);

	while((ASSR & (_BV(TCN2UB)|_BV(TCR2UB))) != 0);
	TIFR2 = 0xFF;
	TIMSK2 |= (1<<TOIE2);
	sec = 0;

}

ISR(TIMER2_OVF_vect) {
	sec++;
}

