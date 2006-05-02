#include "PIMEEngine.h"

static void EngineCodeToText(PIMEEnginePtr engine);
static void EngineClearCode(PIMEEnginePtr engine);

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
	if (engine->imeForm != NULL)
	{
		FrmEraseForm(engine->imeForm);
		FrmDeleteForm(engine->imeForm);
		
		FrmSetActiveForm(engine->currentForm);
		
		engine->imeForm = NULL;

		WinPopDrawState();
	}
}

static RectangleType EngineGetIMEFormRect(PIMEEnginePtr engine)
{
	Coord x, y;
	InsPtGetLocation(&x, &y);
	
	RectangleType result;
	
	result.extent.x = 156;
	result.extent.y = 20;
	
	result.topLeft.x = 2;
	result.topLeft.y = y + InsPtGetHeight() + 4;
	
	if (result.topLeft.y > 130)
		result.topLeft.y = y - 24;
		
	//result.topLeft.y = (y > 80) ? 2 : (158 - result.extent.y);
	
	return result;
}

static void EngineShowIME(PIMEEnginePtr engine)
{
	if (engine->imeForm == NULL)
	{
		WinPushDrawState();
		
		// engine->imeForm = FrmInitForm(frmIME);
		
		RectangleType imeRect = EngineGetIMEFormRect(engine);
		
		engine->imeForm = FrmNewForm(9000, NULL,
			imeRect.topLeft.x, imeRect.topLeft.y,
			imeRect.extent.x, imeRect.extent.y,
			true, 0, 0, 0);
		
		/*engine->imeCodeField = FldNewField(
			(void **)(&(engine->imeForm)), 1000,
			4, 3, 64, 13,
			stdFont, 32, false,
			false, true,
			false,
			leftAlign,
			false, false, false);*/
			
		FrmSetActiveForm(engine->imeForm);
		FrmDrawForm(engine->imeForm);
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

static Boolean EngineStart(PIMEEnginePtr engine)
{
	engine->currentForm = FrmGetActiveForm();
	engine->currentField = GetActiveField();
	
	if (engine->currentField == NULL) return false;
	
	engine->imeForm = NULL;
	
	engine->currentCodeLen = 0;
	engine->maxCodeLen = sizeof(engine->currentCode) / sizeof(Char);
	
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
    
    RectangleType rect;
    rect.topLeft.x = 3;
    rect.topLeft.y = 3;
    rect.extent.x = 150;
    rect.extent.y = 13;
    
    WinEraseRectangle(&rect, 0);
    
	if (engine->currentCodeLen > 0)
	{
		FntSetFont(stdFont);
		
		WinDrawChars(engine->currentCode, engine->currentCodeLen, 3, 3);
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
					handled = EngineRemoveCode(engine);
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
			
		default:
			break;
	}
	
	if (!handled)
	{
		// FrmHandleEvent(engine->currentForm, event);
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