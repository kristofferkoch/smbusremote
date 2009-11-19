#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h> 
#include <inttypes.h>
#include "smb_slave.h"

smb_data smb;

void smb_init() {
	// Set own slave address
	TWAR = SMB_OWN_ADDRESS << 1;
	// Enable TWI-interface, enable ACK, enable interrupt, clear interrupt flag
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
}

ISR(TWI_vect) {
    uint8_t enable_ack = TRUE;
    
	if (smb.state == SMB_STATE_IDLE) {
		smb.tx_length = 0;
		smb.tx_count = 0;
		smb.rx_count = 0;
		smb.error = FALSE;
	}
	switch (TWSR & 0xf8) {
		case TW_SR_SLA_ACK:
			if (smb.state != SMB_STATE_IDLE) {
				smb.tx_length = 0;
				smb.tx_count = 0;
				smb.rx_count = 0;
				smb.error = FALSE;
			}
			smb.state = SMB_STATE_WRITE_REQUESTED;
			break;
		case TW_SR_DATA_ACK:
			smb.rx_buffer[smb.rx_count++] = TWDR;
			if (smb.rx_count == SMB_RX_BUFFER_LENGTH)
				enable_ack = FALSE;
			break;
		case TW_SR_DATA_NACK:
			smb.error = TRUE;
			smb.state = SMB_STATE_IDLE;
			break;
		case TW_SR_STOP:
			smb_process_message(&smb);
		break;
		case TW_ST_SLA_ACK:
			if (smb.state == SMB_STATE_IDLE) {
				smb_receive_byte(&smb);
				smb.state = SMB_STATE_READ_REQUESTED;
			}
			TWDR = smb.tx_buffer[0];
			smb.tx_count++;
		break;
		case TW_ST_DATA_ACK:
			if (smb.tx_count == smb.tx_length) {
				smb.error = TRUE;
				enable_ack = FALSE;
			} else {
				TWDR = smb.tx_buffer[smb.tx_count++];
			}
			break;
		case TW_ST_DATA_NACK:
			if (smb.tx_count != smb.tx_length)
				smb.error = TRUE;
			smb.state = SMB_STATE_IDLE;
			break;
		case TW_ST_LAST_DATA:
			smb.error = TRUE;
			smb.state = SMB_STATE_IDLE;
			break;
		case TW_BUS_ERROR:
			TWCR |= (1<<TWSTO);
			smb.error = TRUE;
			smb.state = SMB_STATE_IDLE;
			break;
		default:
		break;
	}
	if (enable_ack) {
		TWCR = (TWCR | (1<<TWEA)) & ~(1<<TWINT);
	} else {
		TWCR &= ~((1 << TWEA) | (1 << TWINT));
	}
	TWCR |= (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
}

