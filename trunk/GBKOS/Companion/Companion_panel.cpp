/*
 * Companion_panelMain.c
 *
 * main file for Companion_panel
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#include <PalmOS.h>

#include "Companion.hpp"
#include "Companion_panel.hpp"
#include "Companion_panelRsc.h"

#include "common.h"


static Err get_icon_name(
	dbConf &item)
{
    Err err;
    UInt16 attr;
    UInt32 type;
    DmOpenRef db;
    MemHandle h;
    MemPtr p;

	global g;
	GetGlobal(g);
	
	item.conf.charset = g.defaultConf.charset;
	item.conf.smart = g.defaultConf.smart;
	
	if (item.app == 0) {
		StrNCopy(item.name, "Other...", dmDBNameLength - 1);
		return errNone;
	}
				
    err = DmDatabaseInfo(item.card, item.app, item.name, &attr,
	    NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &item.id);
    if (err == errNone
     && (attr & dmHdrAttrResDB) != 0
     && type == APP_DB_TYPE) {
	db = DmOpenDatabase(item.card, item.app, dmModeReadOnly);
	if (db != NULL) {
	    h = DmGet1Resource('tAIN', 1000);
	    if (h != NULL) {
		p = MemHandleLock(h);
		if (p != NULL) {
		    StrNCopy(item.name, (char*)p, dmDBNameLength - 1);
		    item.name[dmDBNameLength - 1] = '\0';
		    MemHandleUnlock(h);
		}
		DmReleaseResource(h);
	    }
	    DmCloseDatabase(db);
	}
    }
    return err;
}

static void find_and_sync_item(confArray &array, dbConf&item)
{
	if (array.ar)
		for (UInt16 i=0; i<array.cnt; i++) {
			if (array.ar[i].id == item.id) {
				item = array.ar[i];
				//StrNCopy(item.name,array.ar[i].name,dmDBNameLength-1);
				break;
			}
		}
}

static void put_db_info(DmOpenRef db, dbConf &item)
{
	
    //if (get_icon_name(item) == errNone) {
    	UInt16 index;
    	index = dmMaxRecordIndex;
		MemHandle hdl = DmNewRecord(db,&index,sizeof(dbConf));
		if (hdl ==0) {
			return;
		}
		dbConf *ip = (dbConf*)MemHandleLock(hdl);
		DmWrite(ip, 0, &item, sizeof(item));
		MemHandleUnlock(hdl);
		DmReleaseRecord(db, index, true);
	//}
}

static Int16 CompareConf (void *rec1, void *rec2,Int16 other, 
		SortRecordInfoPtr rec1SortInfo, 
		SortRecordInfoPtr rec2SortInfo, 
		MemHandle appInfoH)
{
	return StrNCompare(((dbConf*)rec1)->name,((dbConf*)rec2)->name,dmDBNameLength);
}

static void clear_db_list(DmOpenRef db)
{
	global g;
	
	MemSet(&g, sizeof(g), 0);
	
	GetGlobal(g);
	
	g.appIndex = -1;

	MemSet(&g.appConf, sizeof(dbConf), 0);
	
	SetGlobal(g);
		
	UInt16 numRec = DmNumRecords(db);
	for (UInt16 i=0; i<numRec; i++) {
		DmRemoveRecord(db,0);
	}

	g.libRef = NULL;
	
	UInt16 libRef = 0;
   	Err error = SysLibFind ("GBKOS Lib", &libRef);
	if (error) return ;
	error = GBKOS_LibOpen (libRef);
	if (error) return ;
	//error = GBKOS_Init(libRef);
	//if (error) return ;

	g.libRef = libRef;

	GBKOS_GetConfig(g.libRef, &g.defaultConf);
	
}

static void gen_db_list(DmOpenRef db)
{
    Boolean new_search;
    DmSearchStateType state;
    int cnt;
	dbConf item;
	UInt16 i;
    
    global g;
    GetGlobal(g);
    
    cnt = 0;

	MemSet(&item, sizeof(item), 0);
   
    item.card=item.app=item.id=0;
    item.conf.charset=Western;
    item.conf.smart=item.mcfix=false;
    
    for (new_search = true;
		 DmGetNextDatabaseByTypeCreator(new_search, &state,
		APP_DB_TYPE, 0, false, &item.card, &item.app) == errNone && item.app;
		 new_search = false) {
		++cnt;
    }

	UInt16 numRec = DmNumRecords(db);
	
	confArray oldConf;
	oldConf.cnt=0;oldConf.ar=NULL;
	
    if (cnt != 0 && numRec >0) {
    	oldConf.ar = (dbConf*)MemPtrNew(sizeof(dbConf)*numRec);
	 	if (oldConf.ar != NULL) {
	 		oldConf.cnt = numRec;
			for (i=0; i<numRec; i++) {
				MemHandle hdl = DmGetRecord(db, i);
				if (hdl == NULL) break;
				dbConf *dbItem = (dbConf*)MemHandleLock(hdl);
				oldConf.ar[i]=*dbItem;
				//StrNCopy(oldConf.ar[i].name,dbItem->name,dmDBNameLength-1);
				MemHandleUnlock(hdl);
				DmReleaseRecord(db,i,false);
			}
		}

    }
	
	if (cnt == 0) 
		return;

	for (i=0; i<numRec; i++) {
		DmRemoveRecord(db,0);
	}

	
    for (new_search = true;
		 cnt>0 && DmGetNextDatabaseByTypeCreator(new_search, &state,
			APP_DB_TYPE, 0, false,  &item.card, &item.app) == errNone && item.app;
		 new_search = false) {
		 	if (get_icon_name(item) == errNone) {
		    	find_and_sync_item(oldConf, item);
				put_db_info(db, item);
			}
    }
    DmQuickSort(db, CompareConf,0);
 
    item.card=item.app=item.id=0;
    item.conf = g.defaultConf;
    item.mcfix=false;
 	get_icon_name(item);
   	find_and_sync_item(oldConf, item);
	put_db_info(db, item);
    
    
    if (oldConf.ar != NULL) {
    	MemPtrFree(oldConf.ar);
    }
    
}

static char** SetAppList(DmOpenRef db)
{
	UInt16 numRec = DmNumRecords(db);
	Int32 size = numRec*sizeof(char*)+numRec*dmDBNameLength;
 	char **retv = (char**)MemPtrNew(size);
 	if (retv != NULL) {
 		MemSet(retv,size,0);
 		char *p = (char*)&retv[numRec];
 		for (UInt16 i=0;i<numRec; i++) {
 			retv[i]= p+i*dmDBNameLength;
 		}

		if (numRec > 0) {
			UInt16 i;
			for (i=0; i<numRec; i++) {
				MemHandle hdl = DmGetRecord(db, i);
				if (hdl == NULL) break;
				dbConf *item = (dbConf*)MemHandleLock(hdl);
				StrNCopy(retv[i], item->name, dmDBNameLength-1);
				retv[i][sizeof(item->name)-1]='\0';	
				MemHandleUnlock(hdl);
				DmReleaseRecord(db,i,false);
			}

			ListType* appList = GetObjectPtr<ListType>(SetupPanelAppListList);
			ErrFatalDisplayIf(appList==NULL,"applist is null");
			LstSetListChoices(appList,retv,i);
			LstSetSelection(appList, 0);
			ControlType *ctrlPtr = GetObjectPtr<ControlType>(SetupPanelAppPopPopTrigger);
			CtlSetLabel(ctrlPtr, LstGetSelectionText(appList,0));
		}

 	}
 	
	return retv;
}

static char** SetCharsetList(UInt16 libRef, DmOpenRef db)
{
	char **listArray = (char**)MemPtrNew(sizeof(char*)*5);
	Int16 cnt = 5;
	GBKOS_GetCharsets(libRef, listArray, &cnt);

	ListType* lstPtr = GetObjectPtr<ListType>(SetupPanelCharsetListList);
	ErrFatalDisplayIf(lstPtr==NULL,"charset list is null");

	LstSetListChoices(lstPtr, listArray, cnt);
	LstSetHeight(lstPtr, cnt>5?5:cnt);    
	LstSetSelection(lstPtr, 0);
	ControlType *ctrlPtr = GetObjectPtr<ControlType>(SetupPanelCharSetPopTrigger);
	CtlSetLabel(ctrlPtr, LstGetSelectionText(lstPtr,0));
	
	return listArray;
}	

static void SyncConf2Panel()
{
	global g;
	GetGlobal(g);

	ControlType *ctrlPtr;

	ctrlPtr = GetObjectPtr<ControlType>(SetupPanelSmartCheckbox);
	CtlSetValue(ctrlPtr, g.appConf.conf.smart);
	ctrlPtr = GetObjectPtr<ControlType>(SetupPanelMCFixCheckbox);
	CtlSetValue(ctrlPtr, g.appConf.mcfix);
	
	ListType *lstPtr;
	Int16 charset = GBKOS_Charset2ID(g.libRef, g.appConf.conf.charset);
	lstPtr = GetObjectPtr<ListType>(SetupPanelCharsetListList);
	LstSetSelection(lstPtr, charset);
	ctrlPtr = GetObjectPtr<ControlType>(SetupPanelCharSetPopTrigger);
	CtlSetLabel(ctrlPtr, LstGetSelectionText(lstPtr, charset));
	

	SetGlobal(g);
}

static Boolean MainFormInit(FormType *frmP)
{
	global g;
	
	MemSet(&g, sizeof(g), 0);
	
	GetGlobal(g);
	
	g.libRef = NULL;

	UInt16 libRef = 0;
    Err error = SysLibFind ("GBKOS Lib", &libRef);
	if (error) return false;
	error = GBKOS_LibOpen (libRef);
	if (error) return false;
	//error = GBKOS_Init(libRef);
	//if (error) return false;

	g.libRef = libRef;

	if (g.defaultConf.build == 0)
		GBKOS_GetConfig(g.libRef, &g.defaultConf);
	
	g.appIndex = -1;

	MemSet(&g.appConf, sizeof(dbConf), 0);
	
	SetGlobal(g);
		
	DmOpenRef config = OpenConfigDB(dmModeReadWrite, true);
	if (config == NULL) {
		ErrNonFatalDisplay("Fail to open or create config database!");
		return false;
	}
	
	gen_db_list(config);

	char** appList = SetAppList(config);
	if (appList==NULL) {
		ErrNonFatalDisplay("Fail to allocate List Items!");
		FtrSet(CREATOR, 1, (UInt32)NULL);
		return false;
	} else {
		FtrSet(CREATOR, 1, (UInt32)appList);
	}
	
	char** charList = SetCharsetList(g.libRef, config);
	if (charList==NULL) {
		ErrNonFatalDisplay("Fail to allocate List Items!");
		return false;
	} 
	
	CloseConfigDB(config);	
	GetAppConf(0);
	SyncConf2Panel();
    return true;
}

static Boolean MainFormQuit()
{
	global g;
	GetGlobal(g);

	SetAppConf();
	
	char** appList = NULL;
	FtrGet(CREATOR, 1, (UInt32*)&appList);
	if (appList!=NULL) 
		MemPtrFree((MemPtr)appList);
	else  {
		ErrNonFatalDisplay("Fail to get List Items!");
		return false;
	}
	FtrUnregister(CREATOR,1);	

	if (NULL != g.libRef)
	  GBKOS_LibClose(g.libRef);
    return true;
}

/*
 * FUNCTION: MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:
 *
 * command
 *     menu item id
 */

static Boolean MainFormDoCommand(UInt16 command)
{
	global g;
	GetGlobal(g);

    Boolean handled = false;
    FormType * frmP = FrmGetActiveForm();;
	ControlType *ctrlPtr;
	
    switch (command)
    {
    case SetupPanelOKButton:
		MainFormQuit();
		FrmGotoForm(9000);
		handled = true;
        break;
    case SetupPanelResetButton:
	{
		DmOpenRef config = OpenConfigDB(dmModeReadWrite, true);
		clear_db_list(config);
		CloseConfigDB(config);
		MainFormInit(frmP);
        return true;
	}
		break;
    case SetupPanelSmartCheckbox:
		ctrlPtr = GetObjectPtr<ControlType>(SetupPanelSmartCheckbox);
		g.appConf.conf.smart = (UInt8)CtlGetValue(ctrlPtr);
        handled = true;
        break;
    case SetupPanelMCFixCheckbox:
		ctrlPtr = GetObjectPtr<ControlType>(SetupPanelMCFixCheckbox);
		g.appConf.mcfix = (UInt8)CtlGetValue(ctrlPtr);
	    handled = true;
        break;
    }
	SetGlobal(g);
    
    return handled;
}

static Boolean MainFormDoList(FormType *frmP, EventType *eventP)
{


	Int16 lstID = eventP->data.popSelect.listID;
	ListType *lstPtr;
	ControlType *ctrlPtr;    
    Int16 charset , appIndex;
	
	if (lstID == SetupPanelAppListList) { //app changed
    	SetAppConf();
    	appIndex = eventP->data.popSelect.selection;
		GetAppConf(appIndex);
		lstPtr = GetObjectPtr<ListType>(SetupPanelAppListList);
		LstSetSelection(lstPtr, appIndex);
		ctrlPtr = GetObjectPtr<ControlType>(SetupPanelAppPopPopTrigger);
		CtlSetLabel(ctrlPtr, LstGetSelectionText(lstPtr, appIndex));
		SyncConf2Panel();
	} else {
		global g;
		GetGlobal(g);
		charset = eventP->data.popSelect.selection;
		
		g.appConf.conf.charset = GBKOS_ID2Charset(g.libRef, charset);
		lstPtr = GetObjectPtr<ListType>(SetupPanelCharsetListList);
		LstSetSelection(lstPtr, charset);
		ctrlPtr = GetObjectPtr<ControlType>(SetupPanelCharSetPopTrigger);
		CtlSetLabel(ctrlPtr, LstGetSelectionText(lstPtr, charset));
		SetGlobal(g);
	}
	

    return true;
}

/*
 * FUNCTION: MainFormHandleEvent
 *
 * DESCRIPTION:
 *
 * This routine is the event handler for the "MainForm" of this 
 * application.
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

extern "C" Boolean MyHandler(EventType * eventP)
{
    Boolean handled = false;
    FormType * frmP = FrmGetActiveForm();


    switch (eventP->eType) 
        {
        case frmOpenEvent:
            if (!MainFormInit(frmP))
			{
			  FrmAlert (GBKOSNotStartedAlert);
			  //EraseConfigDB();
			  FrmGotoForm(9000);
			}
			else
			{
			  FrmDrawForm(frmP);
			}
            handled = true;
            break;
            
        case frmUpdateEvent:
            /* To do any custom drawing here, first call
             * FrmDrawForm(), then do your drawing, and
             * then set handled to true. */
           	FrmDrawForm(frmP);
           	handled = true;
            break;
		case ctlSelectEvent:
			return MainFormDoCommand(eventP->data.ctlSelect.controlID);
			break;
		case popSelectEvent:
            handled = MainFormDoList(frmP, eventP);
			break;
        default:
            break;
        }
    
    return handled;
}

