 

#include <PalmOS.h>

#include "common.h"
#include "Companion.hpp"
#include "Companion_res.hpp"


#define RESID Trap1
#define OUTCOORD 10000
 
extern "C" void MyHandler(const char* s, Int16 len, Coord x, Coord y)
{
	Err err;

	void (*old_WinDrawChars)(const char* s, Int16 len, Coord x, Coord y);
	err = FtrGet(CREATOR, RESID,(UInt32*)(&old_WinDrawChars));
	ErrFatalDisplayIf(err!=0||old_WinDrawChars==NULL,"HACK Master deliver an EMPTY old trap address of WinDrawChars!");
	

	LocalID app;
	UInt16 card;
	SysCurAppDatabase(&card,&app);
		
	ftrSave *store = NULL;
	err = FtrGet(CREATOR,0,(UInt32*)(&store));

	if (err!=0 || store == NULL)  {
		store = (ftrSave*) MemPtrNew(sizeof(ftrSave));
		ErrFatalDisplayIf(store==NULL,"Cannot allocate memory for HackStore!");
		MemPtrSetOwner(store, 0);
		store->on = false;
		store->card = card;
		store->app = app;
		store->form = 0;
		store->last_phrase = 0;
		store->last_x = OUTCOORD;
		store->last_y = OUTCOORD;
		store->last_remain = 0;
		store->last_rx = -1;
		store->last_ry = -1;
		store->remain = 0;
		store->rx = -1;
		store->ry = -1;
		FtrSet(CREATOR,0,(UInt32)store);
	}
	
	
	if (!store->on) {
		return old_WinDrawChars(s,len,x,y);
	}
	

	char *tmpStr=NULL;
	char *new_s;
	Int16 new_len, new_x, new_y;
	Int16 fheight = FntCharHeight();
		
	new_s = const_cast<char*>(s);
	new_len = len;
	new_x=x;new_y=y;

	union Union_Phrase{
		char c[4];
		UInt32 phrase;
	} u_phrase;

	if (app !=store->app || card!=store->card) {
		store->on = false;
		store->card = card;
		store->app = app;
		store->form = 0;
		store->last_phrase = 0;
		store->last_x = OUTCOORD;
		store->last_y = OUTCOORD;
		store->last_remain = 0;
		store->last_rx = -1;
		store->last_ry = -1;
		store->remain = 0;
		store->rx = -1;
		store->ry = -1;
	} else {

		UInt16 form = FrmGetActiveFormID();
		if (form!=store->form) {
			store->form = form;
			store->last_phrase = 0;
			store->last_x = OUTCOORD;
			store->last_y = OUTCOORD;
			store->last_remain = 0;
			store->last_rx = -1;
			store->last_ry = -1;
			store->remain = 0;
			store->rx = -1;
			store->ry = -1;
		}
	
		if (len>=4) {
			u_phrase.c[0]=s[0];
			u_phrase.c[1]=s[1];
			u_phrase.c[2]=s[len-2];
			u_phrase.c[3]=s[len-1];
		} else {
			u_phrase.phrase = 0;
			switch (len) {
			case 3:
				u_phrase.c[2]=s[2];
			case 2:
				u_phrase.c[1]=s[1];
			case 1:
				u_phrase.c[0]=s[0];
			}
		}

		if (store->last_remain!=0&&u_phrase.phrase!=0 && u_phrase.phrase==store->last_phrase && x==store->last_x) {
			store->remain = store->last_remain;
			store->rx = store->last_rx;
			store->ry = store->last_ry;
		}
		

		if (store->remain !=0) {
			Int16 rwidth = FntCharWidth(store->remain);
			Int16 dx = x - store->rx - rwidth;
			//Int16 dx = x - store->rx;
			Int16 dy = y - store->ry - fheight;
		 	if (((dx<MAX_MIS&&dx>-MAX_MIS)&&(store->ry == y ))||(x<MARGIN||(dy<MAX_MIS&&dy>-MAX_MIS))) 
			{
				tmpStr = (char*)MemPtrNew(len+2);
				tmpStr[0]=store->remain;
				if (len>1) 
					StrNCopy(tmpStr+1,s,len);
				else 
					tmpStr[1] = s[0];
				tmpStr[len+1]=0;
				new_s = tmpStr;
				new_len = len+1;
				if ((dx<MAX_MIS&&dx>-MAX_MIS)&&(store->ry == y )) {
					//new_x = x-FntCharsWidth(tmpStr,2)/2; new_y=y;
					new_x = store->rx; new_y=y;
				} else {
					if (x< MARGIN) {
						new_x= x; 
						new_y = y;
					} else {
						new_x= x-fheight; 
						new_y = y;
					}
	
				}
			} else {
				char ts[1];
				ts[0]=store->remain;
				old_WinDrawChars(ts,1,store->rx- rwidth,store->ry);
			
			}	
		}
	}

	int i;
	Boolean hanzi = false;
	for ( i=0; i<new_len-1 && new_s[i]!=0; i++) {
		hanzi = false;
		if ((unsigned char)new_s[i]==0xA0) 
			new_s[i]=0x20; 
		else if ((unsigned char)new_s[i]>0x80 && (unsigned char)new_s[i+1]>=0x40) {
			i++;
			hanzi = true;
		}
	}
	
	unsigned char t = (unsigned char)new_s[i];
	//RectangleType rect;
	//FrmGetFormBounds(FrmGetActiveForm(),&rect);

	if (i==new_len-1 && t>0x80 && (hanzi==true||i==0) && (t!=0x85 && t!=0x95 && t!=0x99 && t!=0xA9  && t!=0xA9 && t!=0xAE) ){
		if (store->remain != 0) {
			store->last_remain = store->remain;
			store->last_x = x;
			store->last_y = y;
			store->last_rx = store->rx;
			store->last_ry = store->ry;
			store->last_phrase = u_phrase.phrase;
		}
		
		if (i!=0) {
			old_WinDrawChars(new_s,i,new_x,new_y);
			//if (y<rect.topLeft.y+rect.extent.y-fheight) {
				store->remain = new_s[new_len-1];
				store->rx = new_x + FntCharsWidth(new_s,new_len-1);
				store->ry = y;
			//}
		} else {
			store->remain = t;
			store->rx = x;
			store->ry = y;
		}
	} else {
		old_WinDrawChars(new_s,new_len,new_x,new_y);
		//if (y<rect.topLeft.y+rect.extent.y-fheight) {
			store->remain = 0;
			store->rx = store->ry = -1;
		//}
	}
	
	if (tmpStr != NULL) {
		MemPtrFree(tmpStr);
	}
	
}

