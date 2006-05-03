#include <PalmOS.h>
#include "common.h"

#include "CJKFont.h"

#include "Big5.h"

#include "PalmHiRes.h"

Boolean isCJKChar(const char *s)
{
	unsigned char c1,c2;
	c1 = s[0]; c2 = s[1];

	if (canBeCJK1stByte(c1)&&canBeCJK2ndByte(c2)) return true;

	return false;
}

Boolean isCJKChar(WChar ch)
{
	unsigned char c1,c2;
	c1 = (ch&0xFF00)>>8; c2 = ch&0xFF;

	if (canBeCJK1stByte(c1)&&canBeCJK2ndByte(c2)) return true;

	return false;
}

Boolean canBeCJK1stByte(unsigned char ch)
{
#ifndef _GB2312_	
	if (ch > 0x80 && ch <0xFF) return true;
#else
	if (ch > 0xA0 && ch <0xF8) return true;
#endif

	return false;
}

Boolean canBeCJK2ndByte(unsigned char ch)
{
#ifndef _GB2312_	
	if (ch >= 0x40 && ch <0xFF & ch!=0x7F) return true;
#else
	if (ch > 0xA0 && ch <0xFF) return true;
#endif

	return false;
}


int FindNextCJK(const char *s, int len)
{
	if (len == 0) return 0;

	for (int i=0; i<len; i++) {
#ifndef _GB2312_
		if (*((unsigned char*)(s+i))<0x81) {
#else
		if (*((unsigned char*)(s+i))<0xA1) {
#endif
			continue;
		}
		if (isCJKChar(s+i)) return i;
	}
	return len;
}

Boolean isOpenBracePunc(const char *s)
{
	char *pattern = "¡®¡°£à¡²¡´¡¶¡¸¡º¡¼¡¾£¨£Û£û";

	char token[3];

	token[0] = s[0];
	token[1] = s[1];
	token[2] = 0;

	if ( StrStr(pattern, token) != NULL)
		return true;

	return false;
}

Boolean isCloseBracePunc(const char *s)
{
	char *pattern = "¡¯¡±£§¡³¡µ¡·¡¹¡»¡½¡¿£©£Ý£ý";

	char token[3];

	token[0] = s[0];
	token[1] = s[1];
	token[2] = 0;

	if ( StrStr(pattern, token) != NULL)
		return true;

	return false;
}


Boolean isCJKPunc(const char *s)
{
	char *pattern = "¡££¬¡¢£»£º£¿£¡¡­¡ª¡«£ü¡¦¡§¡¥¡¤¡¬¡Ã£¢";

	char token[3];

	token[0] = s[0];
	token[1] = s[1];
	token[2] = 0;

	if ( StrStr(pattern, token) != NULL || isOpenBracePunc(s) || isCloseBracePunc(s))
		return true;

	return false;
}



UInt16 getCJKCharWidth(ftrSave* store, UInt16 font, Boolean forceHR)
{
	UInt16 metric, xpad, metricPad;
	Boolean bold = false;

	store->fontManager->getMetricByFontID(store, font, metric, xpad, metricPad, forceHR);
	switch (font) {
	case 1:
	case 9:
	case 225:
	case 227:
		if (store->bold)
		{
			bold = true;
		}
		break;
	case 7:
	case 15:
	case 229:
	case 231:
		if (store->largeBold)
		{
			bold = true;
		}
		break;
	}

	if (bold)
	{
		xpad *= 2;
		metric += metricPad;
	}
	if (!store->padding)
	{
		xpad=0;
	}

	UInt16 maxWidth = metric+xpad;

	return maxWidth;
}

void SavePref(ftrSave *store)
{
	if (NULL != store)
	{
		UInt16 size = (UInt16)((char*)&store->GB - (char*)store);
		PrefSetAppPreferences(CREATOR, BASE_PREF_ID, BASE_PREF_VER, (const void *)store, size, true);
	}
}

Int16 LoadPref(ftrSave *store)
{
	if (NULL != store)
	{
		UInt16 size;
		return PrefGetAppPreferences(CREATOR, BASE_PREF_ID, (void*)store, &size , true);
	}
	return 0;
}

static Boolean isJap()
{
	UInt32 value;

	Err err = FtrGet(sysFtrCreator, sysFtrNumEncoding, &value);
	if (errNone == err && 
			(charEncodingCP932 == value || charEncodingPalmSJIS == value || charEncodingShiftJIS == value) 
		)
	{
		return true;
	}

	if (DatabaseExists("TxtEncodingExt") || DatabaseExists("TxtEncodingExtJ")) 
	{
		return true;
	}
	
	return false;
}

static Boolean hasPalmHiRes()
{
  Err err;
  UInt32 WinVersion, attr;
  err = FtrGet(sysFtrCreator, sysFtrNumWinVersion, &WinVersion);
  if (err == 0 && WinVersion >= 4) {
    WinScreenGetAttribute(winScreenDensity, &attr);
    if (attr == kDensityDouble)
    {
    	//ErrDisplay("OS4 HiRes found");
	    return true; 
	}
  }
  return false;
}

static Boolean hasHE330QVGA()
{
	UInt32 version; 
	if (FtrGet(TRGSysFtrID, TRGVgaFtrNum, &version) == 0) 
	{
		if (sysGetROMVerMajor(version) >= 1) 
		{
	    	//ErrDisplay("HE330 QVGA found");
			return true;
		}
	}
	return false;
}
		 
 
// charEncoding definations
// Chinese character encodings
#ifndef charEncodingGB2312
#define charEncodingGB2312		CHAR_ENCODING_VALUE(58)
#endif
//#define charEncodingHZ			CHAR_ENCODING_VALUE(59)
#ifndef charEncodingBig5
#define charEncodingBig5		CHAR_ENCODING_VALUE(60)
#endif

static void BackupNumEncodingFtr(ftrSave *store)
{
	FtrGet(sysFtrCreator, sysFtrNumEncoding, &(store->ftrNumEncodingBackup));
}

static void RestoreNumEncodingFtr(ftrSave *store)
{
	FtrSet(sysFtrCreator, sysFtrNumEncoding, store->ftrNumEncodingBackup);
}

void SetNumEncodingFtr(ftrSave *store)
{
	if (store->charset == GBK)
	{
		FtrSet(sysFtrCreator, sysFtrNumEncoding, charEncodingGB2312);
	}
	else if (store->charset == Big5)
	{
		FtrSet(sysFtrCreator, sysFtrNumEncoding, charEncodingBig5);
	}
	else
	{
		FtrSet(sysFtrCreator, sysFtrNumEncoding, store->ftrNumEncodingBackup);
	}
}

Err InitGBKOSBase()
{
	Err err = errNone;

	ftrSave *store = NULL;
	err = FtrGet(CREATOR,0,(UInt32*)(&store));

	if (err==0 && store != NULL) {
		ErrFatalDisplayIf(store->size != sizeof(ftrSave) || store->build != _BUILD_, 
			"The Feature Store is for different version of GBKOS! Please do a soft reset to clean it.");
	}
	else
	{
		store = (ftrSave*) MemPtrNew(sizeof(ftrSave));
		ErrFatalDisplayIf(store==NULL,"Cannot allocate memory for Feature Store!");
		MemPtrSetOwner(store, 0);
		MemSet(store,sizeof(ftrSave),0);
	
		LoadPref(store);

		//store->on = false;
		store->size = sizeof(ftrSave);
		store->build = _BUILD_;
		store->trapPatched = false;

		store->charSortValues = NULL;
		store->charCaselessValues = NULL;

		// backup numEncodingFtr first
		BackupNumEncodingFtr(store);

		// set current numEncodingFtr
		SetNumEncodingFtr(store);

		if (! (store->hasSonyHR = SonyHROpen(store->HRLibRef, store->HRVersion)) ) {
			store->HRLibRef = NULL;
			store->HRVersion = 0;
		}

		store->isJap = isJap();
		store->hasPalmHiRes = hasPalmHiRes();
		store->hasHE330QVGA = hasHE330QVGA();

		if (NULL == store->Big5)
		{
			store->Big5 = InitBig5Table();
		}


		if (NULL == store->fontManager) {
			store->fontManager = (CJKFontManager*)PermanentPtrNew(sizeof(CJKFontManager));
			ErrFatalDisplayIf(NULL == store->fontManager, "Cannot create Font Manager!");
			if (!store->fontManager->loadAllFonts(store)) {
				store->fontManager->Destroy();
				MemPtrFree(store->fontManager);
				store->fontManager = NULL;
				MemPtrFree(store);
				//ErrDisplay("Cannot load fonts!");
				
				return errFontNotFound;
			}
		}

		FtrSet(CREATOR,0,(UInt32)store);

	}
	return errNone;
}

void ReleaseGBKOSBase()
{
	Err err = errNone;

	ftrSave *store = NULL;
	err = FtrGet(CREATOR,0,(UInt32*)(&store));
	if (err!=0 || NULL == store ) {
		return;
	}

	ErrFatalDisplayIf(store->size != sizeof(ftrSave) || store->build != _BUILD_, 
			"The Feature Store is for different version of GBKOS! Please do a soft reset to clean it.");
	
	if (NULL != store->Big5 && (Encoding*) 0xdeadbeef != store->Big5 )
	{
		ReleaseBig5Table(store->Big5);
		store->Big5 = NULL;
	}

	if (NULL != store->fontManager) {
		store->fontManager->Destroy();
		MemPtrFree(store->fontManager);
		store->fontManager=NULL;
	}

	// destory sort tables
	if (NULL != store->charSortValues)
	{
		MemPtrFree(store->charSortValues);
		store->charSortValues = NULL;
	}

	if (NULL != store->charCaselessValues)
	{
		MemPtrFree(store->charCaselessValues);
		store->charCaselessValues = NULL;
	}
	SonyHRClose();
	
	SavePref(store);

	// restore numEncodingFtr
	RestoreNumEncodingFtr(store);

	MemPtrFree(store);
	store = NULL;
	FtrUnregister(CREATOR, 0);

}

ftrSave *GetGBKOSBase()
{
	Err err;

	ftrSave *store = NULL;
	err = FtrGet(CREATOR,0,(UInt32*)(&store));

	if (err == 0) {
		return store;
	}	else {
	    return NULL;
	}
}

void * GetAndSetTrapAddress(UInt16 trapNum, void *proc)
{
	void *retv = SysGetTrapAddress(trapNum);
	Err err = SysSetTrapAddress(trapNum, proc);
	ErrFatalDisplayIf(err!=errNone || retv==NULL,"Cannot Get and Set Trap address!");
	return retv;
}

void PatchTraps()
{
#ifndef THIS_IS_DA	
	ftrSave *store = NULL;
	Err err = FtrGet(CREATOR,0,(UInt32*)(&store));

	if (err==0 && store != NULL && !store->trapPatched) {
		UInt32 romVersion;
		FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

		if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageDevelopment,0)
		        && romVersion < sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0) )
		{
			if (!store->hasPalmHiRes)
			{
				store->BltDrawChars =
			    	(void (*) (void*,RectangleType*,RectangleType*,const char*,UInt16,FontPtr,UInt32))
			    	GetAndSetTrapAddress(sysTrapBltDrawChars, (void*)_BltDrawChars);
				store->BltDrawChars_v35 = NULL;
				store->BltDrawChars_HiRes = NULL;
			}
			else
			{
				store->BltDrawChars_HiRes =
			    	(void (*) (void*,UInt16,UInt16,const char*,UInt16,FontPtr,UInt32))
			    	GetAndSetTrapAddress(sysTrapBltDrawChars, (void*)_BltDrawChars_HiRes);
				store->BltDrawChars_v35 = NULL;
				store->BltDrawChars = NULL;
			}
		}
		else if (romVersion >= sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0) )
		{
			store->BltDrawChars_v35 =
			    (void (*) (BitmapType*,DrawStateType*,Int16,Int16,Int16,Int16,Int16,Int16,Int16,Int16,
			               const Char* const,UInt16,FontPtr))
			    GetAndSetTrapAddress(sysTrapBltDrawChars, (void*)_BltDrawChars_v35);
			store->BltDrawChars = NULL;
			store->BltDrawChars_HiRes = NULL;
		}


		store->FntLineWidth =
		    (Int16 (*) (const char *,UInt16))GetAndSetTrapAddress(sysTrapFntLineWidth, (void*)_FntLineWidth);

		store->FntCharsInWidth =
		    (void (*) (const char*,Int16*,Int16*,Boolean*))
		    GetAndSetTrapAddress(sysTrapFntCharsInWidth, (void*)_FntCharsInWidth);

		store->FntCharsWidth =
		    (Int16 (*) (const char *chars,Int16))GetAndSetTrapAddress(sysTrapFntCharsWidth, (void*)_FntCharsWidth);

		store->FntCharWidth =
		    (Int16 (*) (char))GetAndSetTrapAddress(sysTrapFntCharWidth, (void*)_FntCharWidth);

		store->FntWordWrap =
		    (UInt16 (*) (const char *chars,UInt16))GetAndSetTrapAddress(sysTrapFntWordWrap, (void*)_FntWordWrap);

		store->FntWidthToOffset =
		    (Int16 (*) (Char const *,UInt16,Int16,Boolean *, Int16 *))
		    GetAndSetTrapAddress(sysTrapFntWidthToOffset, (void*)_FntWidthToOffset);
		/*
		store->FldHandleEvent = 
			(Boolean (*) (FieldType *, EventType *))GetAndSetTrapAddress(sysTrapFldHandleEvent, (void*)_FldHandleEvent);
		*/

		store->StrCompare =
		    (Int16 (*)(const Char *, const Char *))
		    GetAndSetTrapAddress(sysTrapStrCompare, (void *)_StrCompare);

		store->StrCaselessCompare =
		    (Int16 (*)(const Char *, const Char *))
		    GetAndSetTrapAddress(sysTrapStrCaselessCompare, (void *)_StrCaselessCompare);

		store->StrNCompare =
		    (Int16 (*)(const Char *, const Char *, Int32))
		    GetAndSetTrapAddress(sysTrapStrNCompare, (void *)_StrNCompare);

		store->StrNCaselessCompare =
		    (Int16 (*)(const Char *, const Char *, Int32))
		    GetAndSetTrapAddress(sysTrapStrNCaselessCompare, (void *)_StrNCaselessCompare);

		store->GetCharSortValue =
		    (UInt8 *(*)(void))GetAndSetTrapAddress(sysTrapGetCharSortValue, (void *)_GetCharSortValue);

		store->GetCharCaselessValue =
		    (UInt8 *(*)(void))GetAndSetTrapAddress(sysTrapGetCharCaselessValue, (void *)_GetCharCaselessValue);

		UInt32 value;
		err = FtrGet(sysFtrCreator, sysFtrNumIntlMgr, &value);
		if (0 == err && 0 != value) {
			setupDispatchTable(store);
		}

		store->trapPatched = true;

		store->on = true;
	
		SavePref(store);

		// Resort the records order.
		NotifySort();
	}
#endif	
}


void RestoreTraps()
{
#ifndef THIS_IS_DA	
	ftrSave *store = NULL;
	Err err = FtrGet(CREATOR,0,(UInt32*)(&store));

	if (err!=0 || store == NULL) return;

	if (store->trapPatched) {
		UInt32 romVersion;
		FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

		if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageDevelopment,0)
		        && romVersion < sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0) )
		{
			if (store->hasPalmHiRes)
			{
				GetAndSetTrapAddress(sysTrapBltDrawChars, (void*)store->BltDrawChars_HiRes);
			}
			else 
			{
				GetAndSetTrapAddress(sysTrapBltDrawChars, (void*)store->BltDrawChars);
			}
		}
		else if (romVersion >= sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0) )
		{
			GetAndSetTrapAddress(sysTrapBltDrawChars, (void*)store->BltDrawChars_v35);
		}


		GetAndSetTrapAddress(sysTrapFntLineWidth, (void*)store->FntLineWidth);
		GetAndSetTrapAddress(sysTrapFntCharsInWidth, (void*)store->FntCharsInWidth);
		GetAndSetTrapAddress(sysTrapFntCharsWidth, (void*)store->FntCharsWidth);
		GetAndSetTrapAddress(sysTrapFntCharWidth, (void*)store->FntCharWidth);
		GetAndSetTrapAddress(sysTrapFntWordWrap, (void*)store->FntWordWrap);
		GetAndSetTrapAddress(sysTrapFntWidthToOffset, (void*)store->FntWidthToOffset);
		//GetAndSetTrapAddress(sysTrapFldHandleEvent, (void*)store->FldHandleEvent);
		GetAndSetTrapAddress(sysTrapStrCompare, (void *)store->StrCompare);
		GetAndSetTrapAddress(sysTrapStrCaselessCompare, (void *)store->StrCaselessCompare);
		GetAndSetTrapAddress(sysTrapStrNCompare, (void *)store->StrNCompare);
		GetAndSetTrapAddress(sysTrapStrNCaselessCompare, (void *)store->StrNCaselessCompare);
		GetAndSetTrapAddress(sysTrapGetCharSortValue, (void *)store->GetCharSortValue);
		GetAndSetTrapAddress(sysTrapGetCharCaselessValue, (void *)store->GetCharCaselessValue);

		UInt32 value;
		err = FtrGet(sysFtrCreator, sysFtrNumIntlMgr, &value);
		if (0 == err && 0 != value) {
			cleanDispatchTable(store);
		}

		// Reset the records order
		NotifySort();

		store->trapPatched = false;
		store->on = false;

		SavePref(store);
	}


#endif	

}


void SwitchToHiRes(UInt16 refNum)
{
	UInt32 width, height;
	width = hrWidth ; height = hrHeight ;
	Err error = HRWinScreenMode ( refNum, winScreenModeSet, &width, &height, NULL, NULL );
	if ( error != errNone ){
		//  Screen mode remains unchanged.
		ErrFatalDisplay("Cannot change to Hi-Res mode!");
	}
}

Boolean SonyHROpen(UInt16 &refNum, UInt16 &version)
{
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error = 0;
	if ((error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		//  Not CLIE: maybe not available
		return false;
	} else {
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR) {
			// HR available
			if ((error = SysLibFind(sonySysLibNameHR, &refNum))) {
				if (error == sysErrLibNotFound) {
					// couldn't find lib
					error = SysLibLoad( 'libr', sonySysFileCHRLib, &refNum );
				}
			}
			if (!error ) {
				// Now we can use HR lib
				error = HROpen(refNum);
				if (!error)
				{
					HRGetAPIVersion(refNum, &version);
					return true;
				}
			}
		}
	}
	return false;
}

void SonyHRClose()
{
	ftrSave *store = GetGBKOSBase();

	if(store && store->hasSonyHR && store->HRLibRef != NULL) {
		HRClose(store->HRLibRef);
		store->HRLibRef = NULL;
		store->HRVersion = 0;
	}
}

#define SONY_HIRES 1
#define SONY_NOSCALING 2

static UInt8 GetHRFlag(UInt16 version)
{
	UInt8 retv = 0;

	UInt8 ***base = (UInt8 ***)772;

	if (version > 0x100) // HR lib version is bigger than 1.0
	{
		retv = base[0][12][7];
	}
	else
	{
		if (base[0][12][7] == 1)
		{
			retv |= SONY_HIRES;
		}
		if (base[0][12][9] == 1)
		{
			retv |= SONY_NOSCALING;
		}
	}
	return retv;
}

Boolean isHiRes(UInt16 version)
{
	UInt8 flag;
	flag = GetHRFlag(version);
	return (flag & SONY_HIRES) != 0;
}


Boolean isAbsolute(UInt16 version)
{
	UInt8 flag;
	flag = GetHRFlag(version);
	return (flag &  (SONY_HIRES|SONY_NOSCALING) ) == SONY_HIRES|SONY_NOSCALING;
}


Boolean isScaled(UInt16 version)
{
	UInt8 flag;
	flag = GetHRFlag(version);
	return (flag & (SONY_HIRES|SONY_NOSCALING) ) == SONY_HIRES;
}

UInt16 GetFont(ftrSave *store)
{
	UInt16 retv;
	if (store && store->hasSonyHR && store->HRLibRef != NULL) {
		retv = (UInt16)HRFntGetFont(store->HRLibRef);
	}
	else {
		retv = (UInt16)FntGetFont();
	}
	return retv;
}

DmOpenRef OpenDatabaseByName(char *name, UInt16 mode)
{
	DmOpenRef retv = NULL;
	UInt16 cards = MemNumCards();
	for (UInt16 i=0; i<cards; i++) {
		LocalID dbID = DmFindDatabase(i, name);
		if (dbID!=0) {
			retv = DmOpenDatabase(i, dbID, mode);
			break;
		}
	}
	return retv;
}

Boolean DatabaseExists(char *name)
{
	Boolean retv = false;
	UInt16 cards = MemNumCards();
	for (UInt16 i=0; i<cards; i++) {
		LocalID dbID = DmFindDatabase(i, name);
		if (dbID!=0) {
			retv = true;
			break;
		}
	}
	return retv;
}

MemPtr PermanentPtrNew(UInt32 size)
{
	MemPtr retv = MemPtrNew(size);
	if (retv) {
		if (MemPtrSetOwner(retv, 0)!=errNone) {
			MemPtrFree(retv);
			retv = NULL;
		}
	}
	MemSet(retv, size, 0);
	return retv;
}

void DrawPopupMessage(const Char *pszMsg, WinHandle *winBackupP, PointType *pntBackupP, Int32 nDelay)
{
	Int16 strLen = StrLen(pszMsg);

	Int16 LineWidth = FntLineWidth(pszMsg, strLen);
	Int16 LineHeight = FntLineHeight();

	Int16 nPadding = 5;
	Int16 nOutter = 3;

	Int16 winWidth, winHeight;
	WinGetWindowExtent(&winWidth, &winHeight);

	RectangleType rec;

	rec.extent.x = LineWidth + nPadding * 2;
	rec.extent.y = LineHeight + nPadding * 2;
	rec.topLeft.x = (winWidth - rec.extent.x) / 2;
	rec.topLeft.y = (winHeight - rec.extent.y) / 2;

	RectangleType recBackup;
	recBackup.topLeft.x = rec.topLeft.x - nOutter;
	recBackup.topLeft.y = rec.topLeft.y - nOutter;
	recBackup.extent.x = rec.extent.x + nOutter * 2;
	recBackup.extent.y = rec.extent.y + nOutter * 2;

	Int16 txtX = (winWidth - LineWidth) / 2;
	Int16 txtY = (winHeight - LineHeight) / 2;

	Err err;
	*winBackupP = WinSaveBits(&recBackup, &err);

	WinEraseRectangle(&rec, 0);
	WinDrawRectangleFrame(popupFrame, &rec);

	WinDrawChars(pszMsg, strLen, txtX, txtY);

	*pntBackupP = recBackup.topLeft;

	if (nDelay != 0)
		SysTaskDelay(nDelay);
}

void ErasePopupMessage(WinHandle winBackup, PointType pntBackup)
{
	WinRestoreBits(winBackup, pntBackup.x, pntBackup.y);
}
