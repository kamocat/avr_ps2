#ifndef DRIVER_DATA_DEFINITIONS
#define DRIVER_DATA_DEFINITIONS

#include <stdint.h>
#include "avr_sub.h"

/* There are four states for the ps2 host:
Idle, recieving, request-to-send, and send.
There is another layer that analyzes the data, but that doesn't happen at 
interrupt level. Data is transferred via two fixed-capacity queues; one for 
send, and one for recieve.
*/

enum ps2_trancieve { idle, rcv, rq, send, hold };

struct ps2_state {
	enum ps2_trancieve now; //What is the current state?
	uint8_t bit_count; 	// Where are we in the dataframe?
};

#define QUEUE_CAP 8
#define QUEUE_MASK 0x07

// This is statically allocated because we have such a small heap
struct queue {
	volatile uint8_t head;
	volatile uint8_t size;
	volatile uint8_t data[QUEUE_CAP];
};



uint8_t add( struct queue * q, uint8_t byte) {
	
	uint8_t r_val = 0;
	if( q->size >= QUEUE_CAP ) {
		r_val = 1;
	} else {
		ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
			q->data[(q->head + q->size) & QUEUE_MASK] = byte;
			++q->size;
		}
	}
	return r_val;
}


uint8_t get( struct queue * q, uint8_t * dest) {

	uint8_t r_val = 0;
	if( !dest ) {
		r_val = 1;
	} else if( q->size == 0 ) {
		r_val = 2;
	} else if( q->size > QUEUE_CAP ) {
		// This catches if size went below zero and rolled over
		r_val = 4;
	} else {
		ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
			*dest = q->data[q->head];
			q->head = (q->head + 1) & QUEUE_MASK;
			--q->size;
		}
	}
	return r_val;
}

	
uint8_t push( struct queue * q, uint8_t byte) {

	uint8_t r_val = 0;
	if( q->size >= QUEUE_CAP ) {
		r_val = 1;
	} else {
		ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
			q->head = (q->head - 1) & QUEUE_MASK;
			q->data[q->head] = byte;
			++q->size;
		}
	}
	return r_val;
}




/* The PS/2 protocol includes parity in each frame.
I wrote this lovely funciton, but it turns out that
avr-libc already has one written for me. */
#ifndef parity_even_bit
uint8_t parity_even_bit( uint8_t byte ) {
	uint8_t par = 0;
	for( int i = 0; i < 8; ++i ) {
		if( byte & 1 ) {
			par = !par;
		}
		byte = byte>>1;
	}
	return par;
}
#endif


#endif
