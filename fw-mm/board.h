/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 mm@c-base.org
 *
 */

#define F_MCP	F_CPU

#define	LED		PG4
#define	SW1		PF3

#define led_on()		(PORTG &= ~(1<<LED))
#define led_off()		(PORTG |= (1<<LED))
#define led_init()		(DDRG  |= (1<<LED))

