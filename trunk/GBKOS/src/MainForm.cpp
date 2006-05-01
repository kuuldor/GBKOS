#include <PalmOS.h>
#include <SonyCLIE.h>

#ifndef THIS_IS_DA
#define GBKOSLIB_TRAP(trapno)
#endif

#include "MainForm_res.h"
#include "MainForm.h"
#include "GBKOS_Lib.h"

/*********************************************************************
 * Internal Constants
 *********************************************************************/

#ifdef THIS_IS_DA
#include <Standalone.h>
STANDALONE_CODE_RESOURCE_ID (1000);
#else

static void SwitchToHiRes(UInt16 refNum)
{
      UInt32 width, height;
      width = hrWidth ; height = hrHeight ;
      Err error = HRWinScreenMode ( refNum, winScreenModeSet, &width, &height, NULL, NULL );
      if ( error != errNone ){
              //  Screen mode remains unchanged.
              ErrFatalDisplay("Cannot change to Hi-Res mode!");
      }
}


#endif


/* Define the minimum OS version we support */
#define ourMinVersion    sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0)


static void ShowHideObjects(FormType *frmP, Boolean show)
{
	if (show) {
		//FrmShowObject(frmP, FrmGetObjectIndex(frmP,MainSmartCheckbox));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP,MainBoldCheckbox));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP,MainLBoldCheckbox));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP,MainSpacingCheckbox));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP,MainLbCharSetLabel));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP,MainCharSetPopTrigger));
	} else {
		FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainSmartCheckbox));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainBoldCheckbox));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainLBoldCheckbox));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainSpacingCheckbox));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainLbCharSetLabel));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainCharSetPopTrigger));
	}
}

static void MainFormRefresh(ftrGBK *store, FormType *frmP, UInt16 libRef)
{
	if (!store) return;

	ControlType *ctrlPtr;

	ctrlPtr = GetObjectPtr<ControlType>(MainEnableCheckbox);
	store->on = (UInt8)CtlGetValue(ctrlPtr);
	ctrlPtr = GetObjectPtr<ControlType>(MainSmartCheckbox);
	store->smart = (UInt8)CtlGetValue(ctrlPtr);
	ctrlPtr = GetObjectPtr<ControlType>(MainBoldCheckbox);
	store->bold = (UInt8)CtlGetValue(ctrlPtr);
	ctrlPtr = GetObjectPtr<ControlType>(MainLBoldCheckbox);
	store->largeBold = (UInt8)CtlGetValue(ctrlPtr);
	ctrlPtr = GetObjectPtr<ControlType>(MainSpacingCheckbox);
	store->padding = (UInt8)CtlGetValue(ctrlPtr);


	ListType *lstPtr;
	lstPtr = GetObjectPtr<ListType>(MainCharSetList);
	store->charset = GBKOS_ID2Charset(libRef, LstGetSelection(lstPtr));

	ShowHideObjects(frmP, store->on);

}

static void MainFormSetup(ftrGBK *store, FormType *frmP, UInt16 libRef, Boolean isInit=false, char*** listArray = NULL)
{
	if (!store
#ifdef THIS_IS_DA
	        ||!store->kernel_started
#endif
	   ) {
		FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainEnableCheckbox));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP,MainWarningLabel));
	}

#ifdef THIS_IS_DA
	FrmHideObject(frmP, FrmGetObjectIndex(frmP,MainEnableCheckbox));
#endif

	ControlType *ctrlPtr;

	ctrlPtr = GetObjectPtr<ControlType>(MainEnableCheckbox);
	CtlSetValue(ctrlPtr, store->on);

	ctrlPtr = GetObjectPtr<ControlType>(MainSmartCheckbox);
	CtlSetValue(ctrlPtr, store->smart);
	ctrlPtr = GetObjectPtr<ControlType>(MainBoldCheckbox);
	CtlSetValue(ctrlPtr, store->bold);
	ctrlPtr = GetObjectPtr<ControlType>(MainLBoldCheckbox);
	CtlSetValue(ctrlPtr, store->largeBold);
	ctrlPtr = GetObjectPtr<ControlType>(MainSpacingCheckbox);
	CtlSetValue(ctrlPtr, store->padding);

	ListType *lstPtr;
	lstPtr = GetObjectPtr<ListType>(MainCharSetList);

	if (isInit) 
	{
	  *listArray = (char**)MemPtrNew(sizeof(char*)*5);
	  Int16 cnt = 5;
	  GBKOS_GetCharsets(libRef, *listArray, &cnt);
	  LstSetListChoices(lstPtr, *listArray, cnt);
	  LstSetHeight(lstPtr, cnt>5?5:cnt);
	}

	LstSetSelection(lstPtr, GBKOS_Charset2ID(libRef, store->charset));
	ctrlPtr = GetObjectPtr<ControlType>(MainCharSetPopTrigger);
	CtlSetLabel(ctrlPtr, 
		LstGetSelectionText(lstPtr,GBKOS_Charset2ID(libRef, store->charset)));

	ShowHideObjects(frmP, store->on);

}

static void GetAppNameVer(Char *pszAppNameVer)
{
	DmOpenRef appDB = DmOpenDatabaseByTypeCreator ('DAcc', CREATOR, dmModeReadOnly);
	
	
	if (appDB == NULL)
		return;
	
	UInt16 idxAIN = DmFindResourceType(appDB, ainRsc, 0);
	UInt16 idxVer = DmFindResourceType(appDB, verRsc, 0);
	
	if (idxAIN != 0xFFFF)
	{
		MemHandle hAIN = DmGetResourceIndex(appDB, idxAIN);
	
		if (hAIN != NULL)
		{
			Char *pAIN = (Char *)MemHandleLock(hAIN);
			StrCopy(pszAppNameVer, pAIN);
		
			MemHandleUnlock(hAIN);
			DmReleaseResource(hAIN);
		}
	}

	if (idxVer != 0xFFFF)
	{
		MemHandle hVer = DmGetResourceIndex(appDB, idxVer);
	
		if (hVer != NULL)
		{
			Char *pVer = (Char *)MemHandleLock(hVer);
			
			StrCat(pszAppNameVer, " v");
			StrCat(pszAppNameVer, pVer);
		
			MemHandleUnlock(hVer);
			DmReleaseResource(hVer);
		}
		else 
		{
			StrCat(pszAppNameVer, " (Version Unknown)");
		}
	}

	DmCloseDatabase(appDB);
}

static Boolean MainFormDoCommand(ftrGBK *store, UInt16 ctrlID, Boolean &done, UInt16 libRef)
{
	FormType * frmP;
	Boolean handled = false;

	handled = false;

	switch (ctrlID)
	{
	case MainOKButton:
		frmP = FrmGetActiveForm();
		MainFormRefresh(store, frmP, libRef);
		done = true;
		handled = true;
		break;
	case MainAboutButton:
		/* Display the About Box. */
#ifndef THIS_IS_DA
		GBKOS_ShowAboutDialog(libRef, NULL);
#else
		{
			Char *pAppNameVer = NULL;
			pAppNameVer = (Char *)MemPtrNew(64);
			
			if (NULL != pAppNameVer)
			{
				GetAppNameVer(pAppNameVer);
				GBKOS_ShowAboutDialog(libRef, pAppNameVer);
			}
			
			MemPtrFree(pAppNameVer);
		}
#endif
		handled = true;
		break;
	case MainEnableCheckbox:
#ifndef THIS_IS_DA
		if (!store->on) GBKOS_Start(libRef);
#endif
	case MainSmartCheckbox:
	case MainBoldCheckbox:
	case MainLBoldCheckbox:
	case MainSpacingCheckbox:
		frmP = FrmGetActiveForm();
		MainFormRefresh(store, frmP, libRef);
		handled = true;
		break;
	}

	return handled;
}

static Boolean MainFormDoList(ftrGBK *store, FormType *frmP, EventType *eventP)
{

	Int16 charset;
	Int16 new_charset;

	Int16 lstID = eventP->data.popSelect.listID;
	ListType *lstPtr;

	if (lstID == MainCharSetList) { //charset changed
		charset = eventP->data.popSelect.priorSelection;
		new_charset = eventP->data.popSelect.selection;
	} else { // whatever i don't know
		return false;
	}


	if (new_charset!=charset) {
		charset = new_charset;

		ControlType *ctrlPtr;
		lstPtr = GetObjectPtr<ListType>(MainCharSetList);
		LstSetSelection(lstPtr, charset);
		ctrlPtr = GetObjectPtr<ControlType>(MainCharSetPopTrigger);
		CtlSetLabel(ctrlPtr, LstGetSelectionText(lstPtr, charset));
	}
	return true;
}

static Boolean IsInside(RectangleType r, Int16 x, Int16 y)
{
	return (0 <= x && x <= r.extent.x && 0 <= y && y <= r.extent.y);
}

extern "C" Boolean MainFormHandleEvent(EventType * eventP, Boolean &done, ftrGBK* store, UInt16 libRef)
{
	Boolean handled = false;
	RectangleType rect;
	FormType *frmP = FrmGetActiveForm();

	done = false;

	switch (eventP->eType) {
	case frmOpenEvent:
		MainFormSetup(store, frmP, libRef);
		FrmDrawForm(frmP);
		handled = true;
		break;

	case appStopEvent:
#ifdef THIS_IS_DA
		EvtAddEventToQueue(eventP);
#endif
		done = true;
		handled = true;
		break;
	case penDownEvent:
		FrmGetFormBounds(frmP,&rect);
		if (!IsInside(rect,eventP->screenX, eventP->screenY)) {
			/* Pen tapped on Outside of form -- quit */
#ifdef THIS_IS_DA
			done = true;
#else 
			SndPlaySystemSound(sndWarning);
#endif
			handled = true;
		}
		break;
	case ctlSelectEvent:
		handled = MainFormDoCommand(store, eventP->data.ctlSelect.controlID,done,libRef);
		break;
	case frmUpdateEvent:
		MainFormSetup(store, frmP, libRef);
		FrmDrawForm(frmP);
		handled = true;
		break;
	case popSelectEvent:
		handled = MainFormDoList(store, frmP, eventP);
		break;
	default:
		break;
	}

	return handled;
}

/*
 * FUNCTION: AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 */

static void AppEventLoop(ftrGBK *store, UInt16 libRef)
{
	UInt16 error;
	EventType event;
	Boolean done = false;
	FormType *frmP = FrmGetActiveForm();

	do {
		/* change timeout if you need periodic nilEvents */
		EvtGetEvent(&event, evtWaitForever);

		if (SysHandleEvent(&event))
			continue;
		if (MenuHandleEvent(0, &event, &error))
			continue;

		if (!MainFormHandleEvent(&event, done, store, libRef))
			FrmHandleEvent(frmP, &event);

	} while (!done);
}


/*
 * FUNCTION: RomVersionCompatible
 *
 * DESCRIPTION: 
 *
 * This routine checks that a ROM version is meet your minimum 
 * requirement.
 *
 * PARAMETERS:
 *
 * requiredVersion
 *     minimum rom version required
 *     (see sysFtrNumROMVersion in SystemMgr.h for format)
 *
 * launchFlags
 *     flags that indicate if the application UI is initialized
 *     These flags are one of the parameters to your app's PilotMain
 *
 * RETURNED:
 *     error code or zero if ROM version is compatible
 */

static Err RomVersionCompatible(UInt32 requiredVersion)
{
	UInt32 romVersion;

	/* See if we're on in minimum required version of the ROM or later. */
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
	{
		FrmAlert (RomIncompatibleAlert);
		return sysErrRomIncompatible;
	}

	return errNone;
}

extern "C" UInt32 DA_Main()
{
	Err error;
	UInt16 formID = FrmGetActiveFormID();
	struct EventType updateEvent;
	UInt16 libRef = 0;
	
	error = RomVersionCompatible (ourMinVersion);
	if (error) return (error);

#ifdef THIS_IS_DA
    error = SysLibFind ("GBKOS Lib", &libRef);
    if (error)  error = SysLibLoad ('libr', 'dGBK', &libRef);	
	if (error) return (error);
	error = GBKOS_LibOpen (libRef);
	if (error) return error;
#endif
	error = GBKOS_Init(libRef);
	if (error) return error;
	
	FormType * frmP = FrmInitForm(MainForm);
	ftrGBK store;
	GBKOS_GetConfig(libRef, &store);

#ifndef THIS_IS_DA

	if (store.hasSonyHR && store.HRLibRef != NULL) {
		SwitchToHiRes(store.HRLibRef);
	}

#endif

	char **listArray = NULL;

	FrmSetActiveForm(frmP);
	MainFormSetup(&store, frmP, libRef, true, &listArray);
	FrmDrawForm(frmP);

	AppEventLoop(&store,libRef);

	FrmEraseForm(frmP);
	FrmDeleteForm(frmP);

	updateEvent.eType = frmUpdateEvent;
	updateEvent.data.frmUpdate.formID = formID;
	updateEvent.data.frmUpdate.updateCode = frmRedrawUpdateCode;

	EvtAddEventToQueue(&updateEvent);

	GBKOS_SetConfig(libRef, &store);
	
#ifndef THIS_IS_DA
	if (!store.on) {
		GBKOS_Stop(libRef);
		GBKOS_UnInit(libRef);
	}
#endif
	GBKOS_LibClose(libRef);
	/*if (!store.kernel_started)
	{
		SysLibRemove(libRef);
	}
	*/
		
	if (NULL != listArray)
	  MemPtrFree(listArray);

	return errNone;
}

static UInt32 GBKOSSetup_PalmMain(
    UInt16 cmd,
    MemPtr /*cmdPBP*/,
    UInt16 launchFlags)
{
	if (cmd != sysAppLaunchCmdNormalLaunch) return errNone;

	return DA_Main();

}

/*
 * FUNCTION: PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 * 
 * PARAMETERS:
 *
 * cmd
 *     word value specifying the launch code. 
 *
 * cmdPB
 *     pointer to a structure that is associated with the launch code
 *
 * launchFlags
 *     word value providing extra information about the launch.
 *
 * RETURNED:
 *     Result of launch, errNone if all went OK
 */

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	return GBKOSSetup_PalmMain(cmd, cmdPBP, launchFlags);
}


