#ifndef COMM_PROTOCOL_H
#define COMM_PROTOCOL_H

#include "Arduino.h"

/*
	Communication protocol defines
*/

#define HEADER_LENGTH 6 // 2 bytes preamble, 1 byte type, 2 bytes length, 1 byte checksum.

#define RES_SUCCESS 0x00
#define RES_FAIL 0x01

#define UART_REC_OK    0x00
#define UART_REC_HEADER_CHECKSUM_FAIL  0x01
#define UART_REC_PAYLOAD_CHECKSUM_FAIL  0x02
#define UART_REC_READY 0x03

 #define MAX_PAYLOAD_LENGTH 32

/* Types */
struct packet_t 
{
  uint8_t type;
  uint8_t length;
  uint8_t payload[MAX_PAYLOAD_LENGTH];
};

struct rec_session_t
{
  uint8_t checksum;
  uint16_t length;
  uint32_t payload_index;
  bool header_received;
};

/* Preamble bytes */

#define PREAMBLE_BYTE_1 0xAA
#define PREAMBLE_BYTE_2 0x55

/* Packet defines */

/* 	ok packet
	LENGTH = 0
	type = 0x00
*/

#define OK_TYPE 0x00
#define OK_LENGTH 0

/* 	fail packet
	LENGTH = 1
	type = 0x01
*/

#define FAIL_TYPE 0x01
#define FAIL_LENGTH 1

#define ERROR_CODE_NFC_INIT_FAIL 0x00

/* 	ping packet
	LENGTH = 0
	type = 0x02
*/

#define PING_TYPE 0x02
#define PING_LENGTH 0


/* 	pong packet
	LENGTH = 0
	type = 0x03
*/

#define PONG_TYPE 0x03
#define PONG_LENGTH 0

/* 	Card id not found
	LENGTH = 0
	type = 0x04
*/

#define NOTFOUND_TYPE 0x04
#define NOTFOUND_LENGTH 0

/* 	buy
	LENGTH = 5
	type = 0x05
*/

#define BUY_TYPE 0x05
#define BUY_LENGTH 5

/* 	NewBalance
	LENGTH = 2
	type = 0x06
*/

#define NEWBALANCE_TYPE 0x06
#define NEWBALANCE_LENGTH 3

/* 	InsufficientFunds
	LENGTH = 2
	type = 0x07
*/

#define INSUFFICIENTFUNDS_TYPE 0x07
#define INSUFFICIENTFUNDS_LENGTH 3


#define INVALIDCARD_TYPE 0x08
#define NEXTSHIFT_TYPE 0x09

#endif

