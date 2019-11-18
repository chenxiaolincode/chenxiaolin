
#ifndef TABLE_FRAME_SINK_H_
#define TABLE_FRAME_SINK_H_
#pragma once

#include <map>
#include <vector>
#include "WorkLogs.h"
#include "ServerMoveManage.h"
#include "game_config.h"
#include "GRand/grand.h"
//////////////////////////////////////////////////////////////////////////
//����ټ��ṹ��Ϣ
struct FishTraceInfo
{
	int fish_id;
	int fish_tag;					//�㸨������ fish_kind
	int fish_king_id;				//��Ⱥ4��������ID��������û���õ�
	int nTrace_id;					//���·��ID
	DWORD build_tick;				//����ʱ��
	FishKind fish_kind;				//��������
	WORD wFireCount[GAME_PLAYER];	//
};

//������ӵ���Ϣ
struct ServerBulletInfo
{
	int bullet_id;					//�����û��ӵ�id
	int bullet_idx_;				//����ʹ��
	int nLockFishID;				//�������ӵ� ���������ײ������ ������͸���
	int bullet_mulriple;			//�ӵ�����
	BulletKind bullet_kind;			//�ӵ�����
	DWORD dwTicktCount;				//
	BYTE cbMaxUse;					// Ĭ��=0 ��ͷ��6�� �ҵ�3�� �����1�� ��OnSubCatchGroupFish��Ч
	bool bCheat;					//Ĭ��false ���ͷ�
	BYTE cbBoomKind;				//�������࣬�������ӵ�ΪINVALID_BYTE
};

//��������ģʽ
struct TimeFreeBullet
{
	WORD wFireCount;				//������ʱ�ۼ��ӵ��� �ο����35
	int mulriple;					//������ʱ��� �ӵ�����
	DWORD dwFreeTime;				//�����ʾ���״̬ ��λms �͵�ǰʱ�������
};

//���ֵ�����
struct ToolGetBullet
{
	BulletKind bullet_kind;			//�ӵ�����
	int bullet_mulriple;			//�ӵ�����
};

struct ToolFreeBullet
{
	BYTE cbMaxBullet;				//������������ӵ���
	int bullet_id;					//����ӵ���id
	int TimeGold;					//��ǰ����ӵ��ۼƵ÷�
};

//�ӵ��ٶ� - �����ٶ�У��
struct tagFireSpeed
{
	int		nTotalBullet;

	int		nSpeed_Cnt;
	DWORD	nSpeed_Time;
	int		nSpeed_CheatCnt;
	int		nEnergyCnt;

	DWORD	nSitTime;

	bool fire(bool bEnergy)
	{
		nTotalBullet++;
		if (nSpeed_CheatCnt > 0)
		{
			nSpeed_CheatCnt--;
			return true;
		}
		if (bEnergy)
		{
			if (nEnergyCnt < 1)		//35�����������꣬����������ڰ���Ч����
			{
				return true;
			}
			else
			{
				nEnergyCnt --;		//35��������������������
				return false;
			}
		}

		DWORD time_now = GetTickCount();
		if (nSpeed_Cnt <= 0)
		{
			nSpeed_Time = time_now;
		}
		nSpeed_Cnt ++;
		if (time_now - nSpeed_Time >= 10*1000)	//����10����һ��;
		{
			DWORD dt = time_now - nSpeed_Time;
			int nCnt = nSpeed_Cnt;
			float fspeed =  1000.f * (float)nSpeed_Cnt / (float)(time_now - nSpeed_Time);
			
			nSpeed_CheatCnt = 0;
			nSpeed_Cnt = 0;
			nSpeed_Time = time_now;
			if (fspeed > 10.0f)		//��10����������ƽ��ÿ�� > 4�ڿ����ж�Ϊ���٣�����30������;
			{
				nSpeed_CheatCnt = 20;

				return true;
			}
		}
		return false;
	}

	//��ձ���
	void clear()
	{
		nSitTime = 0;
		nTotalBullet = 0;

		nSpeed_Cnt = 0;
		nSpeed_Time = 0;

		nSpeed_Cnt = 0;
		nSpeed_Time = GetTickCount();
		nSpeed_CheatCnt = 0;
		nEnergyCnt = 0;
	}
};

#define MAX_FISH_SENCE 60			//�������������
//������������Ϣ
struct tagNewFishInfo
{
	void ResetPart()
	{
		cbFishCount=0;
	}

	BYTE cbCanNewTag;			//����ų���							��A���Ⱦ�����
	BYTE cbFishCount;			//��ǰ������������
	//BYTE cbNewMethod;			//1������� ������ʱ�����
	float distribute_elapsed_;	//������ ��λ���� ������������������Ϊ������maxʱ��+(��/��)rand()%5��
};

//����״̬
enum en_GameState
{
	en_S_Ini=0,
	en_S_BuildSence,
	en_S_NewAcitveFish,
};

#define PATH_SHARE_NUM 60		//60��·�������� �����֤���ظ��켣
#define RECORD_FISH_NUM 54		//ÿʮ���Ӽ�¼49������FISH_CATCH_SMAE����FISH_KIND_12���48 ����Ԥ����
//////////////////////////////////////////////////////////////////////////

//��Ϸ������
class TableFrameSink : public ITableFrameSink, public ITableUserAction
{
	//��������
public:
	//���캯��
	TableFrameSink();
	//��������
	virtual ~TableFrameSink();

	//�����ӿ�
public:
	//�ͷŶ���
	virtual VOID __cdecl  Release() { if(IsValid()) delete this; }
	//�Ƿ���Ч
	virtual bool __cdecl IsValid() { return AfxIsValidAddress(this, sizeof(TableFrameSink)) ? true : false; }
	//�ӿڲ�ѯ
	virtual void * __cdecl QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//����ӿ�
public:
	//��ʼ��
	virtual bool __cdecl InitTableFrameSink(IUnknownEx * pIUnknownEx);
	//��λ����
	virtual void __cdecl RepositTableFrameSink();

	//��Ϣ�ӿ�
public:
	//��ʼģʽ
	virtual enStartMode __cdecl GetGameStartMode();
	//��Ϸ״̬
	virtual bool __cdecl IsUserPlaying(WORD wChairID);

	//��Ϸ�¼�
public:
	//��Ϸ��ʼ
	virtual bool __cdecl OnEventGameStart();
	//��Ϸ����
	virtual bool __cdecl OnEventGameEnd(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);
	//���ͳ���
	virtual bool __cdecl SendGameScene(WORD wChiarID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret);
	//֪ͨ�����ɼ� 2011.9.2
	virtual bool __cdecl OnUserMatchSortResult(WORD wChairID, int nSortID){ return true; }
	virtual bool __cdecl OnRestartGame(WORD wChairID, IServerUserItem * pIServerUserItem){ return true; }

	//�¼��ӿ�
public:
	//��ʱ���¼�
	virtual bool __cdecl OnTimerMessage(WORD wTimerID, WPARAM wBindParam);
	//��Ϸ��Ϣ����
	virtual bool __cdecl OnGameMessage(WORD wSubCmdID, const void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem);
	//�����Ϣ����
	virtual bool __cdecl OnFrameMessage(WORD wSubCmdID, const void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem) { return false; }

	//�����¼�
public:
	//�û�ͬ��
	virtual bool __cdecl OnActionUserReady(WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) { return true; }
	//�û�����
	virtual bool __cdecl OnActionUserOffLine(WORD wChairID, IServerUserItem * pIServerUserItem) { return true; }
	//�û�����
	virtual bool __cdecl OnActionUserReConnect(WORD wChairID, IServerUserItem * pIServerUserItem) { return true; }
	//�û�����
	virtual bool __cdecl OnActionUserSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//�û�����
	virtual bool __cdecl OnActionUserStandUp(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);

	//���׷����¼�
public:
	//����д�ֻص�
	virtual void __cdecl OnFishBombTableUser(WORD chair_id, LONG lAddScore,LONG lError,LPCTSTR szErrorDescribe);
	//��������
	virtual void __cdecl OnFishBombModify(WORD wChairID,BYTE cbTorpedoKind, LONG lTorpedoScore);

	//��Ϸ��Ϣ
public:
	//�һ����
	bool OnSubExchangeFishScore(IServerUserItem* server_user_item, bool increase);
	//��ҿ���
	bool OnSubUserFire(IServerUserItem* server_user_item, CMD_C_UserFire *pMsg);
	//��ҿ���---������
	bool OnSubSanGuanFire(IServerUserItem* server_user_item, CMD_C_SanGuanFire *pMsg);
	//��׽����
	bool OnSubCatchFish(IServerUserItem* server_user_item, CMD_C_CatchFish *pMsg);
	//��׽������
	bool OnSubCatchGroupFish(IServerUserItem* server_user_item, CMD_C_CatchGroupFish *pMsg);
	//��׽������
	bool OnSubCatchFishKingGroupFish(IServerUserItem* server_user_item, CMD_C_CatchGroupFishKing *pMsg);
	//���״���
	bool OnSubCatchFishTorpedo(IServerUserItem* server_user_item, CMD_C_CatchFish_Torpedo *pCatch_torpedo);
	//������
	bool OnSubStockOperate(IServerUserItem* server_user_item, unsigned char operate_code);

	//����ӵ�����
private:
	//��ȡ���
	LONGLONG GetUserGold(WORD wChairID, BYTE cbMethod=0);

	//��������ټ�
	FishTraceInfo* ActiveFishTrace();
	//�ͷ�����ټ�
	bool FreeFishTrace(int nFishID, bool bTimeOutDel=false);
	//�ͷ���������ټ�
	void FreeAllFishTrace();
	//������������ټ�
	void OnTimeClearFishTrace();
	//��ȡ����ټ�
	FishTraceInfo* GetFishTraceInfo(int fish_id, bool bNotLog=false);

	//�����ӵ���Ϣ
	ServerBulletInfo* ActiveBulletInfo(WORD chairid, int bullet_id);
	//�ͷ��ӵ�
	bool FreeBulletInfo(WORD chairid, ServerBulletInfo* bullet_info);
	//�ͷ������ӵ�
	void FreeAllBulletInfo(WORD chairid);
	//��ȡ�ӵ���Ϣ
	ServerBulletInfo* GetBulletInfo(WORD chairid, int bullet_id);

public:
	//����Ƕ�
	bool CalcAngle(WORD wChairID, float MouseX, float MouseY, float &fStartX, float &fStartY);

	//�����ʹ���
public:
	//���������Ͳ��ҿ��
	int GetStockByFishKind(bool &bUseOld, FishKind fishkind,  int lStockGold[20], double fMultiPro[20]);
	//���ͳһ���� bAdd ture�ӿ��
	void StockScore(int nFishKind, int nBullet_mulriple, LONGLONG lFishSocre, bool bAdd=false);
	//��ʱ����ӵ�����+�ӵ�����
    void BuildXianShiFree(WORD wChairID, BulletKind bulletKind, int nBullet_mulriple);

	///�������ֵ�����///////////////////////////////////////////////////////////////////////
	//������ ������ or ��̬����
	bool NormalStockHitGroup(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, DWORD dwAllFishMul, SCORE fish_score, float fMultiFishPro=1.0f);
	//���ˡ������� ������ or ��̬���ʡ�+ �����ֵ����㡿�������
	bool NormalStockHitSingle(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, SCORE fish_score, float fMultiFishPro=1.0f);

	///�����������ֵ�����///////////////////////////////////////////////////////////////////////
	//���ֵ���������ж�
	BYTE BombStockHitTool(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, float fMultiFishPro=1.0f);
	//���ֵ����㲶������������ж�
	bool BombStockHitSingle(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, float fMultiFishPro=1.0f);

	//��ʱ������
private:
	//��ʼ������Ϸ��ʱ��
	void StartAllGameTimer();
	//�ر�������Ϸ��ʱ��
	void KillAllGameTimer();
	//ͳһ������
	void DistributeFish(float delta_time);
	//����ʱ����Ϣ
	bool OnTimerSwitchScene();

	//////////////////////////////////////////////////////////////////////////
	//������Ϣ
	bool SendTableData(WORD sub_cmdid, void* data, WORD data_size, IServerUserItem* only_user_item);
	//������Ϸ������Ϣ
	bool SendGameConfig(IServerUserItem* server_user_item);

	//��������1
	void BuildSceneKind(bool bNoYuZhen=false);
	void BuildFishYuQun(WORD wYuQunStyleID);
	void BuildFishYuZhen();
	//��������ټ�
	void BuildFishTrace(int fish_count, FishKind fish_kind_start, FishKind fish_kind_end);

	//////////////////////////////////////////////////////////////////////////
	//�����ȫ���� ά��ʹ��
	void CalcScore(IServerUserItem* server_user_item);
	//200w�ڲ�ֵд��
	void WriteScore(WORD chair_id);

	//��������
private:
	//�������·��
	WORD RandCardData();
	//ɾ����־
	void DeleteLogFile(LPCTSTR lpPathName);
	//�Ƿ���Ҫsqlite3��ס
	bool GetNeedRecordDB(DWORD dwGameID);
	//��ȡû�˳�֮ǰӮ�˶���Ǯ
	LONGLONG GetMeSore(IServerUserItem *pServerUserItem);
	//����������
	LONGLONG OnGetTorpedoScore(BYTE cbBoomKind, LONGLONG lTorpedoScore, DWORD dwTorpedoCount);

	//���Ա���
public:
	static const WORD												m_wPlayerCount;												//��Ϸ����
	static const enStartMode										m_GameStartMode;											//��ʼģʽ

private:
	ITableFrame *													table_frame_;												//ƽ̨�ӿ�
	ITableFrameControl *											m_pITableFrameControl;										//��ܽӿ�
	const tagGameServiceOption *									game_service_option_;

	static GameConfig												g_game_config;

	//������¼����ͼ �ۼ�ÿʮ���Ӷ�ʱ����AddBlockList(1,0xABCD0000|typeid,lpszMsg)
	static SCORE													g_lFireScoreX[RECORD_FISH_NUM];								//X����ķ����ۼ�
	static SCORE													g_lCatchFishScoreX[RECORD_FISH_NUM];						//X����ķ����ۼ�
	static DWORD													g_dwFireCountX[RECORD_FISH_NUM];							//X�������ײ����
	static DWORD													g_dwDeadCountX[RECORD_FISH_NUM];							//X�������������
	//������¼����ͼ typeid=1ÿ�������ײ���� =2ÿ������������� =100ÿ����Ľ�����(������˰��) =101ÿ����ĳ�ȥ���(������˰��)

	//////////////////////////////////////////////////////////////////////////
public:
	//������Ϣ
	int																m_Scene0Index;												//���ڵ�һ�������л�
	int																fish_id_;													//������ID����
	int																bullet_idx_;												//��������������ڸ���
	int																bullet_id_[GAME_PLAYER];									//�����ӵ�ID����,ͬ�������ID����һ��.
	BYTE															m_cbBaZhuaYu[3];											//ͬ���ͻ��˳���צ��
	BYTE															m_cbNowYuZhenID;											//��ǰ����ID
	BYTE															m_cbNowToolFish;											//���ֵ����� ������
	WORD															m_wRandPathID;												//0-PATH_SHARE_NUM ��ѭ����
	WORD															m_wRandArray[PATH_SHARE_NUM];								//·���ֶ���� ��֤·�����ظ�
	DWORD															m_dwChangeSenceTm;											//�л�����
	DWORD															m_dwBuildSenceTm;											//�½�����
	DWORD															m_dwChangeScenceYuZhenTm;									//����ʱ��
	DWORD															m_dwCleanFishTm;											//����������ʱ��
	DWORD															m_dwToolFishTm[GAME_PLAYER];								//����1 ��ʾ��ȡ(��ͷ��/�����ҵ�/�����) ����ʱ��
	en_GameState													m_GameState;
	SceneKind														now_acitve_scene_kind_;										//��ǰ��������
	TimeFreeBullet													m_TimeFree[GAME_PLAYER];									//ʱ������
	ToolGetBullet													n_ToolsGet[GAME_PLAYER];									//��õ�������ӵ���¼
	ToolFreeBullet													m_ToolsFree[GAME_PLAYER];									//���ֵ������¼
	tagNewFishInfo													m_distribulte_fish[FISH_KIND_COUNT];						//�������� ��ʱͳ�� ���赥��ͳ��
	FishTraceInfo 													storage_fish_trace_vector_[MAX_CONTAIN_FISH_NUM];			//�洢����Ϣ
	std::vector<FishTraceInfo*> 									active_fish_trace_vector_;									//��ǰ�����Ϣ
	std::vector<ServerBulletInfo*> 									server_bullet_info_vector_[GAME_PLAYER];					//��ǰ��ӵ���Ϣ
	CFishMoveManage													FishMoveManage;
	CBulletMoveManage 												BulletMoveManage;
	CCollisionManage 												CollisionManage;

	//////////////////////////////////////////////////////////////////////////
	//ÿ��ʱ�䲻������Ҽ��
	time_t															g_tTimeSitdown[GAME_PLAYER];								//һ�ڲ���,������ʱ���
	DWORD															g_nFireBulletTm[GAME_PLAYER];
	DWORD															m_dwInValidPlayerTm[GAME_PLAYER];							//
	tagFireSpeed													m_tagFireSpeedCheck[GAME_PLAYER];							//�����ٶ�У��.

	//����ץ��Ŀǰ�ݲ���Ҫ ������Ϸ�����ڴ�����Ƽ���
	WORD															g_wKeyClient[GAME_PLAYER];
	//////////////////////////////////////////////////////////////////////////

	//д����Ϣ
	SCORE 															exchange_fish_score_[GAME_PLAYER];							//�ۼ��ܶһ�����
	SCORE 															fish_score_[GAME_PLAYER];									//�ۼƵ�ǰ�������
	SCORE 															user_score[GAME_PLAYER];									//�û�������ʱ���Ͻ��
	SCORE															m_lWriteScore[GAME_PLAYER];									//�Ѿ�д�˶��ٷ�
	DWORD															m_dwSitTime[GAME_PLAYER];									//������¼��Ϸʱ��

	//��֤���� ��ʵ��������ܽ���Ƿ��쳣��
	SCORE															m_lFireScore[GAME_PLAYER];									//�������ӵ�����
	SCORE															m_lWinScore[GAME_PLAYER];									//�����÷���
	SCORE															m_lRetrunScore[GAME_PLAYER];								//�ӵ������ķ���
	SCORE															m_lFireIntoSystemGold[GAME_PLAYER];							//�������̵�
	SCORE															m_lFireNoInKuCunGold[GAME_PLAYER];							//

	int 															m_nKillFishCount[GAME_PLAYER][FISH_KIND_COUNT];				//����ʲô��
	int 															m_nTotalKillFishCount[GAME_PLAYER];							//�ܹ���������
	int																m_nKillFishCount_Death[GAME_PLAYER][FISH_KIND_COUNT];		//����ʲô��
	int																m_nTotalKillFishCount_Death[GAME_PLAYER];					//����������

	//��������·��
	TCHAR															m_szMoudleFilePath[MAX_PATH];								//ģ��·��
	TCHAR															m_szIniFileNameRoom[MAX_PATH];								//����·��

	////������Ϣ//////////////////////////////////////////////////////////////////
	BYTE														 	g_cbTest;
	BYTE														 	g_cbTestStyle;
	std::vector<WORD>												g_vecAllFish;


	//���������
	WORD															m_wDaoJuPaoUser;

	int																m_nTempFishID[GAME_PLAYER];									//�ͻ��˶�����������Ҫ���޸�
	tagTorpedoInfo													m_pTorpedoInfo[GAME_PLAYER];								//����
	LONGLONG														m_lYuLeiScore[GAME_PLAYER];									//�ܼ����׷���
	LONGLONG														m_lYuLeiModTotal[GAME_PLAYER];								//���׳�����
	DWORD															m_dwTorpedoTime[GAME_PLAYER];								//��������ʱ��
};

//���ⳡ�� ���糡���� ǰ3��С��ĸ���*0.7 ��ΪС���ƽʱ���
#endif // TABLE_FRAME_SINK_H_
