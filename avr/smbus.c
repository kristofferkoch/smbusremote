#include <avr/io.h>
#include <avr/interrupt.h>
#include "smbus.h"
void smb_init() {
  TWAR = SMB_OWN_ADDRESS << 1;
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
}
smb_data smb;
ISR(TWI_vect) {
  uint8_t ack = 1;
  if (smb.state == SMB_STATE_IDLE) {
    smb.txlen = 0;
    smb.txcount = 0;
    smb.rxcount = 0;
  }
  switch(TWSR & 0xF8) {
  case 0x60: //we have acked SLA+W
    smb.txlen = 0;
    smb.txcount = 0;
    smb.rxcount = 0;
    smb.state = SMB_STATE_WRITE_REQ;
    break;
  case 0x80: //We have data for SLA+W
    smb.rxbuffer[smb.rxcount] = TWDR;
    smb.rxcount++;
    if (smb.rxcount == sizeof(smb.rxbuffer))
      ack = 0;
    break;
  case 0x88: //Data for SLA+W, but we gave NACK.
    smb.state = SMB_STATE_IDLE;
    break;
  case 0xa0: /*Stop or such*/
    smb_process(&smb);
    break;
  case 0xa8: /*SLA+R, ACKed*/
    if (smb.state == SMB_STATE_IDLE) {
      smb_process_rx_byte(&smb);
      smb.state = SMB_STATE_READ_REQ;
    }
    TWDR = smb.txbuffer[0];
    smb.txcount++;
    break;
  case 0xb8:
    if (smb.txcount == smb.txlen) {
      ack = 0;
    } else {
      TWDR = smb.txbuffer[smb.txcount];
      smb.txcount++;
    }
    break;
  case 0xc0: /*Last byte txed, NACK*/
  case 0xc8: /*Last byte txed, ACK*/
    smb.state = SMB_STATE_IDLE;
    break;
  case 0x00: /*Bus error*/
    TWCR |= (1<<TWSTO);
    smb.state = SMB_STATE_IDLE;
  default:
    smb.state = SMB_STATE_IDLE;
  }
  if(ack)
    TWCR = (TWCR | (1<<TWEA)) & ~(1<<TWINT);
  else
    TWCR = ~((1 << TWEA) | (1 << TWINT));
  
  TWCR |= (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
}
