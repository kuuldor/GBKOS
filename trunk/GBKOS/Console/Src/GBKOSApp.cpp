// GBKOSApp.cpp
//
// This file implements any methods CGBKOSApp
// that differ from the standard implementation in CPalmStApp

#include "Common.h"
#include "GBKOSApp.h"

CGBKOSApp * CGBKOSApp::GetInstance()
{
	return static_cast<CGBKOSApp *>(CPalmStApp::GetInstance());
}

static void SetAlarm()
{
	UInt16 cardNo;
	LocalID dbID;
	DmSearchStateType searchState;
	
	DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'dGBK', true,
		&cardNo, &dbID);
		
	if (dbID != 0)
		AlmSetAlarm(cardNo, dbID, 0, TimGetSeconds() + 1, true);
}

static void TryStartGBKOS()
{
	UInt16 libRef;
	ftrGBK store;

	Err err = SysLibFind("GBKOS Lib", &libRef);
	
	if (err != errNone)
		err = SysLibLoad(sysFileTLibrary, 'dGBK', &libRef);

	if (err != errNone)
		return;
	
	err = GBKOS_LibOpen(libRef);
	if (err != errNone)
	{
		SysLibRemove(libRef);
	}
	
	err = GBKOS_Init(libRef);
	if (err != errNone)
	{
		GBKOS_UnInit(libRef);
		GBKOS_LibClose(libRef);
		return;
	}
	
	
	GBKOS_GetConfig(libRef, &store);
	
	if (store.on)
	{
		GBKOS_Start(libRef);
		GBKOS_GetConfig(libRef, &store);
	}

	if (store.kernel_started)
	{
		GBKOS_LibClose(libRef);
	}
	else
	{
		GBKOS_UnInit(libRef);
		GBKOS_LibClose(libRef);
		SysLibRemove(libRef);
	}
}

Err CGBKOSApp::InitInstance()
{
	if (m_wCommand == sysAppLaunchCmdSystemReset)
	{
		SetAlarm();
		return errNone;
	}
	
	if (m_wCommand == sysAppLaunchCmdAlarmTriggered)
	{
		TryStartGBKOS();
		return errNone;
	}
	
	if (m_wLaunchFlags & sysAppLaunchFlagNewGlobals)
	{
	}
	return CPalmStApp::InitInstance();
}

Err CGBKOSApp::ExitInstance()
{
	return CPalmStApp::ExitInstance();
}

UInt32 CGBKOSApp::GetMinRomVersion()
{
	return sysMakeROMVersion(3, 5, 0, sysROMStageDevelopment, 0);
}

void CGBKOSApp::ReportRomVersionIncompatible(UInt32 dwCurrentRomVersion)
{
	FrmAlert(RomVersionIncompatibleAlert);
}

static void SonyClieHR()
{
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error;

	if (errNone == FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP,
		(UInt32 *)&sonySysFtrSysInfoP))
	{
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR) 
		{
			UInt16 sonyHiResRefNum = sysInvalidRefNum;
			error = SysLibFind(sonySysLibNameHR, &sonyHiResRefNum);
			if (error == sysErrLibNotFound)
			{
				error = SysLibLoad(
					sysFileTLibrary, sonySysFileCHRLib, &sonyHiResRefNum);
			}

			if (!error)
			{
				UInt32 width = hrWidth, height = hrHeight;
				HROpen(sonyHiResRefNum);
				HRWinScreenMode(
					sonyHiResRefNum, winScreenModeSet, 
					&width, &height, NULL, NULL);
				HRClose(sonyHiResRefNum);
			}
		}
	}
}

// Normal launch handler
Err CGBKOSApp::NormalLaunch()
{
	SonyClieHR();
	
	CForm::GotoForm(MainForm);

	return errNone;
}
