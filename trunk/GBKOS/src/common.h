#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#ifndef ALLOW_ACCESS_TO_INTERNALS_OF_FONTS  
#define ALLOW_ACCESS_TO_INTERNALS_OF_FONTS  
#endif

#ifndef ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS 
#define ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS 
#endif

#include <PalmOS.h>

#include <SonyCLIE.h>
#include "HE330/Trg.h"

#include "HE330/Vga.h"

#include "GBKOS_Lib.h"
#include "build.h"

#ifndef CREATOR
#define CREATOR 'dGBK'
#endif

#define BASE_PREF_ID	0
#define BASE_PREF_VER	1

struct CJKFontManager;
struct Encoding;

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

	Boolean trapPatched;

	Boolean hasSonyHR;
	UInt16 HRLibRef;
	UInt16 HRVersion;

	// below is not to be saved in preference and to be	dynamicly created
	Encoding *GB;
	Encoding *Big5;
	Encoding *Shift_JIS;
	Encoding *EUC_JIS;

	CJKFontManager * fontManager;
	
	Boolean isJap;
	Boolean hasPalmHiRes;
	Boolean hasHE330QVGA;

	void (*BltDrawChars_v35)(BitmapType * dstBitmapP, DrawStateType * drawStateP, Int16 toX, Int16 toY, Int16 xExtent, Int16 yExtent, Int16 clipTop, Int16 clipLeft, Int16 clipBottom,  Int16  clipRight , const Char * const charsP, UInt16 len, FontPtr fontP);
	void (*BltDrawChars) (void *winptr, RectangleType *drawRect, RectangleType *clipRect, const Char* const s, UInt16 len, FontPtr font, UInt32 fontTable);
	void (*BltDrawChars_HiRes) (void *winptr, UInt16 x, UInt16 y, const Char* const s, UInt16 len, FontPtr font, UInt32 fontTable);
	Int16 (*FntCharsWidth) (const char *chars,Int16 len);
	void (*FntCharsInWidth) (const char *string,Int16 *stringWidthP, Int16 *stringLengthP,Boolean *fitWithinWidth);
	Int16 (*FntLineWidth) (const char *pChars,UInt16 length);
	Int16 (*FntCharWidth) (Char ch);
	UInt16 (*FntWordWrap)(Char const *chars,UInt16 maxWidth);
	Int16 (*FntWidthToOffset) (Char const *pChars,UInt16 length, Int16 pixelWidth,Boolean *leadingEdge, Int16 *truncWidth);
	Boolean (*FldHandleEvent) (FieldType *fldP, EventType *eventP);
	Int16 (*StrCompare)(const Char *s1, const Char *s2);
	Int16 (*StrCaselessCompare)(const Char *s1, const Char *s2);
	Int16 (*StrNCompare)(const Char *s1, const Char *s2, Int32 n);
	Int16 (*StrNCaselessCompare)(const Char *s1, const Char *s2, Int32 n);
	UInt8 *(*GetCharSortValue)(void);
	UInt8 *(*GetCharCaselessValue)(void);
	void (*IntlDispatch)(void);
	void *funcTable[33];
	void *old_funcTable[33];
	char FontBuffer[512];
	UInt8 *charSortValues;
	UInt8 *charCaselessValues;
	UInt32 ftrNumEncodingBackup;
} ftrSave;

#ifndef THIS_IS_DA
// My own Trap handlers
extern "C" void _BltDrawChars_v35(BitmapType * dstBitmapP, DrawStateType * drawStateP,
	                                  Int16 toX, Int16 toY, Int16 xExtent, Int16 yExtent,
	                                  Int16 clipTop, Int16 clipLeft, Int16 clipBottom,  Int16 /* clipRight */,
	                                  const Char * const charsP, UInt16 len, FontPtr fontP);
extern "C" void _BltDrawChars(void *winptr, RectangleType *drawRect, RectangleType *clipRect,
	                              const char* s, UInt16 len, FontPtr font, UInt32 fontTable);
extern "C" void _BltDrawChars_HiRes(void *winptr, UInt16 x, UInt16 y, const Char* const s, UInt16 len,
									 FontPtr font, UInt32 fontTable);
extern "C" Int16 _FntCharsWidth(const char *chars,Int16 len);
extern "C" Int16 _FntCharWidth(Char ch);
extern "C" void _FntCharsInWidth(const char *string,Int16 *stringWidthP, Int16 *stringLengthP,Boolean *fitWithinWidth);
extern "C" Int16 _FntLineWidth(const char *pChars,UInt16 length);
extern "C" UInt16 _FntWordWrap(const char *s, UInt16 maxWidth);
extern "C" Int16 _FntWidthToOffset (Char const *s,UInt16 len, Int16 pixelWidth,Boolean *leadingEdge, Int16 *truncWidth);
extern "C" Int16 _StrCompare(const Char *s1, const Char *s2);
extern "C" Int16 _StrCaselessCompare(const Char *s1, const Char *s2);
extern "C" Int16 _StrNCompare(const Char *s1, const Char *s2, Int32 n);
extern "C" Int16 _StrNCaselessCompare(const Char *s1, const Char *s2, Int32 n);
extern "C" UInt8 *_GetCharSortValue(void);
extern "C" UInt8 *_GetCharCaselessValue(void);
extern "C" void NotifySort(void);

extern "C" Boolean _FldHandleEvent (FieldType *fldP, EventType *eventP);
extern "C" void _IntlDispatch(void);

extern "C" void **getFuncTable();
void setupDispatchTable(ftrSave *store);
void cleanDispatchTable(ftrSave *store);

#endif

Boolean isCJKChar(const char *s);
Boolean isCJKChar(WChar ch);

Boolean isOpenBracePunc(const char *s);
Boolean isCloseBracePunc(const char *s);
Boolean isCJKPunc(const char *s);

Boolean canBeCJK1stByte(unsigned char ch);
Boolean canBeCJK2ndByte(unsigned char ch);

int FindNextCJK(const char *s, int len);

UInt16 getCJKCharWidth(ftrSave* store, UInt16 font, Boolean forceHR = false);

Err InitGBKOSBase();
void ReleaseGBKOSBase();
ftrSave *GetGBKOSBase();
void SavePref(ftrSave *store);
Int16 LoadPref(ftrSave *store);

void * GetAndSetTrapAddress(UInt16 trapNum, void *proc);
void PatchTraps();
void RestoreTraps();


UInt16 GetFont(ftrSave *store);

// Sony HiRes related Functions Here
Boolean SonyHROpen(UInt16 &refNum, UInt16 &version);
void SwitchToHiRes(UInt16 refNum);
void SonyHRClose();
Boolean isHiRes(UInt16 version);
Boolean isAbsolute(UInt16 version);
Boolean isScaled(UInt16 version);

DmOpenRef OpenDatabaseByName(char *name, UInt16 mode);
Boolean DatabaseExists(char *name);
MemPtr PermanentPtrNew(UInt32 size);

void DrawPopupMessage(const Char *pszMsg, WinHandle *winBackupP, PointType *pntBackupP, Int32 nDelay = 0);
void ErasePopupMessage(WinHandle winBackup, PointType pntBackup);

void SetNumEncodingFtr(ftrSave *store);

#endif
