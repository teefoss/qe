#ifndef PLAT_H
#define PLAT_H

#ifdef PLATFORM_WINDOWS
int __builtin_ctz(int mask);
#endif

#define EXTRACT_FIELD(value, mask)  (((value) & (mask)) >> __builtin_ctz(mask))
	
#endif /* PLAT_H */