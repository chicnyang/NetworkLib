#ifndef _LOG_SYSTEM_H_
#define _LOG_SYSTEM_H_

//--------------------------------------------------------------------------------------------------------
// ����.
//--------------------------------------------------------------------------------------------------------
// LOG_SET_LEVEL(logLevel) - �α� ���� �缳��.
// eLogLevel �������� ����� ����.
//		LOG_LEVEL_DEBUG(0)
//		LOG_LEVEL_WARNING(1)
//		LOG_LEVEL_ERROR(2)
//		LOG_LEVEL_SYSTEM(3)
// �⺻���� LOG_LEVEL_DEBUG(0).
//--------------------------------------------------------------------------------------------------------
// LOG(logLevel, format, ...) - ���� ������ �α׷��� �̻��� �α׸� �α� ���Ͽ� ���.
// �α� ������ �������� ���� ���丮 ���� Log ������ �����.
//		logLevel	- �α׷���.
//		format		- �α׷� ���� ���� �����ڸ� ������ ���ڿ� ����.
//		...			- ���� �����ڿ� �´� ��������.
//--------------------------------------------------------------------------------------------------------
// LOG_HEX(memSrc, byteLen, format, ...) - �޸� ���̳ʸ� ���� ���Ͽ� ���.
// ���̳ʸ� ���� format�� �������� ���� ���丮 ���� LogHex ������ �� ���Ͼ� ����ȴ�.
//		memSrc		- �α׷� ���� �޸��� ���� �ּ�.
//		byteLen		- �α׷� ���� �޸��� ����.
//		format		- �α����� �̸��� �� ���� �����ڸ� ������ ���ڿ� ����.
//		...			- ���� �����ڿ� �´� ��������.
//--------------------------------------------------------------------------------------------------------


#include <stdarg.h>
#include <strsafe.h>
#include <direct.h>

#define LOG_SAVE_DIRECTORY_MAX	100
#define LOG_FILE_NAME_MAX		200
#define LOG_MSG_MAX				500

//--------------------------------------------------------------------------------------------------------
// ���� ����
//--------------------------------------------------------------------------------------------------------
extern const WCHAR* gLogSaveDirectory;
extern const WCHAR* gLogHexSaveDirectory;

extern UINT gLogLevel;
extern WCHAR gLogBuffer[LOG_MSG_MAX];
extern const WCHAR* gLogFileName;
extern LONG gLogNo;
extern bool gbFirstLog;
extern SRWLOCK gSRWLogLock;

extern bool gFirstHexLog;
extern SRWLOCK gSRWLogHexLock;
//--------------------------------------------------------------------------------------------------------
// �α� ���� enum
//--------------------------------------------------------------------------------------------------------
enum eLogLevel
{
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_SYSTEM
};

//--------------------------------------------------------------------------------------------------------
// �α� ���� ���� �Լ�.
//--------------------------------------------------------------------------------------------------------
void SetLogLevel(eLogLevel logLev);

//--------------------------------------------------------------------------------------------------------
// �α� ����� �Լ�.
//--------------------------------------------------------------------------------------------------------
void Log(WCHAR* logString, int logLevel);

//--------------------------------------------------------------------------------------------------------
// �޸� ����Ʈ ���̳ʸ� �α� �Լ�.
//--------------------------------------------------------------------------------------------------------
void LogHex(WCHAR* keyword, BYTE* memSrc, UINT64 byteLen);

#endif	// _LOG_SYSTEM_H_


//--------------------------------------------------------------------------------------------------------
// �Լ����� ������ ��ũ��.
//--------------------------------------------------------------------------------------------------------
#define LOG_USE		// �α׸� ����ϱ� �ȴٸ� �� �� �ּ� ó��.
#ifdef LOG_USE
	#define LOG_SET_LEVEL(logLevel)					SetLogLevel(logLevel);
	#define LOG(logLevel, format, ...)				do {																				\
														if(gLogLevel <= logLevel)														\
														{																				\
															if (gbFirstLog)																\
															{																			\
																InitializeSRWLock(&gSRWLogLock);										\
																_wmkdir(gLogSaveDirectory);												\
																gbFirstLog = false;														\
															}																			\
															AcquireSRWLockExclusive(&gSRWLogLock);										\
															StringCchPrintfW(gLogBuffer, sizeof(gLogBuffer), format, ##__VA_ARGS__);	\
															Log(gLogBuffer, logLevel);													\
															ReleaseSRWLockExclusive(&gSRWLogLock);										\
														}																				\
													} while (0)

	#define LOG_HEX(memSrc, byteLen, format, ...)	do {																				\
															if (gFirstHexLog)															\
															{																			\
																InitializeSRWLock(&gSRWLogHexLock);										\
																_wmkdir(gLogHexSaveDirectory);											\
																gFirstHexLog = false;													\
															}																			\
															AcquireSRWLockExclusive(&gSRWLogHexLock);									\
															StringCchPrintfW(gLogBuffer, sizeof(gLogBuffer), format, ##__VA_ARGS__);	\
															LogHex(gLogBuffer, memSrc, byteLen);										\
															ReleaseSRWLockExclusive(&gSRWLogHexLock);									\
													} while (0)
#else
	#define LOG_SET_LEVEL(logLevel)
	#define LOG(logLevel, format, ...)
	#define LOG_HEX(keyword, memSrc, byteLen)
#endif