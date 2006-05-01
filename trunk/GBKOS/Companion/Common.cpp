#include <PalmOS.h>
#include "common.h"
#include "Companion.hpp"

DmOpenRef OpenConfigDB(UInt16 mode, Boolean create)
{
	LocalID dbID = DmFindDatabase(0,DBNAME);
	if (dbID == 0) {
	  if (create)
	  {
		DmCreateDatabase(0,DBNAME,CREATOR,DBTYPE,false);
		dbID = DmFindDatabase(0,DBNAME);
	  }
		if (dbID == 0) 
			return NULL;
	}

	DmOpenRef config = DmOpenDatabase(0, dbID, mode);
	if (config == 0) {
		Err errno  = DmGetLastErr();
		char msg[256];
		StrPrintF(msg,"DM opendatabase Error code is %04X",errno);
		ErrNonFatalDisplay(msg);
		return NULL;
	}
	return config;
}

void CloseConfigDB(DmOpenRef db)
{
	if(db!=NULL) 
		DmCloseDatabase(db);
}

void EraseConfigDB()
{
	LocalID dbID = 0;
	UInt16 cardNo;
	for (cardNo = 0; cardNo < MemNumCards(); cardNo++)
	{
		dbID = DmFindDatabase(0,DBNAME);
		if (dbID != 0) 
		  break;
	}

	if (dbID == 0) {
		return;
	}

	DmDeleteDatabase(cardNo, dbID);

	return;
}


void SetConf(const ftrGBK &conf) 
{
  UInt16 libRef = 0;
  Err error = SysLibFind ("GBKOS Lib", &libRef);
  if (error) return;
  error = GBKOS_LibOpen (libRef);
  if (error) return;
  //error = GBKOS_Init(libRef);
  //if (error) return;

  ftrGBK curCon;

  GBKOS_GetConfig(libRef, &curCon);
  
  curCon.charset = conf.charset;
  curCon.smart = conf.smart;
  
  GBKOS_SetConfig(libRef, &curCon);

  GBKOS_LibClose(libRef);
}


void GetGlobal(global &g)
{		

	PrefGetAppPreferencesV10(CREATOR,1, (void*)&g,sizeof(global));
}

void SetGlobal(global &g)
{		
	PrefSetAppPreferencesV10(CREATOR,1, (void*)&g,sizeof(global));
}

void SetAppConf()
{
	global g;
	GetGlobal(g);

	DmOpenRef config = OpenConfigDB();
	ErrFatalDisplayIf(config == NULL,"Fail to open or create config database!");

	UInt16 numRec = DmNumRecords(config);
	if (g.appIndex >= 0 && g.appIndex<(Int16)numRec) {
		MemHandle hdl = DmGetRecord(config, g.appIndex);
		dbConf *dbItem = (dbConf*)MemHandleLock(hdl);
		DmWrite(dbItem,0,&g.appConf, sizeof(dbConf));
		MemHandleUnlock(hdl);
		DmReleaseRecord(config,g.appIndex,false);
	}
	CloseConfigDB(config);	
	SetGlobal(g);
}

Boolean GetAppConf(Int16 index)
{
	Boolean retv = false;
	
	global g;
	GetGlobal(g);

	DmOpenRef config = OpenConfigDB();
	ErrFatalDisplayIf(config == NULL,"Fail to open or create config database!");
	UInt16 numRec = DmNumRecords(config);

	if (index != noListSelection && index >= 0 && index < (Int16)numRec) {
		g.appIndex = index;
		MemHandle hdl = DmGetRecord(config, index);
		dbConf *dbItem = (dbConf*)MemHandleLock(hdl);
		g.appConf = *dbItem;
		MemHandleUnlock(hdl);
		DmReleaseRecord(config,index,false);
				
		retv = true;	
	}

	if (g.appConf.conf.build == 0)
	{
		g.appConf.conf = g.defaultConf;
	}

	CloseConfigDB(config);	
	SetGlobal(g);
	return retv;
}

