#ifndef	CDBACTION_H
#define CDBACTION_H
#pragma once

#include "./sqlite/include/sqlite3.h"
#include <iostream>  
#include "sqlitewrapper.h"
#include <afxmt.h>
#include <map>
#include <vector>
#include "../command/CMD_Fish.h"
using namespace std; 
#pragma comment(lib,"./sqlite/lib/sqlite3.lib")

struct tagBulletInfo
{
	int										nBulletID;					//�ӵ�ID	-1�뿪�Ĳ���¼
	int										nBulletKind;				//�ӵ�KIND
	int										nBulletScore;				//�ӵ�����
	int										nWinScore;					//Ӯ�÷��� ==0��ʾ�˳���δ������ӵ�
	int										nBulletOnlyID;				//�ӵ�ΨһID
	int										nElapseTime;				//ʱ��
	int										nGoTime;					//����ʱ��
	int										enFishKind;					//��������
	int										lFishID;					//���ID
	BYTE									cbLeftUseCnt;				//ʣ��ʹ�ô��� ��ϵ����ӵ�ʹ��
	double									fSourcePro;					//ԭ�����
	double									fChangeAddPro;				//��ʱ�¸��� �鿴ϵͳ������ʿ��׷� �Ƿ�����ƽ�����
	LONGLONG								lNowKuCun;					//��ǰ���
	LONGLONG								lScoreCurrent;				//��ǰ����
};

struct tagCatchFishRecord
{
	DWORD									dwGameID;
	int										nBulletID;
	int 									enFishKind;					//������ <100���� =100�����ڷ��� =101����Ⱥ���Ϸ�����  >=200��Ⱥ(�ֽ�����)=200�����ڷ��� =201�������㲻�Ϸ����� 300�������ʹ�û�� 388����
	int 									lFishID;					//fishid<0 �����ڹ���
	int 									nWinScore;
	int 									nBulletOnlyID;
	LONGLONG 								lNowKuCun;					//��ǰ���
	LONGLONG 								lScoreCurrent;				//��ǰ�ڴ���
	double 									nSoucrePro;
	double 									nChangeAddPro;
};

class CDBAction
{
public:
	CDBAction(void);
	~CDBAction(void);

	static CDBAction &GetInstance();

private:
	int										m_nMaxBullet;				//���õ�����ӵ���Ŀ
	TCHAR									m_strPath[256];				//���õ�DB·��
	TCHAR									m_strDbName[256];			//���ݿ�����		
	WORD									m_wKindID;
	WORD									m_wServerID;
	SYSTEMTIME								m_sysStartTime;
	SQLiteWrapper							m_sqlite;
	CCriticalSection						m_CriticalSection;			//��
	map<DWORD, vector<tagBulletInfo> >		m_mapUserFire;				//�ӵ���¼

public:
	//��ʼ��
	void SetService(WORD wKindID, WORD wServerID, SYSTEMTIME sysTime);
	//�����ݿ�����
	bool Open();
	void Close();
	//����
	bool ReadConfig();
	//��ҷ��� dwTicktTime���� ȷ��Ψһ��
	bool UserFire(DWORD dwGameID, int nBulletID, int nBulletKind, int nScore, int bulletOnlyID, BYTE cbUseLeftCnt);
	//������
	bool CatchFish(tagCatchFishRecord *pCathLog);
	//���վ��
	bool StandUp(DWORD dwGameID, LONGLONG lScoreCurrent);
	//��ʱд��Sqlite
	bool WriteUserFireToSqlite(bool bForce= false);

private:
	void TraceString(enTraceLevel level, const char* format, ...);
	bool WriteDataToSqlite(DWORD dwGameID, const tagBulletInfo &Bullet);
	CString FormatTime(SYSTEMTIME sys);
};

#endif//CDBACTION_H
