#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#else
#include <endian.h>
#endif // __APPLE__

#endif // _ENDIAN_H_
