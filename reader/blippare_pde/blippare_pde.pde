/* NFC reader library */
#include <PN532.h>

/* Communication protocol */
#include <comm_protocol.h>

/* Platform */
#include <platform.h>

/* Set to 1 for debugging */
#define DEBUG 1

/* Create NFC object */
PN532 nfc(SCK, MISO, MOSI, SS);



void setup(void) {
  /* Declare error message */
  uint8_t error_code_payload[1] = {ERROR_CODE_NFC_INIT_FAIL};
  
  /* Initialize serial object */
  Serial.begin(9600);

  /* Initialize NFC object */
  nfc.begin();
  
  /* Check correct response from reader */
  uint32_t versiondata = nfc.getFirmwareVersion();
  
  if (!versiondata) 
  {
    /* Didn't get response from NFC board, something is wrong!*/
    send_packet(ERROR_TYPE, &error_code_payload[0], 1);
    while (1); // halt
  }

  /* configure board to read RFID tags and cards */
  nfc.SAMConfig();
}


void loop(void) 
{
  get_card_id();

}

/* 
 * Read card and return card id
 */
 
uint32_t get_card_id(void)
{
  uint32_t id = 0;
  uint8_t response;
  uint8_t spi_rx_buf[22];
  uint8_t i;
  
  response = nfc.sendCommandCheckAck(reader_command, READER_COMMAND_LENGTH, READER_TIMEOUT);
  
  nfc.readspidata(&spi_rx_buf[0],22);
  
  if(response)
  {
    for (i=0; i < spi_rx_buf[12] - 1; i++)
    {
      id |= spi_rx_buf[13+i];
      id <<= 8;
    }
    id |= spi_rx_buf[13];
    
    if (DEBUG)
    {
      Serial.println("Card detected!");
      Serial.print("Card number: "); Serial.println(id,DEC);
    }

  }
  
}

/*
 * Packetize payload and type.
 */
uint8_t  send_packet (uint8_t type, uint8_t * payload, uint8_t length)
{
  uint8_t checksum = 0;
  uint8_t i;
  
  /* Do initial check */
  if (length > MAX_PAYLOAD_LENGTH)
    return RES_FAIL;
  
  /* Send preamble bytes */
  send_byte(PREAMBLE_BYTE_1);
  checksum += PREAMBLE_BYTE_1;
  if (DEBUG)
    Serial.print("Sent preamble byte 1: "); Serial.println(PREAMBLE_BYTE_1,HEX);
  
  send_byte(PREAMBLE_BYTE_2);
  checksum += PREAMBLE_BYTE_2;
  if (DEBUG)
    Serial.print("Sent preamble byte 2: "); Serial.println(PREAMBLE_BYTE_2,HEX);
 
  /* Send packet type */
  send_byte(type);
  checksum += type;
  if (DEBUG)
    Serial.print("Sent type: "); Serial.println(type,HEX);
 
  /* Send packet length */
  send_byte(length);
  checksum += length;
  if (DEBUG)
    Serial.print("Sent length: "); Serial.println(length,DEC);
 
  /* Send checksum 1 */
  send_byte(checksum);
  if (DEBUG)
    Serial.print("Sent checksum 1: "); Serial.println(checksum,HEX);
 
  /* Reset checksum for use in payload loop */
  checksum = 0;
 
  /* Send the payload and compute checksum */
  for (i=0; i<length; i++)
  {
    checksum += *payload;
    send_byte(*payload++);
    if (DEBUG)
      Serial.print("Sent payload "); Serial.print(i,DEC); Serial.print(": "); Serial.println(*payload,HEX);
  }
 
  /* Send checksum 2 */
  send_byte(checksum);
  if (DEBUG)
    Serial.print("Sent checksum 2: "); Serial.println(checksum,HEX);
 
  /* Return ok result*/
  return RES_SUCCESS;
}

/*
 * Handle transmission of individual bytes
 */
uint8_t send_byte (uint8_t data)
{
 /* Wait for previous data to be sent */
 Serial.flush();
 
 /* Send byte */
 Serial.write(data);
 
 return RES_SUCCESS;
}
