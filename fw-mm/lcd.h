/* vim: ts=4 ai fdm=marker
 *  
 * Minimal firmware for the romonit
 * (c) 2015 mm@c-base.org
 *
 */

typedef struct {
    uint8_t digits[3], /* 0-9, 10->disabled */
            warning:1,  /* 022 */
            thermometer:1, /* 021 */
            window:1,   /* 020 */
            percent:1,  /* 019 */
            degrees:1,  /* 016 */
            comma:1,    /* 120 */
            rel:1,      /* 116 */
            bat:2, /* 0-3 */
            bat_frame:1;/* 118 */
} lcd_st;

extern lcd_st lcd;

void lcd_init(void);
void lcd_update(void);
void lcd_off(void);
void lcd_on(void);
void lcd_seg(uint8_t);

