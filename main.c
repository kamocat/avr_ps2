#ifndef F_CPU
#define F_CPU 8000000
#endif

#include "driver_int.h"
#include "tdelta.h"
#include <util/delay.h>

volatile uint8_t badisr_count = 0;
ISR( BADISR_vect ) {
	++badisr_count;
}

int main( void ) {
	uint8_t rq_step = 99; // Disable rq for now
	uint8_t rq_count = 0;

	/* Do tasks based on how much time has elapsed, so that this
	 * loop doesn't get hung up and not respond quickly enough. */
	CLKPR = 0x80;
	CLKPR = 0x00;	// set the system clock prescaler to 1
	/*
	setup_delta_timer();
	uint16_t tmain; // keeps track of timestamp for time delta measurements
	RESET_TDELTA( tmain );
	*/
	init_ps2();

	state.now = idle;
	DDRB = 0xC0;
	DDRC = 0xFF;


	uint8_t data_recieved = 0;

	for( ; ; ) { //infinite loop
		if( state.now == rq ) {
			switch( rq_step ) {
			/* The control outputs are inverted, due to the transistors
			 * used to drive the bus. */
				case 0:
				HOLD_CLOCK();
				rq_step = 1;
				if( state.bit_count != 0 ) { 
					/* If there was an error recieving the message,
					 * ask for the byte to be resent. */
					push( &tx, 0x01); //0x01 is ~0xFE
				}
				break;

				case 1:
				// Wait for data to be released
				if( R_BIT ) {
					rq_step = 2;
				}
				break;

				case 2:
				// Hold data low
				BIT_OUT( 0xFF );
				rq_step = 3;
				break;

				case 3:
				RELEASE_CLOCK();
				rq_step = 4;
				break;

				case 4:
				if( PIND & CLOCK_READ ) {
					rq_step = 5;
				}
				break;

				case 5:
				// Release data and go to tx state
				BIT_OUT( 0 );
				state.now = send;
				rq_step = 0;
				break;
				
				default:
				/* This case isn't meant to happen during normal execution, but is just for debugging. 
				 * It serves as a way to put the device in recieve-only mode, without changing harder-
				 * to-find pieces of code. */
				state.now = idle;
				++rq_count;
				break;
			}
		}


		/* Now do general data processing */
//		_delay_us(100); //Use this to simulate time taken by other functions.

		if( rx.size ) {
			get( &rx, &data_recieved );
		}
		PORTB = 0x80;	// enable green buffer
		PORTC = data_recieved;
		PORTB = 0x40;	// disable green, enable red
		PORTC = err;	//basically a recieve error count



		// TODO: If clock is too slow, force resend.




		// Process bytes recieved, determine commands to send.


	}

	return 0;
}

