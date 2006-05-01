#include <PalmOS.h>

#include "common.h"
#include "Big5.h"
#include "Encoding.h"

static Boolean isBig5String(const char* s, UInt16 len)
{
	Boolean retv = false;

	return retv;
}

Encoding * InitBig5Table()
{
	Encoding *retv = NULL;

	retv = (Encoding *)PermanentPtrNew(sizeof(Encoding));
	ErrFatalDisplayIf(NULL == retv, "Cannot Allocate Memory for Big5 Convert Table!");
	if ((Encoding*)0xdeadbeef == retv )	// what a miracle, the pointer equals to the dumm ptr flag
	{
		void *tmp = retv;
		retv = (Encoding *)PermanentPtrNew(sizeof(Encoding));
		MemPtrFree(tmp);
	}

	MemSet(retv, sizeof(Encoding), 0);

	if (retv->Init(BIG_TABLE_NAME))
	{
		retv->isLocalString = isBig5String;
		return retv;
	}

	MemPtrFree(retv);

	return NULL;
}

void ReleaseBig5Table(Encoding *big5)
{
	big5->Destroy();
	MemPtrFree(big5);

}

