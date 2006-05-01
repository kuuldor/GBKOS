#include <PalmOS.h>

#include "common.h"
#include "Encoding.h"

int QuickFindCode(CodeTable **table, UInt16 tableSize, UInt16 code)
{

	UInt8 reg = (code & 0xFF00) >> 8;
	UInt8 pos = (code & 0x00FF);

	UInt16 top,cur;
	top = tableSize - 1;
	cur = top / 2;
	UInt16 base = 0;

	CodeTable *page = table[cur];

	while (base < cur)
	{

		if (page->region > reg)
		{
			top = cur;
		}
		else if (page->region < reg)
		{
			base = cur;
		}
		else
		{
			break;
		}

		cur = (top - base) / 2 + base;
		page = table[cur];
	}

	if (page->region != reg) return -1;

	Int16 pageDiff = 1;

	if (pos < page->pos_begin)
	{
		pageDiff = -1;
	}
	else if (pos > page->pos_end)
	{
		pageDiff = 1;
	}
	else
	{
		return cur;
	}

	while (page->region == reg)
	{
		if (pos <  page->pos_begin || pos > page->pos_end)
		{
			cur += pageDiff;
		}
		else 
		{
			return cur;
		}
		page = table[cur];
	}

	return -1;
}

UInt16 Encoding::Convert2GBK(UInt16 code)
{
	if (NULL == convert_table || 0 == tableSize)
	{
		return code;
	}

	int pageNo = QuickFindCode(convert_table, tableSize, code);

	if (-1 == pageNo)
	{
		return code;
	}


	UInt8 pos = (code & 0x00FF);

	CodeTable *page = convert_table[pageNo];

	if (pos < page->effective_begin || pos > page->effective_end)
	{
		return code;
	}

	UInt16 index = (pos - page->effective_begin) * 2;

	if (index > page->size - 2)
	{
		return code;
	}

	UInt16 retv = 0;

	retv |= page->table[index];
	retv <<= 8;
	retv |= page->table[index+1];

	return retv;
}

Boolean Encoding::Init(const char * dbName)
{
	dbRef = OpenDatabaseByName((char*)dbName, dmModeReadOnly|dmModeLeaveOpen);

	if (dbRef == NULL)
		return false;

	UInt16 resCnt = DmNumResources(dbRef);

	tableSize = resCnt;

	convert_table = (CodeTable**) PermanentPtrNew(sizeof(CodeTable*) * resCnt);

	handles = (MemHandle*)PermanentPtrNew(resCnt*sizeof(MemHandle));

	if (NULL == convert_table || NULL == handles)
	{
		DmCloseDatabase(dbRef);
		dbRef = NULL;
		return false;
	}

	MemHandle hdl;

	for (UInt16 i=0; i < resCnt; i++)
	{
		hdl = DmGetResourceIndex(dbRef, i);
		convert_table[i] = (CodeTable *) MemHandleLock(hdl);
		handles[i] = hdl;
	}

	return true;
}

void Encoding::Destroy()
{

	if (handles != NULL) {
		for (UInt16 i=0; i<tableSize; i++) {
			if (handles[i] != NULL)
				MemHandleUnlock(handles[i]);
		}
		MemPtrFree(handles);
	}
	if (convert_table != NULL) {
		MemPtrFree(convert_table);
	}
	if (dbRef != NULL) {
		DmCloseDatabase(dbRef);
	}

}

