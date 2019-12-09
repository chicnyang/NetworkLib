#pragma once
#include <Windows.h>
#include <iostream>
#include <dbghelp.h>
#include <crtdbg.h>
#pragma comment(lib, "DbgHelp.Lib")

class cDump
{
public:
	cDump()
	{
		dumpcount = 0;
		_invalid_parameter_handler oldhandler, newhandler;
		newhandler = myInvalidParameterHandler;

		oldhandler = _set_invalid_parameter_handler(newhandler);
		_CrtSetReportMode(_CRT_WARN,0);
		_CrtSetReportMode(_CRT_ASSERT, 0);
		_CrtSetReportMode(_CRT_ERROR, 0);

		_CrtSetReportHook(_custom_Report_hook);


		_set_purecall_handler(myPurecallHandler);

		SetHandlerDump();

	}

	static void Crash(void)
	{
		int* p = nullptr;
		*p = 0;
	}

	static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
	{


		SYSTEMTIME stNowTime;

		long iDunpcount = InterlockedIncrement(&dumpcount);

		//메모리 사용량 알아보는 구간



		WCHAR filename[100];

		GetLocalTime(&stNowTime);
		wsprintf(filename,L"Dump _ %d%02d%02d _ %02d.%02d.%02d _ %d .dmp",stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, iDunpcount);

		wprintf(L"crash error");

		HANDLE hDumpfile = CreateFile(filename,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

		if (hDumpfile != INVALID_HANDLE_VALUE)
		{
			_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

			MinidumpExceptionInformation.ThreadId = GetCurrentThreadId();
			MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
			MinidumpExceptionInformation.ClientPointers = TRUE;


			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),hDumpfile,MiniDumpWithFullMemory,&MinidumpExceptionInformation,NULL,NULL);

			CloseHandle(hDumpfile);

			wprintf(L"dump save");

		}

		return EXCEPTION_EXECUTE_HANDLER;
	}

	static void  SetHandlerDump()
	{
		SetUnhandledExceptionFilter(MyExceptionFilter);
	}


	static void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserverd)
	{
		Crash();
	}
	static int _custom_Report_hook(int ireposttype, char *massage, int* returnvalue)
	{
		Crash();
		return true;
	}

	static void myPurecallHandler(void)
	{
		Crash();
	}

	~cDump() {}

	static long dumpcount;
};