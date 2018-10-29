/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 mm@c-base.org
 *
 */

#define F_MCP			F_CPU

#define	LED				PG4
#define	SW1				PF3

#define SHT_SCL_PORT	PORTE
#define SHT_SCL_PIN		PE6
#define SHT_SCL_DDR		DDRE

#define SHT_SDA_PORT	PORTE
#define SHT_SDA_IN		PINE
#define SHT_SDA_PIN		PE7
#define SHT_SDA_DDR		DDRE
#define SHT_INT_MASK	(1<<SHT_SDA_PIN)
#define SHT_INT_BIT		(1<<PCIE0)
#define SHT_INT_REG		PCMSK0
#define SHT_INT_VECT	PCINT0_vect
#define SHT_TEMP_V_COMP	SHT_3_5V

#define led_on()		do { DDRG |= (1<<LED); PORTG &= ~(1<<LED); } while (0)
#define led_off()		do { PORTG |= (1<<LED); DDRG &= ~(1<<LED); } while (0)

void sleep(void);

