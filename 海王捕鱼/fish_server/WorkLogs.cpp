#include "StdAfx.h"
#include ".\worklogs.h"
#include <io.h>
#include <direct.h>

#define DEFAULT_LOG "gamelog"
#define DEFAULT_PAHT "record"
#define DateTimeFormat "%y/%m/%d %H:%M:%S\t"

#define MAX_FILE_LOG_NUM	500					//超过后 每次增加都删除一个
#define BUFFER_MAX_LOG		(MAX_BUFFER_CHAR+100)				//

//静态变量
//unsigned int CWorkLogs::m_nFileNums = 0;

CWorkLogs &CWorkLogs::GetInstance()
{
	static CWorkLogs log;
	return log;
}

CWorkLogs::CWorkLogs(void)
{
	m_bNowPrintf=false;
	memset(m_chAppPath,0,sizeof(m_chAppPath));
	memset(m_chFilepath,0,sizeof(m_chFilepath));

	m_uWaiteWriteIndx = 0;
	memset(m_cbInputBuf,0,sizeof(m_cbInputBuf));

	m_nFileNums = 0;
	sprintf(m_chLogPath,"%s",DEFAULT_PAHT);

	//打印日记
	char strFile[256]=TEXT("0");
	GetModuleFileName(0,strFile,sizeof(strFile));
	for (size_t i=lstrlen(strFile)-1;i>0;i--)
	{
		if(strFile[i]=='\\')
		{
			strFile[i]=0;
			break;
		}
	}
	_stprintf(m_chAppPath,"%s",strFile);
	_stprintf(m_chFilepath,"%s\\%s\\%s.log",strFile,DEFAULT_PAHT,DEFAULT_LOG);

	char bufFile[MAX_PATH]={0};
	size_t nlen = lstrlen(m_chFilepath);
	for(int i=0;i<nlen;i++)  
	{  
		if (m_chFilepath[i]=='\\')  
		{  
			_sntprintf(bufFile,i,"%s",m_chFilepath);
			if (access(bufFile,6)==-1)  
			{  
				mkdir(bufFile);
			}
		}
	}
	debugString("日志初始化成功xxxxxxxxxxxxxxxxxxxxxxxxxxx！");
}

CWorkLogs::~CWorkLogs(void)
{
}

void CWorkLogs::Set_LogOutPath(char *chLogOutPath)
{
	if(chLogOutPath==0) return;

	_sntprintf(m_chLogPath,sizeof(m_chLogPath),"%s",chLogOutPath);

	Set_Filepath(DEFAULT_LOG,2);
}

void CWorkLogs::Set_Filepath(char *chFileName, BYTE cbAutoIndex)
{
	if(chFileName==0) return;

	if(0==cbAutoIndex)
	{
		_sntprintf(m_chFilepath,sizeof(m_chFilepath),"%s\\%s\\%s.log",m_chAppPath,m_chLogPath,chFileName);
	}
	else
	{
		if(cbAutoIndex==1) _sntprintf(m_chFilepath,sizeof(m_chFilepath),"%s\\%s\\%s%d.log",m_chAppPath,m_chLogPath,DEFAULT_LOG,m_nFileNums++);
		else _sntprintf(m_chFilepath,sizeof(m_chFilepath),"%s\\%s\\%s%d.log",m_chAppPath,m_chLogPath,DEFAULT_LOG,m_nFileNums);
	}

	char bufFile[MAX_PATH]={0};
	size_t nlen = lstrlen(m_chFilepath);
	for(int i=0;i<nlen;i++)  
	{  
		if (m_chFilepath[i]=='\\')  
		{  
			_sntprintf(bufFile,i,"%s",m_chFilepath);
			if (access(bufFile,6)==-1)  
			{  
				mkdir(bufFile);
			}
		}
	}

	//debugString("切换并新建目录完成: %s",m_chFilepath);
}

void CWorkLogs::debugString(const char* format, ... )
{
	FILE *hf = NULL;

	char chFilePathT[MAX_PATH];
	_stprintf(chFilePathT,"%s\\%s\\%s.log",m_chAppPath,m_chLogPath,DEFAULT_LOG);

	hf = fopen(chFilePathT, "a");
	if(!hf) return;
	
	fseek(hf,0L,SEEK_END);
	long fsize=ftell(hf);
	//不需要太大的日记
	if(fsize>1024*1024*2)
	{
		hf = fopen(chFilePathT, "w");
		if(!hf) return;
	}

	char		buffer[BUFFER_MAX_LOG];
	time_t		ltime;
	struct tm	*today;
	int			index;

	//time
	time(&ltime);
	today = localtime(&ltime);
	strftime(buffer, BUFFER_MAX_LOG, DateTimeFormat, today);
	index = int(lstrlen(buffer));

	//char
	va_list	arglist;
	va_start(arglist, format);
	_vsnprintf(buffer+index, BUFFER_MAX_LOG-index-1, format, arglist);
	vfprintf(hf, buffer, arglist);
	va_end(arglist);

	fprintf(hf, "\n");

	fclose(hf);


#ifdef _DEBUG
	TRACE(buffer);
#endif
}


void CWorkLogs::debugStringBuf(const char* format, ... )
{
	char		buffer[BUFFER_MAX_LOG];
	time_t		ltime;
	struct tm	*today;
	int			index;

	//time
	time(&ltime);
	today = localtime(&ltime);
	strftime(buffer, BUFFER_MAX_LOG, DateTimeFormat, today);
	index = int(lstrlen(buffer));

	//char
	va_list	arglist;
	va_start(arglist, format);
	int nRet=_vsnprintf(buffer+index, BUFFER_MAX_LOG-index-1, format, arglist);
	va_end(arglist);

	if(m_uWaiteWriteIndx+strlen(buffer)<MAX_BUFFER_CHAR-100) //留给换行等
	{
		int nlen=_sntprintf(&m_cbInputBuf[m_uWaiteWriteIndx],sizeof(m_cbInputBuf),"%s\n",buffer);
		m_uWaiteWriteIndx += nlen;//strlen(buffer);
	}
	if(m_uWaiteWriteIndx+strlen(buffer)>=MAX_BUFFER_CHAR/2 || m_bNowPrintf)
	{
		//仅一次控制
		if(m_bNowPrintf) m_bNowPrintf=false;

		FILE *hf = NULL;

		hf = fopen(m_chFilepath, "a");
		if(!hf) return;

		fseek(hf,0L,SEEK_END);
		long fsize=ftell(hf);
		//不需要太大的日记
		if(fsize>1024*1024*4)
		{
			//影响心梗
			if(m_nFileNums>MAX_FILE_LOG_NUM)
			{
				char szFind[128]={0};
				_sntprintf(szFind,sizeof(szFind),"%s\\%s\\%s%d.log",m_chAppPath,m_chLogPath,DEFAULT_LOG,m_nFileNums-MAX_FILE_LOG_NUM);
				
				if (!DeleteFile(szFind))
				{
					int dosretval = GetLastError();

					dosretval = ERROR_ACCESS_DENIED;
					dosretval = ERROR_SHARING_VIOLATION;
				}
			}
			fclose(hf);

			//重新打开
			Set_Filepath(DEFAULT_LOG,1);

			hf = fopen(m_chFilepath, "a");
			if(!hf) return;

			//不需要太大的日记
			fsize=ftell(hf);
			if(fsize>1024*1024*4)
			{
				hf = fopen(m_chFilepath, "w");
				if(!hf) return;
			}
			fseek(hf,0L,SEEK_END);
		}

		fwrite(m_cbInputBuf,1,m_uWaiteWriteIndx,hf);
		//fprintf(hf,"%s",m_cbInputBuf);
		//fprintf(hf, "\n");
		fclose(hf);

		m_cbInputBuf[0]=0;
		m_uWaiteWriteIndx=0;
	}
	
	//////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////
//多余的只为简化CWorkLogs而复制
//////////////////////////////////////////////////////////////////////////
CWorkLogsEx &CWorkLogsEx::GetInstance()
{
	static CWorkLogsEx log;
	return log;
}

CWorkLogsEx::CWorkLogsEx(void)
{
	m_bNowPrintf=false;
	memset(m_chAppPath,0,sizeof(m_chAppPath));
	memset(m_chFilepath,0,sizeof(m_chFilepath));

	m_uWaiteWriteIndx = 0;
	memset(m_cbInputBuf,0,sizeof(m_cbInputBuf));

	m_nFileNums = 0;
	sprintf(m_chLogPath,"%s",DEFAULT_PAHT);

	//打印日记
	char strFile[256]=TEXT("0");
	GetModuleFileName(0,strFile,sizeof(strFile));
	for (size_t i=lstrlen(strFile)-1;i>0;i--)
	{
		if(strFile[i]=='\\')
		{
			strFile[i]=0;
			break;
		}
	}
	_stprintf(m_chAppPath,"%s",strFile);
	_stprintf(m_chFilepath,"%s\\%s\\%s.log",strFile,m_chLogPath,DEFAULT_LOG);

	char bufFile[MAX_PATH]={0};
	size_t nlen = lstrlen(m_chFilepath);
	for(int i=0;i<nlen;i++)  
	{  
		if (m_chFilepath[i]=='\\')  
		{  
			_sntprintf(bufFile,i,"%s",m_chFilepath);
			if (access(bufFile,6)==-1)  
			{  
				mkdir(bufFile);
			}
		}
	}
	debugString("日志初始化成功xxxxxxxxxxxxxxxxxxxxxxxxxxx！");
}

CWorkLogsEx::~CWorkLogsEx(void)
{
}

void CWorkLogsEx::Set_LogOutPath(char *chLogOutPath)
{
	if(chLogOutPath==0) return;

	_sntprintf(m_chLogPath,sizeof(m_chLogPath),"%s",chLogOutPath);

	Set_Filepath(DEFAULT_LOG,2);
}

void CWorkLogsEx::Set_Filepath(char *chFileName, BYTE cbAutoIndex)
{
	if(chFileName==0) return;

	if(0==cbAutoIndex)
	{
		_sntprintf(m_chFilepath,sizeof(m_chFilepath),"%s\\%s\\%s.log",m_chAppPath,m_chLogPath,chFileName);
	}
	else
	{
		if(cbAutoIndex==1) _sntprintf(m_chFilepath,sizeof(m_chFilepath),"%s\\%s\\%s%d.log",m_chAppPath,m_chLogPath,DEFAULT_LOG,m_nFileNums++);
		else _sntprintf(m_chFilepath,sizeof(m_chFilepath),"%s\\%s\\%s%d.log",m_chAppPath,m_chLogPath,DEFAULT_LOG,m_nFileNums);
	}

	char bufFile[MAX_PATH]={0};
	size_t nlen = lstrlen(m_chFilepath);
	for(int i=0;i<nlen;i++)  
	{  
		if (m_chFilepath[i]=='\\')  
		{  
			_sntprintf(bufFile,i,"%s",m_chFilepath);
			if (access(bufFile,6)==-1)  
			{  
				mkdir(bufFile);
			}
		}
	}

	//debugString("切换并新建目录完成: %s",m_chFilepath);
}

void CWorkLogsEx::debugString(const char* format, ... )
{
	FILE *hf = NULL;

	char chFilePathT[MAX_PATH];
	_stprintf(chFilePathT,"%s\\%s\\%s.log",m_chAppPath,m_chLogPath,DEFAULT_LOG);

	hf = fopen(chFilePathT, "a");
	if(!hf) return;

	fseek(hf,0L,SEEK_END);
	long fsize=ftell(hf);
	//不需要太大的日记
	if(fsize>1024*1024*2)
	{
		hf = fopen(chFilePathT, "w");
		if(!hf) return;
	}

	char		buffer[BUFFER_MAX_LOG];
	time_t		ltime;
	struct tm	*today;
	int			index;

	//time
	time(&ltime);
	today = localtime(&ltime);
	strftime(buffer, BUFFER_MAX_LOG, DateTimeFormat, today);
	index = int(lstrlen(buffer));

	//char
	va_list	arglist;
	va_start(arglist, format);
	_vsnprintf(buffer+index, BUFFER_MAX_LOG-index-1, format, arglist);
	vfprintf(hf, buffer, arglist);
	va_end(arglist);

	fprintf(hf, "\n");

	fclose(hf);


#ifdef _DEBUG
	TRACE(buffer);
#endif
}


void CWorkLogsEx::debugStringBuf(const char* format, ... )
{
	char		buffer[BUFFER_MAX_LOG];
	time_t		ltime;
	struct tm	*today;
	int			index;

	//time
	time(&ltime);
	today = localtime(&ltime);
	strftime(buffer, BUFFER_MAX_LOG, DateTimeFormat, today);
	index = int(lstrlen(buffer));

	//char
	va_list	arglist;
	va_start(arglist, format);
	int nRet=_vsnprintf(buffer+index, BUFFER_MAX_LOG-index-1, format, arglist);
	va_end(arglist);

	if(m_uWaiteWriteIndx+strlen(buffer)<MAX_BUFFER_CHAR-100) //留给换行等
	{
		int nlen=_sntprintf(&m_cbInputBuf[m_uWaiteWriteIndx],sizeof(m_cbInputBuf),"%s\n",buffer);
		m_uWaiteWriteIndx += nlen;//strlen(buffer);
	}
	if(m_uWaiteWriteIndx+strlen(buffer)>=MAX_BUFFER_CHAR/2 || m_bNowPrintf)
	{
		//仅一次控制
		if(m_bNowPrintf) m_bNowPrintf=false;

		FILE *hf = NULL;

		hf = fopen(m_chFilepath, "a");
		if(!hf) return;

		fseek(hf,0L,SEEK_END);
		long fsize=ftell(hf);
		//不需要太大的日记
		if(fsize>1024*1024*4)
		{
			//影响心梗
			if(m_nFileNums>MAX_FILE_LOG_NUM)
			{
				char szFind[128]={0};
				_sntprintf(szFind,sizeof(szFind),"%s\\%s\\%s%d.log",m_chAppPath,m_chLogPath,DEFAULT_LOG,m_nFileNums-MAX_FILE_LOG_NUM);

				if (!DeleteFile(szFind))
				{
					int dosretval = GetLastError();

					dosretval = ERROR_ACCESS_DENIED;
					dosretval = ERROR_SHARING_VIOLATION;
				}
			}
			fclose(hf);

			//重新打开
			Set_Filepath(DEFAULT_LOG,1);

			hf = fopen(m_chFilepath, "a");
			if(!hf) return;

			//不需要太大的日记
			fsize=ftell(hf);
			if(fsize>1024*1024*4)
			{
				hf = fopen(m_chFilepath, "w");
				if(!hf) return;
			}
			fseek(hf,0L,SEEK_END);
		}

		fwrite(m_cbInputBuf,1,m_uWaiteWriteIndx,hf);
		//fprintf(hf,"%s",m_cbInputBuf);
		//fprintf(hf, "\n");
		fclose(hf);

		m_cbInputBuf[0]=0;
		m_uWaiteWriteIndx=0;
	}

	//////////////////////////////////////////////////////////////////////////
}