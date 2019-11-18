#include "StdAfx.h"
#include ".\dbaction.h"
#include <string>
#include <iosfwd>
#include <sstream>
#include <algorithm>
#include <io.h>
#include <direct.h>
#include <shlwapi.h>
using namespace std;

CDBAction::CDBAction(void)
{
	//m_mapUserFire.clear();
	ReadConfig();
}

CDBAction::~CDBAction(void)
{
	WriteUserFireToSqlite(true);
}

CDBAction &CDBAction::GetInstance()
{
	static CDBAction DBAction;
	return DBAction;
}

bool CDBAction::ReadConfig()
{
	//�����ļ���
	TCHAR szPath[MAX_PATH]=TEXT(""),szIniFileName[MAX_PATH]=TEXT(""), szDbPath[MAX_PATH]=TEXT("");
	GetCurrentDirectory(CountArray(szPath),szPath);	
	PathRemoveFileSpec(szPath);

	_sntprintf(szIniFileName,CountArray(szIniFileName),TEXT("%s\\Sqlite3Config.ini"),szPath);
	m_nMaxBullet= GetPrivateProfileInt(TEXT("DB_HW2"), TEXT("MaxBullet"), 5000, szIniFileName);
	GetPrivateProfileString(TEXT("DB_HW2"),TEXT("Path"),TEXT("D:\\HW2RecordDB\\"),szDbPath, CountArray(szDbPath), szIniFileName);

	//time
	time_t ltime;
	struct tm *today;
	time(&ltime);
	today = localtime(&ltime);
	TCHAR		buffer[256]={0};
	strftime(buffer, 256, "%m-%d %H-%M-%S", today);
	_sntprintf(m_strPath,sizeof(m_strPath),TEXT("%s\\%s\\"),szDbPath,buffer);

	return true;
}

//��־���
void CDBAction::TraceString(enTraceLevel level , const char* format, ... )
{
	va_list		arglist;
	char		buffer[1000]={0};
	va_start(arglist, format);

	_vsnprintf(buffer, 1000, format, arglist);

	CTraceService::TraceString(buffer, level);
}

//��ʽ��ʱ��
CString CDBAction::FormatTime(SYSTEMTIME sys)
{
	CString str;
	str.Format("%0.2d%0.2d%0.2d%0.2d%0.2d%0.2d%0.3d",sys.wYear%2000,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	
	return str;
}

//������ϷID�ͷ���ID
void CDBAction::SetService(WORD wKindID, WORD wServerID, SYSTEMTIME sysTime)
{
	m_wKindID = wKindID;
	m_wServerID = wServerID;
	m_sysStartTime=sysTime;

	if(!PathIsDirectory(m_strPath))
	{
		char bufFile[MAX_PATH]={0};
		size_t nlen = lstrlen(m_strPath);
		for(int i=0;i<nlen;i++)  
		{  
			if (m_strPath[i]=='\\')  
			{  
				_sntprintf(bufFile,i,"%s",m_strPath);
				if (access(bufFile,6)==-1)  
				{  
					mkdir(bufFile);
				}
			}
		}
	}

	_sntprintf(m_strDbName,sizeof(m_strDbName),TEXT("%sDB_%d_%d_%d_%d_%d.db3"), m_strPath,m_wKindID, m_wServerID, sysTime.wYear, sysTime.wMonth, sysTime.wDay);
	if(Open())
	{
	//����д�ּ�¼��
	string strActionUserFire = "CREATE TABLE if not exists [ActionUserFire](\
								[dwGameID] [int] NOT NULL,\
								[nBulletID] [int] NOT NULL,\
								[nBulletKind] [int] NOT NULL,\
								[nBulletScore] [int] NOT NULL,\
								[nWinScore] [int] NOT NULL,\
								[enFishKind] [int] NOT NULL,\
								[lFishID][int] NOT NULL,\
								[fSourcePro][double] NOT NULL,\
								[fChangeAddPro][double] NOT NULL,\
								[lNowKuCun] [bigint] NOT NULL,\
								[lScoreCurrent] [bigint] NOT NULL,\
								[nBulletGoTime] [int] NOT NULL,\
								[dwFireTime] [int] NOT NULL\
								)";

	if (!m_sqlite.DirectStatement(strActionUserFire)) 
	{
		CString strTemp;
		strTemp.Format(TEXT("%s,ActionUserFire�����쳣,%s"), m_strDbName,m_sqlite.LastError().c_str());
		CTraceService::TraceString(strTemp,TraceLevel_Exception);
	}

	////��������
	//if (!m_sqlite.DirectStatement("CREATE INDEX idx_Account ON ActionUserFire(strAccount)")) 
	//{
	//	CTraceService::TraceString("ActionUserFire����������ʧ��,%s", TraceLevel_Exception);
	//}
}
}

//�����ݿ�����
bool CDBAction::Open()
{
	try
	{
		if (!m_sqlite.Open((LPCTSTR)m_strDbName)) {
			CTraceService::TraceString(m_sqlite.LastError().c_str(), TraceLevel_Exception);
			return false;
		}
	}
	catch (...)
	{
		CTraceService::TraceString("SqlLite3��ʧ��", TraceLevel_Info); 
		return false;
	}

	return true;
}

void CDBAction::Close()
{
	sqlite3_close(m_sqlite.db_);
}

//��ҷ���
bool CDBAction::UserFire(DWORD dwGameID, int nBulletID, int nBulletKind, int nScore, int bulletOnlyID, BYTE cbUseLeftCnt)
{
	if (m_mapUserFire.count(dwGameID) == 0)
	{
		vector<tagBulletInfo> vec;
		m_mapUserFire[dwGameID] = vec;
	}

	tagBulletInfo Bullet;
	Bullet.nBulletID = nBulletID;
	Bullet.nBulletKind = nBulletKind;
	Bullet.nBulletScore = nScore;
	Bullet.nBulletOnlyID = bulletOnlyID;
	Bullet.nElapseTime = GetTickCount();
	Bullet.nWinScore = 0;
	Bullet.enFishKind = FISH_KIND_COUNT;
	Bullet.lFishID = 0;
	Bullet.fSourcePro=0.0f;
	Bullet.fChangeAddPro=0.0f;
	Bullet.lScoreCurrent = 0;
	Bullet.lNowKuCun = 0;
	Bullet.nGoTime = 0;
	Bullet.cbLeftUseCnt=(cbUseLeftCnt>0)?cbUseLeftCnt:1;

	m_mapUserFire[dwGameID].push_back(Bullet);
	return true;
}

//������
bool CDBAction::CatchFish(tagCatchFishRecord *pCathLog)
{
	DWORD dwGameID=pCathLog->dwGameID;
	vector<tagBulletInfo>::iterator iter = m_mapUserFire[dwGameID].begin();
	for (;iter != m_mapUserFire[dwGameID].end();iter++)
	{
		tagBulletInfo &DataTmp = (*iter);
		if ( (DataTmp.nBulletID == pCathLog->nBulletID) && (DataTmp.nBulletOnlyID == pCathLog->nBulletOnlyID) )//ȷ��Ψһ��
		{
			DataTmp.enFishKind = pCathLog->enFishKind;
			DataTmp.nWinScore += pCathLog->nWinScore;
			DataTmp.lFishID = pCathLog->lFishID;
			DataTmp.fSourcePro = pCathLog->nSoucrePro;
			DataTmp.fChangeAddPro = pCathLog->nChangeAddPro;
			DataTmp.lNowKuCun = pCathLog->lNowKuCun;
			DataTmp.lScoreCurrent = pCathLog->lScoreCurrent;
			DataTmp.nGoTime = GetTickCount()-DataTmp.nElapseTime;
			if(DataTmp.cbLeftUseCnt>0) DataTmp.cbLeftUseCnt--;

			//_tprintf("��ǰ�ڴ棺%I64d  �ӵ�id��%d\n",DataTmp.lScoreCurrent,DataTmp.nBulletID);
			break;
		}
	}

	{
		CThreadLock lock(m_CriticalSection);

		static int nCount = 0;
		nCount++;
		//if (nCount%1000==0)
		//{
		//	CString strInfo;
		//	strInfo.Format("��ǰ�ӵ���Ŀ%d", nCount);
		//	CTraceService::TraceString(strInfo, TraceLevel_Warning);
		//}
		if (nCount >= m_nMaxBullet)
		{
			if(WriteUserFireToSqlite(false))
			{
				nCount = 0;
				ReadConfig();

				SYSTEMTIME sys; 
				GetLocalTime(&sys);
				if ((sys.wYear!=m_sysStartTime.wYear && sys.wDayOfWeek==m_sysStartTime.wDayOfWeek) || 
					(sys.wMonth!=m_sysStartTime.wMonth && sys.wDayOfWeek==m_sysStartTime.wDayOfWeek) ||
					(sys.wDay!=m_sysStartTime.wDay && sys.wDayOfWeek==m_sysStartTime.wDayOfWeek) )
				{
					sqlite3_close(m_sqlite.db_);
					SetService(m_wKindID,m_wServerID,sys);
				}
			}
		}
	}

	return true;
}

//���վ��
bool CDBAction::StandUp(DWORD dwGameID, LONGLONG lScoreCurrent)
{
	if(m_mapUserFire.count(dwGameID) > 0)
	{
		//��������δ���е��ӵ�
		vector<tagBulletInfo>::iterator iter = m_mapUserFire[dwGameID].begin();
		for (;iter != m_mapUserFire[dwGameID].end();iter++)
		{
			tagBulletInfo &DataTmp = (*iter);
			if (DataTmp.nBulletID>0 && DataTmp.enFishKind == FISH_KIND_COUNT)//ע��������뿪����Ҫ�ٰ���
			{
				DataTmp.enFishKind =  88;
				DataTmp.nWinScore = 0;
				DataTmp.lScoreCurrent = lScoreCurrent;

				//_tprintf("1--GameID:%ld,�˳�--�ӵ���(%d, %d),��ǰ�ڴ棺%I64d\n",dwGameID, DataTmp.nBulletID, DataTmp.nBulletOnlyID, DataTmp.lScoreCurrent);
			}
		}
	}

	//�뿪����дһ�μ�¼
	tagBulletInfo Bullet;
	ZeroMemory(&Bullet,sizeof(Bullet));
	Bullet.nBulletID = -1;
	Bullet.nBulletScore = 0;
	static int nOnlyID=0;
	++nOnlyID;
	if (nOnlyID <= 0) nOnlyID = 1;

	Bullet.nBulletOnlyID = -nOnlyID;
	Bullet.lScoreCurrent = lScoreCurrent;
	m_mapUserFire[dwGameID].push_back(Bullet);
	//_tprintf("2--GameID:%ld,�˳�--�ӵ���(%d, %d),��ǰ�ڴ棺%I64d\n",dwGameID, Bullet.nBulletID, Bullet.nBulletOnlyID, Bullet.lScoreCurrent);
	
	return true;
}

//��ʱд��Sqlite
bool CDBAction::WriteUserFireToSqlite(bool bForce)
{
	//if (Open())
	{
		CThreadLock lock(m_CriticalSection);
		m_sqlite.Begin();

		int nCount = 0,nErrorNum=0;
		for (map<DWORD, vector<tagBulletInfo> >::iterator iter = m_mapUserFire.begin(); iter!= m_mapUserFire.end();)
		{
			for (vector<tagBulletInfo>::iterator i = iter->second.begin(); i!= iter->second.end();)
			{
				tagBulletInfo &DataTmp = (*i);
				bool bWirte=((DataTmp.enFishKind != FISH_KIND_COUNT) ||(-1==DataTmp.nBulletID) || bForce);
				if(bWirte && (DataTmp.cbLeftUseCnt==0))
				{
					if (WriteDataToSqlite(iter->first, DataTmp))
					{
						nCount ++;
					}else nErrorNum++;
					i = iter->second.erase(i);//��Χ��ֹ��ѭ��
				}
				else
				{
					i++;
				}
			}

			if (iter->second.size() == 0)
			{
				iter = m_mapUserFire.erase(iter);
			}
			else
			{
				iter++;
			}
		}
		m_sqlite.Commit();

		//CString strInfo;
		//if(nErrorNum<=0) strInfo.Format("����д���ӵ���%d", nCount);
		//else strInfo.Format("����д���ӵ���%d ʧ�ܣ�%d��", nCount,nErrorNum);
		//CTraceService::TraceString(strInfo, TraceLevel_Info); 
		return true;
	}

	return false;
}

//д��sqlite
bool CDBAction::WriteDataToSqlite(DWORD dwGameID, const tagBulletInfo &Bullet)
{
	CString strSqlInsert;
	strSqlInsert.Format("insert into ActionUserFire \
		(dwGameID,nBulletID,nBulletKind,nBulletScore,nWinScore,enFishKind,lfishID,fSourcePro,fChangeAddPro,lNowKuCun,lScoreCurrent,nBulletGoTime,dwFireTime) \
		values(%ld, %d, %d, %d, %d, %d, %d,%6f,%6f, %I64d,%I64d,%d,%ld)",
		dwGameID,
		Bullet.nBulletID,
		Bullet.nBulletKind,
		Bullet.nBulletScore,
		Bullet.nWinScore,
		Bullet.enFishKind,
		Bullet.lFishID,
		Bullet.fSourcePro,
		Bullet.fChangeAddPro,
		Bullet.lNowKuCun,
		Bullet.lScoreCurrent,
		Bullet.nGoTime,
		Bullet.nElapseTime
		);

	string strInsert = strSqlInsert;
	if (!m_sqlite.DirectStatement(strInsert)) {
		CTraceService::TraceString(m_sqlite.LastError().c_str(), TraceLevel_Info); 
		return false;
	}

	return true;
}
