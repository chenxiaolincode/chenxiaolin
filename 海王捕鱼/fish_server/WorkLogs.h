/*
//hioscar

//打印日记

//不适用afx的东西 方便移植

*/

#ifndef WORK_LOG_FILES
#define WORK_LOG_FILES

#pragma once

#define MAX_BUFFER_CHAR	4000

class CWorkLogs
{
public:
	CWorkLogs(void);
	~CWorkLogs(void);

	static CWorkLogs &GetInstance();

public:
	void Set_LogOutPath(char *chLogOutPath);			//设置运行路径里新建的路径

	void Set_Filepath(char *chFileName, BYTE cbAutoIndex=0);//1自增 2使用id
	void debugString(const char* format, ... );			//简易及时输出
	void debugStringBuf(const char* format, ... );		//缓存定量输出 并记录总日志数

	//外部输出
public:
	bool m_bNowPrintf;

	//内部使用
private:
	char m_chLogPath[MAX_PATH];				//运行路径上新建的路径
	char m_chAppPath[MAX_PATH];				//文件运行路径不包括文件名
	char m_chFilepath[MAX_PATH];			//文件全路径包括文件名

	//debugStringBuf才会使用到
	char m_cbInputBuf[MAX_BUFFER_CHAR];		//缓冲日志信息
	unsigned int m_uWaiteWriteIndx;			//缓冲读写记录
	unsigned int m_nFileNums;				//总共记录个数
};

#define LOG_FILE_X CWorkLogs::GetInstance()
#define LOG_FILE_ONE CWorkLogs::GetInstance().debugString
#define LOG_FILE_TWO CWorkLogs::GetInstance().debugStringBuf

//////////////////////////////////////////////////////////////////////////
//以下是多余的 只为保持日志类的简洁而复制
class CWorkLogsEx
{
public:
	CWorkLogsEx(void);
	~CWorkLogsEx(void);

	static CWorkLogsEx &GetInstance();

public:
	void Set_LogOutPath(char *chLogOutPath);			//设置运行路径里新建的路径

	void Set_Filepath(char *chFileName, BYTE cbAutoIndex=0);//1自增 2使用id
	void debugString(const char* format, ... );			//简易及时输出
	void debugStringBuf(const char* format, ... );		//缓存定量输出 并记录总日志数

	//外部输出
public:
	bool m_bNowPrintf;

	//内部使用
private:
	char m_chLogPath[MAX_PATH];				//运行路径上新建的路径
	char m_chAppPath[MAX_PATH];				//文件运行路径不包括文件名
	char m_chFilepath[MAX_PATH];			//文件全路径包括文件名

	//debugStringBuf才会使用到
	char m_cbInputBuf[MAX_BUFFER_CHAR];		//缓冲日志信息
	unsigned int m_uWaiteWriteIndx;			//缓冲读写记录
	unsigned int m_nFileNums;				//总共记录个数
};

#define LOG_FILE_EX_X CWorkLogsEx::GetInstance()
#define LOG_FILE_EX_ONE CWorkLogsEx::GetInstance().debugString
#define LOG_FILE_EX_TWO CWorkLogsEx::GetInstance().debugStringBuf

#endif
