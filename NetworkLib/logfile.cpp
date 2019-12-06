#include "pch.h"
#include <time.h>
#include "logfile.h"

LONG logcount = 0;
SRWLOCK _srwlock;

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
//	time_t     now = time(0); //���� �ð��� time_t Ÿ������ ����
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


void Logfile(WCHAR *szType, int LogLevel, WCHAR *szStringFormat, ...)
{
	//�������ϴ� �α� ������ �ƴϸ� ������.
	if (g_loglevel > LogLevel)
	{
		return;
	}


	int mlogcount = InterlockedIncrement(&logcount);

	SYSTEMTIME stNowTime;
	GetLocalTime(&stNowTime);

	WCHAR Errbuf[512] = { 0 };


	//�������� �о�ֱ� 
	va_list va;
	va_start(va, szStringFormat);
	HRESULT vprintresulrt = StringCchVPrintf(Errbuf, 512, szStringFormat,va);
	if(FAILED(vprintresulrt))
	{
		//�о�ִµ� ���� �α� ���� 

	}
	va_end(va);


	//���� �̸� ����� 

	WCHAR filename[512];
	HRESULT printresult = StringCchPrintf(filename,512, L"%s[%d%02d]_%s .txt", L".\\logfile\\", stNowTime.wYear, stNowTime.wMonth, szType);
	if (FAILED(printresult))
	{
		//�о�ִµ� ���� �α� ���� 

	}


	WCHAR logbuf[1024];

	switch (LogLevel)
	{
	case LOG_LEVEL_DEBUG:

		HRESULT printresult = StringCchPrintf(logbuf, 1024, L"[DEBUG][%d%02d%02d _ %02d.%02d.%02d]_[%s]_[%08d] : %s", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, szType,mlogcount, Errbuf);
		if (FAILED(printresult))
		{
			//�о�ִµ� ���� �α� ���� 

		}
		break;
	case LOG_LEVEL_WARNING:
		HRESULT printresult = StringCchPrintf(logbuf, 1024, L"[WARNING][%d%02d%02d _ %02d.%02d.%02d]_[%s]_[%08d] : %s", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, szType, mlogcount, Errbuf);
		if (FAILED(printresult))
		{
			//�о�ִµ� ���� �α� ���� 

		}
		break;
	case LOG_LEVEL_ERROR:

		HRESULT printresult = StringCchPrintf(logbuf, 1024, L"[ERROR][%d%02d%02d _ %02d.%02d.%02d]_[%s]_[%08d] : %s", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, szType, mlogcount, Errbuf);
		if (FAILED(printresult))
		{
			//�о�ִµ� ���� �α� ���� 

		}
		break;

	case LOG_LEVEL_SYSTEM:
		HRESULT printresult = StringCchPrintf(logbuf, 1024, L"[SYSTEM][%d%02d%02d _ %02d.%02d.%02d]_[%s]_[%08d] : %s", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, szType, mlogcount, Errbuf);
		if (FAILED(printresult))
		{
			//�о�ִµ� ���� �α� ���� 

		}
		break; 
	default:
		break;
	}
	//���� ���� 

	errno_t err;
	FILE *fp;

	int recount = 0;
	for(;;)
	{
		err = _wfopen_s(&fp, filename, L"a");
		if (err == 0)
		{
			break;
		}
		recount++;
		if (recount > 5)
		{
			//���Ͽ��½��� �α׳��� 
			break;
		}

	}
	fseek(fp, 0, 0);



	fclose(fp);

}

void LogHex(WCHAR *szType, int LogLevel, WCHAR *szLog, BYTE *pByte, int iByteLen)
{

}