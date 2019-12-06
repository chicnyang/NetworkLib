#include "pch.h"
#include <time.h>
#include <strsafe.h>
#include "logfile.h"

LONG logcount = 0;
SRWLOCK _srwlock;
int g_loglevel;
//void logfile(WCHAR *szType, int LogLevel, WCHAR *szStringFormat, ...)
//{
//	static int count = 0;
//
//	errno_t err;
//	FILE *fp;
//	WCHAR type[50] = { 0 };
//	WCHAR buf[100] = { 0 };
//	++count;
//
//	WCHAR Errbuf[1024] = { 0 };
//
//
//	time_t     now = time(0); //현재 시간을 time_t 타입으로 저장
//	struct tm  tstruct;
//
//	WCHAR       namebuf[80] = { 0 };
//	WCHAR       timebuf[80] = { 0 };
//	localtime_s(&tstruct, &now);
//	wcsftime(timebuf, sizeof(timebuf), L"%Y-%m-%d.%X,", &tstruct);
//	wcsftime(namebuf, sizeof(namebuf), L"%Y-%m-%d", &tstruct);
//	//strftime(timebuf, sizeof(timebuf), "%Y-%m-%d", &tstruct);
//
//
//	switch (Type)
//	{
//	case LOG_LEVEL_DEBUG:
//	
//		StringCchPrintf(type,50,L"Log_");
//		StringCchPrintf(buf, 100, L"%s%s%s%s", L".\\logfile\\", type, namebuf, L".txt");
//		//sprintf_s(type, "Log_");
//		//sprintf_s(buf, "%s%s%s", "\\logfile\\", timebuf, ".txt");
//		//sprintf_s(buf, "%s%s", timebuf, ".txt");
//		StringCbCopy(Errbuf, sizeof(Errbuf), errbuf);
//		//sprintf_s(Errbuf,"%s", errbuf);
//
//		err = _wfopen_s(&fp, buf,L"a");
//		if (err == 0)
//		{
//			fseek(fp, 0, 0);
//
//			fwprintf(fp, timebuf);
//			fwprintf(fp, L"                 ");
//			fwprintf(fp, L"DEBUG_LEVEL");
//			fwprintf(fp, L"    	errlog :   ");
//			fwprintf(fp, Errbuf);
//			fwprintf(fp,L"\n");
//
//			fclose(fp);
//
//		}
//		break;
//	case LOG_LEVEL_WARNING:
//
//		StringCchPrintf(type, 50, L"Log_");
//		StringCchPrintf(buf, 100, L"%s%s%s%s", L".\\logfile\\", type, namebuf, L".txt");
//
//		StringCbCopy(Errbuf, sizeof(Errbuf), errbuf);
//
//		err = _wfopen_s(&fp, buf, L"a");
//		if (err == 0)
//		{
//			fseek(fp, 0, 0);
//
//			fwprintf(fp, timebuf);
//			fwprintf(fp, L"                 ");
//			fwprintf(fp, L"WARNING_LEVEL");
//			fwprintf(fp, L"    	errlog :   ");
//			fwprintf(fp, Errbuf);
//			fwprintf(fp, L"\n");
//
//			fclose(fp);
//
//		}
//		break;
//	case LOG_LEVEL_ERROR:
//
//		StringCchPrintf(type, 50, L"Log_");
//		StringCchPrintf(buf, 100, L"%s%s%s%s", L".\\logfile\\", type, namebuf, L".txt");
//
//		StringCbCopy(Errbuf, sizeof(Errbuf), errbuf);
//
//		err = _wfopen_s(&fp, buf, L"a");
//		if (err == 0)
//		{
//			fseek(fp, 0, 0);
//
//			fwprintf(fp, timebuf);
//			fwprintf(fp, L"                 ");
//			fwprintf(fp, L"ERROR_LEVEL");
//			fwprintf(fp, L"    	errlog :   ");
//			fwprintf(fp, Errbuf);
//			fwprintf(fp, L"\n");
//
//			fclose(fp);
//
//		}
//		break;
//
//	case LOG_LEVEL_SYSTEM:
//		break;
//		/*
//	case serialization_buf_resize:
//		StringCchPrintf(type, 50, L"Log_");
//		StringCchPrintf(buf, 100, L"%s%s%s", L".\\logfile\\", timebuf, L".txt");
//
//		err = fopen_s(&fp, buf, "wt");
//		if (err == 0)
//		{
//			fseek(fp, 0, 0);
//
//
//			char		cvalue[30] = { 0 };
//			sprintf_s(cvalue, "%d", value);
//
//
//			fprintf(fp, timebuf);
//			fprintf(fp, "                 ");
//			fprintf(fp, type);
//			fprintf(fp, "    	not enough size :   ");
//			fprintf(fp, cvalue);
//
//			fclose(fp);
//
//		}
//
//
//		break;
//
//	case serialization_buf_lack:
//		StringCchPrintf(type, 50, L"Log_");
//		StringCchPrintf(buf, 100, L"%s%s%s", L".\\logfile\\", timebuf, L".txt");
//
//		StringCbCopy(Errbuf, sizeof(Errbuf), errbuf);
//
//		err = fopen_s(&fp, buf, "wt");
//		if (err == 0)
//		{
//			fseek(fp, 0, 0);
//
//
//			char		cvalue[30] = { 0 };
//			sprintf_s(cvalue, "%d", value);
//
//
//			fprintf(fp, timebuf);
//			fprintf(fp, "                 ");
//			fprintf(fp, type);
//			fprintf(fp, "    	lack size data :   ");
//			fprintf(fp, cvalue);
//			fprintf(fp, (char*)Errbuf);
//
//			fclose(fp);
//
//		}
//		break;
//		*/
//
//	default:
//
//
//		break;
//	}
//
//
//}

void LogInitial()
{
	InitializeSRWLock(&_srwlock);
}


void Logfile(const WCHAR *szType, int LogLevel, const WCHAR *szStringFormat, ...)
{
	//내가원하는 로그 레벨이 아니면 나간다.
	if (g_loglevel > LogLevel)
	{
		return;
	}


	int mlogcount = InterlockedIncrement(&logcount);

	SYSTEMTIME stNowTime;
	GetLocalTime(&stNowTime);

	WCHAR Errbuf[512] = { 0 };
	WCHAR logerr[200];

	//에러내용 밀어넣기 
	va_list va;
	va_start(va, szStringFormat);
	HRESULT vprintresulrt = StringCchVPrintf(Errbuf, 512, szStringFormat,va);
	if(FAILED(vprintresulrt))
	{
		//밀어넣는데 실패 로그 남김 
		memcpy(logerr, Errbuf,200);
		LOG(L"va in errbuf", LOG_LEVEL_ERROR, L"%s", logerr);
	}
	va_end(va);


	//파일 이름 만들기 

	WCHAR filename[512];
	HRESULT printresult = StringCchPrintf(filename,512, L"%s[%d%02d]_%s .txt", L".\\logfile\\", stNowTime.wYear, stNowTime.wMonth, szType);
	if (FAILED(printresult))
	{
		//밀어넣는데 실패 로그 남김 
		LOG(L"filename err", LOG_LEVEL_ERROR,L"type :%s ", szType);
	}


	WCHAR logbuf[1024];
	WCHAR printerrbuf[1024];
	WCHAR loglevel[20];
	switch (LogLevel)
	{
	case LOG_LEVEL_DEBUG:
		StringCchPrintf(loglevel,20,L"[DEBUG]");
		break;
	case LOG_LEVEL_WARNING:
		StringCchPrintf(loglevel, 20, L"[WARNING]");
		break;
	case LOG_LEVEL_ERROR:
		StringCchPrintf(loglevel, 20, L"[ERROR]");
		break;

	case LOG_LEVEL_SYSTEM:
		StringCchPrintf(loglevel, 20, L"[SYSTEM]");
		break; 
	default:
		break;
	}


	printresult = StringCchPrintf(logbuf, 1024, L"%s[%d%02d%02d _ %02d.%02d.%02d]_[%s]_[%08d] : %s", loglevel, stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, szType, mlogcount, Errbuf);
	if (FAILED(printresult))
	{
		//밀어넣는데 실패 로그 남김 
		memcpy(Errbuf, printerrbuf, 500);
		LOG(L"errlog in logbuf", LOG_LEVEL_ERROR, L"%s", printerrbuf);
	}


	//파일 오픈 

	errno_t err;
	FILE *fp;


	AcquireSRWLockExclusive(&_srwlock);

	int recount = 0;
	for(;;)
	{
		err = _wfopen_s(&fp, filename, L"a");
		if (err == 0)
		{
			break;
		}
		recount++;
		if (recount > 10)
		{
			//파일오픈실패 종료 
			ReleaseSRWLockExclusive(&_srwlock);
			return;
		}

	}
	//파일 오픈 성공 
	fseek(fp, 0, 0);
	fwprintf(fp, logbuf);
	fclose(fp);

	ReleaseSRWLockExclusive(&_srwlock);

}

void LogHex(const WCHAR *szType, int LogLevel, const WCHAR *szLog, BYTE *pByte, int iByteLen)
{

}