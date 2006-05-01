
#include <PalmOS.h>


#include "common.h"


Int16 _FntCharWidth(Char ch)
{
	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on || !canBeCJK1stByte(ch)) {
		return store->FntCharWidth(ch);
	}

	UInt16 font = GetFont(store);
	UInt16 cjkWid=getCJKCharWidth(store, font);
	return cjkWid/2;

}

Int16 _FntCharsWidth(const char *s,Int16 len)
{
	if (len <=0) return 0;

	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on ) {
		return store->FntCharsWidth(s,len);
	}


	UInt16 font = GetFont(store);
	Int16 cjkWid=getCJKCharWidth(store, font);
	//if (cjkWid >=16) cjkWid /=2;


	Int16 wid=0;

	for (int i=0; i<len; i++) {
		if (canBeCJK1stByte(s[i])) {
			wid += cjkWid;
			i++;
		} else {
			int first = i;
			i += FindNextCJK(s+i+1, len-i-1) ;
			wid += store->FntCharsWidth(s+first, i+1-first);
		}

	}

	return wid;
}

