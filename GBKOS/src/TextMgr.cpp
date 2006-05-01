

#include <PalmOS.h>

#include "common.h"


static UInt8 _TxtByteAttr (UInt8 inByte)
{
	UInt8 retv = 0;
	if (canBeCJK1stByte(inByte))
	{
		retv |= byteAttrFirst;
	}
	if (canBeCJK2ndByte(inByte))
	{
		retv |= byteAttrLast;
	}
	if (0 == retv)
	{
		retv = byteAttrSingle;
	}

	return retv;
}

static UInt16 _TxtCharAttr (WChar inChar)
{
	if ( (0xFF00&inChar) != 0)
	{
		return charAttrGraph;
	}
	ftrSave *store = GetGBKOSBase();
	return ( (UInt16 (*)(WChar)) store->old_funcTable[intlTxtCharAttr] )(inChar);
}

static UInt16 _TxtCharSize (WChar inChar)
{
	if ( (0xFF00&inChar) != 0)
	{
		return 2;
	}
	return 1;
}

static UInt16 _TxtGetPreviousChar (const Char* inText, UInt32 inOffset, WChar* outChar)
{
	if (0 == inOffset) return 0;
	UInt16 retv, outOffset, i;

	UInt16 isCJK = 0;

	for (i = 0; i < inOffset; i++)
	{
		if (isCJKChar(inText+i))
		{
			isCJK ++;
			i++;
		}
		else
		{
			isCJK = 0;
		}
	}

	if (i == inOffset) // char counting stop before inOffset
	{
		if (isCJK > 0)	// before is CJK
		{
			outOffset = inOffset - 2;
			retv = 2;
		}
		else 	// before is latin1
		{
			outOffset = inOffset - 1;
			retv = 1;
		}
	}
	else // char counting stop on inOffset, it must be a 2nd Byte of CJK char.
	{
		if (isCJK == 1) // itself is the only CJK char
		{
			outOffset = inOffset - 2;
			retv = 1;
		}
		else if (isCJK > 1) // before is CJK
		{
			outOffset = inOffset - 3;
			retv = 2;
		}
		else // strange condition, should never go into this, only in case of i'm a fool
		{
			outOffset = inOffset - 1;
			retv = 1;
		}
	}

	if (NULL != outChar)
	{
		*outChar = inText[outOffset];
		if (retv == 2)
		{
			*outChar = (*outChar<<8) | inText[outOffset+1];
		}
	}

	return retv;
}

static UInt16 _TxtGetNextChar (const Char* inText, UInt32 inOffset, WChar* outChar)
{
	UInt16 retv;

	if (isCJKChar(inText+inOffset))
		retv = 2;
	else
		retv = 1;

	if (NULL != outChar)
	{
		*outChar = inText[inOffset];
		if (retv == 2)
		{
			*outChar = (*outChar<<8) | inText[inOffset+1];
		}
	}

	return retv;
}

static WChar _TxtGetChar (const Char* inText, UInt32 inOffset)
{
	WChar retv;
	retv = inText[inOffset];

	if (isCJKChar(inText+inOffset))
	{
		retv = (retv<<8) | inText[inOffset+1];
	}

	return retv;
}

static UInt16 _TxtSetNextChar (Char* ioText, UInt32 inOffset, WChar inChar)
{
	UInt16 retv;
	UInt8 b1, b2;
	b1 = (inChar & 0xFF00) >> 8;
	b2 = inChar &0xFF;

	if (b1 == 0)
	{
		ioText[inOffset] = b2;
		retv = 1;
	}
	else
	{
		ioText[inOffset] = b1;
		ioText[inOffset+1] = b2;
		retv = 2;
	}
	return retv;
}

static WChar _TxtCharBounds (const Char* inText, UInt32 inOffset, UInt32* outStart, UInt32* outEnd)
{
	WChar retv;
	UInt16 i;
	UInt32 start, end;

	Boolean isCJK = false;

	for (i = 0; i <= inOffset; i++)
	{
		if (isCJKChar(inText+i))
		{
			isCJK = true;
			i++;
		}
		else
		{
			isCJK = false;
		}
	}

	if (i == inOffset + 1) // char counting stop on inOffset
	{
		if (isCJK)
		{
			start = inOffset - 1;
			end = inOffset + 1;
			retv = (unsigned char)inText[start]<<8 | (unsigned char)inText[end];
		}
		else 	// is latin1
		{
			ftrSave *store = GetGBKOSBase();
			retv = (( WChar (*) (const Char*, UInt32, UInt32*, UInt32*))
			        store->old_funcTable[intlTxtCharBounds])(inText,inOffset,&start, &end);
		}
	}
	else 	// char counting stop after inOffset, it must be a CJK Char
	{
		start = inOffset;
		end = inOffset + 2;
		retv = (unsigned char)inText[start]<<8 | (unsigned char)inText[end];
	}

	if (NULL != outStart)
		*outStart = start;
	if (NULL != outEnd)
		*outEnd = end;

	return retv;
}

static Boolean _TxtWordBounds (const Char* inText,UInt32 inLength, UInt32 inOffset,UInt32* outStart, UInt32* outEnd)
{
	Boolean retv;
	UInt16 i;
	UInt32 start, end;

	Boolean isCJK = false;

	for (i = 0; i <= inOffset; i++)
	{
		if (isCJKChar(inText+i))
		{
			isCJK = true;
			i++;
		}
		else
		{
			isCJK = false;
		}
	}

	if (i == inOffset + 1) // char counting stop on inOffset
	{
		if (isCJK)
		{
			start = inOffset - 1;
			end = inOffset + 1;
			retv = true;
		}
		else 	// is latin1
		{
			ftrSave *store = GetGBKOSBase();
			retv = (( Boolean (*) (const Char*,UInt32, UInt32, UInt32*, UInt32*))
			        store->old_funcTable[intlTxtWordBounds])(inText,inLength,inOffset,&start, &end);
		}
	}
	else 	// char counting stop after inOffset, it must be a CJK Char
	{
		start = inOffset;
		end = inOffset + 2;
		retv = true;
	}

	if (NULL != outStart)
		*outStart = start;
	if (NULL != outEnd)
		*outEnd = end;

	return retv;
}

static Boolean _TxtCharIsValid (WChar inChar)
{
	if ( (0xFF00&inChar) != 0)
	{
		return isCJKChar(inChar);
	}
	ftrSave *store = GetGBKOSBase();
	return ( (Boolean (*)(WChar)) store->old_funcTable[intlTxtCharIsValid] )(inChar);
}


Int16 _TxtCharWidth (WChar inChar)
{
	ftrSave *store = GetGBKOSBase();
	if ( (0xFF00&inChar) != 0 && isCJKChar(inChar))
	{
		UInt16 font = GetFont(store);
		return getCJKCharWidth(store, font);
	}
	return ( (Boolean (*)(WChar)) store->old_funcTable[intlTxtCharWidth] )(inChar);
}

static Int16 _TxtCaselessCompare(const Char* s1, UInt16 s1Len,
                                 UInt16* s1MatchLen, const Char* s2, UInt16 s2Len,
                                 UInt16* s2MatchLen);

static Int16 _TxtCompare(const Char* s1, UInt16 s1Len,
                         UInt16* s1MatchLen, const Char* s2, UInt16 s2Len,
                         UInt16* s2MatchLen)
{
	// Check for err
	ErrNonFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	Int16 result = 0;

	// caseless compare at first
	result = _TxtCaselessCompare(s1, s1Len, s1MatchLen, s2, s2Len, s2MatchLen);
	if (0 != result)
		return result;

	// Get pointer to translation table
	const UInt8 *tableP = _GetCharSortValue();

	UInt16 i = 0;

	while (true)
	{
		if ((i >= s1Len) || (i >= s2Len))
		{
			result = s1Len - s2Len;
			break;
		}

		result = (unsigned char)tableP[(unsigned char)(s1[i])] - (unsigned char)tableP[(unsigned char)(s2[i])];
		if (0 != result)
			break;

		if ((0 == s1[i]) || (0 == s2[i]))
			break;

		i ++;
	}

	if (NULL != s1MatchLen)
		*s1MatchLen = i;
	if (NULL != s2MatchLen)
		*s2MatchLen = i;

	return result;
}

static Int16 _TxtCaselessCompare(const Char* s1, UInt16 s1Len,
                                 UInt16* s1MatchLen, const Char* s2, UInt16 s2Len,
                                 UInt16* s2MatchLen)
{
	// Check for err
	ErrNonFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	Int16 result = 0;

	// Get pointer to translation table
	const UInt8 *tableP = _GetCharCaselessValue();

	UInt16 i = 0;

	while (true)
	{
		if ((i >= s1Len) || (i >= s2Len))
		{
			result = s1Len - s2Len;
			break;
		}

		result = (unsigned char)tableP[(unsigned char)(s1[i])] - (unsigned char)tableP[(unsigned char)(s2[i])];
		if (0 != result)
			break;

		if ((0 == s1[i]) || (0 == s2[i]))
			break;

		i ++;
	}

	if (NULL != s1MatchLen)
		*s1MatchLen = i;
	if (NULL != s2MatchLen)
		*s2MatchLen = i;

	return result;
}

static void *_IntlGetRoutineAddress (IntlSelector inSelector)
{
	ftrSave *store = GetGBKOSBase();
	return store->funcTable[inSelector];
}


void setupDispatchTable(ftrSave *store)
{

	if (store->isJap) return;

	UInt16 i;
	for (i = 0; i <= 32; i++)
	{
		store->funcTable[i] = store->old_funcTable[i] = IntlGetRoutineAddress(i);
	}

	/*
	   UInt32 romVersion;
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);		

	if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageDevelopment,0) 
		&& romVersion < sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0) ) 
	{
		Err err;
		err = IntlSetRoutineAddress(intlTxtByteAttr, (void*)_TxtByteAttr);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtByteAttr");
		err = IntlSetRoutineAddress(intlTxtCharAttr, (void*)_TxtCharAttr);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtCharAttr");
		err = IntlSetRoutineAddress(intlTxtCharSize, (void*)_TxtCharSize);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtCharSize");
		err = IntlSetRoutineAddress(intlTxtGetPreviousChar, (void*)_TxtGetPreviousChar);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtGetPreviousChar");
		err = IntlSetRoutineAddress(intlTxtGetNextChar, (void*)_TxtGetNextChar);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtGetNextChar");
		err = IntlSetRoutineAddress(intlTxtGetChar, (void*)_TxtGetChar);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtGetChar");
		err = IntlSetRoutineAddress(intlTxtSetNextChar, (void*)_TxtSetNextChar);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtSetNextChar");
		err = IntlSetRoutineAddress(intlTxtCharBounds, (void*)_TxtCharBounds);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtCharBounds");
		err = IntlSetRoutineAddress(intlTxtWordBounds, (void*)_TxtWordBounds);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtWordBounds");
		err = IntlSetRoutineAddress(intlTxtCharIsValid, (void*)_TxtCharIsValid);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtCharIsValid");
		err = IntlSetRoutineAddress(intlTxtCharWidth, (void*)_TxtCharWidth);
		ErrFatalDisplayIf(err!=0, "Cannot set IntlRoutine _TxtCharWidth");
		err = IntlSetRoutineAddress(intlTxtCompare, (void *)_TxtCompare);
		ErrFatalDisplayIf(err != 0, "Can not set IntlRoutine _TxtCompare");
		err = IntlSetRoutineAddress(intlTxtCaselessCompare, (void *)_TxtCaselessCompare);
		ErrFatalDisplayIf(err != 0, "Can not set IntlRoutine _TxtCaselessCompare");
	} 
	else if (romVersion >= sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0) )
	{*/
	store->funcTable[intlTxtByteAttr] = (void*)_TxtByteAttr;
	store->funcTable[intlTxtCharAttr] = (void*)_TxtCharAttr;
	store->funcTable[intlTxtCharSize] = (void*)_TxtCharSize;
	store->funcTable[intlTxtGetPreviousChar] = (void*)_TxtGetPreviousChar;
	store->funcTable[intlTxtGetNextChar] = (void*)_TxtGetNextChar;
	store->funcTable[intlTxtGetChar] = (void*)_TxtGetChar;
	store->funcTable[intlTxtSetNextChar] = (void*)_TxtSetNextChar;
	store->funcTable[intlTxtCharBounds] = (void*)_TxtCharBounds;
	store->funcTable[intlTxtWordBounds] = (void*)_TxtWordBounds;
	store->funcTable[intlTxtCharIsValid] = (void*)_TxtCharIsValid;
	store->funcTable[intlTxtCharWidth] = (void*)_TxtCharWidth;
	store->funcTable[intlTxtCompare] = (void *)_TxtCompare;
	store->funcTable[intlTxtCaselessCompare] = (void *)_TxtCaselessCompare;
	store->funcTable[intlIntlGetRoutineAddress] = (void*)_IntlGetRoutineAddress;

	store->IntlDispatch =
	    (void (*) (void))GetAndSetTrapAddress(sysTrapIntlDispatch, (void*)_IntlDispatch);
	//}
}

void cleanDispatchTable(ftrSave *store)
{
	if (store->isJap) return;

	/*
	UInt32 romVersion;
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);		

	if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageDevelopment,0) 
	&& romVersion < sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0) ) 
	{
	UInt16 i;
	void *tprocP;
	for (i = 0; i <= 32; i++) 
	{
	tprocP = IntlGetRoutineAddress(i);
	if (store->old_funcTable[i] != tprocP)  // routine patched ,restore it
	{
	Err err = IntlSetRoutineAddress(intlTxtCharIsValid, store->old_funcTable[i]);
	ErrFatalDisplayIf(err!=0, "Cannot restore IntlRoutine");
	}
	}

	}
	else if (romVersion >= sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0) )
	{*/
	GetAndSetTrapAddress(sysTrapIntlDispatch, (void*)store->IntlDispatch);
	//}
}

void **getFuncTable()
{
	ftrSave *store asm("%a2");
	store = GetGBKOSBase();
	if (store->on)
		return store->funcTable;
	else
		return store->old_funcTable;
}


/*
void _IntlDispatch(void)
{
	register UInt16 sel asm("%d2");
	//asm ("move.w reg_d2, %0" : "=r" (sel) );
	register ftrSave *store asm("%a2");
	store = GetGBKOSBase();
	((void(*)())store->funcTable[sel] )();
}*/

