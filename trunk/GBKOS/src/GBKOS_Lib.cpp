#include <PalmOS.h>
#include <SystemMgr.h>

#define GBKOSLIB_TRAP(trapno)

#include "GBKOS_Lib.h"

#include "common.h"
#include "AboutDialog.h"

#ifndef THIS_IS_APP
extern "C" Err start (UInt16 refnum, SysLibTblEntryPtr entryP)
{
	extern void *jmptable();
	entryP->dispatchTblP = (MemPtr *)jmptable;
	entryP->globalsP = NULL;
	return 0;
}
#endif

typedef struct {
	UInt16 refcount;
} GBKOS_globals;

Err GBKOS_LibOpen (UInt16 refnum) 
{
	SysLibTblEntryPtr entryP = SysLibTblEntry (refnum);
	GBKOS_globals *gl = (GBKOS_globals *)entryP->globalsP;

	if (!gl) {
		/* We need to allocate space for the globals.  */
		entryP->globalsP = gl = (GBKOS_globals *)PermanentPtrNew (sizeof (GBKOS_globals));
		gl->refcount = 0;
	}

	gl->refcount++;

	return 0;
}

Err GBKOS_LibClose (UInt16 refnum) 
{
	SysLibTblEntryPtr entryP = SysLibTblEntry (refnum);
	GBKOS_globals *gl = (GBKOS_globals *)entryP->globalsP;

	if (!gl) {
		/* We're not open! */
		return 1;
	}

	/* Clean up.  */

	if (--gl->refcount == 0) {
		MemChunkFree (entryP->globalsP);
		entryP->globalsP = NULL;
	}

	return 0;
}

extern "C" Err nothing (UInt16 refnum) 
{
	return 0;
}

Err GBKOS_Init(UInt16 refNum)
{
	return InitGBKOSBase();
}

void GBKOS_Start(UInt16 refNum)
{
	WinHandle winBackup;
	PointType pntBackup;

	DrawPopupMessage("Starting GBKOS...", &winBackup, &pntBackup, SysTicksPerSecond() / 2);

	ErasePopupMessage(winBackup, pntBackup);
	
	PatchTraps();
}

void GBKOS_Stop(UInt16 refNum)
{
	WinHandle winBackup;
	PointType pntBackup;

	DrawPopupMessage("Stopping GBKOS...", &winBackup, &pntBackup, SysTicksPerSecond() / 2);

	ErasePopupMessage(winBackup, pntBackup);

	RestoreTraps();
}

void GBKOS_UnInit(UInt16 refNum)
{
	ReleaseGBKOSBase();
}

void GBKOS_Enable (UInt16 refNum)
{
	ftrSave *store = GetGBKOSBase();
	store->on = true;
	SavePref(store);
}

void GBKOS_Disable (UInt16 refNum)
{
	ftrSave *store = GetGBKOSBase();
	store->on = false;
	SavePref(store);
}

void GBKOS_GetConfig(UInt16 refNum, ftrGBK *conf)
{
	ftrSave *store = GetGBKOSBase();
	if (!store) 
	{
	  MemSet(conf, sizeof(*conf), 0);
	  return;
	}

	memcpy(conf, store, sizeof(*conf));
	if (NULL != store->GB) 
	{
		conf->hasGB = true;
	}
	else
	{
		conf->hasGB = false;
	}
	
	if (NULL != store->Big5) 
	{
		conf->hasBig5 = true;
	}
	else
	{
		conf->hasBig5 = false;
	}
	
	if (NULL != store->Shift_JIS) 
	{
		conf->hasShift_JIS = true;
	}
	else
	{
		conf->hasShift_JIS = false;
	}
	
	if (NULL != store->EUC_JIS) 
	{
		conf->hasEUC_JIS = true;
	}
	else
	{
		conf->hasEUC_JIS = false;
	}
}

void GBKOS_SetConfig(UInt16 refNum, const ftrGBK *conf)
{
	ftrSave *store = GetGBKOSBase();

	if (!store) return;
	
	store->on = conf->on;
	store->smart = conf->smart;
	store->bold = conf->bold;
	store->largeBold = conf->largeBold;
	store->padding = conf->padding;
	
	store->charset = conf->charset;

	SetNumEncodingFtr(store);

	SavePref(store);
}

#define CHARSET_NAMES	"Western","Chinese GBK","Chinese Big5","Japanese Shift-JIS", "Japanese EUC-JIS"

void GBKOS_GetCharsets(UInt16 refNum, char **names, Int16 *size)
{

	char *charsetNames[] = { CHARSET_NAMES };

	ftrSave *store = GetGBKOSBase();

	Int16 chCnt = 1;

	if (NULL != store->GB) 
	{
		chCnt++ ;
	}
	
	if (NULL != store->Big5) 
	{
		chCnt++ ;
	}
	
	if (NULL != store->Shift_JIS) 
	{
		chCnt++ ;
	}
	
	if (NULL != store->EUC_JIS) 
	{
		chCnt++ ;
	}

	if (chCnt > *size) 
	{
		*size = chCnt;
		return;
	}

	chCnt = 0;
	names[chCnt++] = charsetNames[0];

	if (NULL != store->GB) 
	{
		names[chCnt++] = charsetNames[1];
	}
	
	if (NULL != store->Big5) 
	{
		names[chCnt++] = charsetNames[2];
	}
	
	if (NULL != store->Shift_JIS) 
	{
		names[chCnt++] = charsetNames[3];
	}
	
	if (NULL != store->EUC_JIS) 
	{
		names[chCnt++] = charsetNames[4];
	}

	*size = chCnt;
}


char* GBKOS_Charset2Name(UInt16 refNum, CharSet chs)
{
	char *charsetNames[] = { CHARSET_NAMES };

	Int16 index = (Int16)chs + 1;

	if (index < 0 || index > 4) return NULL;

	return charsetNames[index];
}


CharSet GBKOS_Name2Charset(UInt16 refNum, char * name)
{
	char *charsetNames[] = { CHARSET_NAMES };

	for (Int16 i = 0; i < 5; i++)
	{
		if (StrCompare(charsetNames[i], name) == 0) 
		{
			return (CharSet)(i - 1);
		}
	}

	return Max_Charset;
}


Int16 GBKOS_Charset2ID(UInt16 refNum, CharSet chs)
{
	char *availCharsets[5];
	Int16 chCount = 5;

	GBKOS_GetCharsets(refNum, availCharsets, &chCount);

	char * chsName = GBKOS_Charset2Name(refNum, chs);

	for (Int16 i = 0; i < chCount; i++)
	{
		if (StrCompare(availCharsets[i], chsName) == 0) 
		{
			return i;
		}
	}

	return -1;
}

CharSet GBKOS_ID2Charset(UInt16 refNum, Int16 id)
{
	char *availCharsets[5];
	Int16 chCount = 5;

	GBKOS_GetCharsets(refNum, availCharsets, &chCount);

	if (chCount >= id)
	{
		return GBKOS_Name2Charset(refNum, availCharsets[id]);
	}

	return Max_Charset;
}

void GBKOS_ShowAboutDialog(UInt16 refNum, const Char *pszAppNameVer)
{
	ShowAboutDialog(pszAppNameVer);
}
