#ifndef DRIVER_INTERRUPT_ROUTINE
#define DRIVER_INTERRUPT_ROUTINE

#include "driver_def.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

/* This file contains interrupt routines for the PS/2 driver.
These are not called in any function, and so they must either
be in a header file, or directly in main.c. (They can't be in
another c file)
*/


volatile enum ps2_state state; // current state of the tranciever

struct queue rx; // internally volatile
struct queue tx; // internally volatile 

volatile uint8_t ps2_byte;

volatile uint8_t default_count; // How many times has the default case occured?


void init_ps2( void ) {
	default_count = 0;
	state = idle;
	rx.head = 0;
	rx.size = 0;
	tx.head = 0;
	tx.size = 0;

	/* We need to use seperate pins because we are translating
	 * voltage levels. The PS/2 device is 5v, and the AVR uC is
	 * 3.3v. However, this also makes it easier too troubleshoot,
	 * since I can now tell who the signal is coming from. */
	DDRD &= ~0x03;	// pin 1 is clock, pin 0 is data (recieving)
	DDRD |= 0x30;	// pin 5 is clock, pin 0 is data (sending)

	/* TODO: Set up interrupts. Read/write on low-clock. */

	return;
}

/* TODO: measure timing of signal.
 * The docs say the data should be changed in the MIDDLE
 * of the pulse, not on the clock edge. Not sure how to
 * implement this right now. Maybe with rc delay?
 * 
 * Also, I can't wait too long for a number of things, like the
 * pulse timing or the request-to-send. If I don't put a timeout
 * on the pulse-timing, then I might start in the middle of a byte
 * and just not know it, continuing to wait for bits that aren't
 * going to come.
 * Request-to-send is probably going to happen not at interrupt-level,
 * but regular level, because it's slower timing and it looks for a change
 * in the data line, not the clock. Also, just like hold, it can happen at
 * any time, not just on the clock edge. (It generates a clock edge, doesn't
 * it).
 */


ISR( INT0_vect ) {
	switch( state.now ) {
		case idle:
			bit_count = 1;
			state.now = (PIND & 1) ? rq : rcv; //start bit
			break;
		case rcv:
			if( bit_count < 9 ) { // data bits
				ps2_byte = (ps2_byte << 1) | (PIND & 1);
				++state.bit_count;

			} else if(( bit_count == 9 ) //parity bit
				&& ( (PIND & 1) ==  parity_even_bit(ps2_byte) )){ 
				++state.bit_count;

			} else if( (bit_count == 10) //stop bit
				&& ( PIND & 1 )) {
				add( &rx, ps2_byte );
				state.bit_count = 0;

			} else {
				state.now = rq;
				/* There was an error.
				 * The bit_count will tell us what do do about it. */
			}
			break;

		case send:
			/* TODO: Fill out this case.
			 * Don't forget to look for the ACK. */
			break;

		default:
			++default_count;
			break;
	}
}


				
	
 

#endif