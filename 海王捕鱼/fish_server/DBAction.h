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
	int										nBulletID;					//子弹ID	-1离开的不记录
	int										nBulletKind;				//子弹KIND
	int										nBulletScore;				//子弹分数
	int										nWinScore;					//赢得分数 ==0表示退出还未结算的子弹
	int										nBulletOnlyID;				//子弹唯一ID
	int										nElapseTime;				//时间
	int										nGoTime;					//持续时间
	int										enFishKind;					//命中类型
	int										lFishID;					//鱼的ID
	BYTE									cbLeftUseCnt;				//剩余使用次数 配合道具子弹使用
	double									fSourcePro;					//原随机数
	double									fChangeAddPro;				//当时新概率 查看系统随机概率靠谱否 是否启用平均随机
	LONGLONG								lNowKuCun;					//当前库存
	LONGLONG								lScoreCurrent;				//当前分数
};

struct tagCatchFishRecord
{
	DWORD									dwGameID;
	int										nBulletID;
	int 									enFishKind;					//正常鱼 <100正常 =100不存在返分 =101打鱼群不合法返分  >=200鱼群(又叫鱼阵)=200不存在返分 =201打正常鱼不合法返分 300加鱼类型打没中 388打中
	int 									lFishID;					//fishid<0 能量炮攻击
	int 									nWinScore;
	int 									nBulletOnlyID;
	LONGLONG 								lNowKuCun;					//当前库存
	LONGLONG 								lScoreCurrent;				//当前内存金币
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
	int										m_nMaxBullet;				//配置的最大子弹数目
	TCHAR									m_strPath[256];				//配置的DB路径
	TCHAR									m_strDbName[256];			//数据库名字		
	WORD									m_wKindID;
	WORD									m_wServerID;
	SYSTEMTIME								m_sysStartTime;
	SQLiteWrapper							m_sqlite;
	CCriticalSection						m_CriticalSection;			//锁
	map<DWORD, vector<tagBulletInfo> >		m_mapUserFire;				//子弹记录

public:
	//初始化
	void SetService(WORD wKindID, WORD wServerID, SYSTEMTIME sysTime);
	//打开数据库连接
	bool Open();
	void Close();
	//配置
	bool ReadConfig();
	//玩家发炮 dwTicktTime必须 确定唯一性
	bool UserFire(DWORD dwGameID, int nBulletID, int nBulletKind, int nScore, int bulletOnlyID, BYTE cbUseLeftCnt);
	//命中鱼
	bool CatchFish(tagCatchFishRecord *pCathLog);
	//玩家站起
	bool StandUp(DWORD dwGameID, LONGLONG lScoreCurrent);
	//定时写入Sqlite
	bool WriteUserFireToSqlite(bool bForce= false);

private:
	void TraceString(enTraceLevel level, const char* format, ...);
	bool WriteDataToSqlite(DWORD dwGameID, const tagBulletInfo &Bullet);
	CString FormatTime(SYSTEMTIME sys);
};

#endif//CDBACTION_H
