

#include <PalmOS.h>

#include "common.h"


Int16 _FntLineWidth(const char *s, UInt16 len)
{
	if (len <=0) return 0;

	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on || len <2) {
		return store->FntLineWidth(s,len);
	}

	UInt16 font = GetFont(store);
	UInt16 cjkWid=getCJKCharWidth(store, font);
	//if (cjkWid >=16) cjkWid /=2;

	UInt16 wid=0;

	for (UInt16 i=0; i<len; i++) {
		if (canBeCJK1stByte(*(s+i))) {
			wid += cjkWid;
			i++;
		} else {
			int first = i;
			i += FindNextCJK(s+i+1, len-i-1) ;
			wid += store->FntLineWidth(s+first, i+1-first);
		}

	}
	return wid;
}

