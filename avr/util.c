#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include "port.h"
#include "util.h"
void delay_ds(uint8_t s) {
	while(s--)
		_delay_ms(100);
}
void out(uint8_t num) {//
	LED_OFF;
	delay_ds(25);
	while(num--) {
		LED_ON;
		delay_ds(1);
		LED_OFF;
		delay_ds(9);
	}
	delay_ds(25);
	LED_ON;
	delay_ds(25);
	LED_OFF;
}
void out8(uint8_t num) {
	out((num >>4) & 0xF);
	out((num) & 0xF);	
}
void out16(uint16_t num) {
	out8(num>>8);
	out8(num&0xFF);
}
