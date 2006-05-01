#ifndef ENCODING_H_INCLUDED
#define ENCODING_H_INCLUDED


typedef  struct {
	UInt8 region;
	UInt8 pos_begin;
	UInt8 pos_end;
	UInt8 effective_begin;
	UInt8 effective_end;
	UInt8 factor;
	UInt16 size;
	UInt8 table[0];
} CodeTable;

struct Encoding {
	CodeTable **convert_table;
	UInt16 tableSize;
	DmOpenRef dbRef;
	MemHandle *handles;
	Boolean Init(const char * dbName);
	void Destroy();
	UInt16 Convert2GBK(UInt16 code);
	inline UInt16 Convert2GBK(unsigned char *s)
	{
		UInt16 code = (UInt16)s[0]<<8|s[1];
		return Convert2GBK(code);
	};
	inline void Convert2GBK(unsigned char *s, unsigned char d[2])
	{
		UInt16 code = (UInt16)s[0]<<8|s[1];
		UInt16 retv = Convert2GBK(code);
		d[0] = (retv>>8)&0xFF;
		d[1] = retv&0xFF;
	};
	Boolean (*isLocalString)(const char* s, UInt16 len);
};

int QuickFindCode(CodeTable **table, UInt16 code, UInt16 size);

#endif
