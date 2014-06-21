#include "driver.h"
#include <stdio.h>



int main( int argc, char **argv) {
	struct queue a;
	a.head = 0;
	a.size = 0;

	uint8_t num;


	uint8_t arr[10] = { 0, 15, 22, 39, 104, 233, 100, 34, 98, 207 };


	for( int i = 0; i < 10; ++i ) {
		num = arr[i];
		printf("The parity of 0x%x is %d.\n", num, parity_even_bit(num));
	}

	return 0;
}
