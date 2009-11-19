#ifndef _SMBUS_H
#define _SMBUS_H
#define SMB_OWN_ADDRESS (42)

void smb_init(void);

typedef struct smb_data {
  enum {
    SMB_STATE_IDLE, SMB_STATE_WRITE_REQ,  SMB_STATE_READ_REQ,
    SMB_STATE_WRITE_READ_REQ
  } state;
  uint8_t txlen, txcount, rxcount;
  uint8_t rxbuffer[4], txbuffer[4];
} smb_data;

void smb_process(smb_data *);
void smb_process_rx_byte(smb_data *);
#endif
