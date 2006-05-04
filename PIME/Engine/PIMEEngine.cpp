#include "PIMEEngine.h"
#include "PIME_res.h"
#include <HE330/Vga.h>

static void EngineCodeToText(PIMEEnginePtr engine);
static void EngineClearCode(PIMEEnginePtr engine);

static void EngineHideIME(PIMEEnginePtr engine);
static void EngineShowIME(PIMEEnginePtr engine);

static PIMEEnginePtr GetEngineFromFtr();

PIMEEnginePtr PIME_OpenEngine()
{
	PIMEEnginePtr engine = (PIMEEnginePtr)MemPtrNew(sizeof(PIMEEngineType));
	
	if (engine != NULL)
	{
		StrCopy(engine->version, "1.01.0000"); // TODO: read from resource...
		engine->currentCodeLen = 0;
	}
	
	return engine;
}

void PIME_CloseEngine(PIMEEnginePtr engine)
{
	MemPtrFree(engine);
}

static void DealHE330Form(PIMEEnginePtr engine, FormType *form)
{
	if (engine->isHE330_1To1)
	{
		VgaFormModify(form, vgaFormModify160To240);
	}
}

static void CopyField(FieldType *fromField, FieldType *toField, Boolean copyAll)
{
	if (copyAll)
	{
		// Attr
		FieldAttrType fldAttr;
		FldGetAttributes(fromField, &fldAttr);
		FldSetAttributes(toField, &fldAttr);
		
		// MaxChars
		FldSetMaxChars(toField, FldGetMaxChars(fromField));
	}
	
	// Content
	MemHandle toHandle = NULL;
	
	MemHandle fromHandle = FldGetTextHandle(fromField);
	if (fromHandle != NULL)
	{
		Char* fromText = (Char *)MemHandleLock(fromHandle);
		toHandle = MemHandleNew(StrLen(fromText) + 1);
		Char* toText = (Char *)MemHandleLock(toHandle);
		StrCopy(toText, fromText);
		MemHandleUnlock(toHandle);
		MemHandleUnlock(fromHandle);
	}
	
	MemHandle oldToHandle = FldGetTextHandle(toField);
	FldSetTextHandle(toField, toHandle);
	if (oldToHandle != NULL)
		MemHandleFree(oldToHandle);

	// Selection
	UInt16 startPos, endPos;
	FldGetSelection(fromField, &startPos, &endPos);
	FldSetSelection(toField, startPos, endPos);
	
	// InsPtPosition
	FldSetInsPtPosition(toField, FldGetInsPtPosition(fromField));
	
	// Dirty
	FldSetDirty(toField, FldDirty(fromField));
}

static void EngineShowIME(PIMEEnginePtr engine)
{
	if (engine->imeForm == NULL)
	{
		WinPushDrawState();
		
		engine->imeForm = FrmInitForm(frmIME);
		
		DealHE330Form(engine, engine->imeForm);
		
		engine->imeField = (FieldType *)FrmGetObjectPtr(
			engine->imeForm,
			FrmGetObjectIndex(engine->imeForm, fldIMEIME));
		
		FrmSetActiveForm(engine->imeForm);
		FrmDrawForm(engine->imeForm);

		CopyField(engine->currentField, engine->imeField, true);

		FrmSetFocus(engine->imeForm, FrmGetObjectIndex(engine->imeForm, fldIMEIME));
		 
		FrmDrawForm(engine->imeForm);
	}
}

static void EngineHideIME(PIMEEnginePtr engine)
{
	if (engine->imeForm != NULL)
	{
		CopyField(engine->imeField, engine->currentField, false);

		FrmReturnToForm(0);
		
		FrmDrawForm(engine->currentForm);
		
		engine->imeForm = NULL;
		
		WinPopDrawState();
	}
}

static FieldType *GetActiveField()
{
	FormType	*curForm;
	UInt16		curObject;
	TableType	*curTable;
	
	curForm = FrmGetActiveForm();
	if (curForm)
	{
		curObject = FrmGetFocus(curForm);
		if (curObject != noFocus) //当前对象具备焦点
		{
			if (FrmGetObjectType(curForm, curObject) == frmFieldObj) //普通文本框
			{
				return (FieldType *)FrmGetObjectPtr(curForm, curObject);
			}
			else if (FrmGetObjectType(curForm, curObject) == frmTableObj) //表格中的文本框
			{
				curTable = (TableType *)FrmGetObjectPtr(curForm, curObject);
				return TblGetCurrentField(curTable);
			}
		}
	}
	
	return NULL;
}

static void InsertText(FieldType* field, const Char* txt, UInt32 len)
{
	if (field != NULL)
	{
		FldInsert(field, txt, len);
	} 
}

static PIMEEnginePtr GetEngineFromFtr()
{
	UInt32 value;
	
	FtrGet('PIme', 0, &value);
	
	return (PIMEEnginePtr)value;
}

static Boolean IsHE330()
{
	UInt32 version;

	if (FtrGet(TRGSysFtrID, TRGVgaFtrNum, &version) == errNone)
	{
		if (sysGetROMVerMajor(version) >= 1)
		{
			return true;
		}
	}

	return false;
}

static void FillHE330Info(PIMEEnginePtr engine)
{
	engine->isHE330 = IsHE330();
	if (engine->isHE330)
	{
		VgaScreenModeType mode;
		VgaRotateModeType rotate;

		VgaGetScreenMode(&mode, &rotate);

		engine->isHE330_1To1 = (mode == screenMode1To1);
	}
	else
	{
		engine->isHE330_1To1 = false;
	}
}

static Boolean EngineStart(PIMEEnginePtr engine)
{
	FillHE330Info(engine);
	
	engine->currentForm = FrmGetActiveForm();
	engine->currentField = GetActiveField();
	
	if (engine->currentField == NULL) return false;
	
	engine->imeForm = NULL;
	
	engine->currentCodeLen = 0;
	engine->maxCodeLen = sizeof(engine->currentCode) / sizeof(Char);
	
	FtrSet('PIme', 0, (UInt32)(engine));
	
	EngineShowIME(engine);
	
	return true;
}

static void EngineStop(PIMEEnginePtr engine)
{
	EngineClearCode(engine);
	
	EngineHideIME(engine);
}

static void EngineDrawCode(PIMEEnginePtr engine)
{
    EngineShowIME(engine);
    
    static Char imeCode[33];
    StrNCopy(imeCode, engine->currentCode, engine->currentCodeLen);
    imeCode[engine->currentCodeLen] = 0;
    
	FieldType *imeCodeField = (FieldType *)FrmGetObjectPtr(
		engine->imeForm,
        FrmGetObjectIndex(engine->imeForm, fldIMECode));
		 
    FldSetTextPtr(imeCodeField, imeCode);
	FldDrawField(imeCodeField);
}

static void EngineAppendCode(PIMEEnginePtr engine, Char ch)
{
	if (engine->currentCodeLen < engine->maxCodeLen)
	{
		engine->currentCode[engine->currentCodeLen] = ch;
		engine->currentCodeLen ++;
		
		EngineCodeToText(engine);
		
		EngineDrawCode(engine);
	}
}

static void EngineClearCode(PIMEEnginePtr engine)
{
	engine->currentCodeLen = 0;
	
	EngineDrawCode(engine);
}

static Boolean EngineRemoveCode(PIMEEnginePtr engine)
{
	Boolean handled = false;
	
	if (engine->currentCodeLen > 0)
	{
		engine->currentCodeLen --;
		EngineDrawCode(engine);
		handled = true;
	}
	
	return handled;
}

static void EngineCodeToText(PIMEEnginePtr engine)
{
	if (engine->currentCodeLen >= 4)
	{
		UInt16 textCode = 0;
		for (int i = 0; i < 4; i ++)
		{
			UInt16 curByte = 0;
			Char curChar = engine->currentCode[i];
			if (curChar >= '0' && curChar <= '9')
				curByte = curChar - '0';
			else
				curByte = curChar - 'A' + 10;
				
			textCode = (textCode << 4) + curByte;
		}
		
		EngineClearCode(engine);

		InsertText(engine->imeField, (Char *)(&textCode), 2);
	}
}

static Boolean EngineHandleEvent(EventPtr event, PIMEEnginePtr engine)
{
	Boolean done = false;
	Boolean handled = false;
	Char ch;
	
	switch (event->eType)
	{
		case appStopEvent:
			EvtAddEventToQueue(event);
			done = true;
			handled = true;
			break;
		
		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case btnIMEOK:
					done = true;
					break;
					
				default:
					break;
			}
			break;
			
		case keyDownEvent:
			ch = event->data.keyDown.chr;
			switch (event->data.keyDown.chr)
			{
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					EngineAppendCode(engine, ch);
					handled = true;
					break;
					
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					EngineAppendCode(engine, ch - ('a' - 'A'));
					handled = true;
					break;
					
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					EngineAppendCode(engine, ch);
					handled = true;
					break;
					
				case chrBackspace:
					handled = EngineRemoveCode(engine);
					break;
					
				default:
					break;
			}

			break;
		
		default:
			break;
	}
	
	if (!handled)
	{
		// FrmDispatchEvent(event);
		FrmHandleEvent(engine->imeForm, event);
	}
	
	return done;
}

static void EngineEventLoop(PIMEEnginePtr engine)
{
	Boolean done = false;
	
	do
	{
		EventType event;
		EvtGetEvent(&event, evtWaitForever);
		Err error;
		
		if (SysHandleEvent(&event) == false
	 		&& MenuHandleEvent(NULL, &event, &error) == false)
		{
	    	done = EngineHandleEvent(&event, engine);
		}
	} while (!done);
}

void PIME_Run(PIMEEnginePtr engine)
{
	if (EngineStart(engine))
	{
		EngineEventLoop(engine);
		
		EngineStop(engine);
	}
}