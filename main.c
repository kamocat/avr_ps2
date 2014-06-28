#ifndef F_CPU
#define F_CPU 1000000
#endif

#include "driver_int.h"
#include "tdelta.h"
#include <util/delay.h>


int main( void ) {
	uint8_t rq_step = 0;

	/* Do tasks based on how much time has elapsed, so that this
	 * loop doesn't get hung up and not respond quickly enough. */
	setup_delta_timer();
	uint16_t tmain;
	RESET_TDELTA( tmain );

	for( ; ; ) { //infinite loop
		if( state.now == rq ) {
			switch( rq_step ) {
			/* The control outputs are inverted, due to the transistors
			 * used to drive the bus. */
				case 0:
				HOLD_CLOCK();
				rq_step = 1;
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


		// TODO: If clock is too slow, force resend.




		// Process bytes recieved, determine commands to send.


	}

	return 0;
}

