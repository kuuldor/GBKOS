#ifndef CJKFONT_H_INCLUDED
#define CJKFONT_H_INCLUDED

#include <PalmOS.h>
//#include <SonyCLIE.h>

#include "common.h"


#define ENCODING_NO		2
#define FONT_NO			4

struct CJKFont  {
private:
	UInt16 charset;
	UInt16 metric;
	UInt16 charSize;
	Boolean loaded;
	UInt16 blkCnt;
	UInt16 blkSize;
	unsigned char**blocks;
	MemHandle *handles;
	DmOpenRef dbRef;
public:
	void Init(UInt16 mtrc, UInt16 size){metric = mtrc; charSize=size;loaded = false;dbRef=NULL;};
	void Destroy();

	unsigned char*getRawFontByChar(const char* s);
	Boolean loadFont(UInt16 charset, char* fontName);
	UInt16 getCharSize(){return charSize;};
	void setCharSize(UInt16 size) {charSize=size;};
};

struct CJKFontManager {
private:
	CJKFont *font[ENCODING_NO][FONT_NO];
public:
	void Init() {font[1][0]=font[1][1]=font[1][2]=font[1][3]=font[0][0]=font[0][1]=font[0][2]=font[0][3]=NULL;};
	void Destroy(){releaseAllFonts();};

	Boolean loadAllFonts(ftrSave *store);
	Boolean loadAllFontsForCharset(ftrSave *store, UInt16 charset);
	void releaseAllFonts();
	void releaseAllFontsForCharset(UInt16 charset);

	void getMetricByFontID(ftrSave *store, UInt16 fontID, UInt16 &metric, UInt16 &pad, UInt16 &boldPad, Boolean forceHR);
	UInt16 getRawFontSizeByMetric(UInt16 metric);
	unsigned char*getRawFontByMetricAndChar(UInt16 metric, const char* s);
};

#endif
