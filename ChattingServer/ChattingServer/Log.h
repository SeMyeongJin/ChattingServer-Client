#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

class Log
{
public:
	static BOOL WriteLog(LPTSTR data, ...)
	{
		SYSTEMTIME SystemTime;
		TCHAR		CurrentDate[32] = { 0, };
		TCHAR		CurrentFileName[MAX_PATH] = { 0, };
		FILE*		FilePtr = NULL;
		TCHAR		DebugLog[256] = { 0, };

		va_list		ap;
		TCHAR		Log[256] = { 0, };

		va_start(ap, data);
		_vstprintf(Log, data, ap);
		va_end(ap);

		GetLocalTime(&SystemTime);
		_sntprintf(CurrentDate, 32, _T("%d-%d-%d %d:%d:%d"),
			SystemTime.wYear,
			SystemTime.wMonth,
			SystemTime.wDay,
			SystemTime.wHour,
			SystemTime.wMinute,
			SystemTime.wSecond);

		_sntprintf(CurrentFileName, MAX_PATH, _T("LOG_%d-%d-%d %d.log"),
			SystemTime.wYear,
			SystemTime.wMonth,
			SystemTime.wDay,
			SystemTime.wHour);

		FilePtr = _tfopen(CurrentFileName, _T("a"));
		if (!FilePtr)
			return FALSE;

		_ftprintf(FilePtr, _T("[%s] %s\n"), CurrentDate, Log);
		_sntprintf(DebugLog, 256, _T("[%s] %s\n"), CurrentDate, Log);

		fflush(FilePtr);

		fclose(FilePtr);

		OutputDebugString(DebugLog);
		_tprintf(_T("%s"), DebugLog);

		return TRUE;
	}
};