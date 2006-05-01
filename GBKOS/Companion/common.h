#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define CREATOR 'hGBK'
#define DBNAME "Companion Config"
#define DBTYPE 'HCfg'
#define APP_DB_TYPE 'appl'

#include "../src/GBKOS_Lib.h"

typedef struct {
  Char name[dmDBNameLength];
  UInt32 id;
  UInt16 card;
  LocalID app;
  Boolean mcfix;
  ftrGBK conf;
} dbConf;

typedef struct {
	UInt16 cnt;
	dbConf *ar;
} confArray;

typedef struct {
	UInt16 libRef;
	Int16 appIndex;
	ftrGBK defaultConf;
	dbConf appConf;
} global;

DmOpenRef OpenConfigDB(UInt16 mode = dmModeReadWrite, Boolean create = false);
void CloseConfigDB(DmOpenRef db);
void EraseConfigDB();


void GetGlobal(global &g);
void SetGlobal(global &g);
void SetAppConf();
Boolean GetAppConf(Int16 index);

void SetConf(const ftrGBK &conf) ;


#endif
