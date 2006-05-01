#include <PalmOS.h>

#include "AboutDialog.h"
#include "AboutDialog_Res.h"

#define MAX_APPVER_LEN (32)
#define MAX_APPNAMEVER_LEN (64)

static Boolean AboutFormDoCommand(UInt16 ctlID, Boolean *doneP)
{
	Boolean handled = false;
	
	switch (ctlID)
	{
		case AboutOkButton:
			*doneP = true;
			handled = true;
			break;
		
		default:
			break;
	}
	
	return handled;
}


static void GetAppNameVer(UInt16 cardNo, LocalID dbID, Char *pszAppNameVer)
{
	if (DmDatabaseInfo(cardNo, dbID, pszAppNameVer,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL) != errNone)
	{
		
		return;
		
	}
	
	DmOpenRef appDB = DmOpenDatabase(cardNo, dbID, dmModeReadOnly);
	
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

static void InitAppNameVer(FormType *frmAbout, const Char *pszAppNameVer)
{
	UInt16 cardNo;
	LocalID dbID;
	
	Char *pAppNameVer = NULL;
	
	pAppNameVer = (Char *)MemPtrNew(MAX_APPNAMEVER_LEN);
	if (NULL == pAppNameVer) return;
	
	if (pszAppNameVer != NULL)
	{
		StrCopy(pAppNameVer,pszAppNameVer);
	}
	else 
	{	
		SysCurAppDatabase(&cardNo, &dbID);
		GetAppNameVer(cardNo, dbID, pAppNameVer);
	}
	
	FieldType *ctlP = (FieldType *)FrmGetObjectPtr(
		frmAbout,
		FrmGetObjectIndex(frmAbout, AboutAppNameVerField));
	
	MemHandle hTmp = FldGetTextHandle(ctlP);
	if (hTmp != NULL)
		MemHandleFree(hTmp);
	hTmp = MemHandleNew(StrLen(pAppNameVer) + 1);
	Char *pTmp = (Char *)MemHandleLock(hTmp);
	StrCopy(pTmp, pAppNameVer);
	pTmp[StrLen(pAppNameVer)] = 0;
	MemHandleUnlock(hTmp);
	
	FldSetTextHandle(ctlP, hTmp);

	MemPtrFree(pAppNameVer);
}

static void UpdateScrollbar(FormType *frmAbout, FieldType *ctlP)
{
	ScrollBarType *barP = (ScrollBarType *)FrmGetObjectPtr(
		frmAbout,
		FrmGetObjectIndex(frmAbout, AboutInformationsScrollbar));
	
	UInt16 nScrollPos, nTextHeight, nFieldHeight;
	FldGetScrollValues(ctlP, &nScrollPos, &nTextHeight, &nFieldHeight);
	
	UInt16 maxValue;
	
	if (nTextHeight > nFieldHeight)
		maxValue = (nTextHeight - nFieldHeight);
	else if (nScrollPos)
		maxValue = nScrollPos;
	else
		maxValue = 0;

	SclSetScrollBar(barP, nScrollPos, 0, maxValue, nFieldHeight - 1);
}

static void InitInformations(FormType *frmAbout)
{
	MemHandle hCopyright = DmGetResource(strRsc, CopyrightString);
	
	FieldType *ctlP = (FieldType *)FrmGetObjectPtr(
		frmAbout,
		FrmGetObjectIndex(frmAbout, AboutInformationsField));
	
	if (hCopyright != NULL)
	{
		Char *pCopyright = (Char *)MemHandleLock(hCopyright);

		MemHandle hTmp = FldGetTextHandle(ctlP);
		if (hTmp != NULL)
			MemHandleFree(hTmp);
		
		Int16 copyrightLen = StrLen(pCopyright);
		hTmp = MemHandleNew(copyrightLen + 1);
		if (NULL == hTmp)
		{
			ErrDisplay("Not Enough Memory. Delete Some thing.");
			MemHandleUnlock(hCopyright);
			DmReleaseResource(hCopyright);
			return;
		}
		Char *pTmp = (Char *)MemHandleLock(hTmp);
		StrCopy(pTmp, pCopyright);
		pTmp[copyrightLen] = 0;
		MemHandleUnlock(hTmp);
		
		MemHandle oldTxtH = FldGetTextHandle(ctlP);
		
		FldSetTextHandle(ctlP, hTmp);
		
		if (oldTxtH != NULL)  
  			MemHandleFree(oldTxtH); 

		MemHandleUnlock(hCopyright);
		DmReleaseResource(hCopyright);
	}
	
	UpdateScrollbar(frmAbout, ctlP);	
}

static Boolean AboutInformationsScrollbarRepeat(FormType *frmAbout, EventType *evtP)
{
	FieldType *fldP = (FieldType *)FrmGetObjectPtr(
		frmAbout,
		FrmGetObjectIndex(frmAbout, AboutInformationsField));
	
	Int16 lineToScroll = evtP->data.sclRepeat.newValue - evtP->data.sclRepeat.value;
	if (lineToScroll < 0)
		FldScrollField(fldP, - lineToScroll, winUp);
	else
		FldScrollField(fldP, lineToScroll, winDown);
		
	return false;
}

static Boolean AboutInformationsPageUpDown(FormType *frmAbout, EventType *evtP)
{
	FieldType *fldP = (FieldType *)FrmGetObjectPtr(
		frmAbout,
		FrmGetObjectIndex(frmAbout, AboutInformationsField));

	Int16 lineToScroll = 0;

	UInt16 nScrollPos, nTextHeight, nFieldHeight;
	FldGetScrollValues(fldP, &nScrollPos, &nTextHeight, &nFieldHeight);

	switch (evtP->data.keyDown.chr)
	{
		case vchrPageUp:
			if (nScrollPos > 0)
				lineToScroll = -nFieldHeight + 1;
			break;
		
		case vchrPageDown:
			if (nScrollPos + nFieldHeight < nTextHeight)
				lineToScroll = nFieldHeight - 1;
			break;
		default:
			return false;
	}

	if (lineToScroll < 0)
		FldScrollField(fldP, - lineToScroll, winUp);
	else
		FldScrollField(fldP, lineToScroll, winDown);
	
	UpdateScrollbar(frmAbout, fldP);	
	return true;
}

static Boolean AboutFormHandleEvent(EventType *evtP, Boolean *doneP)
{
	Boolean handled = false;
	FormType *frmP = FrmGetActiveForm();
	
	switch (evtP->eType)
	{
		case frmUpdateEvent:
			FrmDrawForm(frmP);
			handled = true;
			break;
			
		case ctlSelectEvent:
			handled = AboutFormDoCommand(evtP->data.ctlSelect.controlID, doneP);
			break;
		
		case sclRepeatEvent :
			switch (evtP->data.sclRepeat.scrollBarID)
			{
				case AboutInformationsScrollbar:
					handled = AboutInformationsScrollbarRepeat(frmP, evtP);
					break;
			}
			break;
		case keyDownEvent:
			handled = AboutInformationsPageUpDown(frmP, evtP);
			break;
		default:
			break;
	}
	
	return handled;
}

void ShowAboutDialog(const Char *pszAppNameVer)
{
	DmOpenRef dbP = DmOpenDatabaseByTypeCreator(sysFileTLibrary, 'dGBK', dmModeReadOnly);

	FormType *frmSaved = FrmGetActiveForm();
	
	FormType *frmAbout = FrmInitForm(AboutForm);

	FrmSetActiveForm(frmAbout);
	
	InitAppNameVer(frmAbout, pszAppNameVer);

	InitInformations(frmAbout);
	
	FrmDrawForm(frmAbout);
	
	Boolean done = false;
	
	do {
		EventType evt;
		Err err;
		
		EvtGetEvent(&evt, evtWaitForever);
		
		if (SysHandleEvent(&evt)) continue;
		if (MenuHandleEvent(0, &evt, &err)) continue;
		if (AboutFormHandleEvent(&evt, &done)) continue;
		FrmHandleEvent(frmAbout, &evt);
	} while (!done);
	
	FrmEraseForm(frmAbout);
	FrmDeleteForm(frmAbout);
	
	FrmSetActiveForm(frmSaved);

	DmCloseDatabase(dbP);
}
