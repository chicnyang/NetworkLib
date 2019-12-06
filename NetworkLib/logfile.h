#ifndef __LOGFILE__
#define __LOGFILE__

//#define serialization_buf_resize 10
//#define serialization_buf_lack 11


enum LoglevelType
{
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_SYSTEM
};

int g_loglevel;
static WCHAR g_logbuf[1024] = { 0 };

//가변인자 함수 

#define _LOG(type,loglevel,fmt,...) logfile(type,loglevel,fmt,...)


void Logfile(WCHAR *szType, int LogLevel, WCHAR *szStringFormat, ...);
void LogHex(WCHAR *szType, int LogLevel, WCHAR *szLog, BYTE *pByte, int iByteLen);

/*#define _LOG(loglevel,fmt,...)								\
do																\
{																\
	if (g_loglevel <= loglevel)									\
	{															\
		wsprintf(g_logbuf,fmt,##__VA_ARGS__);					\
		logfile_create(loglevel,0,g_logbuf);					\
	}															\
} while (0)	*/													



//void logfile_create(DWORD Type, DWORD value, WCHAR* errbuf);



#endif // !__LOGFILE__


