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

extern int g_loglevel;


//가변인자 함수 

#define LOG Logfile


void Logfile(const WCHAR *szType, int LogLevel, const WCHAR *szStringFormat, ...);
void LogHex(const WCHAR *szType, int LogLevel, const WCHAR *szLog, BYTE *pByte, int iByteLen);

#endif // !__LOGFILE__

//#define LOG(type,loglevel,fmt,...) 

