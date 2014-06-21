/* This header is to substitute AVR-specific commands so that these
functions can be tested on my computer before putting them on an
embedded device. We use F_CPU to tell us if we're compiling this
for linux or for AVR, because AVR always has F_CPU defined in the
makefile. */


#ifndef AVR_SUBSTITUTION_HEADER
#define AVR_SUBSTITUTION_HEADER

#ifdef F_CPU
	#include <util/atomic.h>
	#include <util/parity.h>
	
#else
	// These are the features used from util/atomic
	#define ATOMIC_BLOCK( x )
	#define ATOMIC_RESTORESTATE
#endif


#endif
