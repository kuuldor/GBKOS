// GBKOSApp.h
//
// This file implements any methods CGBKOSApp
// that differ from the standard implementation in CPalmStApp

// Declaration of Application class
#ifndef GBKOSAPP_H_
#define GBKOSAPP_H_

#include "MainForm.h"

// Application class
class CGBKOSApp : public CPalmStApp<CGBKOSApp>
{
public:
	// Overrides
	virtual Err NormalLaunch();
	Err InitInstance();
	Err ExitInstance();
	static CGBKOSApp * GetInstance();

	UInt32 GetMinRomVersion();
	void ReportRomVersionIncompatible(UInt32 dwCurrentRomVersion);

	// Form map
	BEGIN_FORM_MAP()
		FORM_MAP_ENTRY(MainForm, CMainForm)
	END_FORM_MAP()
};

#endif // GBKOSAPP_H_
