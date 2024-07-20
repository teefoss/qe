#ifndef PLAT_H
#define PLAT_H

#include <stdbool.h>

// TODO: stop using this
#ifdef PLATFORM_WINDOWS
int __builtin_ctz(int mask);
#endif

#define EXTRACT_FIELD(value, mask)  (((value) & (mask)) >> __builtin_ctz(mask))

char * ApplicationDirectory(void);
bool ApplicationDirectoryExists(void);
void CreateApplicationDirectory(void);

#endif /* PLAT_H */