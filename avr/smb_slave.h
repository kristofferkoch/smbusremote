#ifndef _SMB_SLAVE_H
#define _SMB_SLAVE_H
#include <inttypes.h>

#define TRUE (1)
#define FALSE (0)

#define SMB_OWN_ADDRESS		42

#define SMB_TX_MAX_LENGTH       32
#define SMB_TX_BUFFER_LENGTH    (SMB_BYTE_COUNT_LENGTH + SMB_TX_MAX_LENGTH)

/*!
 *  Maximum number of data bytes received for Block write and Block write,
 *  block read process call. (Max value is 32).
 */
#define SMB_RX_MAX_LENGTH       32
//! Length of command code.
#define SMB_COMMAND_CODE_LENGTH   1
//! Length of byte count.
#define SMB_BYTE_COUNT_LENGTH     1
#define SMB_RX_BUFFER_LENGTH    (SMB_COMMAND_CODE_LENGTH + SMB_BYTE_COUNT_LENGTH + SMB_RX_MAX_LENGTH)

typedef struct smb_data
{
    uint8_t tx_length;                         //!< Transmit length.
    uint8_t tx_count;                          //!< Transmit counter.
    uint8_t rx_count;                          //!< Receive counter.
    unsigned char state:2;                          //!< SMBus driver state flag.
    unsigned char volatile error : 1;               //!< Error flag.
    unsigned char rx_buffer[SMB_RX_BUFFER_LENGTH];   //!< Receive buffer.
    unsigned char tx_buffer[SMB_TX_BUFFER_LENGTH];   //!< Transmit buffer.
} smb_data;
#define SMB_STATE_IDLE                  0x00    //!< Idle state flag.
#define SMB_STATE_READ_REQUESTED        0x01    //!< Read requested flag.
#define SMB_STATE_WRITE_REQUESTED       0x02    //!< Write requested flag.
#define SMB_STATE_WRITE_READ_REQUESTED  0x03    //!< Write, then read requested flag.

void smb_init(void);
void smb_receive_byte(smb_data *smb);
void smb_process_message(smb_data *smb);
#endif
