#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "main.h"
#include "util.h"
#include "codes.h"
#include "ir.h"
#include "smbus.h"
#include "port.h"

static void init(void) {
	LED_OFF;
	LED_DDR |= (1<<LED_N);	
	ir_init();
	smb_init();

	TCCR0A = (1<<COM0A1) | (1<<COM0B1) | (1<<WGM01) | (1<<WGM00); 
	TCCR0B = (1<<CS00);
	DDRD = (1<<PD5) | (1<<PD6);
	sei();
}

int main(void) {
	init();
	for(;;) {
		/*while((key=ir_getcode())==-1);
		if (key & KEY_DOWN) LED_ON;
		else if (key & KEY_UP) LED_OFF;*/
	}
}


static void return_ir_code(smb_data *smb) {
  ir_event evt;
  if (ir_getcode(&evt) > 0) {
    smb->txbuffer[0] = evt.repititions;
    smb->txbuffer[1] = evt.code;
  } else {
    smb->txbuffer[0] = 0;
    smb->txbuffer[1] = 0xff;
  }

  smb->txlen = 2;
  LED_TOOGLE;
  smb->state = SMB_STATE_WRITE_READ_REQ;
}
static void set_leds(smb_data *smb) {
  if (smb->rxcount != 3) {
    smb->state = SMB_STATE_IDLE;
    return;
  }
  OCR0A = smb->rxbuffer[1];
  OCR0B = smb->rxbuffer[2];
  smb->state = SMB_STATE_IDLE;
}
static void undefined_command(smb_data *smb) {
  smb->state = SMB_STATE_IDLE;
}
void smb_process_rx_byte(smb_data *smb) {
  smb->txbuffer[0] = 0;
  smb->txlen = 1;
}
void smb_process(smb_data *smb) {
  if (smb->state != SMB_STATE_WRITE_REQ) {
    smb->state = SMB_STATE_IDLE;
    return;
  }
  LED_TOOGLE;
  switch(smb->rxbuffer[0]) {
  case 42:
    LED_TOOGLE;
    return_ir_code(smb);
    break;
  case 11:
    set_leds(smb);
    break;
  default:
    undefined_command(smb);
  }
}

