
 
#include <PalmOS.h>

#include "Companion.hpp"
#include "Companion_res.hpp"
#include "common.h"


#define RESID Trap2

extern "C" void MyHandler(UInt16 card, LocalID app,UInt16 cmd, MemPtr cmdPBP)
{
	Err err;
	void (*old_AppSwitch)(UInt16, LocalID,UInt16, MemPtr);
	err = FtrGet(CREATOR,RESID,(UInt32*)(&old_AppSwitch));
	ErrFatalDisplayIf(err!=0||old_AppSwitch==NULL,"HACK Master deliver an EMPTY old trap address of SysUIAppSwitch!");
	
	
	UInt16 form = FrmGetActiveFormID();

	ftrSave *store = NULL;
	err = FtrGet(CREATOR,0,(UInt32*)(&store));
		
	if (err!=0||store == NULL)  {
		store = (ftrSave*) MemPtrNew(sizeof(ftrSave));
		ErrFatalDisplayIf(store==NULL,"Cannot allocate memory for HackStore!");
		MemPtrSetOwner(store, 0);

		store->on = false;
		store->card = card;
		store->app = app;
		store->form = form;
		store->remain = 0;
		store->rx = -1;
		store->ry = -1;
		FtrSet(CREATOR,0,(UInt32)store);
	}

	DmOpenRef config = OpenConfigDB(dmModeReadOnly);
	if (config == NULL) 
			return old_AppSwitch(card,app,cmd,cmdPBP);

	global g;
	GetGlobal(g);
	
	UInt16 numRec = DmNumRecords(config);
	Boolean found = false;
	for (UInt16 i=0; i<numRec && !found; i++) {
		MemHandle hdl = DmGetRecord(config, i);
		if (hdl == NULL) break;
		dbConf *item = (dbConf*)MemHandleLock(hdl);
		if (item->card==card && item->app==app) {
			found = true;
			store->card = card;
			store->app = app;
			store->form = 0;
			store->on = item->mcfix;
			if (item->conf.build != 0)	
			{
				SetConf(item->conf);
			}
			else
			{
				SetConf(g.defaultConf);
			}

		}
		MemHandleUnlock(hdl);
		DmReleaseRecord(config,i,false);
	}
	if (!found) {
		MemHandle hdl = DmGetRecord(config, numRec-1);
		if (hdl != NULL) {
		dbConf *item = (dbConf*)MemHandleLock(hdl);
		if (item->card==0 && item->app==0) {
			store->card = card;
			store->app = app;
			store->form = 0;
			store->on = item->mcfix;
			if (item->conf.build != 0)	
			{
				SetConf(item->conf);
			}
			else
			{
				SetConf(g.defaultConf);
			}
		}
		MemHandleUnlock(hdl);
		DmReleaseRecord(config,numRec-1,false);
		}
	}

	CloseConfigDB(config);
	return old_AppSwitch(card,app,cmd,cmdPBP);
}

