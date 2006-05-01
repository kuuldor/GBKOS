 
#include <PalmOS.h>

#include "common.h"

#include "Companion.hpp"
#include "Companion_res.hpp"


#define RESID Trap3

extern "C" void MyHandler(WChar ch, Coord x, Coord y)
{
	void (*old_WinDrawChar)(WChar ch, Coord x, Coord y);
	FtrGet(CREATOR,RESID,(UInt32*)(&old_WinDrawChar));
	ErrFatalDisplayIf(old_WinDrawChar==NULL,"HACK Master deliver an EMPTY old trap address of WinDrawChar!");
	
	void (*old_WinDrawChars)(const char* s, Int16 len, Coord x, Coord y);
	FtrGet(CREATOR,1000,(UInt32*)(&old_WinDrawChars));
	ErrFatalDisplayIf(old_WinDrawChars==NULL,"HACK Master deliver an EMPTY old trap address of WinDrawChars!");
	
	ftrSave *store = NULL;
	FtrGet(CREATOR,0,(UInt32*)(&store));

	LocalID app;
	UInt16 card;
	SysCurAppDatabase(&card,&app);
	UInt16 form = FrmGetActiveFormID();
		
	if (store == NULL)  {
		store = (ftrSave*) MemPtrNew(sizeof(ftrSave));
		ErrFatalDisplayIf(store==NULL,"Cannot allocate memory for HackStore!");
		MemPtrSetOwner(store, 0);
		
		store->on = false;
		store->card = card;
		store->app = app;
		store->form = form;
		store->remain = 0;
		store->rx = -1;
		store->ry = -1;
		FtrSet(CREATOR,0,(UInt32)store);
	}
	
	if (app !=store->app || card!=store->card) {
		store->on = false;
		store->remain = 0;
		store->rx = -1;
		store->ry = -1;
	}

	if (!store->on) {
		return old_WinDrawChar(ch,x,y);
	} else {
		if (form!=store->form) {
			store->form = form;
			store->remain = 0;
			store->rx = -1;
			store->ry = -1;
		}
	}

	char tmpStr[2];
	Int16 new_len, new_x, new_y;
	unsigned char nc = ch&0xFF;
	
	Int16 fheight = FntCharHeight();
	if (store->remain !=0) {
		Int16 rwidth = FntCharWidth(store->remain);
		Int16 dx = x - store->rx - rwidth;
		Int16 dy = y - store->ry - fheight;
	 	if (((dx<MAX_MIS&&dx>-MAX_MIS)&&(store->ry == y ))||(x<10||(dy<MAX_MIS&&dy>-MAX_MIS))) {
			tmpStr[0]=store->remain;
			tmpStr[1]=nc;
			if ((dx<MAX_MIS&&dx>-MAX_MIS)&&(store->ry == y )) {
				//new_x = x-FntCharsWidth(tmpStr,2)/2; new_y=y;
				new_x = store->rx; new_y=y;
				new_len = 2;
			} else {
				new_len = 2;
				new_x= x-rwidth; new_y = y;

				/*
				old_WinDrawChars(tmpStr,2,rx-rwidth/2,ry);
				new_s = tmpStr+2;
				new_len = len-1;
				new_x= x; new_y = y;	
				*/
			}
			return old_WinDrawChars(tmpStr,2,new_x,new_y);
		} else {
			char ts[1];
			ts[0]=store->remain;
			old_WinDrawChars(ts,1,store->rx,store->ry);
		}
	}
	
	
	RectangleType rect;
	FrmGetFormBounds(FrmGetActiveForm(),&rect);

	if (nc>0x80 &&(nc!=0x85 && nc!=0x99 && nc!=0xA9 && nc!=0xB0 && nc!=0xAE) ){
		if (y<rect.topLeft.y+rect.extent.y-fheight) {
			store->remain = nc;
			store->rx = x ;
			store->ry = y;
		}
	} else {
		old_WinDrawChar(ch,x,y);
		if (y<rect.topLeft.y+rect.extent.y-fheight) {
			store->remain = 0;
			store->rx = store->ry = -1;
		}
	}
	
}

