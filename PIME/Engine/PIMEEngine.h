#ifndef PIMEENGINE_H_INCLUDED
#define PIMEENGINE_H_INCLUDED

#include <PalmOS.h>

typedef struct tagPIMEEngine {
	Boolean isHE330;
	Boolean isHE330_1To1;
	
	Char version[16];
	FormType *currentForm;
	FieldType *currentField;
	
	FormType *imeForm;
	FieldType *imeField;
	
	Char currentCode[32];
	Int8 currentCodeLen;
	Int8 maxCodeLen;
} PIMEEngineType, *PIMEEnginePtr;

PIMEEnginePtr PIME_OpenEngine();
void PIME_CloseEngine(PIMEEnginePtr engine);

void PIME_Run(PIMEEnginePtr engine);

#endif
