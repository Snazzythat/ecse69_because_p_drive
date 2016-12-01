////////////////////////////////////////////////////////////////////////////////
//	File Name				: transceiver_protocol.h
//	Description			: program header 
//	Author					: Team 12
//	Date						: Nov 29th, 2016
////////////////////////////////////////////////////////////////////////////////

#ifndef _TRANSCEIVER_PROTOCOL_H
#define _TRANSCEIVER_PROTOCOL_H

//	Exported Functions
static void transmit_ack(void);
static void receive_ack(void);
static int transmit_comfirm(uint8_t);
static int receive_comfirm(uint8_t);
void byte_transmit(uint8_t);
void burst_transmit(uint8_t*, int);
uint8_t byte_receive(void);
void burst_receive(uint8_t *, int);

#endif	/*_TRANSCEIVER_PROTOCOL_H*/
