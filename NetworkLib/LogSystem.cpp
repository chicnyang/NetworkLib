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
// 로그 레벨 설정 함수.
//--------------------------------------------------------------------------------------------------------
void SetLogLevel(eLogLevel logLev)
{
	gLogLevel = logLev;
}

//--------------------------------------------------------------------------------------------------------
// 로그 남기는 함수.
//--------------------------------------------------------------------------------------------------------
void Log(WCHAR* logString, int logLevel)
{
	WCHAR logData[LOG_MSG_MAX] = { 0 };
	FILE* filePtr = nullptr;
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);
	
	//--------------------------------------------------------------------------------------------------------
	// 로그 파일 이름 세팅. 전역 변수에 있는 gLogFileName 값을 설정해서 사용.
	//--------------------------------------------------------------------------------------------------------
	WCHAR fileName[LOG_FILE_NAME_MAX] = { 0 };
	StringCchPrintfW(fileName, sizeof(fileName), L"%s%04d-%02d %s Log.txt", gLogSaveDirectory, curTime.wYear, curTime.wMonth, gLogFileName);
	
	//--------------------------------------------------------------------------------------------------------
	// 멀티 스레드에서 안전하기 위해 락을 걸고 Log 번호를 +1.
	//--------------------------------------------------------------------------------------------------------
	//AcquireSRWLockExclusive(&gSRWLogLock);
	gLogNo++;
	
	//--------------------------------------------------------------------------------------------------------
	// 로그 메시지 세팅.
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
	// 파일 열기.
	//--------------------------------------------------------------------------------------------------------
	for (;;)
	{
		errno_t err = _wfopen_s(&filePtr, fileName, L"ab");
		if (0 != err)
		{
			//--------------------------------------------------------------------------------------------------------
			// Node:	로그는 지금 남겨야 한다. 
			//			파일 오픈에 실패 했으면 성공할 때까지 시도.
			//--------------------------------------------------------------------------------------------------------
			continue;
		}
		break;
	}
	
	//--------------------------------------------------------------------------------------------------------
	// 파일에 로그를 쓰고 파일 닫음.
	//--------------------------------------------------------------------------------------------------------
	fwprintf_s(filePtr, logData);
	fclose(filePtr);
	
	//--------------------------------------------------------------------------------------------------------
	// 락 해제.
	//--------------------------------------------------------------------------------------------------------
	//ReleaseSRWLockExclusive(&gSRWLogLock);
}

//--------------------------------------------------------------------------------------------------------
// 메모리 바이트 바이너리 로그 함수.
// 디버그 모드가 아닌 실행해서 
// HxD 처럼 메모리 값을 보기 편하게 남기고 싶었다....
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
	// 로그 파일 이름 세팅.
	//--------------------------------------------------------------------------------------------------------
	WCHAR fileName[LOG_FILE_NAME_MAX] = { 0, };
	StringCchPrintfW(fileName, sizeof(fileName), L"%s%4d-%02d-%02d %02dH-%02dM-%02dS %s MemDump.txt",
		gLogHexSaveDirectory, curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, keyword);

	//--------------------------------------------------------------------------------------------------------
	// 파일 열기.
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
	// 한줄당 16바이트씩 메모리값 출력.
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
		// Offset(16진수) 출력.
		//--------------------------------------------------------------------------------------------------------
		fwprintf_s(filePtr, L"%016llX    ", bytePos);

		//--------------------------------------------------------------------------------------------------------
		// 메모리 바이너리값 출력.
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
		// 바이너리 출력과 Decode 출력 사이 공백 출력. 
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
		// Decode 값 출력.
		//--------------------------------------------------------------------------------------------------------
		for (writePos = bytePos; writePos < (bytePos + 16); writePos++)
		{
			if (writePos == byteLen)
			{
				break;
			}

			//--------------------------------------------------------------------------------------------------------
			// ASCII 코드값으로 출력가능 하다면 ASCII 코드로 출력, 그렇지 않다면 '.'으로 출력.
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
		// 한줄 출력 후 개행, bytePos 설정.
		//--------------------------------------------------------------------------------------------------------
		fwprintf_s(filePtr, L"\n");
		bytePos += (writePos - bytePos);
	}



	//--------------------------------------------------------------------------------------------------------
	// 파일 닫기.
	//--------------------------------------------------------------------------------------------------------
	fclose(filePtr);
}
