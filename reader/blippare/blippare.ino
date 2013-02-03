/* NFC reader library */
#include "PN532.h"

/* Communication protocol */
#include "comm_protocol.h"

/* Platform */
#include "platform.h"

/* Set to 1 for debugging */
#define DEBUG 0

/* Create NFC object */
PN532 nfc(SCK, MISO, MOSI, SS);

uint8_t uart_rx_buf[32];
uint8_t uart_rx_buf_wr_pos;


void setup(void) {
  /* Consctruct error message */
  packet_t packet;
  
  //packet.type = ERROR_TYPE;
  //packet.length = ERROR_LENGTH;
  //packet.payload[0] = ERROR_CODE_NFC_INIT_FAIL;

  /* Initialize serial object */
  Serial.begin(9600);

  /* Initialize NFC object */
  nfc.begin();
  
  /* Check correct response from reader */
  uint32_t versiondata = nfc.getFirmwareVersion();
  
  if (!versiondata) 
  {
    /* Didn't get response from NFC board, something is wrong!*/
    //send_packet(&packet);
    while (1); /* Stop here */
  }

  /* configure board to read RFID tags and cards */
  nfc.SAMConfig();
}


void loop(void) 
{
  uint32_t card_id;
  uint8_t order_type;
  uint8_t response;
  packet_t rx_packet;
  rec_session_t session;
  uint8_t timeout = 0;
  uint8_t i;
  bool packet_received = false;
  
  order_type = read_buttons();
  
  /* If a card is scanned, send order*/
  card_id = get_card_id();
  
  if (card_id && order_type)
  {
     send_order(card_id, order_type);
     delay(1000); // This delay is for testing, makes sure that only one order are sent every 1 second.
     if (DEBUG)
     {
       Serial.println("Scanned card, sending order!");
       Serial.println(card_id,HEX);
     }
  }
  
  if (Serial.available())
  {
    reset_session(&session);
    response = uart_rec(&rx_packet, &session);
    packet_received = true;
  }
  else
  {
    packet_received = false;
  }
  
  if (DEBUG)
  {
    if (packet_received)
    {
      Serial.print("Uart receive response (hex): "); Serial.println(response, HEX);
    }
  }
  
  /* Determine what to do... */
  if (response == UART_REC_HEADER_CHECKSUM_FAIL)
  {
    /* Print corresponding error message! */
    if (DEBUG)
      Serial.println("Ooops! Something went wrong, grab your stuff and move along..");
  }
  else if (response == UART_REC_PAYLOAD_CHECKSUM_FAIL)
  {
    /* Print corresponding error message! */
    if (DEBUG)
      Serial.println("Ooops! Something went wrong, grab your stuff and move along..");
  }
  else if (response == UART_REC_READY)
  {
    /* Packet was ok */
    if (DEBUG)
      Serial.println("Got packet!");
    
    handle_server_response(response, &rx_packet);
  }

    
  if (response != UART_REC_OK || response == UART_REC_READY)
  {
     reset_session(&session);
  }

}

uint8_t read_buttons(void)
{
  /* Read buttons and return type
     Order type > 0 indicates valid order
     Should return 0x00 if no buttons are pressed!
     Preferrably, reset button status after reading them.
   */
  return 0x01;
}

/* 
 * Read card and return card id
 * if no card is scanned, returns card id 0.
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
      Serial.print("Card number: "); Serial.println(id,HEX);
    }
    
    
  }
  return id;
}

void handle_server_response(uint8_t response, packet_t * packet)
{
  uint16_t balance;
  packet_t tx_packet;
  uint16_t i = 0;
  
  /* Print correct error messages and so forth */
  if (packet->type == NEWBALANCE_TYPE)
  {
    /* Calculate new balance */
    balance = (uint16_t)packet->payload[0];
    balance <<= 8;
    balance |= (0x00FF & (uint16_t)packet->payload[1]);
    
    /* Print order ok at display */
    if (DEBUG)
    {
      Serial.print("Order ok, your new balance is: ");
      Serial.print(balance,  DEC);
      Serial.println(" SEK.");
    }
  }
  else if (packet->type == NEXTSHIFT_TYPE)
  {
    /* A baljan worker purchased coffee! Print next shift as ASCII string*/
    while(i < packet->length)
    {
      if (DEBUG)
        Serial.print(packet->payload[i]);
      i++;
    }
    if (DEBUG)
      Serial.println("");
  }
  else if (packet->type == INSUFFICIENTFUNDS_TYPE)
  {
    /* Print order insufficient funds at display */
    balance = (uint16_t)packet->payload[0];
    balance <<= 8;
    balance |= (0x00FF & (uint16_t)packet->payload[1]);
    
    /* Print insufficient funds at display */
    if (DEBUG)
    {
      Serial.print("Insufficient funds, your balance is: ");
      Serial.print(balance,  DEC);
      Serial.println(" SEK.");
    }
  }
  else if (packet->type == INVALIDCARD_TYPE)
  {
    /* Print card id not found */
    if (DEBUG)
    {
      Serial.print("Order failed: card not found or not LiU card!");
    }
  }
  else if (packet->type == PING_TYPE)
  {
    /* Got ping, respond with pong */
    if (DEBUG)
    {
      Serial.print("Got ping, responding with pong.");
    }
    tx_packet.type = PONG_TYPE;
    tx_packet.length = PONG_LENGTH;
    send_packet(&tx_packet);
  }
  else
  {
    /* Print unknown server command at display */
    if (DEBUG)
      Serial.println("Server response not recognized.");
  }
}

uint8_t send_order(uint32_t card_id, uint8_t order_type)
{
  packet_t tx_packet;
  uint8_t timeout;
  uint8_t response;
  
  /* Construct packet */
  tx_packet.type = BUY_TYPE;
  tx_packet.length = BUY_LENGTH;
  tx_packet.payload[0] = order_type;
  tx_packet.payload[1] = (uint8_t)(card_id >> 24);
  tx_packet.payload[2] = (uint8_t)(card_id >> 16);
  tx_packet.payload[3] = (uint8_t)(card_id >> 8);
  tx_packet.payload[4] = (uint8_t)(card_id >> 0);

  
  /* Send packet */
  send_packet(&tx_packet);
}

/*
 * Parse uart rx buffer.
 */
 
uint8_t uart_rec(packet_t * pack, rec_session_t * session )
{
  uint8_t rec_byte;
  uint8_t rx_cnt;
  uint8_t i = 0;
  uint8_t type;
  uint8_t response;
  
  uint8_t  * payload_ptr;
  
  /* Set response default to OK */
  response = UART_REC_OK;
  
  /* Check if there is any received data */
  rx_cnt = Serial.available();
  
  if (DEBUG)
  {
    if (rx_cnt)
    {
      Serial.print("Received bytes: "); Serial.println(rx_cnt, DEC);
    }  
  }
  
  /*   We got the entire header,
   *   check if the header is correct.
   */
   if (rx_cnt >= HEADER_LENGTH && !(session->header_received))
   {
     /* Initalize checksum */
     session->checksum = 0;
     
     rec_byte = receive_byte(); 
     session->checksum += rec_byte;
     
     if (rec_byte == PREAMBLE_BYTE_1)
     {
       if (DEBUG)
         Serial.println("Got preamble byte 1.");
         
      rec_byte = receive_byte();
      session->checksum += rec_byte;
      
      if (rec_byte == PREAMBLE_BYTE_2)
      {
        if (DEBUG)
         Serial.println("Got preamble byte 2.");
         
        /* We got preamble, read out type and length */
        type = receive_byte();
        session->checksum += type;
        
        rec_byte = receive_byte();
        session->checksum += rec_byte;
        session->length = (0x00FF & (uint16_t)rec_byte);
        session->length <<= 8;
               
        rec_byte = receive_byte();
        session->checksum += rec_byte;
        session->length |= (0x00FF & (uint16_t)rec_byte);
        
        /* Check if checksum is correct */
        rec_byte = receive_byte();
        if (session->checksum == rec_byte)
        {
          if (DEBUG)
            Serial.println("Got correct header checksum.");
            
          /* Checksum was correct, set flag to indicate we got entire header. Also reset checksum */
          session->header_received = true;
          session->checksum = 0;
          
          pack->length = session->length;
          pack->type = type;
        }
        else
        {
          if (DEBUG)
          {
            Serial.println("Invalid header checksum.");
            Serial.print("Expected "); Serial.println(session->checksum,HEX);
            Serial.print("Got: "); Serial.println(rec_byte, HEX);
          }
          
          
          if (DEBUG)
            Serial.println("Flushing receive buffer.");
            
          /* Wait for rest of packet to arrive, then flush buffer */
          delay(1000);
          flush_buffer();
          
          response = UART_REC_HEADER_CHECKSUM_FAIL;
        }
      }
      else
      {
        if (DEBUG)
        {
          Serial.println("Invalid preamble byte 2.");
          Serial.print("Expected: "); Serial.println("55");
          Serial.print("Got: "); Serial.println(rec_byte, HEX);
        }
      }
    }
    else
    {
      if (DEBUG)
      {
        Serial.println("Invalid preamble byte 1.");
        Serial.print("Expected: "); Serial.println("AA");
        Serial.print("Got: "); Serial.println(rec_byte, HEX);
      }
    }
   }

   /* Header was received, read out payload. */
   if (session->header_received && session->length > 0)
   {
     if (DEBUG)
     {
       Serial.println("Header received, parsing payload.");
     }
     
      while (Serial.available() && session->length > 0)
      {
        rec_byte = receive_byte();
        session->checksum += rec_byte;
        pack->payload[session->payload_index] = rec_byte;
        session->payload_index++;
        session->length--;
      }
   }
   
   /* Header and payload received, check checksum. */
   if (session->header_received && session->length == 0)
   {
     if (Serial.available())
     {
       rec_byte = receive_byte();
       if (session->checksum == rec_byte)
       {
         if (DEBUG)
         {
           Serial.println("Got correct body checksum.");
         }
         response = UART_REC_READY;
       }
       else
       {
         if (DEBUG)
         {
           Serial.println("Invalid body checksum.");
           Serial.print("Got: "); Serial.println(rec_byte, HEX);
           Serial.print("Expected: "); Serial.println(session->checksum, HEX);
         }
         response = UART_REC_PAYLOAD_CHECKSUM_FAIL;
       }
       session->checksum = 0;
     }
   }
   
   return response;
}

void reset_session(rec_session_t * session)
{
  session->header_received = false;
  session->checksum = 0;
  session->length = 0;
  session->payload_index = 0;
}

/*
 * Packetize and send packet.
 */
uint8_t  send_packet (packet_t * pack)
{
  uint8_t tx_byte;
  uint8_t checksum = 0;
  uint8_t i;
  
  uint8_t * payload_ptr;
  
  /* Do initial check */
  if (pack->length > MAX_PAYLOAD_LENGTH)
    return RES_FAIL;
  
  /* Send preamble bytes */
  send_byte(PREAMBLE_BYTE_1);
  checksum += PREAMBLE_BYTE_1;
  if (DEBUG)
  {
    Serial.print("Sent preamble byte 1: "); Serial.println(PREAMBLE_BYTE_1,HEX);
  }
  
  send_byte(PREAMBLE_BYTE_2);
  checksum += PREAMBLE_BYTE_2;
  if (DEBUG)
  {
    Serial.print("Sent preamble byte 2: "); Serial.println(PREAMBLE_BYTE_2,HEX);
  }
 
  /* Send packet type */
  tx_byte = pack->type;
  send_byte(tx_byte);
  checksum += tx_byte;
  if (DEBUG)
  {
    Serial.print("Sent type: "); Serial.println(tx_byte,HEX);
  }
  /* Send packet length 1*/
  tx_byte = (uint8_t)(pack->length >> 8);
  send_byte(tx_byte);
  checksum += tx_byte;
  if (DEBUG)
  {
    Serial.print("Sent length 1 : "); Serial.println(tx_byte,DEC);
  }
    
    
  /* Send packet length 2*/
  tx_byte = (uint8_t)((pack->length & 0x00FF) & 0x00FF);
  send_byte(tx_byte);
  checksum += tx_byte;
  if (DEBUG)
  {
    Serial.print("Sent length 2 : "); Serial.println(tx_byte,DEC);
  }
 
  /* Send checksum 1 */
  send_byte(checksum);
  if (DEBUG)
  {
    Serial.print("Sent checksum 1: "); Serial.println(checksum,HEX);
  }
 
  /* Reset checksum for use in payload loop */
  checksum = 0;
 
  /* Send the payload and compute checksum */
  payload_ptr = &(pack->payload[0]);
  
  for (i=0; i< pack->length; i++)
  {
    if (DEBUG)
    {
      Serial.print("Sent payload "); Serial.print(i,DEC); Serial.print(": "); Serial.println(*payload_ptr,HEX);
    }
    
    checksum += *payload_ptr;
    send_byte(*payload_ptr++);
    
  }
 
  /* Send checksum 2 */
  send_byte(checksum);
  if (DEBUG)
  {
    Serial.print("Sent checksum 2: "); Serial.println(checksum,HEX);
  }
 
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

/*
 * Receive byte
 */
uint8_t receive_byte(void)
{
  uint8_t rec_byte;
  
  rec_byte = Serial.read();
  
  if (DEBUG)
  {
    Serial.print("Received byte: "); Serial.println(rec_byte, HEX);
  }
  
  return rec_byte;
}

/* Called when header checksum fails.*/
void flush_buffer(void)
{
  while (Serial.available())
    Serial.read();
}
