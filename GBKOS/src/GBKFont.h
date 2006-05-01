#ifndef GBK_FONT_H_INCLUDED
#define GBK_FONT_H_INCLUDED

#include <PalmOS.h>
#include <SonyCLIE.h>

#include "common.h"

class GBKFont {
protected:
	UInt16 metric;
	UInt16 charSize;
public:
	GBKFont(UInt16 mtrc): metric(mtrc){};
	virtual ~GBKFont(){};

	UInt16 getCharSize(){return charSize;};
	void setCharSize(UInt16 size) {charSize=size;};
	virtual unsigned char*getRawFontByChar(const char* s)=0;
};

class GBKFontManager {
protected:
	GBKFont *fnt10;
	GBKFont *fnt12;
	GBKFont *fnt16;
	GBKFont *fnt24;
	Boolean fontAvailable;
public:
	GBKFontManager() {fnt10=fnt12=fnt16=fnt24=NULL;};
	virtual ~GBKFontManager(){releaseAllFonts();};

	virtual Boolean loadAllFonts(){return false;};
	virtual void releaseAllFonts(){};

	UInt16 getRawFontSizeByMetric(UInt16 metric) {
		switch (metric) {
		case 10:
			return fnt10->getCharSize();
		case 12:
			return fnt12->getCharSize();
		case 16:
			return fnt12->getCharSize();
		case 24:
			return fnt12->getCharSize();
		}
		return 0;
	};

	unsigned char*getRawFontByMetricAndChar(UInt16 metric, const char* s) {
		switch (metric) {
		case 10:
			return fnt10->getRawFontByChar(s);
		case 12:
			return fnt12->getRawFontByChar(s);
		case 16:
			return fnt12->getRawFontByChar(s);
		case 24:
			return fnt12->getRawFontByChar(s);
		}
		return NULL;
	};
};

#endif
