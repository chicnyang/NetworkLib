#ifndef _LOG_SYSTEM_H_
#define _LOG_SYSTEM_H_

//--------------------------------------------------------------------------------------------------------
// 사용법.
//--------------------------------------------------------------------------------------------------------
// LOG_SET_LEVEL(logLevel) - 로그 레벨 재설정.
// eLogLevel 열거형을 멤버로 받음.
//		LOG_LEVEL_DEBUG(0)
//		LOG_LEVEL_WARNING(1)
//		LOG_LEVEL_ERROR(2)
//		LOG_LEVEL_SYSTEM(3)
// 기본값은 LOG_LEVEL_DEBUG(0).
//--------------------------------------------------------------------------------------------------------
// LOG(logLevel, format, ...) - 현재 설정된 로그레벨 이상의 로그를 로그 파일에 출력.
// 로그 파일은 월단위로 현재 디렉토리 하위 Log 폴더에 저장됨.
//		logLevel	- 로그레벨.
//		format		- 로그로 남길 형식 지정자를 포함한 문자열 포맷.
//		...			- 형식 지정자에 맞는 변수값들.
//--------------------------------------------------------------------------------------------------------
// LOG_HEX(memSrc, byteLen, format, ...) - 메모리 바이너리 값을 파일에 출력.
// 바이너리 값은 format을 기준으로 현재 디렉토리 하위 LogHex 폴더에 한 파일씩 저장된다.
//		memSrc		- 로그로 남길 메모리의 시작 주소.
//		byteLen		- 로그로 남길 메모리의 길이.
//		format		- 로그파일 이름이 될 형식 지정자를 포함한 문자열 포맷.
//		...			- 형식 지정자에 맞는 변수값들.
//--------------------------------------------------------------------------------------------------------


#include <stdarg.h>
#include <strsafe.h>
#include <direct.h>

#define LOG_SAVE_DIRECTORY_MAX	100
#define LOG_FILE_NAME_MAX		200
#define LOG_MSG_MAX				500

//--------------------------------------------------------------------------------------------------------
// 전역 변수
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
// 로그 레벨 enum
//--------------------------------------------------------------------------------------------------------
enum eLogLevel
{
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_SYSTEM
};

//--------------------------------------------------------------------------------------------------------
// 로그 레벨 설정 함수.
//--------------------------------------------------------------------------------------------------------
void SetLogLevel(eLogLevel logLev);

//--------------------------------------------------------------------------------------------------------
// 로그 남기는 함수.
//--------------------------------------------------------------------------------------------------------
void Log(WCHAR* logString, int logLevel);

//--------------------------------------------------------------------------------------------------------
// 메모리 바이트 바이너리 로그 함수.
//--------------------------------------------------------------------------------------------------------
void LogHex(WCHAR* keyword, BYTE* memSrc, UINT64 byteLen);

#endif	// _LOG_SYSTEM_H_


//--------------------------------------------------------------------------------------------------------
// 함수들을 랩핑한 매크로.
//--------------------------------------------------------------------------------------------------------
#define LOG_USE		// 로그를 사용하기 싫다면 이 줄 주석 처리.
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