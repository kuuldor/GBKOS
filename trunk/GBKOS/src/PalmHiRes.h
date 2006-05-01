#ifndef __PALMHIRES_H__
#define __PALMHIRES_H__
// Taken from Palm OS5 SDK Bitmap.h


#ifdef _cplusplus
extern "C" {
#endif

typedef struct BitmapFlagsType
{
	UInt16 	compressed:1;  				// Data format:  0=raw; 1=compressed
	UInt16 	hasColorTable:1;				// if true, color table stored before bits[]
	UInt16 	hasTransparency:1;			// true if transparency is used
	UInt16 	indirect:1;						// true if bits are stored indirectly
	UInt16 	forScreen:1;					// system use only
	UInt16	directColor:1;					// direct color bitmap
	UInt16	indirectColorTable:1;		// if true, color table pointer follows BitmapType structure
	UInt16	noDither:1;						// if true, blitter does not dither
	UInt16 	reserved:8;
} BitmapFlagsType;

// This data structure is the PalmOS 5 version 3 BitmapType.
typedef struct BitmapTypeV3
{
	// BitmapType
	Int16  				width;
	Int16  				height;
	UInt16  				rowBytes;
	BitmapFlagsType	flags;					// see BitmapFlagsType
	UInt8					pixelSize;				// bits per pixel
	UInt8					version;					// data structure version 3
	
	// version 3 fields
	UInt8					size;						// size of this structure in bytes (0x16)
	UInt8					pixelFormat;			// format of the pixel data, see pixelFormatType
	UInt8					unused;
	UInt8					compressionType;		// see BitmapCompressionType
	UInt16				density;					// used by the blitter to scale bitmaps
	UInt32				transparentValue;		// the index or RGB value of the transparent color
	UInt32				nextBitmapOffset;		// byte offset to next bitmap in bitmap family

	// if (flags.hasColorTable)
	//		{
	//		if (flags.indirectColorTable)
	//			ColorTableType* colorTableP;	// pointer to color table
	//		else
	//	  		ColorTableType	colorTable;		// color table, could have 0 entries (2 bytes long)
	//		}
	//
	// if (flags.indirect)
	//	  	void*	  bitsP;							// pointer to actual bits
	// else
	//   	UInt8	  bits[];						// or actual bits
	//
} BitmapTypeV3;
typedef BitmapTypeV3* BitmapPtrV3;

typedef struct FontDensityTag
{
	Int16 density;
	UInt32 glyphBitsOffset;
} FontDensityType;

typedef struct FontTagV2
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FONTS	// These fields will not be available in the next OS release!
{
	// first part is basically the same as NFNT FontRec
	Int16	fontType;
	Int16	firstChar;		// character code of first character
	Int16	lastChar;		// character code of last character
	Int16	maxWidth;		// widMax = maximum character width
	Int16	kernMax;			// negative of maximum character kern
	Int16	nDescent;		// negative of descent
	Int16	fRectWidth;		// width of font rectangle
	Int16	fRectHeight;	// height of font rectangle
	Int16	owTLoc;			// offset to offset/width table
	Int16	ascent;			// ascent
	Int16	descent;			// descent
	Int16	leading;			// leading
	Int16	rowWords;		// row width of bit image / 2
	
	// New fields (if fntExtendedFormatMask is set)
	Int16	version;			// 1 = PalmNewFontVersion
	Int16	densityCount;

	FontDensityType	densities[2];	// array of 1 or more records
}
#endif
FontTypeV2;

// constants used by density field
typedef enum DensityTag {
	kDensityLow				= 72,
	kDensityOneAndAHalf	= 108,
	kDensityDouble			= 144,
	kDensityTriple			= 216,
	kDensityQuadruple		= 288
} DensityType;

typedef FontPtr *FontTablePtr;

// constants used by WinSetCoordinateSystem
#define kCoordinatesNative			0
#define kCoordinatesStandard		72
#define kCoordinatesOneAndAHalf		108
#define kCoordinatesDouble			144
#define kCoordinatesTriple			216
#define kCoordinatesQuadruple		288

// selectors for WinScreenGetAttribute
typedef enum WinScreenAttrTag 
{
	winScreenWidth,
	winScreenHeight,
	winScreenRowBytes,
	winScreenDepth,
	winScreenAllDepths,
	winScreenDensity,
	winScreenPixelFormat,
	winScreenResolutionX,
	winScreenResolutionY
} WinScreenAttrType;

// high density trap selectors
#define HDSelectorBmpGetNextBitmapAnyDensity		0
#define HDSelectorBmpGetVersion						1
#define HDSelectorBmpGetCompressionType				2
#define HDSelectorBmpGetDensity						3
#define HDSelectorBmpSetDensity						4
#define HDSelectorBmpGetTransparentValue			5
#define HDSelectorBmpSetTransparentValue			6
#define HDSelectorBmpCreateBitmapV3					7
#define HDSelectorWinSetCoordinateSystem			8
#define HDSelectorWinGetCoordinateSystem			9
#define HDSelectorWinScalePoint						10
#define HDSelectorWinUnscalePoint					11
#define HDSelectorWinScaleRectangle					12
#define HDSelectorWinUnscaleRectangle				13
#define HDSelectorWinScreenGetAttribute				14
#define HDSelectorWinPaintTiledBitmap				15
#define HDSelectorWinGetSupportedDensity			16
#define HDSelectorEvtGetPenNative					17
#define HDSelectorWinScaleCoord						18
#define HDSelectorWinUnscaleCoord					19
#define HDSelectorWinPaintRoundedRectangleFrame	20
#define HDSelectorWinSetScalingMode					21
#define HDSelectorWinGetScalingMode					22

#define HDSelectorInvalid							23			// leave this selector at end

#define sysTrapHighDensityDispatch						0xA3EC // taken from CoreTrap.h

// If nobody has explicitly specified whether the high density trap dispatcher 
// should be used, set it based on the emulation level.
#ifndef USE_HIGH_DENSITY_TRAPS
	#if EMULATION_LEVEL == EMULATION_NONE
		#define	USE_HIGH_DENSITY_TRAPS	1
	#else
		#define	USE_HIGH_DENSITY_TRAPS	0
	#endif
#endif

#if USE_HIGH_DENSITY_TRAPS
	#define HIGH_DENSITY_TRAP(selector)	 _SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapHighDensityDispatch, selector)
#else
	#define HIGH_DENSITY_TRAP(selector)
#endif

// taken from Window.h

// constants used by WinSetCoordinateSystem
#define kCoordinatesNative			0
#define kCoordinatesStandard		72
#define kCoordinatesOneAndAHalf		108
#define kCoordinatesDouble			144
#define kCoordinatesTriple			216
#define kCoordinatesQuadruple		288

//-----------------------------------------------
// High Density support functions
//  (HIGH_DENSITY_TRAP is defined in Bitmap.h)
//-----------------------------------------------
UInt16 	WinSetCoordinateSystem(UInt16 coordSys)
							HIGH_DENSITY_TRAP(HDSelectorWinSetCoordinateSystem);
							
UInt16	WinGetCoordinateSystem(void)
							HIGH_DENSITY_TRAP(HDSelectorWinGetCoordinateSystem);
							
Coord		WinScaleCoord(Coord coord, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinScaleCoord);

Coord		WinUnscaleCoord(Coord coord, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinUnscaleCoord);

void		WinScalePoint(PointType* pointP, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinScalePoint);

void		WinUnscalePoint(PointType* pointP, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinUnscalePoint);

void		WinScaleRectangle(RectangleType* rectP)
							HIGH_DENSITY_TRAP(HDSelectorWinScaleRectangle);

void		WinUnscaleRectangle(RectangleType* rectP)
							HIGH_DENSITY_TRAP(HDSelectorWinUnscaleRectangle);

Err		WinScreenGetAttribute(WinScreenAttrType selector, UInt32* attrP)
							HIGH_DENSITY_TRAP(HDSelectorWinScreenGetAttribute);

void		WinPaintTiledBitmap(BitmapType* bitmapP, RectangleType* rectP)
							HIGH_DENSITY_TRAP(HDSelectorWinPaintTiledBitmap);
							
Err		WinGetSupportedDensity(UInt16* densityP)
							HIGH_DENSITY_TRAP(HDSelectorWinGetSupportedDensity);
							
void 		EvtGetPenNative(WinHandle winH, Int16* pScreenX, Int16* pScreenY, Boolean* pPenDown)
							HIGH_DENSITY_TRAP(HDSelectorEvtGetPenNative);

void 		WinPaintRoundedRectangleFrame(const RectangleType *rP, Coord width, Coord cornerRadius, Coord shadowWidth)
							HIGH_DENSITY_TRAP(HDSelectorWinPaintRoundedRectangleFrame);
							
UInt32	WinSetScalingMode(UInt32 mode)
							HIGH_DENSITY_TRAP(HDSelectorWinSetScalingMode);

UInt32	WinGetScalingMode(void)
							HIGH_DENSITY_TRAP(HDSelectorWinGetScalingMode);

#ifdef _cplusplus
}
#endif

#endif

/* examples
#include <PalmHiRes.h>

  UInt16 err;
  UInt32 WinVersion, attr;
  err = FtrGet(sysFtrCreator, sysFtrNumWinVersion, &WinVersion);
  if (err == 0 && WinVersion >= 4) {
    WinScreenGetAttribute(winScreenDensity, &attr);
    if (attr == kDensityDouble)
	    FrmAlert(TwDoubleDensityOK);
  }

examples end
*/
