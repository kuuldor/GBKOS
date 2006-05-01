

#include <PalmOS.h>

#include "common.h"



Boolean _FldHandleEvent (FieldType *fldP, EventType *eventP)
{

	ftrSave *store = GetGBKOSBase();

	Boolean retv = store->FldHandleEvent(fldP,eventP);

	if (Western == store->charset || !store->on || eventP->eType != fldEnterEvent)
	{
		return retv;
	}


	if (eventP->tapCount == 2) // double click to select half a CJK char, fix it
	{

		Char * s = FldGetTextPtr(fldP);

		UInt16 start,end;
		FldGetSelection(fldP, &start, &end);
		if (end - start == 1 && isCJKChar(s+start))
		{
			end ++;
			FldSetSelection(fldP, start, end);
		}
	}
	return retv;
}

