/*
	Communication protocol defines
*/

#define RES_SUCCESS 0x00
#define RES_FAIL 0x01

 #define MAX_PAYLOAD_LENGTH 32

/* Types */
struct packet_t 
{
  uint8_t type;
  uint8_t length;
  uint8_t payload[MAX_PAYLOAD_LENGTH];
};

/* Preamble bytes */

#define PREAMBLE_BYTE_1 0xAA
#define PREAMBLE_BYTE_2 0x55

/* Packet defines */

/* 	error packet
	LENGTH = 1
	type = 0x00
*/

#define ERROR_TYPE 0x00
#define ERROR_LENGTH 0

#define ERROR_CODE_NFC_INIT_FAIL 0x00

/* 	ping packet
	LENGTH = 0
	type = 0x01
*/

#define PING_TYPE 0x01
#define PING_LENGTH 0


/* 	pong packet
	LENGTH = 0
	type = 0x02
*/

#define PONG_TYPE 0x02
#define PONG_LENGTH 0

/* 	order_req
	LENGTH = 5
	type = 0x03
*/

#define ORDER_REQ_TYPE 0x03
#define ORDER_REQ_LENGTH 5

/* 	order_ok
	LENGTH = 3
	type = 0x04
*/

#define ORDER_OK_TYPE 0x04
#define ORDER_OK_LENGTH 3

/* 	order_fail
	LENGTH = 1
	type = 0x05
*/

#define ORDER_FAIL_TYPE 0x05
#define ORDER_FAIL_LENGTH 1

/* 	buyer_name
	LENGTH = variable
	type = 0x06
*/

#define BUYER_NAME_TYPE 0x06

/* 	next_shift
	LENGTH = ?? (undefined)
	type = 0x07
*/

#define NEXT_SHIFT_TYPE 0x07