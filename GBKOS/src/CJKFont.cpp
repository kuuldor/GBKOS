#include <PalmOS.h>

#include "CJKFont.h"
#include "Encoding.h"
#include "PalmHiRes.h"

void CJKFont::Destroy()
{
	if (handles != NULL) {
		for (UInt16 i=0; i<blkCnt; i++) {
			if (handles[i] != NULL)
				MemHandleUnlock(handles[i]);
		}
		MemPtrFree(handles);
	}
	if (blocks != NULL) {
		MemPtrFree(blocks);
	}
	if (dbRef != NULL) {
		DmCloseDatabase(dbRef);
	}
	loaded = false;
}


UInt16 EUC2CharOffset(UInt16 charset, const char*s)
{
	UInt16 retv = 0;
	if (!canBeCJK1stByte(s[0])) return 0;
	if (!canBeCJK2ndByte(s[1])) return 0;
	if (charset == 0)
	{
#ifndef _GB2312_
		retv = ((unsigned char)s[0]-0x81)*0x00BF+(unsigned char)s[1]-0x40;
#else
		retv = ((unsigned char)s[0]-0xA1)*0x005E+(unsigned char)s[1]-0xA1;
#endif
	} else if (charset == 1) {
		UInt16 tmp;
		if ((unsigned char)s[1] >= 0xA1)
			tmp = (unsigned char)s[1] - 0x62;
		else
			tmp = (unsigned char)s[1] - 0x40;

		retv = ((unsigned char)s[0]-0xA1)*0x009D + tmp;
	}
	return retv;
}

unsigned char* CJKFont::getRawFontByChar(const char* s)
{

	UInt16 offset = EUC2CharOffset(charset, s);

	UInt16 blk = offset / blkSize;

	if (blk >= blkCnt)
	{
		ErrFatalDisplay("Cannot locate the character!");
	}

	UInt16	pos = offset % blkSize;

	return &blocks[blk][pos*charSize];
}

Boolean CJKFont::loadFont(UInt16 chs, char* fontName)
{
	charset = chs;

	switch (metric) {
	case 10:
	case 12:
		blkSize = 0x800;
		if (charset == 0)
		{
#ifndef _GB2312_		
			blkCnt = 0xc;
#else
			blkCnt = 0x4;
#endif
		} else if (charset == 1) {
			blkCnt = 0x7;
		}
		break;
	case 16:
	case 24:
		if (charset == 0)
		{
#ifndef _GB2312_		
			blkSize = 0x300;
			blkCnt = 0x20;
#else
			blkSize = 0x200;
			blkCnt = 0x10;
#endif		
		} else if (charset == 1) {
			blkSize = 0x200;
			blkCnt = 0x1c;
		}
		break;
	}

	dbRef = OpenDatabaseByName(fontName, dmModeReadOnly|dmModeLeaveOpen);

	if (dbRef == NULL)
		return false;

	blocks = (unsigned char**)PermanentPtrNew(blkCnt*sizeof(unsigned char*));

	handles = (MemHandle*)PermanentPtrNew(blkCnt*sizeof(MemHandle));

	UInt16 resCnt = DmNumResources(dbRef);

	if (blocks == NULL || handles ==NULL || resCnt != blkCnt) {
		//fail to alloc mem , or font is not GBK font , perhaps GB font
		DmCloseDatabase(dbRef);
		dbRef = NULL;
		return false;
	}

	MemHandle hdl;

	for (UInt16 i=0; i<blkCnt; i++)
	{
		hdl = DmGetResourceIndex(dbRef, i);

		if (hdl == NULL)
			return false;

		blocks[i] =(unsigned char*) MemHandleLock(hdl);
		handles[i] = hdl;
	}

	return true;
}

Boolean CJKFontManager::loadAllFontsForCharset(ftrSave *store, UInt16 charset)
{
	if (NULL == store) return false;

	char *encodings[ENCODING_NO]  = {"ChineseGB","ChineseBIG5"};
	int fontmetric[FONT_NO] = {10, 12, 16, 24};
	int fontsize[FONT_NO] = {13, 18, 32, 72};

	char buf[32];

	CJKFont *fnt;

	UInt16 fontCnt = 0;
	int j;
	for (j=0; j<FONT_NO; j++)
	{
		//if (j > 1 && !store->hasSonyHR && !store->hasPalmHiRes) continue;
		
		fnt = (CJKFont*)PermanentPtrNew(sizeof(CJKFont));
		if (NULL == fnt) return false;
		fnt->Init(fontmetric[j], fontsize[j]);
		MemSet(buf, sizeof(buf), 0);
		StrPrintF(buf, "CJK Font %dx%d %s", fontmetric[j], fontmetric[j], encodings[charset]);
		if (!fnt->loadFont(charset, buf)) {
			MemPtrFree(fnt);
			//releaseAllFontsForCharset(charset);
			//break;
			continue;
		}
		font[charset][j] = fnt;
		fontCnt++;
	}

	if (0 != fontCnt)
		return true;
	return false;
}

Boolean CJKFontManager::loadAllFonts(ftrSave *store)
{
	if (store->GB == NULL)
	{
		if (loadAllFontsForCharset(store, 0))
			store->GB = (Encoding*) 0xdeadbeef;
	}

	if (store->Big5 == NULL)
	{
		if (loadAllFontsForCharset(store, 1))
			store->Big5 = (Encoding*) 0xdeadbeef;
	}

	if (store->GB == (Encoding*)0xdeadbeef || store->Big5 == (Encoding*)0xdeadbeef )
	{
		return true;
	}

	releaseAllFonts();
	return false;
}

UInt16 CJKFontManager::getRawFontSizeByMetric(UInt16 metric) 
{

	ftrSave *store = GetGBKOSBase();
	
	Int16 charset = store->charset;

	switch (metric) {
	case 10:
		return font[charset][0]->getCharSize();
	case 12:
		return font[charset][1]->getCharSize();
	case 16:
		return font[charset][2]->getCharSize();
	case 24:
		return font[charset][3]->getCharSize();
	}
	return 0;
}

unsigned char* CJKFontManager::getRawFontByMetricAndChar(UInt16 metric, const char* s)
{
	ftrSave *store = GetGBKOSBase();
	Int16 charset = store->charset;
	char buf[2];
	if (charset == 0)
	{
		buf[0] = s[0];
		buf[1] = s[1];
	}
	else if (charset == 1)
	{
		if (store->Big5 == (Encoding *) 0xdeadbeef)
		{
			buf[0] = s[0];
			buf[1] = s[1];
		}
		else
		{
			store->Big5->Convert2GBK((unsigned char*)s, (unsigned char*)buf);
			charset = 0;
		}
	}

	switch (metric) {
	case 10:
		return font[charset][0]->getRawFontByChar(buf);
	case 12:
		return font[charset][1]->getRawFontByChar(buf);
	case 16:
		return font[charset][2]->getRawFontByChar(buf);
	case 24:
		return font[charset][3]->getRawFontByChar(buf);
	}
	return NULL;
}

void CJKFontManager::releaseAllFonts()
{
	for (int j=0; j<ENCODING_NO; j++)
		for (int i=0; i<FONT_NO; i++)
		{
			if (font[j][i] != NULL) {
				font[j][i]->Destroy();
				MemPtrFree(font[j][i]);
				font[j][i] = NULL;
			}
		}
}

void CJKFontManager::releaseAllFontsForCharset(UInt16 charste)
{
	for (int i=0; i<FONT_NO; i++)
	{
		if (font[charste][i] != NULL) {
			font[charste][i]->Destroy();
			MemPtrFree(font[charste][i]);
			font[charste][i] = NULL;
		}
	}
}


void CJKFontManager::getMetricByFontID(ftrSave *store, UInt16 fontID, UInt16 &metric, UInt16 &pad, UInt16 &boldPad, Boolean forceHR)
{
	Int16 charset = store->charset;
	char msg[64];
	
	if (charset == 1)
	{
		if (store->Big5 != (Encoding *) 0xdeadbeef)
		{
			charset = 0;
		}
	}
	

	switch (fontID) {
	case 15:
	case 10:
		metric = 24,pad=2;
		boldPad = 2;
		if (font[charset][3] != NULL)
		  break;
	case 9:
	case 8:
	case 11:
	case 12:
	case 13:
	case 14:
		metric = 16,pad=2;
		boldPad = 2;
		break;
	case 7:
	case 2:
		if (store->hasPalmHiRes && forceHR)
		{
			metric = 24,pad=2;
			boldPad = 2;
			if (font[charset][3] != NULL)
			  break;
		}
		else 
		{
			metric = 12,pad=1;
			boldPad = 1;
			if (font[charset][1] != NULL)
			  break;
		}
	case 1:
	case 0:
	case 3:
	case 4:
	case 5:
	case 6:
		if ((store->hasSonyHR && isScaled(store->HRVersion)))
		{
			if (!forceHR) 
			{
				metric = 8,pad=1;
				boldPad = 1;
			}
			else
			{
				metric = 10,pad=1;
				boldPad = 1;
			}
		}
		else if (store->hasPalmHiRes)
		{
			if(forceHR)
			{
				metric = 16,pad=2;
				boldPad = 2;
			}
			else
			{
				BitmapTypeV3 *bitmap =(BitmapTypeV3 *) WinGetBitmap(WinGetDrawWindow());
				if (bitmap->version != 3 || bitmap->density != kDensityDouble) 
				{
					metric = 10,pad=1;
					boldPad = 1;
				}
				else
				{
					metric = 8,pad=1;
					boldPad = 1;
				}
			}
		}
		else
		{
			metric = 10,pad=1;
			boldPad = 1;
		}
		break;
	case 231:
	case 226:
		metric = 16,pad=2;
		boldPad = 2;
		break;
	case 227:
	case 228:
	case 225:
	case 224:
		if (store->hasHE330QVGA)
		{
			metric = 12,pad=1;
			boldPad = 1;
			if (font[charset][1] != NULL)
			  break;
		}
	default:
		StrPrintF(msg, "Font ID: %d", fontID);
		ErrDisplay(msg);
		if ((store->hasSonyHR && isScaled(store->HRVersion)))
		{
			if (!forceHR) 
			{
				metric = 8,pad=1;
				boldPad = 1;
			}
			else
			{
				metric = 10,pad=1;
				boldPad = 1;
			}
		}
		else if (store->hasPalmHiRes)
		{
			if(forceHR)
			{
				metric = 16,pad=2;
				boldPad = 2;
			}
			else
			{
				BitmapTypeV3 *bitmap =(BitmapTypeV3 *) WinGetBitmap(WinGetDrawWindow());
				if (bitmap->version != 3 || bitmap->density != kDensityDouble) 
				{
					metric = 10,pad=1;
					boldPad = 1;
				}
				else
				{
					metric = 8,pad=1;
					boldPad = 1;
				}
			}
		}
		else
		{
			metric = 10,pad=1;
			boldPad = 1;
		}


		break;

	}
}

