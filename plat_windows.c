#include "plat.h"

int __builtin_ctz(int mask)
{
	int count = 0;
	int num_bits = sizeof(mask) * 8;
	
	for ( int i = 0; i < num_bits; i++ ) {
		if ( ((1 << i) & mask) == 0 ) {
			count++;
		} else {
			break;
		}
	}
	
	return count;
}