#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "port.h"
#include "ir.h"
#include "util.h"

struct {
	enum {
		IDLE, FIRST, SYNC, RECORD, RECORD2
	} state;
	int is_rising:1;
	uint16_t now, last, delta;
	uint8_t stuffed;
	uint8_t stuff[16];
} s;


/*These definitions are reversed for "clarity" */
#define RISING  s.is_rising=1;TCCR1B &= ~(1<<ICES1)
#define FALLING s.is_rising=0;TCCR1B |= (1<<ICES1)

static void disable_timeout(void) {
	TIMSK1 &= ~(1<<OCIE1A);
	TIFR1 |= (1<<OCF1A);
}
static void set_timeout(uint16_t to) {
	OCR1A   = s.now + to;
	TIMSK1 |= (1<<OCIE1A);
	TIFR1  |= (1<<OCF1A);
}
static void abort(void) {
	disable_timeout();
	RISING;
	s.state = IDLE;
	s.stuffed = 0;
}

#define SIZE_Q (4)

struct {
  ir_event q[SIZE_Q];
  uint8_t w,r,s;
} evt_q;

void ir_init() {
	TCCR1A = 0;
	TCCR1B = (1<<ICNC1) | (1<<CS10); //Noise canceler, falling edge. clk/1
	TIMSK1 = (1<<ICIE1);
	evt_q.w=0;
	evt_q.r=0;
	evt_q.s=0;
	abort();
}
static void stuff_n_bits(uint8_t n, uint8_t bit) {
  uint8_t *byte, mask;
  if (s.stuffed + n >= sizeof(s.stuff)*8)
    return; //Buffer full
  byte = &(s.stuff[s.stuffed >> 3]);
  mask = 1<<(s.stuffed & 7);
  while(n--) {
    if (bit)
      *byte |= mask; 
    else
      *byte &= ~mask;
    s.stuffed++;
    if ((s.stuffed & 7) == 0) {
      mask = 1;
      byte++;
    } else {
      mask <<= 1;
    }
  }
}

int8_t ir_getcode(ir_event *evt) {
  if (evt_q.s == 0) return -1;
  
  evt->code = evt_q.q[evt_q.r].code;
  evt->repititions = evt_q.q[evt_q.r].repititions;

  evt_q.r++;
  if (evt_q.r >= SIZE_Q)
    evt_q.r = 0;
  evt_q.s--;
  return 1;
}


static void decode(void) {
	uint8_t data[4] = {0,0,0,0};
	uint8_t pair, i, byte, bitn, length=0;

	if ((s.stuffed != 75 && s.stuffed != 76) ||
		s.stuff[0] != 0x6a ||
		s.stuff[1] != 0x63)
			return;
	if (s.stuffed == 75)
		stuff_n_bits(1,1);
	for(i = 12;i < s.stuffed-1;i+=2) {
		byte = i>>3;
		bitn = i & 7;
		pair = (s.stuff[byte] >> bitn) & 3;
		byte = length >> 3;
		bitn = 7-(length & 7);
		switch(pair) {
			case 1: //01 0
				data[byte] &= ~(1<<bitn);
				break;
			case 0: //00 Invalid
			case 3: //11 Invalid
				return;
			case 2: //10 1
				data[byte] |= (1<<bitn);
				break;
		}
		length++;
	}
	if (data[0] != 0x80 || data[1] !=0x0F ||
		(data[2] & 0x7F) != 4) return;

	if (evt_q.s>0) {
	  i = evt_q.w==0?SIZE_Q-1:evt_q.w;
	  if(evt_q.q[i].code == data[3]) {
	    evt_q.q[i].repititions++;
	    return;
	  }
	}
	if (evt_q.s < SIZE_Q) {
	  evt_q.q[evt_q.w].code = data[3];
	  evt_q.q[evt_q.w].repititions = 1;
	  evt_q.w++;
	  if(evt_q.w  >= SIZE_Q)
	    evt_q.w=0;
	  evt_q.s++;
	}
}
ISR(TIMER1_CAPT_vect) {
	s.now = ICR1;
	s.delta = s.now - s.last;
	switch(s.state) {
		case IDLE:
			s.state = FIRST;
			set_timeout(FIRST_MAX);
			FALLING;
			break;
		case FIRST:
			if (s.delta > FIRST_MIN) {
				s.state = SYNC;
				set_timeout(2*T + T/2);
				RISING;
				//LED_DDR &= ~(1<<LED_N); LED_ON;
			} else {
				//Sync pulse too short
				abort();
			}
			break;
		case SYNC:
			if (s.delta > T - T/2) {
				set_timeout(4*T + T/2);
				s.state = RECORD;
				FALLING;
				//out16(s.delta);
			} else {
				abort();
			}
			break;
		case RECORD:
		case RECORD2: {
			uint8_t n;
			if		(s.delta < T/2)			{abort(); break;}
			else if	(s.delta < T + T/2)		n = 1;
			else if (s.delta < 2*T + T/2)	n = 2;
			else if (s.state == RECORD && s.delta < 3*T + T/2)
				{n = 3; s.state = RECORD;}
			//else if (s.delta < 4*T + T/2)	n = 4;
			else							{abort(); break;}
			set_timeout(4*T + T/2);
			stuff_n_bits(n, s.is_rising);
			if (s.is_rising) {FALLING;/*LED_OFF;*/}
			else {RISING;/*LED_ON;*/}
			//if(n==2) {LED_DDR &= ~(1<<LED_N); LED_ON;}
			//else	 LED_DDR |= (1<<LED_N);
			break;}
	}
	s.last = s.now;
}
ISR(TIMER1_COMPA_vect) {
  decode();	
  abort();
  //LED_OFF;
}

