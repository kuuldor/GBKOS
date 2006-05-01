#include <PalmOS.h>

#include "common.h"

Int16 _StrCompare(const Char *s1, const Char *s2)
{
	// Check for err
	ErrNonFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	Int16 result = 0;

	// caseless compare at first
	result = _StrCaselessCompare(s1, s2);
	if (0 != result)
		return result;

	// Get pointer to translation table
	const UInt8 *tableP = _GetCharSortValue();

	UInt16 i = 0;

	while (true)
	{
		result = (unsigned char)tableP[(unsigned char)(s1[i])] - (unsigned char)tableP[(unsigned char)(s2[i])];
		if (0 != result)
			break;

		if ((0 == s1[i]) || (0 == s2[i]))
			break;

		i ++;
	};

	return result;
}

Int16 _StrCaselessCompare(const Char *s1, const Char *s2)
{
	// Check for err
	ErrNonFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	Int16 result = 0;

	// Get pointer to translation table
	const UInt8 *tableP = _GetCharCaselessValue();

	UInt16 i = 0;

	while (true)
	{
		result = (unsigned char)tableP[(unsigned char)(s1[i])] - (unsigned char)tableP[(unsigned char)(s2[i])];
		if (0 != result)
			break;

		if ((0 == s1[i]) || (0 == s2[i]))
			break;

		i ++;
	};

	return result;
}

Int16 _StrNCompare(const Char *s1, const Char *s2, Int32 n)
{
	// Check for err
	ErrNonFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	Int16 result = 0;

	// caseless compare at first
	result = _StrNCaselessCompare(s1, s2, n);
	if (0 != result)
		return result;

	// Get pointer to translation table
	const UInt8 *tableP = _GetCharSortValue();

	UInt16 i = 0;

	while (true)
	{
		if (i >= n)
			break;

		result = (unsigned char)tableP[(unsigned char)(s1[i])] - (unsigned char)tableP[(unsigned char)(s2[i])];
		if (0 != result)
			break;

		if ((0 == s1[i]) || (0 == s2[i]))
			break;

		i ++;
	};

	return result;
}

Int16 _StrNCaselessCompare(const Char *s1, const Char *s2, Int32 n)
{
	// Check for err
	ErrNonFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	Int16 result = 0;

	// Get pointer to translation table
	const UInt8 *tableP = _GetCharCaselessValue();

	UInt16 i = 0;

	while (true)
	{
		if (i >= n)
			break;

		result = (unsigned char)tableP[(unsigned char)(s1[i])] - (unsigned char)tableP[(unsigned char)(s2[i])];
		if (0 != result)
			break;

		if ((0 == s1[i]) || (0 == s2[i]))
			break;

		i ++;
	};

	return result;
}

static void FillSortTable(UInt8 *pTable)
{
	const UInt8 sortTable[] = {
	                              0x00,	0x01,	0x02,	0x03,	0x04,	0x05,	0x06,	0x07,
	                              0x08,	0x0E,	0x0F,	0x10,	0x11,	0x12,	0x13,	0x14,
	                              0x15,	0x16,	0x17,	0x18,	0x19,	0x1A,	0x1B,	0x1C,
	                              0x1D,	0x1E,	0x1F,	0x7F,	0x27,	0x2D,	0x20,	0x09,
	                              0x0A,	0x0B,	0x0C,	0x0D,	0x21,	0x22,	0x23,	0x24,
	                              0x25,	0x26,	0x28,	0x29,	0x2A,	0x2C,	0x2E,	0x2F,
	                              0x3A,	0x3B,	0x3F,	0x40,	0x5B,	0x5C,	0x5D,	0x5E,
	                              0x5F,	0x60,	0x7B,	0x7C,	0x7D,	0x7E,	0x2B,	0x3C,
	                              0x3D,	0x3E,	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,
	                              0x36,	0x37,	0x38,	0x39,	0x61,	0x41,	0x62,	0x42,
	                              0x63,	0x43,	0x64,	0x44,	0x65,	0x45,	0x66,	0x46,
	                              0x67,	0x47,	0x68,	0x48,	0x69,	0x49,	0x6A,	0x4A,
	                              0x6B,	0x4B,	0x6C,	0x4C,	0x6D,	0x4D,	0x6E,	0x4E,
	                              0x6F,	0x4F,	0x70,	0x50,	0x71,	0x51,	0x72,	0x52,
	                              0x73,	0x53,	0x74,	0x54,	0x75,	0x55,	0x76,	0x56,
	                              0x77,	0x57,	0x78,	0x58,	0x79,	0x59,	0x7A,	0x5A
	                          };

	for (int i = 0; i < 0x100; i ++)
		pTable[i] = i;

	for (int i = 0; i < 0x80; i ++)
		pTable[sortTable[i]] = i;
}

UInt8 *_GetCharSortValue(void)
{

	ftrSave *store = GetGBKOSBase();

	if (!store->on)
		return store->GetCharSortValue();

	if (NULL == store->charSortValues)
	{
		store->charSortValues = (UInt8 *)PermanentPtrNew(0x100);
		ErrFatalDisplayIf(NULL == store->charSortValues, "Can not create the sort table buffer!");

		FillSortTable(store->charSortValues);
	}

	return store->charSortValues;
}

UInt8 *_GetCharCaselessValue(void)
{
	ftrSave *store = GetGBKOSBase();

	if (!store->on)
		return store->GetCharCaselessValue();

	if (NULL == store->charCaselessValues)
	{
		store->charCaselessValues = (UInt8 *)PermanentPtrNew(0x100);
		ErrFatalDisplayIf(NULL == store->charCaselessValues, "Can not create the caseless sort table buffer!");

		FillSortTable(store->charCaselessValues);

		for (int i = 'a'; i <= 'z'; i ++)
		{
			UInt8 value = store->charCaselessValues[i - 'a' + 'A'];
			store->charCaselessValues[i] = value;
		}
	}

	return store->charCaselessValues;
}

void NotifySort()
{
	Char pszSorting[] = "Sorting...";

	LocalID dbID;
	UInt16 cardNo;
	UInt32 result;
	DmSearchStateType state;
	Err err;

	WinHandle winBackup;
	PointType pntBackup;

	DrawPopupMessage(pszSorting, &winBackup, &pntBackup);

//	SysBroadcastActionCode(sysAppLaunchCmdSyncNotify, NULL);
//	Broadcast SyncNotify to Built-In Apps
	err = DmGetNextDatabaseByTypeCreator(true, &state, 'appl', 'addr', true, &cardNo, &dbID);
	if (err == errNone) {
		SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdSyncNotify, NULL, &result);
	}

	err = DmGetNextDatabaseByTypeCreator(true, &state, 'appl', 'date', true, &cardNo, &dbID);
	if (err == errNone) {
		SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdSyncNotify, NULL, &result);
	}

	err = DmGetNextDatabaseByTypeCreator(true, &state, 'appl', 'memo', true, &cardNo, &dbID);
	if (err == errNone) {
		SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdSyncNotify, NULL, &result);
	}

	err = DmGetNextDatabaseByTypeCreator(true, &state, 'appl', 'todo', true, &cardNo, &dbID);
	if (err == errNone) {
		SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdSyncNotify, NULL, &result);
	}

	ErasePopupMessage(winBackup, pntBackup);
}

