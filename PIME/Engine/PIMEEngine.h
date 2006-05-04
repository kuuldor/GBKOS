#ifndef PIMEENGINE_H_INCLUDED
#define PIMEENGINE_H_INCLUDED

#include <PalmOS.h>

typedef struct tagPIMEEngine {
	Char version[16];
	FormType *currentForm;
	FieldType *currentField;
	
	RectangleType imeAreaRect;
	WinHandle imeAreaBackup;
	
	Char currentCode[32];
	Int8 currentCodeLen;
	Int8 maxCodeLen;
	
	void *oldTrapGsiSetShiftState;
} PIMEEngineType, *PIMEEnginePtr;

PIMEEnginePtr PIME_OpenEngine();
void PIME_CloseEngine(PIMEEnginePtr engine);

void PIME_Run(PIMEEnginePtr engine);

#endif
