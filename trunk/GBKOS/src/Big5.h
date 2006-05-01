#ifndef BIG5_H_INCLUDED
#define BIG5_H_INCLUDED

#include <PalmOS.h>

#include <SonyCLIE.h>

#include "common.h"

#define BIG_TABLE_NAME		"Big5 to GBK Convert Table"

Encoding * InitBig5Table();
void ReleaseBig5Table(Encoding *table);


#endif
