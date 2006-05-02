


#include "common.h"

#include "CJKFont.h"


#include <PalmOS.h>

#include "PalmHiRes.h"

void getCJKFontBitmap(ftrSave* store, const char* s, UInt16 metric, UInt16 fheight, unsigned char* bitmap, Boolean isBold)
{
	CJKFontManager *fntMan = store->fontManager;

	unsigned char *rawFont = NULL;

	rawFont = fntMan->getRawFontByMetricAndChar(metric, s);

	if (fheight > metric)
		bitmap += 4*((fheight-metric)/2);

	UInt16 i,j;
	//UInt16 m2 = metric*2;
	UInt16 tmp;
	switch (metric) {
	case 10:
		for (i=0;i <metric; i++) {
			/*
			bitmap[i*4] = rawFont[i];
			bitmap[i*4+1]=(rawFont[metric+i/4]<<((i%4)*2))&0xC0;
			*/
			tmp = rawFont[i];
			tmp <<= 8;
			tmp |= (rawFont[metric+i/4]<<((i%4)*2))&0xC0;
			if (isBold && store->bold)
				tmp |= tmp>>1;
			*(UInt16*)(bitmap+i*4) = tmp;
		}
		break;
	case 12:
		for (i=0,j=0;i <metric; i+=2,j+=3) {

			tmp = rawFont[j];
			tmp <<= 8;
			tmp |= (rawFont[j+1]&0xF0);
			if (isBold && store->largeBold)
				tmp |= tmp>>1;
			*(UInt16*)(bitmap+i*4) = tmp;
			tmp = rawFont[j+1]&0x0F;
			tmp <<=12;
			tmp |= rawFont[j+2]<<4;
			if (isBold && store->largeBold)
				tmp |= tmp>>1;
			*(UInt16*)(bitmap+(i+1)*4) = tmp;
			/* /
			bitmap[i*4] = rawFont[i];
			bitmap[i*4+1]=(rawFont[metric+i/2]<<((i%2)*4))&0xF0;
			*/
		}
		break;
	case 16:
		for (i=0;i <metric; i++) {
			tmp = rawFont[i*2];
			tmp <<= 8;
			tmp |= rawFont[i*2+1];
			if (isBold && store->bold)
				tmp |= tmp>>1;
			*(UInt16*)(bitmap+i*4) = tmp;
			/* /
			bitmap[i*4] = rawFont[i];
			bitmap[i*4+1]= rawFont[i+metric];
			*/
		}
		break;
	case 24:
		for (i=0;i <metric; i++) {

			bitmap[i*4] = rawFont[i*3];
			bitmap[i*4+1]= rawFont[i*3+1];
			bitmap[i*4+2] = rawFont[i*3+2];
			if (isBold && store->largeBold)
				*((UInt32*)(bitmap+i*4)) |= (*((UInt32*)(bitmap+i*4)))>>1;
			/* /
			bitmap[i*4] = rawFont[i];
			bitmap[i*4+1]= rawFont[i+metric];
			bitmap[i*4+2] = rawFont[i+m2];
			*/

		}
		break;
	}
}


UInt16 makeCJKFont(ftrSave* store, const char* s, FontPtr font, FontID fontId, FontPtr newFont)
{
	
	UInt16 metric, xpad, metricPad;
	UInt16 fheight = font->fRectHeight;

	if (store->hasSonyHR && store->HRVersion <=0x100
	        && isHiRes(store->HRVersion) && isScaled(store->HRVersion) )
	{
		fheight *= 2;
	}
	

	/*
	if (fheight < 12) metric = 10,xpad=1,metricPad = 1;
	else if (fheight < 16) metric = 12,xpad=1,metricPad = 1;
	else if (fheight < 24) metric = 16,xpad=2,metricPad = 2;
	else metric = 24,xpad=2,metricPad = 2;
	*/
	store->fontManager->getMetricByFontID(store, fontId, metric, xpad, metricPad, true);

	Boolean isBold=false;
	switch (fontId) {
	case 1:
	case 9:
	case 225:
	case 227:
		if (store->bold)
		{
			isBold = true;
		}
		break;
	case 7:
	case 15:
	case 229:
	case 231:
		if (store->largeBold)
		{
			isBold = true;
		}
		break;
	default:
		break;
	}
	
	if(isBold) 
	{
		xpad *= 2;
	}
	else 
	{
		metricPad = 0;
	}
	
	if (!store->padding) 
	{
		xpad=0;
	}
	
	newFont->maxWidth = metric + xpad + metricPad;
	newFont->fRectWidth  = metric + xpad + metricPad;
	newFont->fRectHeight   = fheight;
	newFont->ascent   = fheight;
	newFont->rowWords   = 2;
	

	unsigned char * fontBitmap = (unsigned char*)(newFont+1);
	UInt16 *locTable = (UInt16*)(fontBitmap+4*fheight);
	UInt16 *offTable = locTable+2;

	getCJKFontBitmap(store, s, metric,fheight, fontBitmap, isBold);

	locTable[0] = 0;
	locTable[1] = newFont->maxWidth;
	offTable[0] = newFont->maxWidth;

	return newFont->maxWidth;
}


UInt16 makeCJKFontV2(ftrSave* store, const char* s, FontPtr font, FontID fontId,UInt16 density, FontTypeV2 *newFont)
{
	
	UInt16 metric, xpad, metricPad;
	UInt16 fheight = font->fRectHeight;
	UInt16 scale = 1;

	//if (fheight < 12) metric = 10,xpad=1,metricPad = 1;
	//else if (fheight < 16) metric = 12,xpad=1,metricPad = 1;
	//else 
	/*
	if (fheight < 12)
	{
		if (kDensityLow == density)
		{
			metric = 10,xpad=1,metricPad = 1;
		}
		else 
		{
			metric = 16,xpad=2,metricPad = 2;
			scale = 2;
		}
	}
	else 
	{
		if (kDensityLow == density)
		{
			metric = 12,xpad=1,metricPad = 1;
		}
		else 
		{
			metric = 24,xpad=2,metricPad = 2;
			scale = 2;
		}
	}
	*/
	if (kDensityLow == density)
	{
	  store->fontManager->getMetricByFontID(store, fontId, metric, xpad, metricPad, false);
	}
	else 
	{
	  store->fontManager->getMetricByFontID(store, fontId+8, metric, xpad, metricPad, true);
	  scale = 2;
	}

	Boolean isBold=false;
	switch (fontId) {
	case 1:
		if (store->bold)
		{
			isBold = true;
		}
		break;
	case 7:
		if (store->largeBold)
		{
			isBold = true;
		}
		break;
	default:
		break;
	}
	
	if(isBold) 
	{
		xpad *= 2;
	}
	else 
	{
		metricPad = 0;
	}
	
	if (!store->padding) 
	{
		xpad=0;
	}
	
	newFont->fontType = 0x9200;		// Application defined Extended Font
	newFont->maxWidth = (metric + xpad + metricPad) / scale;
	newFont->fRectWidth  = (metric + xpad + metricPad) / scale;
	newFont->fRectHeight   = fheight;
	newFont->ascent   = fheight;
	newFont->rowWords   = 2 / scale;
	newFont->version = 1;
	newFont->densityCount = 2;
	newFont->densities[0].density = kDensityLow; 
	newFont->densities[0].glyphBitsOffset = sizeof(FontTypeV2) + 6; 
	newFont->densities[1].density = kDensityDouble; 
	newFont->densities[1].glyphBitsOffset = sizeof(FontTypeV2) + 6; 

	UInt16 *locTable = (UInt16*)(newFont+1);
	UInt16 *offTable = locTable+2;
	unsigned char * fontBitmap =(unsigned char*) (offTable + 1);

	getCJKFontBitmap(store, s, metric,fheight*scale, fontBitmap, isBold);

	locTable[0] = 0;
	locTable[1] = newFont->maxWidth;
	offTable[0] = newFont->maxWidth;

	return newFont->maxWidth;
}

typedef struct {
	BitmapType * dstBitmapP;
	DrawStateType * drawStateP;
} v35_WinType;


Int16 DrawCJKChar(
    ftrSave *store, void *winptr, const RectangleType &drawRect, const RectangleType &clipRect,
    const char* s, FontPtr font,UInt32 fontTable = 0)
{

	char ch =0;
	FontPtr cjkFont = (FontPtr)(store->FontBuffer);
	MemSet(cjkFont, sizeof(store->FontBuffer), 0);

	UInt16 fmetric;
	if (store->BltDrawChars)
	{
		fmetric = makeCJKFont(store, s, (*(DrawStateType**)((char*)winptr+12))->font, 
			(*(DrawStateType**)((char*)winptr+12))->fontId, cjkFont);
	}
	else if (store->BltDrawChars_v35)
	{
		fmetric = makeCJKFont(store, s, font, ((v35_WinType*)winptr)->drawStateP->fontId, cjkFont);
	}
	else
	{
		fmetric = makeCJKFontV2(store, s, (*(DrawStateType**)((char*)winptr+8))->font, 
			(*(DrawStateType**)((char*)winptr+8))->fontId,
			(*(BitmapTypeV3**)((char*)winptr+12))->density,  
			(FontTypeV2 *)cjkFont);
	}

	RectangleType drawIn, clipOff;

	drawIn = drawRect;
	clipOff= clipRect;


	if (store->BltDrawChars)
	{
		store->BltDrawChars(winptr,&drawIn, &clipOff, &ch, 1, cjkFont, NULL);
	}
	else if (store->BltDrawChars_v35)
	{
		v35_WinType * tmpWinPtr = (v35_WinType *)winptr;
		store->BltDrawChars_v35(tmpWinPtr->dstBitmapP, tmpWinPtr->drawStateP,
		                        drawIn.topLeft.x, drawIn.topLeft.y, drawIn.extent.x, drawIn.extent.y,
		                        clipOff.topLeft.y, clipOff.topLeft.x, clipOff.extent.y, clipOff.extent.x,
		                        &ch, 1, cjkFont);
	}
	else if (store->BltDrawChars_HiRes)
	{
		store->BltDrawChars_HiRes(winptr,drawIn.topLeft.x, drawIn.topLeft.y, &ch, 1, cjkFont, NULL);
		if(kDensityLow ==(*(BitmapTypeV3**)((char*)winptr+12))->density)  
			return fmetric ;
		else
			return fmetric * 2;
	}

	if (store->hasHE330QVGA)
		return fmetric;
		
	if (fmetric>=16 && 
		(Int16)fmetric>(drawRect.extent.y+clipRect.topLeft.y+clipRect.extent.y))
			return fmetric/2;

	return fmetric;
}

void _BltDrawChars(
    void *winptr, RectangleType *drawRect, RectangleType *clipRect,
    const Char* const s, UInt16 len, FontPtr font, UInt32 fontTable)
{

	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on || len < 2) {
		return store->BltDrawChars(winptr, drawRect, clipRect, s,len, font, fontTable);
	}

	RectangleType drawIn, clipOff;

	drawIn = *drawRect;
	clipOff= *clipRect;

	Int16 wid;

	for (UInt16 i=0; i<len; i++) {
		if (isCJKChar(s+i)) {
			wid = DrawCJKChar(store, winptr, drawIn, clipOff, s+i, font, fontTable);
			i++;
		} else {
			int firstLatin1 = i;
			i += FindNextCJK(s+i+1, len-i-1) ;

			RectangleType dI, cO;
			dI =drawIn;cO=clipOff;
			store->BltDrawChars(winptr, &dI, &cO, s+firstLatin1,i+1-firstLatin1, font, fontTable);
			wid = store->FntCharsWidth(s+firstLatin1, i+1-firstLatin1);
			Int16 hig = FntCharHeight();
			if (hig>=16 && hig>(drawIn.extent.y+clipOff.topLeft.y+clipOff.extent.y)) wid /= 2;
		}
		if (clipOff.topLeft.x !=0)
		{
			if (wid < clipOff.topLeft.x)
			{
				clipOff.topLeft.x -= wid;
			}
			else
			{
				drawIn.topLeft.x += wid - clipOff.topLeft.x;
				drawIn.extent.x -= wid - clipOff.topLeft.x;
				clipOff.topLeft.x = 0;
			}
		}
		else
		{
			drawIn.topLeft.x += wid;
			drawIn.extent.x -= wid;
		}

	}

}


void _BltDrawChars_v35(BitmapType * dstBitmapP, DrawStateType * drawStateP,
                       Int16 toX, Int16 toY, Int16 xExtent, Int16 yExtent,
                       Int16 clipTop, Int16 clipLeft, Int16 clipBottom,  Int16  clipRight ,
                       const Char * const s, UInt16 len, FontPtr fontP)
{

	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on || len < 2) {
		return store->BltDrawChars_v35(dstBitmapP, drawStateP, toX, toY, xExtent, yExtent,clipTop, clipLeft, clipBottom, clipRight , s, len, fontP);
	}

	RectangleType drawIn, clipOff;

	drawIn.topLeft.x = toX; drawIn.topLeft.y = toY; drawIn.extent.x = xExtent;  drawIn.extent.y = yExtent;
	clipOff.topLeft.x = clipLeft; clipOff.topLeft.y = clipTop; clipOff.extent.x = clipRight; clipOff.extent.y = clipBottom;

	Int16 wid;
	v35_WinType win;
	win.dstBitmapP = dstBitmapP;
	win.drawStateP = drawStateP;

	for (UInt16 i=0; i<len; i++) {
		if (isCJKChar(s+i)) {
			wid = DrawCJKChar(store, (void*)&win, drawIn, clipOff, s+i, fontP);
			i++;
		} else {
			int firstLatin1 = i;
			i += FindNextCJK(s+i+1, len-i-1) ;

			store->BltDrawChars_v35(dstBitmapP, drawStateP,
			                        drawIn.topLeft.x, drawIn.topLeft.y, drawIn.extent.x, drawIn.extent.y,
			                        clipOff.topLeft.y, clipOff.topLeft.x, clipOff.extent.y, clipOff.extent.x,
			                        s+firstLatin1, i+1-firstLatin1, fontP);

			wid = store->FntCharsWidth(s+firstLatin1, i+1-firstLatin1) ;
			Int16 hig = FntCharHeight();
			if (hig>=16 && hig>(drawIn.extent.y+clipOff.topLeft.y+clipOff.extent.y)) wid /= 2;
		}
		if (clipOff.topLeft.x !=0)
		{
			if (wid < clipOff.topLeft.x)
			{
				clipOff.topLeft.x -= wid;
			}
			else
			{
				drawIn.topLeft.x += wid - clipOff.topLeft.x;
				drawIn.extent.x -= wid - clipOff.topLeft.x;
				clipOff.topLeft.x = 0;
			}
		}
		else
		{
			drawIn.topLeft.x += wid;
			drawIn.extent.x -= wid;
		}

	}

}

void _BltDrawChars_HiRes(void *winptr, UInt16 x, UInt16 y,
    const Char* const s, UInt16 len, FontPtr font, UInt32 fontTable)
{

	ftrSave *store = GetGBKOSBase();

	if (Western == store->charset || !store->on || len < 2) {
		return store->BltDrawChars_HiRes(winptr, x, y, s,len, font, fontTable);
	}

	RectangleType drawIn, clipOff;

	drawIn.topLeft.x = x; drawIn.topLeft.y = y; drawIn.extent.x = 320;  drawIn.extent.y = 320;
	clipOff.topLeft.x = 0; clipOff.topLeft.y = 0; clipOff.extent.x = 0; clipOff.extent.y = 0;

	Int16 wid;

	for (UInt16 i=0; i<len; i++) {
		if (isCJKChar(s+i)) {
			wid = DrawCJKChar(store, winptr, drawIn, clipOff, s+i, font);
			i++;
		} else {
			int firstLatin1 = i;
			i += FindNextCJK(s+i+1, len-i-1) ;

			store->BltDrawChars_HiRes(winptr,drawIn.topLeft.x, drawIn.topLeft.y,
			                        s+firstLatin1, i+1-firstLatin1, font, fontTable);

			wid = store->FntCharsWidth(s+firstLatin1, i+1-firstLatin1)  ;
			Int16 hig = FntCharHeight();
			if ((kDensityDouble ==(*(BitmapTypeV3**)((char*)winptr+12))->density) && (hig < 16) )
			wid *= 2;
		}
		if (clipOff.topLeft.x !=0)
		{
			if (wid < clipOff.topLeft.x)
			{
				clipOff.topLeft.x -= wid;
			}
			else
			{
				drawIn.topLeft.x += wid - clipOff.topLeft.x;
				drawIn.extent.x -= wid - clipOff.topLeft.x;
				clipOff.topLeft.x = 0;
			}
		}
		else
		{
			drawIn.topLeft.x += wid;
			drawIn.extent.x -= wid;
		}

	}

}

