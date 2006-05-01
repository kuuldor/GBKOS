 
#ifndef __COMPANION_H
#define __COMPANION_H



typedef struct {
	Boolean on;
	UInt16 card;
	LocalID app;
	UInt16 form;
	UInt32 last_phrase;
	Coord last_x;
	Coord last_y;
	char last_remain;
	Coord last_rx;
	Coord last_ry;
	char remain;
	Coord rx;
	Coord ry;
} ftrSave;

#endif
