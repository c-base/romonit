#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "board.h"

#define SHT_CMD_TEMP	0x03
#define SHT_CMD_HUM	0x05
#define SHT_CMD_WSTAT	0x06
#define SHT_CMD_RSTAT	0x07
#define SHT_CMD_RESET	0x1E
#define SHT_RES_LOW	0x01
#define SHT_RES_HIGH	0x00

#define SHT_UNAVAIL	-32768
#define SHT_CRC_FAIL	-32767

#define SHT_5V		4010
#define SHT_4V		3980
#define SHT_3_5V	3970
#define SHT_3V		3960
#define SHT_2_5V	3940

#define sht_valid(v)	((v) > -32000)

void sleep(void);

typedef enum {
	IDLE,
	TEMP,
	HUM,
} sht_mode_t;

volatile sht_mode_t sht_mode;

volatile uint16_t sht_hum_raw=0,
		  sht_tmp_raw=0;

volatile int16_t sht_hum=0,
		 sht_tmp=0;

#define scl_hi()	(PORTE |=  (1<<6))
#define scl_lo()	(PORTE &= ~(1<<6))
#define sda_hi()	(DDRE  &= ~(1<<7))
#define sda_lo()	(DDRE  |=  (1<<7))

#define sda_val()	((uint8_t)!!(PINE & (1<<7)))
#define delay()		(_delay_us(2))
static void	scl_pulse(void)	{ scl_hi(); delay() ; scl_lo(); }

static void start(void) {
	// switch port modes
	PORTE &= ~((1<<6)|(1<<7));
	DDRE  |=  (1<<6);
	DDRE  &= ~(1<<7);
	//SHT_SCL_PORT &= ~(1<<SHT_SCL_PIN);
	//SHT_SDA_PORT &= ~(1<<SHT_SDA_PIN);
	//SHT_SCL_DDR  |=  (1<<SHT_SCL_PIN);
	//SHT_SDA_DDR  &= ~(1<<SHT_SDA_PIN);
	//delay();
	delay();

	// reset
	for (uint8_t i = 0; i < 10; i++ ) {
		scl_pulse();
		delay();
	}

	// "start" sequence
	scl_hi(); delay();
	sda_lo(); delay();
	scl_lo(); delay();
	scl_hi(); delay();
	sda_hi(); delay();
	scl_lo(); delay();
}

static uint8_t crc_value;
static void crc8(uint8_t b) {
	for (uint8_t i = 0; i < 8; ++i) {
	if ((crc_value ^ b) & 0x80) {
		crc_value <<= 1;
		crc_value ^= 0x31;
	} else
		crc_value <<= 1;
	b <<= 1;
	}
}

static uint8_t send(uint16_t b) {
	crc8(b);
	
	// data
	for (uint8_t i = 0; i < 8; ++i) {
		if( b & 0x80 ) 
			sda_hi();
		else
			sda_lo();
		b <<= 1;
		delay();
		scl_pulse();
	}

	// acknowledge
	sda_hi();
	delay();
	uint8_t ack = sda_val();
	scl_pulse();
	return ack;
}

uint8_t recv_data(void) {
	// data
	uint8_t b = 0;
	for (uint8_t i = 0; i < 8; i++) {
		// data is transmitted MSB first
		b <<= 1;
		if (sda_val())
			b |= 1;
		scl_pulse();
		delay();
	}

	// lo acknowledge
	sda_lo();
	delay();
	scl_pulse();
	sda_hi();
	delay();

	//crc8(b);
	return b;
}

static uint8_t recv_crc(void) {
	// data
	uint8_t b = 0;
	for (uint8_t i = 0; i < 8; ++i) {
	// CRC is transmitted LSB first
	b >>= 1;
	if (sda_val())
		b |= 0x80;
	scl_pulse();
	delay();
	}

	// hi acknowledge
	sda_hi();
	delay();
	scl_pulse();
	delay();

	return b;
}

void sht_init(void) {
	start();
	send(SHT_CMD_RESET);
	_delay_ms(11);

	start();
	send(SHT_CMD_WSTAT);
	send(SHT_RES_LOW);    //set resolution to low, we measure 12bit temp and 12bit hum

	sht_mode=IDLE;
	EIMSK  |= (1<<PCIE0);
}

void sht_start(void) {
	start();
	// switch to low resolution for temp as 12-bits are enough
	send(SHT_CMD_WSTAT);
	send(SHT_RES_LOW);
	crc_value = SHT_RES_LOW << 7; // bit-reversed
	sht_mode = TEMP;
	start();
	send(SHT_CMD_TEMP);
	delay(); // this seems important for the interrupt to work
	PCMSK0 |= (1<<7);
	while ( sht_mode != IDLE )
		sleep();
}

static int16_t result(void) {
	int16_t v = recv_data() << 8; v |= recv_data();
	uint8_t crc = recv_crc();
	//if (crc != crc_value)
	//return SHT_CRC_FAIL;
	return(v);
}

static int16_t sht_temp(void) {
	return(sht_tmp_raw * 4 - SHT_TEMP_V_COMP);
}

static int16_t sht_humid(void) {
	int16_t v = sht_hum_raw;
	#if 0 // low resolution
	// inspired by Werner Hoch, modified for low resolution mode
	const int32_t C1 = (int32_t)(-4.0 * 100);
	const int32_t C2 = (int32_t)(0.648 * 100 * (1L<<24));
	const int32_t C3 = (int32_t)(-7.2e-4 * 100 * (1L<<24));
	v = (int16_t)((((C3 * v + C2) >> 7) * v + (1L<<16)) >> 17) + C1;
	const int32_t T1 = (uint32_t)(0.01 * (1L<<30));
	const int32_t T2 = (uint32_t)(0.00128 * (1L<<30));
	#else // high resolution
	// inspired by Werner Hoch
	const int32_t C1 = (int32_t)(-4.0 * 100);
	const int32_t C2 = (int32_t)(0.0405 * 100 * (1L<<28));
	const int32_t C3 = (int32_t)(-2.8e-6 * 100 * (1L<<30));
	v = (int16_t)((((((C3 * v) >> 2) + C2) >> 11) * v + (1L<<16)) >> 17) + C1;
	const int32_t T1 = (uint32_t)(0.01 * (1L<<30));
	const int32_t T2 = (uint32_t)(0.00008 * (1L<<30));
	#endif
	// implemented by whitejack (temp compensation)
	v = (int16_t)( (((((int32_t)sht_tmp-2500) * (int32_t)( (T1+T2*((int32_t)sht_hum_raw)) >>13))>>17) + ((int32_t)v)) );
	if(v>9999)v=9999;
	if(v<0001)v=0001;
	return v;
}

ISR(PCINT0_vect) {
	// we get called when a mesurement is done
	// disable interrupts on this pin
	PCMSK0 &= ~(1<<7);
	sei();
	int16_t v = result();
	if (sht_mode == TEMP) {
		sht_tmp_raw = v;
		sht_tmp = sht_temp();
		start();
		send(SHT_CMD_WSTAT);
		send(SHT_RES_HIGH);
		crc_value = SHT_RES_HIGH << 7;
		sht_mode = HUM;
		start();
		send(SHT_CMD_HUM);
		delay();
		PCMSK0 |= (1<<7);
	} else if (sht_mode == HUM) {
		sht_hum_raw = v;
		sht_hum = sht_humid();
		sht_mode = IDLE;
		// switch port to input and pullup on to minimize current
		DDRE  &= ~( (1<<6) | (1<<7) );
		PORTE |=  ( (1<<6) | (1<<7) );
	}
	//} else {
	//	sht_tmp = SHT_UNAVAIL;
	//	sht_hum_raw = SHT_UNAVAIL;
	//	sht_tmp = SHT_UNAVAIL;
	//	sht_hum = SHT_UNAVAIL;
	//}
}

int16_t sht_get_tmp(void) {
	return(sht_tmp);
}

int16_t sht_get_hum(void) {
	return(sht_hum);
}

