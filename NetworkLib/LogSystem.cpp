#include "pch.h"
#include "LogSystem.h"

const WCHAR* gLogSaveDirectory		= L"Log\\";
const WCHAR* gLogHexSaveDirectory	= L"LogHex\\";

UINT gLogLevel						= eLogLevel::LOG_LEVEL_DEBUG;
WCHAR gLogBuffer[LOG_MSG_MAX]		= { 0 };
const WCHAR* gLogFileName			= L"Default_File_Name";
LONG gLogNo							= 0;
bool gbFirstLog						= true;
SRWLOCK gSRWLogLock;

bool gFirstHexLog					= true;
SRWLOCK gSRWLogHexLock;

//--------------------------------------------------------------------------------------------------------
// �α� ���� ���� �Լ�.
//--------------------------------------------------------------------------------------------------------
void SetLogLevel(eLogLevel logLev)
{
	gLogLevel = logLev;
}

//--------------------------------------------------------------------------------------------------------
// �α� ����� �Լ�.
//--------------------------------------------------------------------------------------------------------
void Log(WCHAR* logString, int logLevel)
{
	WCHAR logData[LOG_MSG_MAX] = { 0 };
	FILE* filePtr = nullptr;
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);
	
	//--------------------------------------------------------------------------------------------------------
	// �α� ���� �̸� ����. ���� ������ �ִ� gLogFileName ���� �����ؼ� ���.
	//--------------------------------------------------------------------------------------------------------
	WCHAR fileName[LOG_FILE_NAME_MAX] = { 0 };
	StringCchPrintfW(fileName, sizeof(fileName), L"%s%04d-%02d %s Log.txt", gLogSaveDirectory, curTime.wYear, curTime.wMonth, gLogFileName);
	
	//--------------------------------------------------------------------------------------------------------
	// ��Ƽ �����忡�� �����ϱ� ���� ���� �ɰ� Log ��ȣ�� +1.
	//--------------------------------------------------------------------------------------------------------
	//AcquireSRWLockExclusive(&gSRWLogLock);
	gLogNo++;
	
	//--------------------------------------------------------------------------------------------------------
	// �α� �޽��� ����.
	//--------------------------------------------------------------------------------------------------------
	switch (logLevel)
	{
	case eLogLevel::LOG_LEVEL_DEBUG:
		StringCchPrintfW(logData, sizeof(logData), L"[DEBUG][%4d-%02d-%02d %02d:%02d:%02d No.%d] : %s\n",
			curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, gLogNo, logString);
		break;
	case eLogLevel::LOG_LEVEL_WARNING:
		StringCchPrintfW(logData, sizeof(logData), L"[WARNING][%4d-%02d-%02d %02d:%02d:%02d No.%d] : %s\n",
			curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, gLogNo, logString);
		break;
	case eLogLevel::LOG_LEVEL_ERROR:
		StringCchPrintfW(logData, sizeof(logData), L"[ERROR][%4d-%02d-%02d %02d:%02d:%02d No.%d] : %s\n",
			curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, gLogNo, logString);
		break;
	case eLogLevel::LOG_LEVEL_SYSTEM:
		StringCchPrintfW(logData, sizeof(logData), L"[SYSTEM][%4d-%02d-%02d %02d:%02d:%02d No.%d] : %s\n",
			curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, gLogNo, logString);
		break;
	default:
		break;
	}
	   	
	//--------------------------------------------------------------------------------------------------------
	// ���� ����.
	//--------------------------------------------------------------------------------------------------------
	for (;;)
	{
		errno_t err = _wfopen_s(&filePtr, fileName, L"ab");
		if (0 != err)
		{
			//--------------------------------------------------------------------------------------------------------
			// Node:	�α״� ���� ���ܾ� �Ѵ�. 
			//			���� ���¿� ���� ������ ������ ������ �õ�.
			//--------------------------------------------------------------------------------------------------------
			continue;
		}
		break;
	}
	
	//--------------------------------------------------------------------------------------------------------
	// ���Ͽ� �α׸� ���� ���� ����.
	//--------------------------------------------------------------------------------------------------------
	fwprintf_s(filePtr, logData);
	fclose(filePtr);
	
	//--------------------------------------------------------------------------------------------------------
	// �� ����.
	//--------------------------------------------------------------------------------------------------------
	//ReleaseSRWLockExclusive(&gSRWLogLock);
}

//--------------------------------------------------------------------------------------------------------
// �޸� ����Ʈ ���̳ʸ� �α� �Լ�.
// ����� ��尡 �ƴ� �����ؼ� 
// HxD ó�� �޸� ���� ���� ���ϰ� ����� �;���....
//--------------------------------------------------------------------------------------------------------
void LogHex(WCHAR* keyword, BYTE* memSrc, UINT64 byteLen)
{
	if (0 == byteLen)
	{
		return;
	}

	FILE* filePtr = nullptr;
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);

	//--------------------------------------------------------------------------------------------------------
	// �α� ���� �̸� ����.
	//--------------------------------------------------------------------------------------------------------
	WCHAR fileName[LOG_FILE_NAME_MAX] = { 0, };
	StringCchPrintfW(fileName, sizeof(fileName), L"%s%4d-%02d-%02d %02dH-%02dM-%02dS %s MemDump.txt",
		gLogHexSaveDirectory, curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, keyword);

	//--------------------------------------------------------------------------------------------------------
	// ���� ����.
	//--------------------------------------------------------------------------------------------------------
	errno_t err = _wfopen_s(&filePtr, fileName, L"wb");
	int errorCode = 0;
	if (0 != err)
	{
		errorCode = GetLastError();
		wprintf_s(L"ERR_FILE_OPEN : %d", errorCode);
		return;
	}

	//--------------------------------------------------------------------------------------------------------
	// ���ٴ� 16����Ʈ�� �޸𸮰� ���.
	//--------------------------------------------------------------------------------------------------------
	UINT64 bytePos = 0;
	UINT64 writePos = 0;
	fwprintf_s(filePtr, L"Offset(h)           00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F     Decode :\n\n");
	fwprintf_s(filePtr, L"%d Bytes\n", byteLen);
	for (;;)
	{
		if (byteLen == bytePos)
		{
			break;
		}

		//--------------------------------------------------------------------------------------------------------
		// Offset(16����) ���.
		//--------------------------------------------------------------------------------------------------------
		fwprintf_s(filePtr, L"%016llX    ", bytePos);

		//--------------------------------------------------------------------------------------------------------
		// �޸� ���̳ʸ��� ���.
		//--------------------------------------------------------------------------------------------------------
		for (writePos = bytePos; writePos < (bytePos + 16); writePos++)
		{
			if (writePos == byteLen)
			{
				break;
			}

			fwprintf_s(filePtr, L"%02X ", memSrc[writePos]);
		}

		//--------------------------------------------------------------------------------------------------------
		// ���̳ʸ� ��°� Decode ��� ���� ���� ���. 
		//--------------------------------------------------------------------------------------------------------
		if (0 != (writePos % 16))
		{
			for (int cnt = 0; cnt < (16 - (writePos % 16)); cnt++)
			{
				fwprintf_s(filePtr, L"   ");
			}
		}
		fwprintf_s(filePtr, L"    ");

		//--------------------------------------------------------------------------------------------------------
		// Decode �� ���.
		//--------------------------------------------------------------------------------------------------------
		for (writePos = bytePos; writePos < (bytePos + 16); writePos++)
		{
			if (writePos == byteLen)
			{
				break;
			}

			//--------------------------------------------------------------------------------------------------------
			// ASCII �ڵ尪���� ��°��� �ϴٸ� ASCII �ڵ�� ���, �׷��� �ʴٸ� '.'���� ���.
			//--------------------------------------------------------------------------------------------------------
			if (0x21 <= memSrc[writePos] && memSrc[writePos] <= 0x7E)
			{
				fwprintf_s(filePtr, L"%c", memSrc[writePos]);
			}
			else
			{
				fwprintf_s(filePtr, L".");
			}
		}

		//--------------------------------------------------------------------------------------------------------
		// ���� ��� �� ����, bytePos ����.
		//--------------------------------------------------------------------------------------------------------
		fwprintf_s(filePtr, L"\n");
		bytePos += (writePos - bytePos);
	}



	//--------------------------------------------------------------------------------------------------------
	// ���� �ݱ�.
	//--------------------------------------------------------------------------------------------------------
	fclose(filePtr);
}
