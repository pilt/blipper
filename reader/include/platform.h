/* Pinouts */ 
#define SCK 13
#define MOSI 11
#define SS 10
#define MISO 12

/* NFC reader */
static byte reader_command[3] = {0x4a, 0x01, 0x00};
#define READER_COMMAND_LENGTH 3

#define READER_TIMEOUT 1000
