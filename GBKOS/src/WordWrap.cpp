

#include <PalmOS.h>

#include "common.h"


UInt16 _FntWordWrap(const char *s, UInt16 maxWidth)
{
	if (maxWidth == 0) return 0;
	//int len = strlen(s);		//In most time, the string is very long and this is time-consuming

	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on ) {
		return store->FntWordWrap(s,maxWidth);
	}


	UInt16 font = GetFont(store);
	UInt16 cjkWid = getCJKCharWidth(store, font, maxWidth>160);
	//if (cjkWid >=16) cjkWid /=2;

	UInt16 wid=0;

	int i;
	for (i=0; s[i] != 0; i++) {
		if (canBeCJK1stByte(s[i])) {
			wid += cjkWid;
			if (wid > maxWidth) return i;
			i++;
		} else {
			int nextPrase = i;

			// search until next line or next CJK character.
			while (s[nextPrase] != 0)
			{
				if (canBeCJK1stByte(s[nextPrase]))
					break;
				if ((s[nextPrase] == '\r') || (s[nextPrase] == '\n'))
				{
					nextPrase ++;
					/*
					while (s[nextPrase] != 0)
					{
						if ((s[nextPrase] != '\r') && (s[nextPrase] != '\n'))
							break;
						
						nextPrase ++;
					}
					*/

					break;
				}
				nextPrase ++;
			}

			UInt16 tmpWidth = maxWidth - wid;
			UInt16 tmpLen = nextPrase - i;

			Char *pTmp = (Char*)MemPtrNew(tmpLen+1);

			StrNCopy(pTmp, s + i, tmpLen);
			pTmp[tmpLen] = 0;

			UInt16 tmpRet = store->FntWordWrap(pTmp, tmpWidth);
			wid += store->FntLineWidth(pTmp, tmpRet);
			MemPtrFree(pTmp);

			if (tmpRet ==1 && wid > maxWidth && i>0) return i;

			i += tmpRet;
			if ((tmpRet < tmpLen) || (s[i - 1] == '\r') || (s[i - 1] == '\n'))
				return i;
			i--;
		}
	}
	return i;
}

