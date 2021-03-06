#ifndef DRIVER_INTERRUPT_ROUTINE
#define DRIVER_INTERRUPT_ROUTINE

#include "driver_def.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "tdelta.h"

/* This file contains interrupt routines for the PS/2 driver.
These are not called in any function, and so they must either
be in a header file, or directly in main.c. (They can't be in
another c file)
*/

volatile uint8_t err = 0;

volatile struct ps2_state state; // current state of the tranciever

struct queue rx; // internally volatile
struct queue tx; // internally volatile 

volatile uint8_t ps2_byte;

volatile uint8_t default_count; // How many times has the default case occured?


/* Define bitmasks for clock and data read/control.
 * All four pins are on PORTD */
#define CLOCK_CONTROL 0x40
#define CLOCK_READ 0x02
#define DATA_CONTROL 0x01
#define DATA_READ 0x80

void init_ps2( void ) {
	default_count = 0;
	state.now = idle;
	state.bit_count = 0;
	rx.head = 0;
	rx.size = 0;
	tx.head = 0;
	tx.size = 0;

	/* We need to use seperate pins because we are translating
	 * voltage levels. The PS/2 device is 5v, and the AVR uC is
	 * 3.3v. However, this also makes it easier too troubleshoot,
	 * since I can now tell who the signal is coming from.
	 * It's important to note here that the control bits are inverted, due
	 * to how it is wired (The bus is open-collector, meaning it has a pull-up
	 * resistor to 5v, and so we pull it low using a transistor)
	 */
	DDRD = (DDRD & ~(CLOCK_READ | DATA_READ)) | CLOCK_CONTROL | DATA_CONTROL;

	/* Set up interrupts. Read/write on low-clock. */
	EICRA = 0x08;	// falling edge for INT1
	EIMSK = 0x02;	// enable INT1
	sei();	// enable global interrupts

	return;
}

/* Define macros for the send_bit and recieve_bit */
#define R_BIT (PIND & DATA_READ )
#define BIT_OUT(d) PORTD = (PORTD & ~DATA_CONTROL) | ( d & DATA_CONTROL)

/* Define macros for controlling the clock */
#define HOLD_CLOCK() PORTD |= CLOCK_CONTROL
#define RELEASE_CLOCK() PORTD &= ~CLOCK_CONTROL

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


ISR( INT1_vect ) {
	switch( state.now ) {
		case idle:
			state.bit_count = 1;
			state.now = R_BIT ? rq : rcv; //start bit should be 0
			ps2_byte = 0;
			break;
		case rcv:
			if( state.bit_count < 9 ) { // data bits, little-endian
				ps2_byte = (ps2_byte >> 1) | R_BIT;
				++state.bit_count;

			} else if(( state.bit_count == 9 ) //parity bit
				&& ( (R_BIT >> 7) !=  parity_even_bit(ps2_byte) )){
				++state.bit_count;

			} else if( (state.bit_count == 10) //stop bit
				&& R_BIT ) {
				add( &rx, ps2_byte );
				state.bit_count = 0;
				state.now = idle;

			} else {
				//state.now = rq;
				err = state.bit_count;
				++state.bit_count;
				if( state.bit_count > 10 ) {
					state.now = idle;
				}
				/* There was an error.
				 * The bit_count will tell us what do do about it. 
				 * (if bit_count is zero, then that means we recieved
				 * the whole frame successfully, so the request-to-send
				 * must actually be to send a command to the device, not
				 * just ask it to repeat the last byte it sent. )
				 */
			}
			break;

		case send:
			/* Because the output is inverted (due to the level shifting),
			 * ps2_byte should be inverted before sending. This is a faster
			 * operation than inverting each individual bit, and can also
			 * happen outside of interrupt scope.
			 */
			if( state.bit_count == 0 ) { // start bit
				BIT_OUT( 1 );
				get( &tx, &ps2_byte );
			} else if( state.bit_count < 9 ) { // data bits
				BIT_OUT( ps2_byte >> (state.bit_count - 1));
			} else if( state.bit_count == 9 ) { //parity bit
				BIT_OUT( parity_even_bit( ps2_byte ) ? 0x00 : 0xFF );
			} else if( state.bit_count == 10 ) { //stop bit
				BIT_OUT( 0 );
			} else if( state.bit_count == 11 ) { //ACK
				/* Bit recieved should be low. If it's not,
				 * then we need to send the byte again. */
				if( R_BIT ) {
				 	push( &tx, ps2_byte );
				}
				/* If there's more data, send it. If not, wait. */
				state.now = tx.size ? rq : idle;
				state.bit_count = -1;	//this will roll over at increment
			} else {
				// This state should not happen.
			}

			++state.bit_count;

			break;

		default:
			++default_count;
			/* This is just a diagnostic tool, to see if the clock is
			 * going when it's not in send or recieve state (which means
			 * it should probably be in the send or recieve state) 
			 */
			break;
	}
}


				
	
 

#endif
