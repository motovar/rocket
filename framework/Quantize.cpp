#include "Quantize.h"
#include <assert.h>
#include <math.h>
#include <string.h>

using namespace Sexy;

bool Sexy::Quantize8Bit(const ulong32* theSrcBits, int theWidth, int theHeight, uchar* theDestColorIndices, ulong32* theDestColorTable)
{
	int aSize = theWidth*theHeight;

	int aColorTableSize = 0;
		
	ulong32 aSearchTable[256];
	uchar aTranslationTable[256]; // From search table to color table

	if (aSize > 0)
	{
		aSearchTable[0] = theSrcBits[0];
		theDestColorTable[0] = theSrcBits[0];
		aTranslationTable[0] = 0;
		theDestColorIndices[0] = 0;
		aColorTableSize++;
	}

	for (int anIdx = 1; anIdx < aSize; anIdx++)
	{
		ulong32 aColor = theSrcBits[anIdx];		

		int aLeftPos = 0;
		int aRightPos = aColorTableSize-1;
		int aMiddlePos = (aLeftPos+aRightPos)/2;

		for (;;)
		{	
			ulong32 aCheckColor = aSearchTable[aMiddlePos];
			
			if (aColor < aCheckColor)
				aRightPos = aMiddlePos - 1;
			else if (aColor > aCheckColor)
				aLeftPos = aMiddlePos + 1;			
			else
			{
				theDestColorIndices[anIdx] = aTranslationTable[aMiddlePos];
				break;
			}

			if (aLeftPos > aRightPos)
			{
				if (aColorTableSize >= 256)
					return false;

				int anInsertPos = aLeftPos;
				if ((anInsertPos < aColorTableSize) && (aColor > aSearchTable[anInsertPos]))
					anInsertPos++;

				// Insert color into the table
				memmove(aSearchTable+anInsertPos+1, aSearchTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(ulong32));
				aSearchTable[anInsertPos] = aColor;

				memmove(aTranslationTable+anInsertPos+1, aTranslationTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(uchar));
				aTranslationTable[anInsertPos] = aColorTableSize;

				theDestColorTable[aColorTableSize] = aColor;

				theDestColorIndices[anIdx] = aColorTableSize;

				aColorTableSize++;

				break;
			}

			aMiddlePos = (aLeftPos+aRightPos)/2;
		}
	}

	return true;
}

