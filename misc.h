//
//  misc.h
//  qe
//
//  Created by Thomas Foster on 8/1/24.
//

#ifndef misc_h
#define misc_h

#include <stdlib.h>

void DieGracefully(const char * message, ...);
void * Allocate(size_t bytes);
void * Reallocate(void * ptr, size_t bytes);

#endif /* misc_h */
