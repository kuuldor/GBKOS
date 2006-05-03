#include "PIMEEngine.h"

static void EngineCodeToText(PIMEEnginePtr engine);
static void EngineClearCode(PIMEEnginePtr engine);

static void EngineHideIME(PIMEEnginePtr engine);
static void EngineShowIME(PIMEEnginePtr engine);

static PIMEEnginePtr GetEngineFromFtr();

static void MyGsiSetShiftState(const UInt16 lockFlags, const UInt16 tempShift) 
{
	PIMEEnginePtr engine = GetEngineFromFtr();
	
	void (*func_ptr)(const UInt16, const UInt16);
	
	(void *)func_ptr = (engine->oldTrapGsiSetShiftState);
	
	EngineHideIME(engine);
	
	func_ptr(lockFlags, tempShift);
	
	EngineShowIME(engine);
}

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

static void EngineHideIME(PIMEEnginePtr engine)
{
	if (engine->imeAreaBackup != NULL)
	{
		RectangleType srcRect;
		srcRect.topLeft.x = 0;
		srcRect.topLeft.y = 0;
		srcRect.extent = engine->imeAreaRect.extent;
		
		WinHandle oldDrawWindow = WinSetDrawWindow(WinGetDisplayWindow());
		
		WinCopyRectangle(engine->imeAreaBackup, WinGetDisplayWindow(), &srcRect,
			engine->imeAreaRect.topLeft.x, engine->imeAreaRect.topLeft.y,
			winPaint);
		
		WinSetDrawWindow(oldDrawWindow);
			
		WinPopDrawState();
		
		WinDeleteWindow(engine->imeAreaBackup, false);
		engine->imeAreaBackup = NULL;
	}
}

static RectangleType EngineGetIMEFormRect(PIMEEnginePtr engine)
{
	Coord x, y;
	InsPtGetLocation(&x, &y);
	
	RectangleType result;
	
	result.extent.x = 160;
	result.extent.y = 20;
	
	result.topLeft.x = 0;
	result.topLeft.y = y + InsPtGetHeight();
	
	if (result.topLeft.y > 140)
		result.topLeft.y = y - 20;
		
	return result;
}

static void EngineShowIME(PIMEEnginePtr engine)
{
	if (engine->imeAreaBackup == NULL)
	{
		WinPushDrawState();
		
		engine->imeAreaRect = EngineGetIMEFormRect(engine);
		
		Err error;
		engine->imeAreaBackup = WinCreateOffscreenWindow(
			engine->imeAreaRect.extent.x,
			engine->imeAreaRect.extent.y,
			screenFormat,
			&error
			);
		
		WinCopyRectangle(WinGetDisplayWindow(), engine->imeAreaBackup,
			&(engine->imeAreaRect), 0, 0,
			winPaint);
	}
	
    WinHandle oldDrawWindow = WinSetDrawWindow(WinGetDisplayWindow());
    
    WinEraseRectangle(&(engine->imeAreaRect), 0);
    
    RectangleType rectFrame = engine->imeAreaRect;
    rectFrame.topLeft.x ++;
    rectFrame.topLeft.y ++;
    rectFrame.extent.x -= 2;
    rectFrame.extent.y -= 2;
    
    WinDrawRectangleFrame(roundFrame, &rectFrame);
    
    WinSetDrawWindow(oldDrawWindow);
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

static Boolean EngineStart(PIMEEnginePtr engine)
{
	engine->currentForm = FrmGetActiveForm();
	engine->currentField = GetActiveField();
	
	if (engine->currentField == NULL) return false;
	
	engine->imeAreaBackup = NULL;
	
	engine->currentCodeLen = 0;
	engine->maxCodeLen = sizeof(engine->currentCode) / sizeof(Char);
	
	FtrSet('PIme', 0, (UInt32)(engine));
	
	engine->oldTrapGsiSetShiftState = SysGetTrapAddress(sysTrapGsiSetShiftState);
	
	SysSetTrapAddress(sysTrapGsiSetShiftState, MyGsiSetShiftState);
	EngineShowIME(engine);
	
	return true;
}

static void EngineStop(PIMEEnginePtr engine)
{
	EngineClearCode(engine);
	
	EngineHideIME(engine);
	
	SysSetTrapAddress(sysTrapGsiSetShiftState, engine->oldTrapGsiSetShiftState);
}

static void EngineDrawCode(PIMEEnginePtr engine)
{
    EngineShowIME(engine);
    
    static Char imeCode[33];
    StrNCopy(imeCode, engine->currentCode, engine->currentCodeLen);
    imeCode[engine->currentCodeLen] = 0;
    
	if (engine->currentCodeLen > 0)
	{
		WinHandle oldDrawWindow = WinSetDrawWindow(WinGetDisplayWindow());
		
		FntSetFont(stdFont);
		
		WinDrawChars(engine->currentCode, engine->currentCodeLen,
			engine->imeAreaRect.topLeft.x + 4,
			engine->imeAreaRect.topLeft.y + 4);
			
		WinSetDrawWindow(oldDrawWindow);
	}
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

		EngineHideIME(engine);
		InsertText(engine->currentField, (Char *)(&textCode), 2);
		EngineShowIME(engine);
	}
}

static Boolean EngineHandleEvent(EventPtr event, PIMEEnginePtr engine)
{
	Boolean done = false;
	Boolean handled = false;
	Boolean ignoreHideIME = false;
	Char ch;
	
	switch (event->eType)
	{
		case appStopEvent:
			EvtAddEventToQueue(event);
			done = true;
			handled = true;
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
					if (!EngineRemoveCode(engine))
					{
						EngineHideIME(engine);
						FrmHandleEvent(engine->currentForm, event);
						EngineShowIME(engine);
					}
					
					handled = true;
					break;
					
				case chrEscape:
					EngineClearCode(engine);
					handled = true;
					break;
					
				case chrLineFeed:
					done = true;
					handled = true;
					break;
				
				default:
					break;
			}

			break;
		
		case nilEvent:
			ignoreHideIME = true;
			break;
			
		default:
			break;
	}
	
	if (!handled)
	{
		if (!ignoreHideIME)
			EngineHideIME(engine);

		FrmDispatchEvent(event);

		if (!ignoreHideIME)
		{
			EngineShowIME(engine);
			EngineDrawCode(engine);
		}
	}
	
	// check field
	engine->currentField = GetActiveField();
	if (engine->currentField == NULL)
		done = true;

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