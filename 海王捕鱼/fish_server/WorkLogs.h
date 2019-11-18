/*
//hioscar

//��ӡ�ռ�

//������afx�Ķ��� ������ֲ

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
	void Set_LogOutPath(char *chLogOutPath);			//��������·�����½���·��

	void Set_Filepath(char *chFileName, BYTE cbAutoIndex=0);//1���� 2ʹ��id
	void debugString(const char* format, ... );			//���׼�ʱ���
	void debugStringBuf(const char* format, ... );		//���涨����� ����¼����־��

	//�ⲿ���
public:
	bool m_bNowPrintf;

	//�ڲ�ʹ��
private:
	char m_chLogPath[MAX_PATH];				//����·�����½���·��
	char m_chAppPath[MAX_PATH];				//�ļ�����·���������ļ���
	char m_chFilepath[MAX_PATH];			//�ļ�ȫ·�������ļ���

	//debugStringBuf�Ż�ʹ�õ�
	char m_cbInputBuf[MAX_BUFFER_CHAR];		//������־��Ϣ
	unsigned int m_uWaiteWriteIndx;			//�����д��¼
	unsigned int m_nFileNums;				//�ܹ���¼����
};

#define LOG_FILE_X CWorkLogs::GetInstance()
#define LOG_FILE_ONE CWorkLogs::GetInstance().debugString
#define LOG_FILE_TWO CWorkLogs::GetInstance().debugStringBuf

//////////////////////////////////////////////////////////////////////////
//�����Ƕ���� ֻΪ������־��ļ�������
class CWorkLogsEx
{
public:
	CWorkLogsEx(void);
	~CWorkLogsEx(void);

	static CWorkLogsEx &GetInstance();

public:
	void Set_LogOutPath(char *chLogOutPath);			//��������·�����½���·��

	void Set_Filepath(char *chFileName, BYTE cbAutoIndex=0);//1���� 2ʹ��id
	void debugString(const char* format, ... );			//���׼�ʱ���
	void debugStringBuf(const char* format, ... );		//���涨����� ����¼����־��

	//�ⲿ���
public:
	bool m_bNowPrintf;

	//�ڲ�ʹ��
private:
	char m_chLogPath[MAX_PATH];				//����·�����½���·��
	char m_chAppPath[MAX_PATH];				//�ļ�����·���������ļ���
	char m_chFilepath[MAX_PATH];			//�ļ�ȫ·�������ļ���

	//debugStringBuf�Ż�ʹ�õ�
	char m_cbInputBuf[MAX_BUFFER_CHAR];		//������־��Ϣ
	unsigned int m_uWaiteWriteIndx;			//�����д��¼
	unsigned int m_nFileNums;				//�ܹ���¼����
};

#define LOG_FILE_EX_X CWorkLogsEx::GetInstance()
#define LOG_FILE_EX_ONE CWorkLogsEx::GetInstance().debugString
#define LOG_FILE_EX_TWO CWorkLogsEx::GetInstance().debugStringBuf

#endif
