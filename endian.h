#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#elif defined(__sgi)
#include <inttypes.h>
uint32_t be32toh( uint32_t x ) { return x; }
uint16_t be16toh( uint32_t x ) { return x; }
#else
#include <endian.h>
#endif

#endif // _ENDIAN_H_
