////////////////////////////////////////////////////////////////////////////////
//	File Name				: transceiver_protocol.c
//	Description			: program entry 
//	Author					: Team 12
//	Date						: Nov 29th, 2016
////////////////////////////////////////////////////////////////////////////////

// Includes
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "transceiver_protocol.h"
#include "cc2500.h"

/**
 * Send an ACK from one board to another
 * An ACK is simply a packet of 1 byte: 0xFF
 *
 * @param void
 *
 * @return void
 */
static void transmit_ack(void)
{
	CC2500_flush_tx();
	CC2500_write_tx_one(0xFF);
}


/**
 * Comfirm that an ACK has been received
 *
 * @param uint8_t hex which is the ACK packet
 * received
 *
 * @return int 1 for confirmation, 0 for fail
 */
static int transmit_comfirm(uint8_t hex)
{
	if (hex == 0xFF)
		return 1;
	return 0;
}

/////////////////////////////////////////////////SENDING

// Single byte transmission
void byte_transmit(uint8_t data)
{
	CC2500_flush_tx();
	CC2500_write_tx_one(data);
}

// Burst byte transmission
void burst_transmit(uint8_t *data, int len)
{
	uint8_t state;
	uint8_t num_of_bytes;
	
	CC2500_flush_tx();
	CC2500_read_one(CC2500_SIDLE);
	
	state = CC2500_get_state();
	
	
	if (state == 1)
	{
		
		CC2500_write_tx(data, len);
		state = CC2500_get_state();
		printf("SFM Mouse before read one :\t\t%d\n", state);
		num_of_bytes = CC2500_get_txbytes();
		printf("\nBytes in  :\t\t%d\n", num_of_bytes);
		
		for(int i=0; i<len;i++)
	  {
		 printf("Packet sent %d : %x\n", i, data[i]);
	  }
		CC2500_read_one(CC2500_STX);
		state = CC2500_get_state();
		printf("SFM Mouse after read one :\t\t%d\n", state);
		
	}
}

/////////////////////////////////////////////////RECEIVING

// Single byte receiving
uint8_t byte_receive(void)
{
	uint8_t data;
	
	CC2500_read_one(CC2500_SRX);
	data = CC2500_read_rx_one();
	return data;
}

// Burst byte receiving
void burst_receive(uint8_t *data, int len)
{
	CC2500_read_one(CC2500_SRX);
	CC2500_Read(data, 0x3f|0xC0, len);
}
