#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include "Common.h"

namespace Sexy
{

bool Quantize8Bit(const ulong32* theSrcBits, int theWidth, int theHeight, uchar* theDestColorIndices, ulong32* theDestColorTable);

}

#endif //__QUANTIZE_H__