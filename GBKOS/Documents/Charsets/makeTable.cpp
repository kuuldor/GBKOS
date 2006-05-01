#include <stdio.h>
#include <string.h>

typedef unsigned char UInt8;
typedef unsigned short UInt16;

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
#define EndianSwap16(n)	(((((unsigned int) n) << 8) & 0xFF00) | \
                         ((((unsigned int) n) >> 8) & 0x00FF))


#define EndianSwap32(n)	(((((unsigned long) n) << 24) & 0xFF000000) |	\
                         ((((unsigned long) n) <<  8) & 0x00FF0000) |	\
                         ((((unsigned long) n) >>  8) & 0x0000FF00) |	\
                         ((((unsigned long) n) >> 24) & 0x000000FF))


int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Usage:%1 TableName PDBName\n", argv[0]);
		return 1;
	}

	int charNum=0;

	char  dbName[32]      = { 0 };
	unsigned char  buffer[0x2000]   = { 0 };
	int bufSize;

	snprintf(dbName,sizeof(dbName), "Big5 to GBK Convert Table");

	FILE *inFile = fopen(argv[1], "rt");
	FILE *outFile = fopen(argv[2],"wb");
	if (inFile == NULL || outFile == NULL)
	{
		printf("Open file failed.\n");
		return 1;
	}


	// write the HEADER
	fwrite(dbName,1,32,outFile);              // database name
	buffer[0x00] = 0x01; buffer[0x01] = 0x01;
	buffer[0x02] = 0x00; buffer[0x03] = 0x01;  // flags + version
	buffer[0x04] = 0xAD; buffer[0x05] = 0xC0;
	buffer[0x06] = 0xBE; buffer[0x07] = 0xA0;  // creation time
	buffer[0x08] = 0xAD; buffer[0x09] = 0xC0;
	buffer[0x0A] = 0xBE; buffer[0x0B] = 0xA0;  // modification time
	buffer[0x0C] = 0xAD; buffer[0x0D] = 0xC0;
	buffer[0x0E] = 0xBE; buffer[0x0F] = 0xA0;  // backup time
	buffer[0x10] = 0x00; buffer[0x11] = 0x00;
	buffer[0x12] = 0x00; buffer[0x13] = 0x00;  // modification number
	buffer[0x14] = 0x00; buffer[0x15] = 0x00;
	buffer[0x16] = 0x00; buffer[0x17] = 0x00;  // app info
	buffer[0x18] = 0x00; buffer[0x19] = 0x00;
	buffer[0x1A] = 0x00; buffer[0x1B] = 0x00;  // sort info
	buffer[0x1C] = 'B';  buffer[0x1D] = 'G';
	buffer[0x1E] = 'C';  buffer[0x1F] = 'V';   // type
	buffer[0x20] = 'd';  buffer[0x21] = 'G';
	buffer[0x22] = 'B';  buffer[0x23] = 'K';   // creator
	buffer[0x24] = 0x00; buffer[0x25] = 0x00;
	buffer[0x26] = 0x00; buffer[0x27] = 0x00;  // unique ID seed
	buffer[0x28] = 0x00; buffer[0x29] = 0x00;
	buffer[0x2A] = 0x00; buffer[0x2B] = 0x00;  // next record list
	fwrite(buffer,1,0x2C,outFile);

	long dbSize = (0xF9- 0xA0)* ((0x7F - 0x40)+ (0xFE - 0xA0) + sizeof(CodeTable)) *2;

	unsigned int truncSize[2] = {(0x7F -0x40)*2+sizeof(CodeTable), (0xFE -0xA0)*2+sizeof(CodeTable) };
	unsigned short numRec;

	numRec = (0xF9-0xA0) *2 ;

	buffer[0x00] = numRec / 256;
	buffer[0x01] = numRec % 256;
	fwrite(buffer,1,2,outFile);                // number of resource

	// write the resource headers
	bool padFlag = false;
	unsigned int offset = 0x4E + (numRec * 10);
	if (offset%4 != 0) {
		offset += 2;
		padFlag = true;
	}

	for (int j=0; j<numRec; j++) {

		buffer[0x00] = 'C';  buffer[0x01] = 'V';
		buffer[0x02] = 'T';  buffer[0x03] = 'B';
		buffer[0x04] = 0x00; buffer[0x05] = (unsigned char)j;


		buffer[0x06] = (offset>>24);
		buffer[0x07] = (offset>>16)&0xFF;
		buffer[0x08] = (offset>>8)&0xFF;
		buffer[0x09] = offset & 0xFF;

		offset += truncSize[j%2];
		fwrite(buffer,1,0x0A,outFile);
	}

	if (padFlag) {
		buffer[0x00] = 0x00; buffer[0x01] = 0x00;
		fwrite(buffer,1,2,outFile);
	}

	char buf[20];
	UInt8  reg,pos;

	reg = pos = 0;
	CodeTable tbl;

	unsigned int tr,tp;

	while (!feof(inFile))
	{

		fscanf(inFile, "%02X%02X: %s", &tr, &tp, buf);
		//printf("region=%02X, pos=%02X, string=%s\n",tr,tp,buf);

		buffer[sizeof(tbl)+pos++]=buf[0];
		buffer[sizeof(tbl)+pos++]=buf[1];

		if (reg != tr)
		{
			tbl.region = tr;
			reg = tr;
			tbl.pos_begin = tp;
			tbl.effective_begin = tp;
			tbl.factor = 2;

			printf("Setting region=%02X, pos_begin=%02X\n",
			       tbl.region, tbl.pos_begin);
		}
		if (0xFE == tp || 0x7E == tp)
		{
			tbl.pos_end = tp;
			tbl.effective_end = tp;
			tbl.size = EndianSwap16(pos);
			memcpy(buffer, &tbl, sizeof(tbl));
			if (tbl.pos_end > tbl.pos_begin )
			{
				printf("Writing region=%02X, pos_begin=%02X, pos_begin=%02X, size=%d\n",
				       tbl.region, tbl.pos_begin, tbl.pos_end, tbl.size);
				fwrite(buffer,1,sizeof(tbl)+pos, outFile);
			}
			pos = 0;
			reg = 0;
		}


	}
	printf("sizeof(CodeTable) is %d \n", sizeof(CodeTable));

	fclose(inFile);
	fclose(outFile);

	return 0;
}
