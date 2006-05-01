// MainForm.cpp
//
// This file implements the main form for
// your application.

#include "Common.h"
#include "MainForm.h"
#include "GBKOSApp.h"

static void ExitToLauncher()
{
	EventType evt;
	MemSet(&evt, sizeof(evt), 0);
	
	evt.eType = keyDownEvent;
	evt.data.keyDown.chr = vchrLaunch;
	evt.data.keyDown.modifiers = commandKeyMask;
	
	EvtAddEventToQueue(&evt);
}

void CMainForm::InitCharsetControls()
{
	Char **pCharsetList = NULL;
	Int16 nCharsetCount = 0;
	
	GBKOS_GetCharsets(_gbkosLibRef, pCharsetList, &nCharsetCount);
	
	pCharsetList = (Char **)MemPtrNew(sizeof(Char *) * nCharsetCount);
	GBKOS_GetCharsets(_gbkosLibRef, pCharsetList, &nCharsetCount);
	
	_lstCharset.DeleteAllItems();
	_lstCharset.SetHeight(nCharsetCount);
	
	for (Int16 i = 0; i < nCharsetCount; i ++)
		_lstCharset.AddItem(pCharsetList[i]);
		
	MemPtrFree(pCharsetList);
}

void CMainForm::UpdateControls()
{
	// Set value
	_cbxEnable.SetValue(_store.kernel_started);
	_lstCharset.SetSelection(GBKOS_Charset2ID(_gbkosLibRef, _store.charset));
	_triCharset.SetLabel(_lstCharset.GetSelectionText(GBKOS_Charset2ID(_gbkosLibRef, _store.charset)));
	_cbxDetectCharset.SetValue(_store.smart);
	_cbxBoldFont.SetValue(_store.bold);
	_cbxLargeBoldFont.SetValue(_store.largeBold);
	_cbxWordWrap.SetValue(0);
	_cbxSpace.SetValue(_store.padding);
	
	// Set visible
	if (_store.kernel_started)
	{
		_labCharset.CopyLabel("Char Set:");
		_triCharset.ShowControl();
		_cbxDetectCharset.ShowControl();
		_cbxBoldFont.ShowControl();
		_cbxLargeBoldFont.ShowControl();
		_cbxWordWrap.ShowControl();
		_cbxSpace.ShowControl();
	}
	else
	{
		_labCharset.CopyLabel("");
		_triCharset.HideControl();
		_cbxDetectCharset.HideControl();
		_cbxBoldFont.HideControl();
		_cbxLargeBoldFont.HideControl();
		_cbxWordWrap.HideControl();
		_cbxSpace.HideControl();
	}
}

// Form open handler
Boolean CMainForm::OnOpen(EventPtr pEvent, Boolean& bHandled)
{
	Err err = SysLibFind("GBKOS Lib", &_gbkosLibRef);
	
	if (err != errNone)
		err = SysLibLoad(sysFileTLibrary, 'dGBK', &_gbkosLibRef);
		
	if (err != errNone)
	{
		FrmAlert(NoLibAlert);
		_gbkosLibRef = 0;
		ExitToLauncher();
		return false;
	}
	
	err = GBKOS_LibOpen(_gbkosLibRef);
	ErrFatalDisplayIf(err != errNone, "GBKOS kernel error!");
	
	err = GBKOS_Init(_gbkosLibRef);
	
	if (errFontNotFound == err) 
	{
		FrmAlert(NoFontAlert);
		ExitToLauncher();
		GBKOS_UnInit(_gbkosLibRef);
		GBKOS_LibClose(_gbkosLibRef);
		return false;
	} 
	
	ErrFatalDisplayIf(err != errNone, "GBKOS kernel error!");
	
	GBKOS_GetConfig(_gbkosLibRef, &_store);
	
	// Attach controls
	_cbxEnable.Attach(MainEnableCheckbox);
	_labCharset.Attach(MainCharsetLabel);
	_triCharset.Attach(MainCharsetTrigger);
	_lstCharset.Attach(MainCharsetList);
	_cbxDetectCharset.Attach(MainDetectCharsetCheckbox);
	_cbxBoldFont.Attach(MainBoldCheckbox);
	_cbxLargeBoldFont.Attach(MainLargeBoldCheckbox);
	_cbxWordWrap.Attach(MainWordWrapCheckbox);
	_cbxSpace.Attach(MainSpaceCheckbox);
	
	// Init charset controls
	InitCharsetControls();
	
	// refresh
	UpdateControls();
	
	bHandled = false;
	return true;
}

// Form close handler
Boolean CMainForm::OnClose(EventPtr pEvent, Boolean& bHandled)
{
	if (_gbkosLibRef != 0)
	{
		if (!_store.kernel_started)
			GBKOS_UnInit(_gbkosLibRef);
		
		GBKOS_LibClose(_gbkosLibRef);
		if (!_store.kernel_started)
			SysLibRemove(_gbkosLibRef);
	}
		
	bHandled = false;
	return true;
}

// About command handler
Boolean CMainForm::OnAbout(EventPtr pEvent, Boolean& bHandled)
{
//	CAboutDialog frmAbout;
//	frmAbout.DoModal();
	GBKOS_ShowAboutDialog(_gbkosLibRef, NULL);
	return true;
}

Boolean CMainForm::OnEnableCheckboxSelect(EventType *pEvent, Boolean &bHandled)
{
	if (_cbxEnable.GetValue() != 0)
	{
		if (!_store.kernel_started)
			GBKOS_Start(_gbkosLibRef);
	}
	else
	{
		if (_store.kernel_started)
			GBKOS_Stop(_gbkosLibRef);
	}
	GBKOS_GetConfig(_gbkosLibRef, &_store);
	
	UpdateControls();
	
	return true;
}

Boolean CMainForm::OnCharsetPopupSelect(EventType *pEvent, Boolean &bHandle)
{
	_store.charset = GBKOS_ID2Charset(_gbkosLibRef, _lstCharset.GetSelection());
	
	GBKOS_SetConfig(_gbkosLibRef, &_store);
	
	UpdateControls();
	
	return true;
}

Boolean CMainForm::OnDetectCharsetCheckboxSelect(EventType *pEvent, Boolean &bHandled)
{
	///////////////////////////////
	FrmAlert(UnimplementedAlert);
	///////////////////////////////

	_store.smart = _cbxDetectCharset.GetValue();
	
	GBKOS_SetConfig(_gbkosLibRef, &_store);
	
	UpdateControls();
	
	return true;
}

Boolean CMainForm::OnBoldCheckboxSelect(EventType *pEvent, Boolean &bHandled)
{
	_store.bold = _cbxBoldFont.GetValue();
	
	GBKOS_SetConfig(_gbkosLibRef, &_store);
	
	UpdateControls();
	
	return true;
}

Boolean CMainForm::OnLargeBoldCheckboxSelect(EventType *pEvent, Boolean &bHandled)
{
	_store.largeBold = _cbxLargeBoldFont.GetValue();
	
	GBKOS_SetConfig(_gbkosLibRef, &_store);
	
	UpdateControls();
	
	return true;
}

Boolean CMainForm::OnWordWrapCheckboxSelect(EventType *pEvent, Boolean &bHandled)
{
	///////////////////////////////
	FrmAlert(UnimplementedAlert);
	///////////////////////////////

//	_store.wordWrap = _cbxWordWrap.GetValue();
	
	GBKOS_SetConfig(_gbkosLibRef, &_store);
	
	UpdateControls();
	
	return true;
}

Boolean CMainForm::OnSpaceCheckboxSelect(EventType *pEvent, Boolean &bHandled)
{
	_store.padding = _cbxSpace.GetValue();
	
	GBKOS_SetConfig(_gbkosLibRef, &_store);
	
	UpdateControls();
	
	return true;
}