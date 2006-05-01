

#include <PalmOS.h>

#include "common.h"


Int16 _FntWidthToOffset (Char const *s,UInt16 len, Int16 pixelWidth,Boolean *leadingEdgeP, Int16 *truncWidthP)
{
	if (len == 0)
	{
		if (leadingEdgeP)
			*leadingEdgeP = true;

		if (truncWidthP)
			*truncWidthP = 0;

		return 0;
	}

	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on ) {
		return store->FntWidthToOffset(s, len, pixelWidth, leadingEdgeP, truncWidthP);
	}


	UInt16 font = GetFont(store);
	UInt16 cjkWid=getCJKCharWidth(store, font);
	//if (cjkWid >=16) cjkWid /=2;

	Int16 wid=0;
	Boolean onEdge = true;
	Int16 truncWid = 0;

	int i;
	for (i=0; s[i] != 0; i++) {
		if (canBeCJK1stByte(s[i]))
		{
			wid += cjkWid;
			if (wid > pixelWidth)
			{
				onEdge = false;
				truncWid = wid - cjkWid;
				break;
			}
			i++;
			if (wid == pixelWidth)
			{
				onEdge = true;
				truncWid = wid;
				i++;
				break;
			}
		} else {

			int nextPrase = i;

			// search until next line or next CJK character.
			while (s[nextPrase] != 0)
			{
				if (canBeCJK1stByte(s[nextPrase]))
					break;
				if ((s[nextPrase] == '\r') || (s[nextPrase] == '\n'))
				{
					break;
				}
				nextPrase ++;
			}

			UInt16 tmpWidth = pixelWidth - wid;
			UInt16 tmpLen = nextPrase - i;

			Char *pTmp = (Char*)MemPtrNew(tmpLen+1);

			StrNCopy(pTmp, s + i, tmpLen);
			pTmp[tmpLen] = 0;

			UInt16 tmpRet = store->FntWidthToOffset(pTmp, tmpLen, tmpWidth, &onEdge, &truncWid);
			wid += truncWid;
			MemPtrFree(pTmp);

			i += tmpRet;
			if (tmpRet < tmpLen || (s[i] == '\r') || (s[i] == '\n'))
			{
				truncWid = wid;
				break;
			}
			else if (wid == pixelWidth)
			{
				onEdge = true;
				truncWid = wid;
				break;
			}
			i--;
		}
	}

	if (leadingEdgeP)
		*leadingEdgeP = onEdge;
	if (truncWidthP)
		*truncWidthP = truncWid;
	return i;
}

