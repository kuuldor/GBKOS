

#include <PalmOS.h>

#include "common.h"


void _FntCharsInWidth(const char *s,Int16 *widP, Int16 *lenP,Boolean *fitP)
{
	if (*lenP <=0) return;

	ftrSave *store = GetGBKOSBase();

	if (!store->on || Western == store->charset ) {
		return store->FntCharsInWidth(s,widP, lenP,fitP);
	}

	UInt16 font = GetFont(store);
	UInt16 cjkWid=getCJKCharWidth(store, font, *widP > 160);
	//if (cjkWid >=16) cjkWid /=2;

	Int16 wid=0;
	Int16 len=strlen(s);
	if (len> *lenP) len = *lenP;
	*fitP = false;

	for (int i=0; i<len; i++) {
		if (canBeCJK1stByte(*(s+i))) {
			wid += cjkWid;
			if (wid>*widP) {
				*widP = wid - cjkWid;
				*lenP = i;
				return;
			}
			i++;
		} else {
			int first = i;
			i += FindNextCJK(s+i+1, len-i-1);
			Int16 tmpLen = i+1-first;
			Int16 tmpWid = *widP - wid;
			Boolean tmpFit = false;
			store->FntCharsInWidth(s+first, &tmpWid, &tmpLen, &tmpFit);
			if (!tmpFit) {
				*widP = tmpWid + wid;
				*lenP = tmpLen + first;
				return;
			}
			wid += tmpWid;
		}

	}
	*lenP = len;
	*widP = wid;
	*fitP = true;
}

