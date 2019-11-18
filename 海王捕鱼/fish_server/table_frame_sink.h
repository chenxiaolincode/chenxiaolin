
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
//鱼的踪迹结构信息
struct FishTraceInfo
{
	int fish_id;
	int fish_tag;					//鱼辅助类型 fish_kind
	int fish_king_id;				//鱼群4中鱼王的ID，其他鱼没有用到
	int nTrace_id;					//随机路径ID
	DWORD build_tick;				//产生时间
	FishKind fish_kind;				//鱼主类型
	WORD wFireCount[GAME_PLAYER];	//
};

//服务端子弹信息
struct ServerBulletInfo
{
	int bullet_id;					//单个用户子弹id
	int bullet_idx_;				//辅助使用
	int nLockFishID;				//锁定的子弹 最后正常碰撞其他鱼 随机降低概率
	int bullet_mulriple;			//子弹倍数
	BulletKind bullet_kind;			//子弹类型
	DWORD dwTicktCount;				//
	BYTE cbMaxUse;					// 默认=0 钻头炮6次 砸蛋3次 电池炮1次 仅OnSubCatchGroupFish有效
	bool bCheat;					//默认false 不惩罚
	BYTE cbBoomKind;				//鱼雷种类，非鱼雷子弹为INVALID_BYTE
};

//个人限免模式
struct TimeFreeBullet
{
	WORD wFireCount;				//本次限时累计子弹数 参考最大35
	int mulriple;					//本次限时免费 子弹倍数
	DWORD dwFreeTime;				//非零表示免费状态 单位ms 和当前时间检测结束
};

//三种道具鱼
struct ToolGetBullet
{
	BulletKind bullet_kind;			//子弹类型
	int bullet_mulriple;			//子弹倍数
};

struct ToolFreeBullet
{
	BYTE cbMaxBullet;				//单包赠送免费子弹数
	int bullet_id;					//免费子弹绑定id
	int TimeGold;					//当前免费子弹累计得分
};

//子单速度 - 发射速度校验
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
			if (nEnergyCnt < 1)		//35发能量炮用完，后面的能量炮按无效处理；
			{
				return true;
			}
			else
			{
				nEnergyCnt --;		//35发以内能量炮算正常。
				return false;
			}
		}

		DWORD time_now = GetTickCount();
		if (nSpeed_Cnt <= 0)
		{
			nSpeed_Time = time_now;
		}
		nSpeed_Cnt ++;
		if (time_now - nSpeed_Time >= 10*1000)	//大于10秒检测一次;
		{
			DWORD dt = time_now - nSpeed_Time;
			int nCnt = nSpeed_Cnt;
			float fspeed =  1000.f * (float)nSpeed_Cnt / (float)(time_now - nSpeed_Time);
			
			nSpeed_CheatCnt = 0;
			nSpeed_Cnt = 0;
			nSpeed_Time = time_now;
			if (fspeed > 10.0f)		//若10秒以上区间平均每秒 > 4炮可以判定为加速，给与30炮哑炮;
			{
				nSpeed_CheatCnt = 20;

				return true;
			}
		}
		return false;
	}

	//清空变量
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

#define MAX_FISH_SENCE 60			//单个场景最大数
//服务器出鱼信息
struct tagNewFishInfo
{
	void ResetPart()
	{
		cbFishCount=0;
	}

	BYTE cbCanNewTag;			//非零才出鱼							【A】先决条件
	BYTE cbFishCount;			//当前类型鱼总条数
	//BYTE cbNewMethod;			//1立马出鱼 其他按时间出鱼
	float distribute_elapsed_;	//出鱼间隔 单位：秒 特殊鱼死亡立马重置为配置中max时间+(正/负)rand()%5秒
};

//流程状态
enum en_GameState
{
	en_S_Ini=0,
	en_S_BuildSence,
	en_S_NewAcitveFish,
};

#define PATH_SHARE_NUM 60		//60条路径被公用 随机保证不重复轨迹
#define RECORD_FISH_NUM 54		//每十分钟记录49种鱼因FISH_CATCH_SMAE仅到FISH_KIND_12需改48 但先预留吧
//////////////////////////////////////////////////////////////////////////

//游戏桌子类
class TableFrameSink : public ITableFrameSink, public ITableUserAction
{
	//方法集合
public:
	//构造函数
	TableFrameSink();
	//析构函数
	virtual ~TableFrameSink();

	//基础接口
public:
	//释放对象
	virtual VOID __cdecl  Release() { if(IsValid()) delete this; }
	//是否有效
	virtual bool __cdecl IsValid() { return AfxIsValidAddress(this, sizeof(TableFrameSink)) ? true : false; }
	//接口查询
	virtual void * __cdecl QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//管理接口
public:
	//初始化
	virtual bool __cdecl InitTableFrameSink(IUnknownEx * pIUnknownEx);
	//复位桌子
	virtual void __cdecl RepositTableFrameSink();

	//信息接口
public:
	//开始模式
	virtual enStartMode __cdecl GetGameStartMode();
	//游戏状态
	virtual bool __cdecl IsUserPlaying(WORD wChairID);

	//游戏事件
public:
	//游戏开始
	virtual bool __cdecl OnEventGameStart();
	//游戏结束
	virtual bool __cdecl OnEventGameEnd(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);
	//发送场景
	virtual bool __cdecl SendGameScene(WORD wChiarID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret);
	//通知比赛成绩 2011.9.2
	virtual bool __cdecl OnUserMatchSortResult(WORD wChairID, int nSortID){ return true; }
	virtual bool __cdecl OnRestartGame(WORD wChairID, IServerUserItem * pIServerUserItem){ return true; }

	//事件接口
public:
	//定时器事件
	virtual bool __cdecl OnTimerMessage(WORD wTimerID, WPARAM wBindParam);
	//游戏消息处理
	virtual bool __cdecl OnGameMessage(WORD wSubCmdID, const void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem);
	//框架消息处理
	virtual bool __cdecl OnFrameMessage(WORD wSubCmdID, const void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem) { return false; }

	//动作事件
public:
	//用户同意
	virtual bool __cdecl OnActionUserReady(WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) { return true; }
	//用户断线
	virtual bool __cdecl OnActionUserOffLine(WORD wChairID, IServerUserItem * pIServerUserItem) { return true; }
	//用户重入
	virtual bool __cdecl OnActionUserReConnect(WORD wChairID, IServerUserItem * pIServerUserItem) { return true; }
	//用户坐下
	virtual bool __cdecl OnActionUserSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//用户起来
	virtual bool __cdecl OnActionUserStandUp(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);

	//鱼雷返回事件
public:
	//鱼雷写分回调
	virtual void __cdecl OnFishBombTableUser(WORD chair_id, LONG lAddScore,LONG lError,LPCTSTR szErrorDescribe);
	//更新鱼雷
	virtual void __cdecl OnFishBombModify(WORD wChairID,BYTE cbTorpedoKind, LONG lTorpedoScore);

	//游戏消息
public:
	//兑换渔币
	bool OnSubExchangeFishScore(IServerUserItem* server_user_item, bool increase);
	//玩家开火
	bool OnSubUserFire(IServerUserItem* server_user_item, CMD_C_UserFire *pMsg);
	//玩家开火---三管炮
	bool OnSubSanGuanFire(IServerUserItem* server_user_item, CMD_C_SanGuanFire *pMsg);
	//捕捉到鱼
	bool OnSubCatchFish(IServerUserItem* server_user_item, CMD_C_CatchFish *pMsg);
	//捕捉多条鱼
	bool OnSubCatchGroupFish(IServerUserItem* server_user_item, CMD_C_CatchGroupFish *pMsg);
	//捕捉多条鱼
	bool OnSubCatchFishKingGroupFish(IServerUserItem* server_user_item, CMD_C_CatchGroupFishKing *pMsg);
	//鱼雷打鱼
	bool OnSubCatchFishTorpedo(IServerUserItem* server_user_item, CMD_C_CatchFish_Torpedo *pCatch_torpedo);
	//库存操作
	bool OnSubStockOperate(IServerUserItem* server_user_item, unsigned char operate_code);

	//鱼和子弹管理
private:
	//获取金币
	LONGLONG GetUserGold(WORD wChairID, BYTE cbMethod=0);

	//激活鱼的踪迹
	FishTraceInfo* ActiveFishTrace();
	//释放鱼的踪迹
	bool FreeFishTrace(int nFishID, bool bTimeOutDel=false);
	//释放所有鱼的踪迹
	void FreeAllFishTrace();
	//定期清理鱼的踪迹
	void OnTimeClearFishTrace();
	//获取鱼的踪迹
	FishTraceInfo* GetFishTraceInfo(int fish_id, bool bNotLog=false);

	//激活子弹信息
	ServerBulletInfo* ActiveBulletInfo(WORD chairid, int bullet_id);
	//释放子弹
	bool FreeBulletInfo(WORD chairid, ServerBulletInfo* bullet_info);
	//释放所有子弹
	void FreeAllBulletInfo(WORD chairid);
	//获取子弹信息
	ServerBulletInfo* GetBulletInfo(WORD chairid, int bullet_id);

public:
	//计算角度
	bool CalcAngle(WORD wChairID, float MouseX, float MouseY, float &fStartX, float &fStartY);

	//库存概率管理
public:
	//根据鱼类型查找库存
	int GetStockByFishKind(bool &bUseOld, FishKind fishkind,  int lStockGold[20], double fMultiPro[20]);
	//库存统一操作 bAdd ture加库存
	void StockScore(int nFishKind, int nBullet_mulriple, LONGLONG lFishSocre, bool bAdd=false);
	//限时免费子弹类型+子弹倍数
    void BuildXianShiFree(WORD wChairID, BulletKind bulletKind, int nBullet_mulriple);

	///除了三种道具鱼///////////////////////////////////////////////////////////////////////
	//闪电鱼 旋风鱼 or 动态倍率
	bool NormalStockHitGroup(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, DWORD dwAllFishMul, SCORE fish_score, float fMultiFishPro=1.0f);
	//除了【闪电鱼 旋风鱼 or 动态倍率】+ 【三种道具鱼】以外的鱼
	bool NormalStockHitSingle(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, SCORE fish_score, float fMultiFishPro=1.0f);

	///仅仅处理三种道具鱼///////////////////////////////////////////////////////////////////////
	//三种道具鱼概率判断
	BYTE BombStockHitTool(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, float fMultiFishPro=1.0f);
	//三种道具鱼捕获正常鱼逐个判断
	bool BombStockHitSingle(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, float fMultiFishPro=1.0f);

	//定时器触发
private:
	//开始所有游戏定时器
	void StartAllGameTimer();
	//关闭所有游戏定时器
	void KillAllGameTimer();
	//统一发布鱼
	void DistributeFish(float delta_time);
	//换屏时间消息
	bool OnTimerSwitchScene();

	//////////////////////////////////////////////////////////////////////////
	//发送消息
	bool SendTableData(WORD sub_cmdid, void* data, WORD data_size, IServerUserItem* only_user_item);
	//发送游戏配置信息
	bool SendGameConfig(IServerUserItem* server_user_item);

	//建立场景1
	void BuildSceneKind(bool bNoYuZhen=false);
	void BuildFishYuQun(WORD wYuQunStyleID);
	void BuildFishYuZhen();
	//建立鱼的踪迹
	void BuildFishTrace(int fish_count, FishKind fish_kind_start, FishKind fish_kind_end);

	//////////////////////////////////////////////////////////////////////////
	//玩家完全结算 维护使用
	void CalcScore(IServerUserItem* server_user_item);
	//200w内差值写分
	void WriteScore(WORD chair_id);

	//辅助函数
private:
	//随机常用路径
	WORD RandCardData();
	//删除日志
	void DeleteLogFile(LPCTSTR lpPathName);
	//是否需要sqlite3记住
	bool GetNeedRecordDB(DWORD dwGameID);
	//获取没退出之前赢了多少钱
	LONGLONG GetMeSore(IServerUserItem *pServerUserItem);
	//计算获得鱼雷
	LONGLONG OnGetTorpedoScore(BYTE cbBoomKind, LONGLONG lTorpedoScore, DWORD dwTorpedoCount);

	//属性变量
public:
	static const WORD												m_wPlayerCount;												//游戏人数
	static const enStartMode										m_GameStartMode;											//开始模式

private:
	ITableFrame *													table_frame_;												//平台接口
	ITableFrameControl *											m_pITableFrameControl;										//框架接口
	const tagGameServiceOption *									game_service_option_;

	static GameConfig												g_game_config;

	//辅助记录走势图 累计每十分钟定时调用AddBlockList(1,0xABCD0000|typeid,lpszMsg)
	static SCORE													g_lFireScoreX[RECORD_FISH_NUM];								//X种鱼的发射累计
	static SCORE													g_lCatchFishScoreX[RECORD_FISH_NUM];						//X种鱼的返分累计
	static DWORD													g_dwFireCountX[RECORD_FISH_NUM];							//X种鱼的碰撞次数
	static DWORD													g_dwDeadCountX[RECORD_FISH_NUM];							//X种鱼的死亡次数
	//辅助记录走势图 typeid=1每种鱼的碰撞次数 =2每种鱼的死亡次数 =100每种鱼的进入金币(不区分税收) =101每种鱼的出去金币(不区分税收)

	//////////////////////////////////////////////////////////////////////////
public:
	//运行信息
	int																m_Scene0Index;												//用于第一个场景切换
	int																fish_id_;													//生成鱼ID号用
	int																bullet_idx_;												//下面连续方便后期跟踪
	int																bullet_id_[GAME_PLAYER];									//生成子弹ID号用,同上面的鱼ID号用一样.
	BYTE															m_cbBaZhuaYu[3];											//同步客户端出八爪鱼
	BYTE															m_cbNowYuZhenID;											//当前鱼阵ID
	BYTE															m_cbNowToolFish;											//三种道具鱼 轮流出
	WORD															m_wRandPathID;												//0-PATH_SHARE_NUM 轮循索引
	WORD															m_wRandArray[PATH_SHARE_NUM];								//路径分段随机 保证路径不重复
	DWORD															m_dwChangeSenceTm;											//切换场景
	DWORD															m_dwBuildSenceTm;											//新建场景
	DWORD															m_dwChangeScenceYuZhenTm;									//鱼阵时间
	DWORD															m_dwCleanFishTm;											//定点清理鱼时间
	DWORD															m_dwToolFishTm[GAME_PLAYER];								//非零1 表示获取(钻头炮/连环砸蛋/电磁炮) 其他时间
	en_GameState													m_GameState;
	SceneKind														now_acitve_scene_kind_;										//当前场景类型
	TimeFreeBullet													m_TimeFree[GAME_PLAYER];									//时间限免
	ToolGetBullet													n_ToolsGet[GAME_PLAYER];									//获得道具鱼的子弹记录
	ToolFreeBullet													m_ToolsFree[GAME_PLAYER];									//三种道具鱼记录
	tagNewFishInfo													m_distribulte_fish[FISH_KIND_COUNT];						//发布控制 定时统计 不需单独统计
	FishTraceInfo 													storage_fish_trace_vector_[MAX_CONTAIN_FISH_NUM];			//存储鱼信息
	std::vector<FishTraceInfo*> 									active_fish_trace_vector_;									//当前活动鱼信息
	std::vector<ServerBulletInfo*> 									server_bullet_info_vector_[GAME_PLAYER];					//当前活动子弹信息
	CFishMoveManage													FishMoveManage;
	CBulletMoveManage 												BulletMoveManage;
	CCollisionManage 												CollisionManage;

	//////////////////////////////////////////////////////////////////////////
	//每段时间不打炮外挂检查
	time_t															g_tTimeSitdown[GAME_PLAYER];								//一炮不打,以坐下时间记
	DWORD															g_nFireBulletTm[GAME_PLAYER];
	DWORD															m_dwInValidPlayerTm[GAME_PLAYER];							//
	tagFireSpeed													m_tagFireSpeedCheck[GAME_PLAYER];							//发射速度校验.

	//其他抓挂目前暂不需要 建议游戏单独内存二进制加密
	WORD															g_wKeyClient[GAME_PLAYER];
	//////////////////////////////////////////////////////////////////////////

	//写分信息
	SCORE 															exchange_fish_score_[GAME_PLAYER];							//累计总兑换数量
	SCORE 															fish_score_[GAME_PLAYER];									//累计当前鱼币数量
	SCORE 															user_score[GAME_PLAYER];									//用户坐下来时身上金币
	SCORE															m_lWriteScore[GAME_PLAYER];									//已经写了多少分
	DWORD															m_dwSitTime[GAME_PLAYER];									//辅助记录游戏时间

	//验证数据 核实个人玩家总金币是否异常的
	SCORE															m_lFireScore[GAME_PLAYER];									//发出的子弹分数
	SCORE															m_lWinScore[GAME_PLAYER];									//捕鱼获得分数
	SCORE															m_lRetrunScore[GAME_PLAYER];								//子弹返还的分数
	SCORE															m_lFireIntoSystemGold[GAME_PLAYER];							//单个人吞弹
	SCORE															m_lFireNoInKuCunGold[GAME_PLAYER];							//

	int 															m_nKillFishCount[GAME_PLAYER][FISH_KIND_COUNT];				//打了什么鱼
	int 															m_nTotalKillFishCount[GAME_PLAYER];							//总共打了数量
	int																m_nKillFishCount_Death[GAME_PLAYER][FISH_KIND_COUNT];		//打死什么鱼
	int																m_nTotalKillFishCount_Death[GAME_PLAYER];					//共打死多少

	//房间配置路径
	TCHAR															m_szMoudleFilePath[MAX_PATH];								//模块路径
	TCHAR															m_szIniFileNameRoom[MAX_PATH];								//配置路径

	////测试信息//////////////////////////////////////////////////////////////////
	BYTE														 	g_cbTest;
	BYTE														 	g_cbTestStyle;
	std::vector<WORD>												g_vecAllFish;


	//道具炮玩家
	WORD															m_wDaoJuPaoUser;

	int																m_nTempFishID[GAME_PLAYER];									//客户端动画依赖后期要求修复
	tagTorpedoInfo													m_pTorpedoInfo[GAME_PLAYER];								//鱼雷
	LONGLONG														m_lYuLeiScore[GAME_PLAYER];									//总计鱼雷分数
	LONGLONG														m_lYuLeiModTotal[GAME_PLAYER];								//鱼雷除不尽
	DWORD															m_dwTorpedoTime[GAME_PLAYER];								//发射鱼雷时间
};

//特殊场景 比如场景与 前3条小鱼的概率*0.7 因为小鱼比平时鱼多
#endif // TABLE_FRAME_SINK_H_
