#ifndef F_CPU
#define F_CPU 8000000
#endif

#include "driver_int.h"
#include "tdelta.h"
#include <util/delay.h>

volatile badisr_count = 0;
ISR( BADISR_vect ) {
	++badisr_count;
}

int main( void ) {
	uint8_t rq_step = 0;

	/* Do tasks based on how much time has elapsed, so that this
	 * loop doesn't get hung up and not respond quickly enough. */
	CLKPR = 0x80;
	CLKPR = 0x00;	// set the system clock prescaler to 1
	setup_delta_timer();
	init_ps2();
	uint16_t tmain;
	RESET_TDELTA( tmain );

	state.now = hold;	// This will force the interrupt to run the default case, which counts the number of occurances.
	DDRD |= CLOCK_READ;
	DDRB = 0xC0;
	DDRC = 0xFF;

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
					push( &tx, ~0xFE );
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
				break;
			}
		}


		/* Now do general data processing */
		_delay_ms(100); //Use this to simulate time taken by other functions.
		PIND = CLOCK_READ;	//According to the datasheet, this will toggle the output.
		PORTB = 0x80;	// enable green
		PORTC = default_count;	// The running count of how many time the default case has been called in the ISR
		PORTB = 0x40;
		PORTC = badisr_count;



		// TODO: If clock is too slow, force resend.




		// Process bytes recieved, determine commands to send.


	}

	return 0;
}

