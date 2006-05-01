#ifndef GBKOS_LIB_H_INCLUDED
#define GBKOS_LIB_H_INCLUDED

#include <PalmOS.h>

#include <LibTraps.h>

#include <ErrorBase.h>

#ifndef CREATOR
#define CREATOR 'dGBK'
#endif

// define errors here
#define errFontNotFound (appErrorClass | 1)


// define Lib Trap Number here
#define GBKOSLib_Init				(sysLibTrapCustom)
#define GBKOSLib_Start				(sysLibTrapCustom+1)
#define GBKOSLib_Stop				(sysLibTrapCustom+2)
#define GBKOSLib_Enable				(sysLibTrapCustom+3)
#define GBKOSLib_Disable			(sysLibTrapCustom+4)
#define GBKOSLib_GetConfig			(sysLibTrapCustom+5)
#define GBKOSLib_SetConfig			(sysLibTrapCustom+6)
#define GBKOSLib_UnInit				(sysLibTrapCustom+7)
#define GBKOSLib_GetCharsets		(sysLibTrapCustom+8)
#define GBKOSLib_Name2Charset		(sysLibTrapCustom+9)
#define GBKOSLib_Charset2Name		(sysLibTrapCustom+10)
#define GBKOSLib_ID2Charset			(sysLibTrapCustom+11)
#define GBKOSLib_Charset2ID			(sysLibTrapCustom+12)
#define GBKOSLib_ShowAboutDialog	(sysLibTrapCustom+13)

#ifndef GBKOSLIB_TRAP
#define GBKOSLIB_TRAP(trapno)	SYS_TRAP(trapno)
#endif

#define MAX_CHARSET_NO		12345

typedef enum {
	Western		= -1,
	GBK			=  0,
	Big5		=  1,
	Shift_JIS	=  2,
	EUC_JIS		=  3,

	Max_Charset	=  MAX_CHARSET_NO
} CharSet;

typedef struct {
	UInt16 size;
	UInt32 build;

	Boolean on;
	Boolean smart;
	Boolean bold;
	Boolean largeBold;
	Boolean padding;

	UInt8 nouse;

	CharSet charset;
	
	Boolean kernel_started;
	
	Boolean hasSonyHR;
	UInt16 HRLibRef;
	UInt16 HRVersion;

	// below is not to be saved in preference and to be	dynamicly created
	Boolean hasGB;
	Boolean hasBig5;
	Boolean hasShift_JIS;
	Boolean hasEUC_JIS;

	
} ftrGBK;

#ifdef __cplusplus
extern "C" {
#endif

	Err GBKOS_LibOpen(UInt16 refNum)
	GBKOSLIB_TRAP(sysLibTrapOpen);

	Err GBKOS_LibClose(UInt16 refNum)
	GBKOSLIB_TRAP(sysLibTrapClose);

	Err GBKOS_Init(UInt16 refNum)
	GBKOSLIB_TRAP(GBKOSLib_Init);

	void GBKOS_UnInit(UInt16 refNum)
	GBKOSLIB_TRAP(GBKOSLib_UnInit);

	void GBKOS_Start(UInt16 refNum)
	GBKOSLIB_TRAP(GBKOSLib_Start);

	void GBKOS_Stop(UInt16 refNum)
	GBKOSLIB_TRAP(GBKOSLib_Stop);

	void GBKOS_Enable(UInt16 refNum)
	GBKOSLIB_TRAP(GBKOSLib_Enable);

	void GBKOS_Disable(UInt16 refNum)
	GBKOSLIB_TRAP(GBKOSLib_Disable);

	void GBKOS_GetConfig(UInt16 refNum, ftrGBK *store)
	GBKOSLIB_TRAP(GBKOSLib_GetConfig);

	void GBKOS_SetConfig(UInt16 refNum, const ftrGBK *store)
	GBKOSLIB_TRAP(GBKOSLib_SetConfig);

	void GBKOS_GetCharsets(UInt16 refNum,char **names, Int16 *size)
	GBKOSLIB_TRAP(GBKOSLib_GetCharsets);
	
	char* GBKOS_Charset2Name(UInt16 refNum, CharSet chs)
	GBKOSLIB_TRAP(GBKOSLib_Charset2Name);

	CharSet GBKOS_Name2Charset(UInt16 refNum, char * name)
	GBKOSLIB_TRAP(GBKOSLib_Name2Charset);

	Int16 GBKOS_Charset2ID(UInt16 refNum, CharSet chs)
	GBKOSLIB_TRAP(GBKOSLib_Charset2ID);

	CharSet GBKOS_ID2Charset(UInt16 refNum, Int16 id)
	GBKOSLIB_TRAP(GBKOSLib_ID2Charset);

	void GBKOS_ShowAboutDialog(UInt16 refNum, const Char *pszAppNameVer)
	GBKOSLIB_TRAP(GBKOSLib_ShowAboutDialog);
#ifdef __cplusplus
}
#endif

#endif
