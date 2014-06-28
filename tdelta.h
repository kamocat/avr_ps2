#ifndef TDELTA_HEADER
#define TDELTA_HEADER
/* This header is for measuring delta time since last transaction.
 * Since it may be used in interrupt scope, it is defined as macros.
 * The setup function, however, is a regular function.
 */

uint8_t setup_delta_timer( void ) {
	TCCR3B = 1;
	/* This give 1us resolution, and a 64ms max count.
	 * If we change the prescaler to 2, then we have a 8us resolution
	 * and a 512ms max count.
	 */

	 return 0; // no error possible (yet?)
}

#define CHECK_TDELTA( tprev ) ( TCNT3 - tprev )

#define RESET_TDELTA( tprev ) tprev = TCNT3


#endif
