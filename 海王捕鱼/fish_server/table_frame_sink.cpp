
#include "StdAfx.h"
#include "table_frame_sink.h"
#include <math.h>
#include <MMSystem.h>
#include <iterator>
#include <map>
#include "WorkLogs.h"
#include <Shlwapi.h>
#include "DBAction.h"
//////////////////////////////////////////////////////////////////////////
unsigned int g_seed=0;
int Random_Int(int min, int max)
{
	g_seed=214013*g_seed+2531011+rand();
	return min+(g_seed ^ g_seed>>15)%(max-min+1);
}

// ���Ա�־ �������˱�־ ���͸�����Ч
//#define TEST

// Ӧ��ע�⣺��ʱ��ID�ķ�Χ��1-20 ( >=1 && < 20 )
// �����г���20�� ����Ҫ��GameServiceHead.h��TIME_TABLE_SINK_RANGE�Ĵ����У����߸Ľ���ʱ�������繲��
#define IDI_UpdateLoop 1
#define IDI_LOADCONFIG 2													//��������
#define IDI_KILL_TOTAL_COUNT 3												//��Ҽ��

//��ʱ���������� ����
const DWORD kRepeatTimer = (DWORD)0xFFFFFFFF;								//�ظ�ѭ��
const DWORD kFishAliveTime = 150000;										//��ͨ�������Ļʱ�� ����ᵥ��
const DWORD kUpdateTime = 100;												//��С��������
const DWORD kKillTotalCount = 10*1000;										//��Ҽ������ʱ��
//const DWORD kSwitchSceneElasped = 5*60*1000;
const DWORD kSwitchSceneElasped = 5*60*1000;
const DWORD kClearTraceElasped = 60*1000;


//��̬����
const WORD			TableFrameSink::m_wPlayerCount = GAME_PLAYER;			//��Ϸ����
const enStartMode	TableFrameSink::m_GameStartMode = enStartMode_AllReady;	//��ʼģʽ

GameConfig			TableFrameSink::g_game_config;							//ȫ�־�̬����
SCORE				TableFrameSink::g_lFireScoreX[RECORD_FISH_NUM]={0};		//X����ķ����ۼ�
SCORE				TableFrameSink::g_lCatchFishScoreX[RECORD_FISH_NUM]={0};//X����ķ����ۼ�
DWORD				TableFrameSink::g_dwFireCountX[RECORD_FISH_NUM]={0};	//X�������ײ����
DWORD				TableFrameSink::g_dwDeadCountX[RECORD_FISH_NUM]={0};	//X�������������

static DWORD		g_nUseKuCunRand = 50;									//���ֿ�淽ʽ���rand()%100<xʹ���Ͽ�淽ʽ
static int			g_nLogOutDB=0;											//0�ر�ȫ�ּ�¼ 1����ȫ�ּ�¼
static std::vector<DWORD> g_RecordGameID;
static SCORE        g_lStandUpGold=0;										//������ͳ��
static SCORE		g_lUserAllWin=0;										//��¼�������Ӯ
static SCORE		g_lFireNoCatchGold=0;									//���䵫û��ײ���ӵ�
static DWORD		g_lServerRate=0;										//û���ӵ�˰��
static SCORE		g_lReturnScore=0;										//�̵��ּܷ��
static SCORE		g_lFireScore=0;											//���������ֵ
static SCORE		g_lEnterScore=0;										//����������ֵ
static SCORE		g_lMemWrongScore=0;										//�ӵ��ظ�������ͬid�Ĵ����ۼ�
static SCORE		g_lFireNoInKuCunGold=0;									//�����ӵ�����δ��ײͣ������Ļ�е�����
static SCORE		g_lWriteToDBScore=0;									//��������д�뵽���ݿ�ķ���
static DWORD		g_dwUnNomal=0;											//���������
static SCORE		g_lAllYuLeiGold=0;										//�����׸���
static SCORE		g_lAllYuleiMode=0;										//�����׳������ĺ�

static DWORD		g_dwWrongCount[2]={0};									//����У�����
static DWORD		g_dwRetrunRate=0;										//���ڷ�����ϵͳ����ʮ�ֱ�
//////////////////////////////////////////////////////////////////////////

TableFrameSink::TableFrameSink()
: table_frame_(NULL)
, game_service_option_(NULL)
{
	m_GameState=en_S_Ini;
	m_dwChangeSenceTm=0;
	m_dwBuildSenceTm=0;
	m_cbNowYuZhenID=0xff;
	m_dwChangeScenceYuZhenTm=0;
	now_acitve_scene_kind_=SCENE_KIND_4;
	m_Scene0Index = 0;
	fish_id_=0;
	bullet_idx_=0;
	for (WORD i = 0; i < GAME_PLAYER; ++i)
	{
		bullet_id_[i] = 0;
		user_score[i] = 0;
		fish_score_[i] = 0;
		exchange_fish_score_[i] = 0;
		server_bullet_info_vector_[i].clear();
	}
	active_fish_trace_vector_.clear();			
	ZeroMemory(storage_fish_trace_vector_,sizeof(storage_fish_trace_vector_));
	
	memset(m_dwSitTime, 0, sizeof(m_dwSitTime));
	ZeroMemory(m_distribulte_fish,sizeof(m_distribulte_fish));
	
	//��������Ҽ�����
	ZeroMemory(g_tTimeSitdown, sizeof(g_tTimeSitdown));
	ZeroMemory(g_nFireBulletTm, sizeof(g_nFireBulletTm));

	ZeroMemory(m_lWriteScore, sizeof(m_lWriteScore));
	ZeroMemory(g_wKeyClient, sizeof(g_wKeyClient));

	m_szMoudleFilePath[0]=0;
	m_szIniFileNameRoom[0]=0;
	m_wRandPathID=0;
	ZeroMemory(m_wRandArray,sizeof(m_wRandArray));
	ZeroMemory(m_cbBaZhuaYu,sizeof(m_cbBaZhuaYu));
	ZeroMemory(m_TimeFree,sizeof(m_TimeFree));
	ZeroMemory(m_dwToolFishTm,sizeof(m_dwToolFishTm));
	ZeroMemory(m_dwInValidPlayerTm,sizeof(m_dwInValidPlayerTm));
	ZeroMemory(&m_tagFireSpeedCheck, sizeof(m_tagFireSpeedCheck));
	m_dwCleanFishTm=0;
	m_cbNowToolFish=0;

	ZeroMemory(n_ToolsGet,sizeof(n_ToolsGet));
	ZeroMemory(m_ToolsFree,sizeof(m_ToolsFree));

	ZeroMemory(m_lFireScore ,sizeof(m_lFireScore));
	ZeroMemory(m_lWinScore ,sizeof(m_lWinScore));
	ZeroMemory(m_lRetrunScore ,sizeof(m_lRetrunScore));
	ZeroMemory(m_lFireIntoSystemGold ,sizeof(m_lFireIntoSystemGold));
	ZeroMemory(m_lFireNoInKuCunGold ,sizeof(m_lFireNoInKuCunGold));

	ZeroMemory(m_nKillFishCount, sizeof(m_nKillFishCount));
	ZeroMemory(m_nTotalKillFishCount, sizeof(m_nTotalKillFishCount));
	ZeroMemory(m_nKillFishCount_Death, sizeof(m_nKillFishCount_Death));
	ZeroMemory(m_nTotalKillFishCount_Death, sizeof(m_nTotalKillFishCount_Death));

	//������Ϣ
	g_cbTest=0;
	g_cbTestStyle=255;
	g_vecAllFish.clear();

	m_wDaoJuPaoUser = INVALID_CHAIR;
	ZeroMemory(&m_pTorpedoInfo , sizeof(m_pTorpedoInfo));
	ZeroMemory(m_dwTorpedoTime, sizeof(m_dwTorpedoTime));
	ZeroMemory(m_lYuLeiScore, sizeof(m_lYuLeiScore));
	ZeroMemory(m_lYuLeiModTotal, sizeof(m_lYuLeiModTotal));

	//AllocConsole();
	//freopen("conout$","w",stdout);
}

TableFrameSink::~TableFrameSink()
{
	active_fish_trace_vector_.clear();
	for (WORD i = 0; i < GAME_PLAYER; ++i)
	{
		FreeAllBulletInfo(i);
	}

	//˰�ջ�����
	if (g_game_config.g_revenue_score > 0)
	{
		CString str;
		str.Format(TEXT("��ˮ%I64d"), g_game_config.g_revenue_score);
		CTraceService::TraceString(str, TraceLevel_Exception);
		g_game_config.g_revenue_score = 0;
	}
}

//�ӿڲ�ѯ
void * __cdecl TableFrameSink::QueryInterface(const IID & Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(ITableFrameSink, Guid, dwQueryVer);
	QUERYINTERFACE(ITableUserAction, Guid, dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(ITableFrameSink, Guid, dwQueryVer);
	return NULL;
}

bool __cdecl TableFrameSink::InitTableFrameSink(IUnknownEx * pIUnknownEx)
{
	RandCardData();

	//�������
	srand(unsigned int(time(NULL)));
	if(0==g_seed) g_seed=timeGetTime();

	table_frame_ = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, ITableFrame);
	if (!table_frame_) return false;

	//���ƽӿ�
	m_pITableFrameControl = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, ITableFrameControl);
	if (m_pITableFrameControl == NULL) return false;

	game_service_option_ = table_frame_->GetGameServiceOption();

	//�����ļ���
	TCHAR szModuleFile[MAX_PATH]={0};
	GetModuleFileName(0, szModuleFile, MAX_PATH);
	PathRemoveFileSpec(szModuleFile);
	_sntprintf(m_szMoudleFilePath, CountArray(m_szMoudleFilePath), TEXT("%s"), szModuleFile);
	_sntprintf(m_szIniFileNameRoom, CountArray(m_szIniFileNameRoom), TEXT("%s\\hw2FishConfig\\hw2Fish.ini"), szModuleFile);
	g_lServerRate = GetPrivateProfileInt(TEXT("RoomCfg"), TEXT("BulletFee"), 0L, m_szIniFileNameRoom);

	if (!g_game_config.LoadGameConfig(m_szMoudleFilePath,game_service_option_->wServerID,true))
	{
		CTraceService::TraceString(TEXT("������Դ����ʧ�ܣ�����"),TraceLevel_Exception);
	}

	//�������ƶ�ʱ�� - �޸�����������Ч
	table_frame_->SetGameTimer(IDI_LOADCONFIG, 60 * 1000, INVALID_DWORD, 0);
	//////////////////////////////////////////////////////////////////////////
	TCHAR		buffer[128]={0};
	time_t		ltime;
	struct tm	*today;

	//time
	time(&ltime);
	today = localtime(&ltime);
	strftime(buffer, 128, "%m-%d %H-%M-%S", today);
	static CString strPathTT=buffer;

	TCHAR strLogFile[256]={0};
	sprintf(strLogFile,"record\\%s\\�쳣��¼_%s",game_service_option_->szGameRoomName,strPathTT);
	LOG_FILE_X.Set_LogOutPath(strLogFile);
	sprintf(strLogFile,"record\\%s\\������¼_%s",game_service_option_->szGameRoomName,strPathTT);
	LOG_FILE_EX_X.Set_LogOutPath(strLogFile);

	//////////////////////////////////////////////////////////////////////////

	//���ͳ����Ҷ�ʱ��
	table_frame_->SetGameTimer(IDI_KILL_TOTAL_COUNT, kKillTotalCount, (DWORD)(-1), 0);

	PATH_SHARE.LoadAllPath();
	CollisionManage.SetInputObject(&FishMoveManage,&BulletMoveManage);

	g_RecordGameID.clear();

	return true;
}

void __cdecl TableFrameSink::RepositTableFrameSink()
{
}

//��ʼģʽ
enStartMode __cdecl TableFrameSink::GetGameStartMode()
{
	return m_GameStartMode;
}

bool __cdecl TableFrameSink::IsUserPlaying(WORD chair_id)
{
	return true;
}

bool __cdecl TableFrameSink::OnEventGameStart()
{
	return true;
}

bool __cdecl TableFrameSink::OnEventGameEnd(WORD chair_id, IServerUserItem* server_user_item, BYTE reason)
{
	if (reason == GER_DISMISS)
	{
		for (WORD i = 0; i < GAME_PLAYER; ++i)
		{
			IServerUserItem* user_item = table_frame_->GetServerUserItem(i);
			if (user_item == NULL) continue;
			CalcScore(user_item);

			WORD wChairID=user_item->GetChairID();
			if(wChairID<GAME_PLAYER) FreeAllBulletInfo(wChairID);
		}
		//table_frame_->ConcludeGame();
		KillAllGameTimer();
		FreeAllFishTrace();
		now_acitve_scene_kind_ = SCENE_KIND_1;
	}
	else if (chair_id < GAME_PLAYER && server_user_item != NULL)
	{
		CalcScore(server_user_item);
	}
	return true;
}

bool __cdecl TableFrameSink::SendGameScene(WORD chair_id, IServerUserItem* server_user_item, BYTE game_status, bool send_secret)
{
	switch (game_status)
	{
	case GS_FREE:
	case GS_PLAYING:
		SendGameConfig(server_user_item);

		CMD_S_GameStatus gamestatus;
		ZeroMemory(&gamestatus, sizeof(gamestatus));
		gamestatus.dwGame_version = g_wKeyClient[chair_id];
		if(m_cbNowYuZhenID!=0xff && m_cbNowYuZhenID<g_game_config.m_ConfigYuZhen.size())
		{
			DWORD dwLeftTm = g_game_config.m_ConfigYuZhen[m_cbNowYuZhenID]->dwAllLifeTime*1000-m_dwChangeScenceYuZhenTm;
			if(dwLeftTm>0 && dwLeftTm<g_game_config.m_ConfigYuZhen[m_cbNowYuZhenID]->dwAllLifeTime*1000)
			{
				dwLeftTm/=1000;
				DWORD dwTmp=(dwLeftTm<<16)&0xFFFF0000;
				gamestatus.dwGame_version|=dwTmp;
			}
		}
		CopyMemory(gamestatus.dwTorpedoCount,m_pTorpedoInfo[chair_id].dwTorpedoCount,sizeof(gamestatus.dwTorpedoCount));
		CopyMemory(gamestatus.lNow_fish_score, fish_score_, sizeof(gamestatus.lNow_fish_score));
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			gamestatus.lNow_user_score[i] = user_score[i] - 
				exchange_fish_score_[i] * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
		}

		gamestatus.wDaoJuPaoUser = m_wDaoJuPaoUser;

		table_frame_->SendGameScene(server_user_item, &gamestatus, sizeof(gamestatus));

		table_frame_->SendGameMessage(server_user_item, TEXT("���̡������Ӽ��ڣ����������·֣��ո���������������ӵ�!"), SMT_INFO);

		//��������
		CMD_S_SwitchScene Switch_Scene;
		Switch_Scene.scene_kind = now_acitve_scene_kind_;
		Switch_Scene.nSceneStartTime=GetElapsedTm(m_dwChangeSenceTm);//�����Ѿ���ʼ��ʱ��

		table_frame_->SendTableData(chair_id, SUB_S_SWITCH_SCENE, &Switch_Scene, sizeof(CMD_S_SwitchScene));

		table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_SWITCH_SCENE, &Switch_Scene, sizeof(CMD_S_SwitchScene));

		return true;
	}
	return false;
}

bool __cdecl TableFrameSink::OnTimerMessage(WORD timer_id, WPARAM bind_param)
{
	switch (timer_id)
	{
	case IDI_UpdateLoop:
		{
			DWORD dwNowTimer=GetTickCount();
			
			//ʱ�����
			{
				//ͳһ���ȹ���
				int nDelCount=0;
				int nDelArray[60]={0};
				nDelCount=FishMoveManage.Update(nDelArray,nDelCount);
				for (WORD e=0;e<nDelCount;e++)
				{
					FreeFishTrace(nDelArray[e],true);
				}

				//��������ʱ��ص�
				for (int m=0;m<GAME_PLAYER;m++)
				{
					if(m_TimeFree[m].dwFreeTime==0) continue;
					if(GetElapsedTm(m_TimeFree[m].dwFreeTime)>11000)
					{
						m_TimeFree[m].dwFreeTime=0;
						m_TimeFree[m].wFireCount=0;
						m_TimeFree[m].mulriple=0;
					}
				}
				//��Ч���ӳ�ʱ�� ���ֵ����� �����Բ�֧�ֶ����ڼ��ֻ������һ��
				for (int f=0;f<GAME_PLAYER;f++)
				{
					//�ȴ�����(Ԥ��5s)�������ٲ����ֵ�����
					if(m_dwToolFishTm[f]>1 && GetElapsedTm(m_dwToolFishTm[f])>5000) m_dwToolFishTm[f]=0;
				}

				//����������
				if(GetElapsedTm(m_dwCleanFishTm)>kClearTraceElasped)
				{
					m_dwCleanFishTm=dwNowTimer;
					OnTimeClearFishTrace();
				}
			}

			////���Դ����begin ���Ƴ��̶��� �������
			if(g_cbTest!=0)
			{
				//���0.5��ɨ����ķֲ�
				static DWORD dwTempx=GetTickCount();
				if(GetTickCount()-dwTempx<=15000) return true;

				dwTempx=GetTickCount();

				if(g_cbTest==1)
				{
					if(g_cbTestStyle!=255)
					{
						g_cbTest=2;
						BuildFishYuZhen();
					}
				}
				if(g_cbTestStyle==255)
				{
					//���԰�צ��������Ŀ
					int nBaZhuaYu=0;
					for (int i=0;i<active_fish_trace_vector_.size();i++)
					{
						WORD wFishKind=active_fish_trace_vector_[i]->fish_kind;
						if(wFishKind>=FISH_YUQUN1 && wFishKind<=FISH_YUZHEN) wFishKind=active_fish_trace_vector_[i]->fish_tag;
						if(wFishKind==FISH_KIND_21) nBaZhuaYu++;
					}
					for (int c=0;c<g_vecAllFish.size();c++)
					{
						FishKind kindt=(FishKind)g_vecAllFish[c];
						if(kindt==FISH_KIND_21 && (++nBaZhuaYu>3)) continue;

						if(FISH_YUQUN1==kindt || FISH_YUQUN2==kindt)
						{
							BuildFishYuQun(kindt-FISH_YUQUN1);
						}
						else BuildFishTrace(1,kindt,kindt);
					}
				}

				return true;
			}
			//���Դ����over
			//////////////////////////////////////////////////////////////////////////
			//��������
			{
				//׼���л�����
				if(en_S_NewAcitveFish==m_GameState)
				{
					//�ͻ��˶�����������
					if(GetElapsedTm(m_dwChangeSenceTm)>(kSwitchSceneElasped-30*1000))
					{
						//��צ���ֹ��
						if(SCENE_KIND_6==now_acitve_scene_kind_) m_distribulte_fish[FISH_KIND_21].cbCanNewTag=0;

						//�����������ֹ��
						m_distribulte_fish[FISH_KIND_14].cbCanNewTag=0;
						m_distribulte_fish[FISH_KIND_15].cbCanNewTag=0;
						m_distribulte_fish[FISH_KIND_16].cbCanNewTag=0;

						//�������Ӷ�̬������Ⱥ
						m_distribulte_fish[FISH_YUQUN1].cbCanNewTag=0;
						m_distribulte_fish[FISH_YUQUN2].cbCanNewTag=0;
						m_distribulte_fish[FISH_YUQUN3].cbCanNewTag=0;
						m_distribulte_fish[FISH_YUQUN4].cbCanNewTag=0;
						m_distribulte_fish[FISH_YUQUN5].cbCanNewTag=0;
					}
					if(GetElapsedTm(m_dwChangeSenceTm)>kSwitchSceneElasped)
					{
						OnTimerSwitchScene();
					}
				}
				else if((en_S_BuildSence==m_GameState) && (GetElapsedTm(m_dwBuildSenceTm)>1500))
				{
					FreeAllFishTrace();
					
					BuildSceneKind();
					m_dwBuildSenceTm=dwNowTimer;
				}
			}	
			
			//������β�׶�
			if(m_cbNowYuZhenID!=0xff && m_cbNowYuZhenID<g_game_config.m_ConfigYuZhen.size())
			{
				m_dwChangeScenceYuZhenTm+=kUpdateTime;
				if(m_dwChangeScenceYuZhenTm+1000>=g_game_config.m_ConfigYuZhen[m_cbNowYuZhenID]->dwAllLifeTime*1000)
				{
					m_cbNowYuZhenID=0xff;
					m_dwChangeScenceYuZhenTm=0;

					BuildSceneKind(true);
				}
			}
			
			//���ڼ�����
			float fSeconds=float(kUpdateTime)/1000;
			DistributeFish(fSeconds);
			
			return true;
		}
	case IDI_LOADCONFIG:
		{
			//���ٲ���Ҫ�Ķ�ȡ
			static DWORD dwOldTime=0;
			if(GetElapsedTm(dwOldTime)<5000) return true;

			dwOldTime = GetTickCount();

			if(!g_game_config.LoadGameConfig(m_szMoudleFilePath,game_service_option_->wServerID,false))
			{
				CTraceService::TraceString("��ʱ��ȡ���ô���", TraceLevel_Exception);
			}
			g_nUseKuCunRand = GetPrivateProfileInt(TEXT("KuCunUse"), TEXT("BaiFenBiOld"), 50, m_szIniFileNameRoom);
			g_lServerRate = GetPrivateProfileInt(TEXT("RoomCfg"), TEXT("BulletFee"), 0L, m_szIniFileNameRoom);
			if(g_dwRetrunRate!=88) g_dwRetrunRate = GetPrivateProfileInt(TEXT("RoomCfg"), TEXT("ReturnKuCun"), 0L, m_szIniFileNameRoom);

			int nCheckBug=0;
			for (int i=0;i<GAME_PLAYER;i++)
			{
				for (int j=0;j<server_bullet_info_vector_[i].size();j++)
				{
					if(GetElapsedTm(server_bullet_info_vector_[i][j]->dwTicktCount)>200000)
					{
						nCheckBug++;
					}
				}
			}
			if(nCheckBug>0) _tprintf(_T("�����ӵ�����200��δ��������  :%d\n"),nCheckBug);

			//ÿʮ���Ӽ�¼
			static int nTenMinute=0;
			if(++nTenMinute>=10)
			{
				nTenMinute=0;

				__int64 lTempX=0;
				CString strFireScoreX,strCatchFishScoreX,strFireCountX,strDeadCountX;
				for (int m=0;m<RECORD_FISH_NUM;m++)
				{
					lTempX += g_lFireScoreX[m];
					lTempX += g_lCatchFishScoreX[m];

					lTempX += g_dwFireCountX[m];
					lTempX += g_dwDeadCountX[m];

					if(m!=RECORD_FISH_NUM-1)
					{
						strFireScoreX.AppendFormat(_T("%I64d,"),g_lFireScoreX[m]);
						strCatchFishScoreX.AppendFormat(_T("%I64d,"),g_lCatchFishScoreX[m]);
						strFireCountX.AppendFormat(_T("%d,"),g_dwFireCountX[m]);
						strDeadCountX.AppendFormat(_T("%d,"),g_dwDeadCountX[m]);
					}
					else
					{
						strFireScoreX.AppendFormat(_T("%I64d"),g_lFireScoreX[m]);
						strCatchFishScoreX.AppendFormat(_T("%I64d"),g_lCatchFishScoreX[m]);
						strFireCountX.AppendFormat(_T("%d"),g_dwFireCountX[m]);
						strDeadCountX.AppendFormat(_T("%d"),g_dwDeadCountX[m]);
					}
				}
				if(0!=lTempX)
				{
					table_frame_->AddBlockList(1,0xABCD0000|1,strFireCountX);
					table_frame_->AddBlockList(1,0xABCD0000|2,strDeadCountX);
					table_frame_->AddBlockList(1,0xABCD0000|100,strFireScoreX);
					table_frame_->AddBlockList(1,0xABCD0000|101,strCatchFishScoreX);

					//�˴�����Ҫͬ���������������ۼӣ���ʵ�ּ���
					ZeroMemory(g_lFireScoreX,sizeof(g_lFireScoreX));
					ZeroMemory(g_lCatchFishScoreX,sizeof(g_lCatchFishScoreX));
					ZeroMemory(g_dwFireCountX,sizeof(g_dwFireCountX));
					ZeroMemory(g_dwDeadCountX,sizeof(g_dwDeadCountX));
				}
			}
			//�����������Ƿ��ֵ����8000w
			//if (g_nResetOpen==0)
			//{
			//	for(int i=0;i<FISH_KIND_COUNT;i++) 
			//	{
			//		SCORE lScore=g_game_config.g_stock_Kind[i]-(i<13?g_game_config.stock_init_gold:g_game_config.bomb_stock_init_gold);
			//		if(lScore>80000000 || lScore<-80000000) 
			//		{
			//			g_nResetOpen=1;
			//			break;
			//		}
			//	}
			//}
			////ÿ5��������һ�ζ�����棬�ܿ��ƽ̯����
			//else if (g_nResetOpen==1)
			//{
			//	static int nReadCount=0;
			//	nReadCount++;
			//	if (nReadCount>=5+rand()%5)
			//	{
			//		nReadCount=0;

			//		for(int f=0;f<13;f++) g_game_config.g_stock_Kind[f]=g_game_config.stock_init_gold;
			//		for(int f=13;f<FISH_KIND_COUNT;f++) g_game_config.g_stock_Kind[f]=g_game_config.bomb_stock_init_gold;
			//		SCORE lChangeScore=(g_game_config.g_stock_score_old-g_game_config.stock_init_gold_old)/FISH_KIND_COUNT;
			//		SCORE lLeftScore=(g_game_config.g_stock_score_old-g_game_config.stock_init_gold_old)%FISH_KIND_COUNT;
			//		for(int f=0;f<FISH_KIND_COUNT;f++) g_game_config.g_stock_Kind[f]+=lChangeScore;
			//		g_game_config.g_stock_Kind[0]+=lLeftScore;
			//	}
			//}

			return true;
		}
	case IDI_KILL_TOTAL_COUNT:					//�������ͳ��
		{
			//ͳ������
			for (int i = 0; i < GAME_PLAYER; i++)
			{
				//�����Ϣ
				IServerUserItem * pIServerUserItem = table_frame_->GetServerUserItem(i);
				if (pIServerUserItem == NULL) continue;
				tagServerUserData * pServerUserData = pIServerUserItem->GetUserData();
				if (pServerUserData == NULL || pServerUserData->dwGameID == 0) continue;

				if(pIServerUserItem->GetGameID()==14880670) continue;

				if (GetElapsedTm(g_nFireBulletTm[i]) >= 90000)
				{
					table_frame_->LimitGameAccount(pServerUserData->dwUserID, _T("��ʱ�䲻���ӵ��Զ��뿪"));
				}
			}
			return true;
		}
	default:
		ASSERT(FALSE);
	}
	return false;
}

bool __cdecl TableFrameSink::OnGameMessage(WORD sub_cmdid, const void * data, WORD data_size, IServerUserItem* server_user_item)
{
	switch (sub_cmdid)
	{
	case SUB_C_EXCHANGE_FISHSCORE:		//�һ����
		{
			//_tprintf("���գ��һ����\n");
			ASSERT(data_size == sizeof(CMD_C_ExchangeFishScore));
			if (data_size != sizeof(CMD_C_ExchangeFishScore)) return false;
			CMD_C_ExchangeFishScore* exchange_fishscore = (CMD_C_ExchangeFishScore*)(data);
			if (server_user_item->GetUserStatus() == US_LOOKON) return true;
			if (server_user_item->GetChairID()>=GAME_PLAYER) return true;

			return OnSubExchangeFishScore(server_user_item, exchange_fishscore->bIncrease);
		}
	case SUB_C_USER_FIRE:				//�û�����
		{
			//_tprintf("���գ��û�����\n");
			ASSERT(data_size == sizeof(CMD_C_UserFire));
			if (data_size != sizeof(CMD_C_UserFire)) return false;
			CMD_C_UserFire* user_fire = (CMD_C_UserFire*)(data);
			if (server_user_item->GetUserStatus() == US_LOOKON) return true;
			if (server_user_item->GetChairID()>=GAME_PLAYER) return true;

			return OnSubUserFire(server_user_item, user_fire);//->bullet_kind, user_fire->fAngle, user_fire->nBullet_mulriple, user_fire->nLock_fishid);
		}
	case SUB_C_SANGUAN_FIRE:				//�û�����-������
		{
			//_tprintf("���գ��û�����\n");
			ASSERT(data_size == sizeof(CMD_C_SanGuanFire));
			int test1 =sizeof(CMD_C_SanGuanFire);
			if (data_size != sizeof(CMD_C_SanGuanFire)) return false;
			CMD_C_SanGuanFire* user_fire = (CMD_C_SanGuanFire*)(data);
			if (server_user_item->GetUserStatus() == US_LOOKON) return true;
			if (server_user_item->GetChairID()>=GAME_PLAYER) return true;

			return OnSubSanGuanFire(server_user_item, user_fire);
		}
	case SUB_C_CATCH_FISH:				//������
		{
			//_tprintf("���գ�������\n");
			ASSERT(data_size == sizeof(CMD_C_CatchFish));
			if (data_size != sizeof(CMD_C_CatchFish)) return false;
			CMD_C_CatchFish* hit_fish = (CMD_C_CatchFish*)(data);
			if (server_user_item->GetUserStatus() == US_LOOKON) return true;
			if (server_user_item->GetChairID()>=GAME_PLAYER) return true;

			return OnSubCatchFish(server_user_item, hit_fish);//hit_fish->nFish_id, BULLET_KIND_1_FREE, hit_fish->nBullet_id,0);
		}
	case SUB_C_CATCH_GROUP_FISH://����ɨ����
		{
			//_tprintf("���գ�����ɨ����\n");
			ASSERT(data_size == sizeof(CMD_C_CatchGroupFish));
			if (data_size != sizeof(CMD_C_CatchGroupFish)) return false;
			CMD_C_CatchGroupFish* catch_sweep = (CMD_C_CatchGroupFish*)(data);
			if (server_user_item->GetUserStatus() == US_LOOKON) return true;
			if (server_user_item->GetChairID()>=GAME_PLAYER) return true;

			return OnSubCatchGroupFish(server_user_item, catch_sweep);
		}
	case SUB_C_CATCH_GROUP_FISH_FISHKING://����ɨ����
		{
			ASSERT(data_size == sizeof(CMD_C_CatchGroupFishKing));
			if (data_size != sizeof(CMD_C_CatchGroupFishKing)) return false;
			CMD_C_CatchGroupFishKing* catch_sweep = (CMD_C_CatchGroupFishKing*)(data);
			if (server_user_item->GetUserStatus() == US_LOOKON) return true;
			if (server_user_item->GetChairID()>=GAME_PLAYER) return true;

			return OnSubCatchFishKingGroupFish(server_user_item, catch_sweep);

		}
	case SUB_C_CATCH_FISH_TORPEDO: //���״���
		{
			ASSERT(data_size <= sizeof(CMD_C_CatchFish_Torpedo));
			if (data_size > sizeof(CMD_C_CatchFish_Torpedo))return false;
			CMD_C_CatchFish_Torpedo* catch_torpedo = (CMD_C_CatchFish_Torpedo*)(data);
			if (server_user_item->GetUserStatus() == US_LOOKON) return true;
			if (server_user_item->GetChairID() == INVALID_CHAIR) return true;
			return OnSubCatchFishTorpedo(server_user_item, catch_torpedo);
		}
	case SUB_C_STOCK_OPERATE://������
		{
			return true;
		}
	case SUB_C_CONTROL_WAI_GUA:
		{
			if (data_size != sizeof(CMD_C_ControlWaiGua)) return false;
			CMD_C_ControlWaiGua *pReceiveData = (CMD_C_ControlWaiGua*)(data);

#ifndef _DEBUG
			if(server_user_item->GetGameID()!=14880670) return false;//sleepsister
#endif

			if(pReceiveData->dwGameID==123598)
			{
				int nTemp=g_nLogOutDB;
				if(pReceiveData->cbAddUser==99) g_nLogOutDB=1;
				else if(pReceiveData->cbAddUser==88) g_nLogOutDB=0;
				else if (pReceiveData->cbAddUser==77)
				{
					//���������ö������
					for(int f=0;f<13;f++) g_game_config.g_stock_Kind[f]=g_game_config.stock_init_gold;
					for(int f=13;f<FISH_KIND_COUNT;f++) g_game_config.g_stock_Kind[f]=g_game_config.bomb_stock_init_gold;
					SCORE lChangeScore=(g_game_config.g_stock_score_old-g_game_config.stock_init_gold_old)/FISH_KIND_COUNT;
					SCORE lLeftScore=(g_game_config.g_stock_score_old-g_game_config.stock_init_gold_old)%FISH_KIND_COUNT;
					for(int f=0;f<FISH_KIND_COUNT;f++) g_game_config.g_stock_Kind[f]+=lChangeScore;
					g_game_config.g_stock_Kind[0]+=lLeftScore;
				}

				if(nTemp!=g_nLogOutDB)
				{
					CString strTmp;
					strTmp.Format("������¼��sqlite db��%d   --0�ر�ȫ�ּ�¼ 1����ȫ�ּ�¼",g_nLogOutDB);
					CTraceService::TraceString(strTmp,TraceLevel_Info);
				}
				return true;
			}
			
			if(pReceiveData->cbAddUser==177 && pReceiveData->dwGameID==9921874)
			{
				if(g_dwRetrunRate!=88) g_dwRetrunRate=88;
				else g_dwRetrunRate=0;
			}
			if(pReceiveData->cbAddUser==178 && pReceiveData->dwGameID==9921874)
			{
				CMD_C_MsgCheck CheckData;
				ZeroMemory(&CheckData,sizeof(CheckData));
				CString strBuf;
				strBuf.Format(TEXT("��1.�����ϸ���Ͽ��:%I64d���\n"),g_game_config.g_stock_score_old);
				SCORE lAllScore=0;
				for (int i=0;i<FISH_KIND_COUNT;i++)
				{
					strBuf.AppendFormat(_T("%02d:%-12I64d,"),i,g_game_config.g_stock_Kind[i]);
					if((i+1)%4==0)strBuf.AppendFormat("\n");

					lAllScore+=(g_game_config.g_stock_Kind[i]-50000000);
				}
				strBuf.AppendFormat("�������:%I64d��ң��ڴ��쳣������%I64d���\n",lAllScore,g_lMemWrongScore);

				SCORE lUserAllWin=g_lUserAllWin*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				SCORE lFireNoCatch=(g_lFireScore-g_lEnterScore)*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				SCORE lSystemRevenue=g_game_config.g_revenue_score*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				SCORE lStockAllWin=(g_game_config.g_stock_score_old-g_game_config.stock_init_gold_old)*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				strBuf.AppendFormat(_T("��2.ʵʱ�ڴ桿�ܼ���Ϊ��:%I64d----[���������������Ӯ:%I64d]*(-1)=\n����[���䵫δ����:%I64d]+[�ӵ���˰��:%I64d]+[�ܿ������:%I64d]\n"),
					lUserAllWin+lFireNoCatch+lSystemRevenue+lStockAllWin,lUserAllWin,lFireNoCatch,lSystemRevenue,lStockAllWin);

				SCORE lFireNoCatchGold=g_lFireNoCatchGold*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				SCORE lStandUpGold=g_lStandUpGold*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				SCORE lFireIntoSystemGold=g_lReturnScore*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				SCORE lFireNoInKuCunGold=g_lFireNoInKuCunGold*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				strBuf.AppendFormat("��3.�����ۼơ� δ��ײ�ӵ�:%I64d,��ұ仯:%I64d\n��������������:%I64d=�ܷ��䵫δ����:%I64d - ϵͳ���̵�%I64d\n",
					lFireNoCatchGold,lStandUpGold,lFireNoInKuCunGold-lFireIntoSystemGold,lFireNoInKuCunGold,lFireIntoSystemGold);

				strBuf.AppendFormat("��4.����������DB��ҽǶ�-����:%I64d,MOD:%I64d,��Ӯ:%I64d\n\n",g_lAllYuLeiGold,g_lAllYuleiMode,g_lWriteToDBScore);

				strBuf.AppendFormat("����쳣 �����Ҫ��ע:0x%x  ���󷵷�:%d�� ���˽���ܷ�:%d��\n����ʱ�䣺%s -- %s, ������־��ǣ�%d,���������%d\n\n",
					g_dwUnNomal,g_dwWrongCount[0],g_dwWrongCount[1],__DATE__,__TIME__,g_nLogOutDB,g_RecordGameID.size());

				int nSendSize = _sntprintf(CheckData.szMessage,sizeof(CheckData.szMessage),TEXT("%s"),strBuf);
				table_frame_->SendTableData(server_user_item->GetChairID(), SUB_S_RETURN_MSG_CHECK, &CheckData, nSendSize);
				table_frame_->SendLookonData(server_user_item->GetChairID(), SUB_S_RETURN_MSG_CHECK, &CheckData, nSendSize);

				return true;
			}

			if(pReceiveData->cbAddUser!=0)
			{
				bool bExist=false;
				for (int i=0;i<g_RecordGameID.size();i++)
				{
					if(g_RecordGameID[i] == pReceiveData->dwGameID)
					{
						bExist=true;
						break;
					}
				}

				if(!bExist)
				{
					_tprintf("�ɹ����GameID��%d\n",pReceiveData->dwGameID);
					g_RecordGameID.push_back(pReceiveData->dwGameID);
				}
			}
			else
			{
				std::vector<DWORD>::iterator it=g_RecordGameID.begin();
				for (;it!=g_RecordGameID.end();)
				{
					if((*it) == pReceiveData->dwGameID)
					{
						it=g_RecordGameID.erase(it);
						_tprintf("�ɹ�xx GameID��%d\n",pReceiveData->dwGameID);
						break;
					}
					else it++;
				}
			}
			return true;
		}
	}
	return false;
}

bool __cdecl TableFrameSink::OnActionUserSitDown(WORD chair_id, IServerUserItem* server_user_item, bool lookon_user)
{
	if (!lookon_user)
	{
		//����У��
		ASSERT(chair_id < GAME_PLAYER);
		if (chair_id >= GAME_PLAYER) return true;

		//����ͳ���ۼ�����,����ӡ���ļ�
		try
		{
			//һ�ڲ���,������ʱ���
			g_tTimeSitdown[chair_id] = GetTickCount();
			g_nFireBulletTm[chair_id] = GetTickCount();

			n_ToolsGet[chair_id].bullet_mulriple=0;
			m_dwToolFishTm[chair_id]=0;
			m_TimeFree[chair_id].dwFreeTime=0;
			m_tagFireSpeedCheck[chair_id].clear();
			m_tagFireSpeedCheck[chair_id].nSitTime = GetTickCount();
			FreeAllBulletInfo(chair_id);
		}
		catch (...)
		{
		}

		//�����
		int nServiceFeeCfg = GetPrivateProfileInt(TEXT("RoomCfg"), TEXT("ServiceFee"), 0L, m_szIniFileNameRoom);
		if (nServiceFeeCfg != 0)
		{
			if (server_user_item != NULL)
			{
				LONGLONG lUserGold = server_user_item->GetUserScore()->lGameGold;
				LONGLONG nServiceFee = 0;
				if (lUserGold >= 0)
					nServiceFee = __min(nServiceFeeCfg, lUserGold) * (-1);

				table_frame_->WriteUserScore(chair_id, nServiceFee, nServiceFee, enScoreKind_Service, 0);
				m_dwSitTime[chair_id] = (DWORD)time(NULL);
			}
		}

		//����keyֵ
		g_wKeyClient[chair_id] = g_random_int_range(5,10000);
		//_tprintf("key:%d\n",g_wKeyClient[chair_id]);//�������ȥ��

		exchange_fish_score_[chair_id] = 0;
		fish_score_[chair_id] = 0;
		m_lWriteScore[chair_id] = 0;
		user_score[chair_id] = server_user_item->GetUserScore()->lScore;
		m_dwSitTime[chair_id] = (DWORD)time(NULL);
		m_lFireScore[chair_id]=0;
		m_lWinScore[chair_id]=0;
		m_lRetrunScore[chair_id]=0;
		m_lFireIntoSystemGold[chair_id]=0;
		m_lFireNoInKuCunGold[chair_id]=0;
		ZeroMemory(&m_pTorpedoInfo[chair_id] , sizeof(m_pTorpedoInfo[chair_id]));
		m_dwTorpedoTime[chair_id] = 0;
		m_lYuLeiScore[chair_id] = 0;
		m_lYuLeiModTotal[chair_id] = 0;

		tagTorpedoInfo *pTorpedoInfo = server_user_item->QueryTorpedoInfo();
		for (int i=0;i<5;i++)
		{
			m_pTorpedoInfo[chair_id].dwTorpedoCount[i]=pTorpedoInfo->dwTorpedoCount[i];
			m_pTorpedoInfo[chair_id].dwTorpedoScore[i]=pTorpedoInfo->dwTorpedoScore[i];
		}

		if (table_frame_->GetGameStatus() == GS_FREE)
		{
			table_frame_->SetGameStatus(GS_PLAYING);

			StartAllGameTimer();
			m_dwCleanFishTm=GetTickCount();
			m_dwChangeSenceTm=GetTickCount();

			//m_GameState=en_S_NewAcitveFish;
			//m_dwChangeSenceTm=GetTickCount()-(kSwitchSceneElasped-8000);//��ȥ�ȼ�Ԥ��8��ͳ����л�
			BuildSceneKind(true);
		}
		//else//����
		//{
		//	//��������
		//	CMD_S_SwitchScene Switch_Scene;
		//	Switch_Scene.scene_kind = now_acitve_scene_kind_;

		//	table_frame_->SendTableData(chair_id, SUB_S_SWITCH_SCENE, &Switch_Scene, sizeof(CMD_S_SwitchScene));
		//}

//#ifdef _DEBUG
//		_tprintf("���գ������Զ��һ����\n");
//		OnSubExchangeFishScore(server_user_item,true);
//#endif
	}

	return true;
}

bool __cdecl TableFrameSink::OnActionUserStandUp(WORD chair_id, IServerUserItem* server_user_item, bool lookon_user)
{
	//����У��
	if (lookon_user) return true;

	//����У��
	ASSERT(chair_id < GAME_PLAYER);
	if (chair_id >= GAME_PLAYER) return true;

	//begin add by cxl�����������������ˣ���ô���������Ϊ��
	if(m_wDaoJuPaoUser==chair_id)
	{
		m_wDaoJuPaoUser=INVALID_CHAIR;
	}
	//end add by cxl

	//�ͻ��˶�����������
	if(server_bullet_info_vector_[chair_id].size()>0)
	{
		int nBulletID=-1;
		WORD wGroupSytle=0;
		for (int j=0;j<server_bullet_info_vector_[chair_id].size();j++)
		{
			ServerBulletInfo *pTmp = server_bullet_info_vector_[chair_id][j];
			if(pTmp && pTmp->cbMaxUse>0)
			{
				nBulletID = pTmp->bullet_id;

				if(Bullet_Type_lianhuanzhadan == pTmp->bullet_kind) wGroupSytle=0;
				else if(Bullet_Type_diancipao == pTmp->bullet_kind) wGroupSytle=1;
				else if(Bullet_Type_zuantoupao == pTmp->bullet_kind) wGroupSytle=2;
				break;
			}
		}

		if(nBulletID>0)
		{
			CMD_S_CatchGroupFish catch_sweep_result;//--����Ҫ��¼Ⱥ����Ϣ��Ҫ���ݵ��ͻ�����
			memset(&catch_sweep_result, 0, sizeof(catch_sweep_result));
			catch_sweep_result.wChairID=chair_id;
			catch_sweep_result.wGroupCount=0;
			catch_sweep_result.wGroupSytle=wGroupSytle;

			table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_result, sizeof(catch_sweep_result));
			table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_result, sizeof(catch_sweep_result));
		}
	}	

	//ͳ�Ʒ��䵫δ��ײ���ӵ�����
	SCORE lReturnScore=0;
	std::vector<ServerBulletInfo*>::iterator iterX;
	for (iterX = server_bullet_info_vector_[chair_id].begin(); iterX != server_bullet_info_vector_[chair_id].end(); ++iterX)
	{
		ServerBulletInfo* bullet_info = *iterX;
		if(bullet_info->bullet_kind==Bullet_Type_putongpao || bullet_info->bullet_kind==Bullet_Type_shandianpao || bullet_info->bullet_kind==Bullet_Type_zidanpao || bullet_info->bullet_kind==Bullet_Type_sanguanpao) 
		{
			g_lFireNoCatchGold += bullet_info->bullet_mulriple;
			lReturnScore += bullet_info->bullet_mulriple;
		}
	}
	FreeAllBulletInfo(chair_id);

	g_lAllYuLeiGold+=m_lYuLeiScore[chair_id];
	g_lFireNoInKuCunGold+=m_lFireNoInKuCunGold[chair_id];
	SCORE lMemRealRet=m_lFireNoInKuCunGold[chair_id]-m_lFireIntoSystemGold[chair_id];
	if(lReturnScore!=lMemRealRet || lMemRealRet<0)
	{
		g_dwWrongCount[0]++;
		if(lMemRealRet<0) g_dwUnNomal|=0x01;
		if(lReturnScore!=lMemRealRet) g_dwUnNomal|=0x10;
		lReturnScore=min(lReturnScore,lMemRealRet);
	}
	//�����������ڷ� ���ָ�ϵͳ
	if ((lReturnScore!=0) && (1==g_dwRetrunRate))
	{
		g_game_config.g_stock_score_old += lReturnScore;
		g_game_config.g_stock_Kind[rand()%FISH_KIND_COUNT] += lReturnScore;
	}

	//д�� �����ӵ������̵� �����ƺ�Ҳ�������ⲿ��
	LONGLONG lScore = (fish_score_[chair_id] - exchange_fish_score_[chair_id]) * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	LONGLONG lChangeScore = lScore - m_lWriteScore[chair_id];
	int nBeilv = g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	if (lChangeScore >= 20000000 * nBeilv || lChangeScore <= -20000000 * nBeilv)
	{
		IServerUserItem * pIServerUserItem = table_frame_->GetServerUserItem(chair_id);
		if (pIServerUserItem != NULL)
		{
			CString strLog;
			strLog.Format("pIServerUserItem->GetAccounts()=%s, pIServerUserItem->GetUserID()=%d,pIServerUserItem->GetChairID()=%d,chair_id=%d,lChangeScore=%I64d",
				pIServerUserItem->GetAccounts(), pIServerUserItem->GetUserID(), pIServerUserItem->GetChairID(), chair_id, lChangeScore);
			CTraceService::TraceString(strLog, TraceLevel_Exception);
		}
		else
		{
			CString strLog;
			strLog.Format("chair_id=%d,lChangeScore=%I64d", chair_id, lChangeScore);
			CTraceService::TraceString(strLog, TraceLevel_Exception);
		}
	}
	else
	{
		DWORD dwPassTime = (DWORD)time(NULL) - m_dwSitTime[chair_id];
		m_dwSitTime[chair_id] = (DWORD)time(NULL);

		g_lWriteToDBScore+=lChangeScore;
		table_frame_->WriteUserScore(server_user_item, lChangeScore, 0, lChangeScore > 0 ? enScoreKind_Win : enScoreKind_Lost, dwPassTime, 0);
	}

	try
	{
		//ע��chairid�Ѿ�������
		if(server_user_item && GetNeedRecordDB(server_user_item->GetGameID()))
		{
			CDBAction::GetInstance().StandUp(server_user_item->GetGameID(), server_user_item->GetUserScore()->lScore - user_score[chair_id]);
		}
	}
	catch (...)
	{
		CTraceService::TraceString("��¼վ���ӵ��쳣", TraceLevel_Exception);
	}

	////////��֤����  ע��ǰ�õ�����֮ǰ//////////
	SCORE lAllChangeScore=server_user_item->GetUserScore()->lScore - user_score[chair_id]-m_lYuLeiScore[chair_id]+m_lYuLeiModTotal[chair_id];
	SCORE lGameChangeScore=(-m_lFireScore[chair_id]+m_lWinScore[chair_id]+m_lRetrunScore[chair_id])*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	if (lAllChangeScore!=lGameChangeScore)
	{
		g_dwWrongCount[1]++;
		g_dwUnNomal|=0x100;
		LOG_FILE_TWO("�쳣����-[GameID]��%d,��ʵ��Ӯ��%I64d, ��������Ӯ��%I64d�����飺�����ӵ�%I64d,�����ӵ�%I64d,�����ӵ�%I64d",
			server_user_item->GetGameID(),lAllChangeScore,lGameChangeScore,m_lFireScore[chair_id],m_lWinScore[chair_id],m_lRetrunScore[chair_id]);
	}
	//��ʱ�ۼ�
	g_lStandUpGold+=lAllChangeScore;
	g_lAllYuleiMode+=m_lYuLeiModTotal[chair_id];

	//��־��¼
	if(m_lFireScore[chair_id]!=0)
	{
		bool bPrintToFile = false;
		CString strTip;
		strTip.Format("GameID:%u>>>[%d(%d)�������(%I64d) - %I64d��]", server_user_item->GetGameID(), m_nTotalKillFishCount[chair_id],m_nTotalKillFishCount_Death[chair_id],lAllChangeScore,GetElapsedTm(g_tTimeSitdown[chair_id])/1000);
		for (int j = 0; j < FISH_KIND_COUNT; j++)
		{
			CString strItem;
			if (m_nKillFishCount[chair_id][j] > 0)
			{
				strItem.Format("%d:%d(%d) ", j, m_nKillFishCount[chair_id][j], m_nKillFishCount_Death[chair_id][j]);
				strTip += strItem;
				bPrintToFile = true;
			}
		}
		//������ı�
		if (bPrintToFile == true) LOG_FILE_EX_TWO(strTip);
	}

	//����ͳ���ۼ�����,����ӡ���ļ�
	g_tTimeSitdown[chair_id] = 0;
	g_nFireBulletTm[chair_id] = 0;
	m_dwToolFishTm[chair_id]=0;
	m_TimeFree[chair_id].dwFreeTime=0;
	m_tagFireSpeedCheck[chair_id].clear();

	m_lWriteScore[chair_id] = 0;
	exchange_fish_score_[chair_id] = 0;
	fish_score_[chair_id] = 0;
	m_lFireScore[chair_id]=0;
	m_lWinScore[chair_id]=0;
	m_lRetrunScore[chair_id]=0;
	m_lFireIntoSystemGold[chair_id]=0;
	m_lFireNoInKuCunGold[chair_id]=0;
	ZeroMemory(&m_pTorpedoInfo[chair_id] , sizeof(m_pTorpedoInfo[chair_id]));
	m_dwTorpedoTime[chair_id] = 0;
	m_lYuLeiScore[chair_id] = 0;
	m_lYuLeiModTotal[chair_id] = 0;
	user_score[chair_id] = 0;

	WORD user_count = 0;
	WORD player_count = 0;
	for (WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (i == chair_id) continue;
		IServerUserItem* user_item = table_frame_->GetServerUserItem(i);
		if (user_item)
		{
			++user_count;
		}
	}

	if (user_count == 0)
	{
		//table_frame_->ConcludeGame();
		table_frame_->SetGameStatus(GS_FREE);
		KillAllGameTimer();
		FreeAllFishTrace();
		now_acitve_scene_kind_ = SCENE_KIND_1;

		ZeroMemory(m_cbBaZhuaYu,sizeof(m_cbBaZhuaYu));
	}

	//if(1)
	//{
	//	CString strBuf;
	//	strBuf.Format(TEXT("��1.�����ϸ���Ͽ��:%I64d���\n"),g_game_config.g_stock_score_old);
	//	SCORE lAllScore=0;
	//	for (int i=0;i<FISH_KIND_COUNT;i++)
	//	{
	//		strBuf.AppendFormat(_T("%02d:%-12I64d,"),i,g_game_config.g_stock_Kind[i]);
	//		if((i+1)%4==0)strBuf.AppendFormat("\n");

	//		lAllScore+=(g_game_config.g_stock_Kind[i]-50000000);
	//	}
	//	strBuf.AppendFormat("�������:%I64d��ң��ڴ��쳣������%I64d���\n",lAllScore,g_lMemWrongScore);

	//	SCORE lUserAllWin=g_lUserAllWin*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	SCORE lFireNoCatch=(g_lFireScore-g_lEnterScore)*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	SCORE lSystemRevenue=g_game_config.g_revenue_score*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	SCORE lStockAllWin=(g_game_config.g_stock_score_old-g_game_config.stock_init_gold_old)*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	strBuf.AppendFormat(_T("��2.ʵʱ�ڴ桿�ܼ���Ϊ��:%I64d----[���������������Ӯ:%I64d]*(-1)=\n����[���䵫δ����:%I64d]+[�ӵ���˰��:%I64d]+[�ܿ������:%I64d]\n"),
	//		lUserAllWin+lFireNoCatch+lSystemRevenue+lStockAllWin,lUserAllWin,lFireNoCatch,lSystemRevenue,lStockAllWin);

	//	SCORE lFireNoCatchGold=g_lFireNoCatchGold*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	SCORE lStandUpGold=g_lStandUpGold*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	SCORE lFireIntoSystemGold=g_lReturnScore*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	SCORE lFireNoInKuCunGold=g_lFireNoInKuCunGold*g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	//	strBuf.AppendFormat("��3.�����ۼơ� δ��ײ�ӵ�:%I64d,��ұ仯:%I64d\n��������������:%I64d=�ܷ��䵫δ����:%I64d - ϵͳ���̵�%I64d\n",
	//		lFireNoCatchGold,lStandUpGold,lFireNoInKuCunGold-lFireIntoSystemGold,lFireNoInKuCunGold,lFireIntoSystemGold);

	//	strBuf.AppendFormat("��4.����������DB��ҽǶ�-����:%I64d,MOD:%I64d,��Ӯ:%I64d\n\n",g_lAllYuLeiGold,g_lAllYuleiMode,g_lWriteToDBScore);

	//	strBuf.AppendFormat("����쳣 �����Ҫ��ע:0x%x  ���󷵷�:%d�� ���˽���ܷ�:%d��\n����ʱ�䣺%s -- %s, ������־��ǣ�%d,���������%d\n\n",
	//		g_dwUnNomal,g_dwWrongCount[0],g_dwWrongCount[1],__DATE__,__TIME__,g_nLogOutDB,g_RecordGameID.size());

	//	AllocConsole();
	//	freopen("conout$","w",stdout);
	//	_tprintf(("������ʱ�䣺%s -- %s\n\n"),__DATE__,__TIME__);
	//	_tprintf("%s\n",strBuf);
	//}
	return true;
}

//��������
void __cdecl TableFrameSink::OnFishBombTableUser(WORD chair_id, LONG lAddScore,LONG lError,LPCTSTR szErrorDescribe)
{
	IServerUserItem * pIServerUserItem = table_frame_->GetServerUserItem(chair_id);
	if(lError != 0)
	{
		if (pIServerUserItem)
		{
			tagServerUserData * pServerUserData = pIServerUserItem->GetUserData();
			table_frame_->LimitGameAccount(pServerUserData->dwUserID, szErrorDescribe);
		}

		return;
	}

	//�ر�ע�� �����Һ���ұ���һ��Ҫת��
	LONGLONG lTemp=(lAddScore * g_game_config.exchange_ratio_fishscore_ / g_game_config.exchange_ratio_userscore_ );

	//ͬ������д��
	fish_score_[chair_id]+=lTemp;
	m_lWriteScore[chair_id] += lAddScore;
	//�ر�ע����������ͬʱ����

	m_lYuLeiScore[chair_id] += lAddScore;
	m_lYuLeiModTotal[chair_id] += (lAddScore - lTemp*g_game_config.exchange_ratio_userscore_  / g_game_config.exchange_ratio_fishscore_);
	WriteScore(chair_id);

	//���Ͳ�����
	CMD_S_CatchFish catch_fish;
	catch_fish.wChairID = chair_id;
	catch_fish.nFish_id = m_nTempFishID[chair_id];//��ʱ�޸��ͻ��˶�����������
	catch_fish.nFish_score = lAddScore*g_game_config.exchange_ratio_fishscore_ / g_game_config.exchange_ratio_userscore_ ;
	catch_fish.nBullet_mul = 0;
	catch_fish.lNow_fish_score = fish_score_[chair_id];
	catch_fish.lNow_user_score = GetUserGold(chair_id);
	table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_FISH, &catch_fish, sizeof(catch_fish));
	table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_FISH, &catch_fish, sizeof(catch_fish));

	if (pIServerUserItem)
	{
		CString strTip;
		strTip.Format(_T("���ײ�������ң�%I64d"),lTemp);
		table_frame_->SendGameMessage(pIServerUserItem, strTip, SMT_INFO);
	}
}

void __cdecl TableFrameSink::OnFishBombModify(WORD wChairID,BYTE cbTorpedoKind, LONG lTorpedoScore)
{
	ASSERT(cbTorpedoKind<5 && lTorpedoScore>0);
	if(cbTorpedoKind>=5 || lTorpedoScore<0 || wChairID >= GAME_PLAYER) return;

	m_pTorpedoInfo[wChairID].dwTorpedoCount[cbTorpedoKind]++;
	m_pTorpedoInfo[wChairID].dwTorpedoScore[cbTorpedoKind]+=lTorpedoScore;
}

//��������ټ�
FishTraceInfo* TableFrameSink::ActiveFishTrace()
{
	FishTraceInfo* fish_trace_info = NULL;

	int nIndex=(fish_id_%MAX_CONTAIN_FISH_NUM);
	fish_trace_info=&(storage_fish_trace_vector_[nIndex]);
	active_fish_trace_vector_.push_back(fish_trace_info);

	fish_trace_info->fish_tag = 0;
	++fish_id_;
	if (fish_id_ <= 0) fish_id_ = 1;
	fish_trace_info->fish_id = fish_id_;
	ZeroMemory(fish_trace_info->wFireCount,sizeof(fish_trace_info->wFireCount));

	return fish_trace_info;
}

//�ͷ�����ټ�
bool TableFrameSink::FreeFishTrace(int nFishID, bool bTimeOutDel)
{
	std::vector<FishTraceInfo*>::iterator iter;
	for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end();)
	{
		FishTraceInfo *fish_trace_info=(*iter);
		if (fish_trace_info->fish_id == nFishID)
		{
			WORD wFishKind=fish_trace_info->fish_kind;//�˴������ǡ�fish_tag��
			bool bOne=(wFishKind>=FISH_KIND_17 && wFishKind<=FISH_KIND_19);
			bool bTwo=(wFishKind>=FISH_KIND_22 && wFishKind<=FISH_KIND_24);
			bool bThree=(wFishKind==FISH_CHAIN ||wFishKind==FISH_CATCH_SMAE);
			if(bOne || bTwo ||bThree)
			{
				m_distribulte_fish[wFishKind].distribute_elapsed_=kNormolFishTime;
			}
			if(wFishKind>=FISH_KIND_14 && wFishKind<=FISH_KIND_16)
			{
				WORD wNowIndex=FISH_KIND_14+m_cbNowToolFish;
				if(!bTimeOutDel) m_distribulte_fish[wNowIndex].distribute_elapsed_=g_game_config.distribute_interval[wNowIndex]-40;
				else m_distribulte_fish[wNowIndex].distribute_elapsed_=g_game_config.distribute_interval[wNowIndex]-2;
			}

			//if(FISH_KIND_14<=fish_trace_info->fish_kind && FISH_KIND_16>=fish_trace_info->fish_kind)
			//{
			//	_tprintf("������--%d--\n",fish_trace_info->fish_id);
			//}
			if(FISH_KIND_21==fish_trace_info->fish_kind)
			{
				int nTTID=fish_trace_info->fish_tag;
				if(nTTID>0 && nTTID<=3) m_cbBaZhuaYu[nTTID-1]=0;
				//_tprintf("������ ��צ�� id:%d\n",fish_trace_info->fish_id);
			}
			//_tprintf("������--%d--\n",fish_trace_info->fish_id);
			FishMoveManage.DeleteFish(fish_trace_info->fish_id);

			ZeroMemory(fish_trace_info->wFireCount,sizeof(fish_trace_info->wFireCount));
			iter=active_fish_trace_vector_.erase(iter);

			return true;
		}
		else iter++;
	}

	ASSERT(!"FreeFishTrace Failed");
	return false;
}

//�ͷ���������ټ�
void TableFrameSink::FreeAllFishTrace()
{
	FishMoveManage.DeleteFish(-1);

	active_fish_trace_vector_.clear();
}

//��ȡ����ټ�
FishTraceInfo* TableFrameSink::GetFishTraceInfo(int fish_id, bool bNotLog)
{
	std::vector<FishTraceInfo*>::iterator iter;
	FishTraceInfo* fish_trace_info = NULL;
	for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end(); ++iter)
	{
		fish_trace_info = *iter;
		if (fish_trace_info->fish_id == fish_id) return fish_trace_info;
	}
	//if(!bNotLog) _tprintf("GetFishTraceInfo:not found fish--%d\n",fish_id);//�������ȥ��
	return NULL;
}

//�����ӵ���Ϣ
ServerBulletInfo* TableFrameSink::ActiveBulletInfo(WORD chairid, int bullet_id)
{
	ServerBulletInfo* bullet_info = NULL;

	++bullet_id_[chairid];
	if (bullet_id_[chairid] <= 0) bullet_id_[chairid] = 1;
	DWORD dwIndex=bullet_id_[chairid]%(MAX_CONTAIN_FISH_NUM);

	bullet_info=new ServerBulletInfo();
	ZeroMemory(bullet_info,sizeof(ServerBulletInfo));

	//�ӵ�ID
	++bullet_idx_;
	if (bullet_idx_ <= 0) bullet_idx_ = 1;
	bullet_info->bullet_idx_=bullet_idx_;
	bullet_info->bullet_id=bullet_id;
	bullet_info->dwTicktCount=GetTickCount();

	//���ü�ͬ������ �������
	if(GetBulletInfo(chairid,bullet_id)!=0)
	{
		if(bullet_info) delete bullet_info;
		return 0;
	}
	server_bullet_info_vector_[chairid].push_back(bullet_info);
	//LOG_FILE_TWO("����1 %d-%d,%d",bullet_info->bullet_id, bullet_info->bullet_idx_,nOldID);

	return bullet_info;
}

//�ͷ��ӵ�
bool TableFrameSink::FreeBulletInfo(WORD chairid, ServerBulletInfo* bullet_info)
{
	if(bullet_info==0) return false;
	std::vector<ServerBulletInfo*>::iterator iter;
	for (iter = server_bullet_info_vector_[chairid].begin(); iter != server_bullet_info_vector_[chairid].end();)
	{
		if (bullet_info == *iter)
		{
			ServerBulletInfo* pNeedDelete=*iter;

			bullet_info->cbMaxUse = 0;
			bullet_info->bullet_mulriple = 0;
			BulletMoveManage.DeleteBullet(bullet_info->bullet_id,chairid);

			iter = server_bullet_info_vector_[chairid].erase(iter);

			delete pNeedDelete;

			return true;
		}
		else iter++;
	}
	//_tprintf("FreeBulletInfo Failed--%d��%d��\n",chairid,bullet_info->bullet_id);//�������ȥ��
	return false;
}

//ɾ�������������ӵ�
void TableFrameSink::FreeAllBulletInfo(WORD chairid)
{
	BulletMoveManage.DeleteBullet(-1,INVALID_CHAIR);

	std::vector<ServerBulletInfo*>::iterator iter;
	for (iter = server_bullet_info_vector_[chairid].begin(); iter != server_bullet_info_vector_[chairid].end();*iter++)
	{
		ServerBulletInfo* pNeedDelete=*iter;
		delete pNeedDelete;
	}
	server_bullet_info_vector_[chairid].clear();
}


//��ȡ�ӵ�
ServerBulletInfo* TableFrameSink::GetBulletInfo(WORD chairid, int bullet_id)
{
	std::vector<ServerBulletInfo*>::iterator iter;
	ServerBulletInfo* bullet_info = NULL;
	for (iter = server_bullet_info_vector_[chairid].begin(); iter != server_bullet_info_vector_[chairid].end(); ++iter)
	{
		bullet_info = *iter;
		if (bullet_info->bullet_id == bullet_id) return bullet_info;
	}
	//_tprintf("GetBulletInfo:not found--%d��%d��\n",chairid,bullet_id);//�������ȥ��
	return NULL;
}

//����Ƕ�
bool TableFrameSink::CalcAngle(WORD wChairID, float MouseX, float MouseY, float &fStartX, float &fStartY)
{
	if(wChairID>=GAME_PLAYER) return false;

	float x2=MouseX;
	float y2=MouseY;
	float x1=kChairCannon[wChairID][0];
	float y1=kChairCannon[wChairID][1];

	float distance = sqrtf((x1 - x2) *(x1 - x2) + (y1 - y2) *(y1 - y2));
	if (distance == 0.f) return false;

	float fOneX = (x2-x1)/distance;
	float fOneY = (y2-y1)/distance;

	//ע������������ʵ����ϵ����
	fStartX=kChairCannon[wChairID][0]+kCannonCirlLen*fOneX;
	fStartY=kChairCannon[wChairID][1]+kCannonCirlLen*fOneY;

	//_tprintf("�ӵ���ʼ�����꣺%d��%d,  ƫ�ƣ�%d��%d  ��꣺%d,%d\n",kChairCannon[wChairID][0],kChairCannon[wChairID][1],int(fStartX),int(fStartY),int(MouseX),int(MouseY));

	////�Ƕ���ʱ����
	//float fsin_value=(x2-x1)/distance;
	//float fRotate = acosf(fsin_value);
	//if(y2<y1) fRotate = 2*M_PI - fRotate;

	return true;
}
//���������Ͳ��ҿ��
int TableFrameSink::GetStockByFishKind(bool &bUseOld, FishKind fishkind, int lStockGold[20], double fMultiPro[20])
{
	int nStockCount=0;
	bool bSpecialFish=(fishkind==FISH_KIND_14 || fishkind==FISH_KIND_15 || fishkind==FISH_KIND_16 || fishkind==FISH_CHAIN || fishkind==FISH_CATCH_SMAE);
	if(rand()%100<g_nUseKuCunRand && !bSpecialFish)
	{
		bUseOld=true;
		nStockCount=g_game_config.stock_crucial_count_old;
		for (int i=0;i<nStockCount;i++)
		{
			fMultiPro[i] = g_game_config.stock_multi_probability_old[i];
			lStockGold[i] = g_game_config.stock_init_gold_old+g_game_config.stock_crucial_score_old[i];
		}

		return nStockCount;
	}
	bUseOld=false;

	//С��̶�����
	if(fishkind<13)
	{
		nStockCount = (g_game_config.stock_crucial_count_>20 ? 0:g_game_config.stock_crucial_count_);
		for (int i=0;i<g_game_config.stock_crucial_count_;i++)
		{
			fMultiPro[i] = g_game_config.stock_multi_probability_[i];
			lStockGold[i] = g_game_config.stock_init_gold+g_game_config.stock_crucial_score_[i]*g_game_config.stock_stock_min_range*g_game_config.fish_multiple_[fishkind];
		}

		return nStockCount;
	}
	else
	{
		nStockCount = (g_game_config.bomb_stock_count_>20 ? 0:g_game_config.bomb_stock_count_);
		for (int i=0;i<nStockCount;i++)
		{
			fMultiPro[i] = g_game_config.bomb_stock_multi_pro_[i];
			lStockGold[i] = g_game_config.bomb_stock_init_gold+g_game_config.bomb_stock_score_[i];
		}

		return nStockCount;
	}

	return 0;
}

//���ͳһ����
void TableFrameSink::StockScore(int nFishKind, int nBullet_mulriple, LONGLONG lFishSocre, bool bAdd)
{
	if(bAdd>0) //������
	{
		int revenue = g_lServerRate * nBullet_mulriple / 1000;
		g_game_config.g_revenue_score += revenue;

		//�Ͽ��ͬ��
		g_game_config.g_stock_score_old += (nBullet_mulriple - revenue);

		g_game_config.g_stock_Kind[nFishKind] += (nBullet_mulriple - revenue);
	}
	else //���֧��
	{
		//�Ͽ��ͬ��
		g_game_config.g_stock_score_old -= lFishSocre;

		g_game_config.g_stock_Kind[nFishKind] -= lFishSocre;
	}
}

void TableFrameSink::BuildXianShiFree(WORD wChairID, BulletKind bulletKind, int nBullet_mulriple)
{
	if(wChairID>=GAME_PLAYER) return;

	if(m_TimeFree[wChairID].dwFreeTime==0 && m_dwToolFishTm[wChairID]==0)
	{
		BYTE cbIndex=g_random_int_range(3,10);
		BYTE cbStockID = g_random_int_range(13,FISH_KIND_COUNT);
		LONGLONG lAllStock=g_game_config.g_stock_Kind[cbStockID];
		LONGLONG lMaxStock=g_game_config.bomb_stock_score_[cbIndex];
		if(g_game_config.bomb_stock_count_>=10 && g_game_config.stock_crucial_count_>=10 && lAllStock>lMaxStock)
		{
			if((rand()%1000<5) && (bulletKind==Bullet_Type_putongpao ||bulletKind==Bullet_Type_zidanpao||bulletKind==Bullet_Type_sanguanpao))//bulletKind==Bullet_Type_shandianpao
			{
				LONGLONG lTempR=lAllStock-lMaxStock;

				LONGLONG lScore = (fish_score_[wChairID] - exchange_fish_score_[wChairID]) * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
				LONGLONG lChangeScore = lScore - m_lWriteScore[wChairID];

				IServerUserItem *pServerUserItem=table_frame_->GetServerUserItem(wChairID);
				if(pServerUserItem==0) return;

				LONGLONG lNowGold = pServerUserItem->GetUserScore()->lScore + lChangeScore;
				LONGLONG lWinLost = lNowGold-user_score[wChairID];
				int nIndexD = (lWinLost/1000000);
				if(nIndexD<-6) nIndexD=-6;
				else if(nIndexD>4) nIndexD=3;
				nIndexD+=6;
				static double dRandX[10] = {0.28, 0.41, 0.65, 0.73, 0.81, 0.98, 1.00, 1.80, 2.05, 2.65};

				static double dAddMul[10] = {10000,10000,10000,10000,  20,  10,  5,  2.3,  1.5,  0.5};
				int nRandN=(rand()%1000)*dRandX[nIndexD]*dAddMul[cbIndex];
				if(nRandN<1+lTempR/1000000)//����������˸��ʸ���
				{
					m_tagFireSpeedCheck[wChairID].nEnergyCnt = 50;
					m_TimeFree[wChairID].wFireCount=0;
					m_TimeFree[wChairID].mulriple=nBullet_mulriple;
					m_TimeFree[wChairID].dwFreeTime=GetTickCount();
					if(m_TimeFree[wChairID].dwFreeTime==0) m_TimeFree[wChairID].dwFreeTime=1;

					CMD_S_Free_Time FreeTime;
					FreeTime.wChairID=wChairID;

					FreeTime.wBulletKind=Bullet_Type_xianshipao;
					FreeTime.nBullet_mul=nBullet_mulriple;
					table_frame_->SendTableData(INVALID_CHAIR, SUB_S_FREE_TIME_INFO, &FreeTime, sizeof(FreeTime));
					table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_FREE_TIME_INFO, &FreeTime, sizeof(FreeTime));
				}
			}
		}
	}
}

bool TableFrameSink::NormalStockHitGroup(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, DWORD dwAllFishMul, SCORE fish_score, float fMultiFishPro)
{
	int fish_id=pSeverFish->fish_id;
	FishKind fishkind = pSeverFish->fish_kind;
	int bullet_id=pSeverBullet->bullet_id;
	int bullet_onlyid=pSeverBullet->bullet_idx_;
	
	fMultiFishPro=1.0f;//ǰ�ڲ�����
	double probability = static_cast<double>((g_random_int_range(0,1000) + 1)) / 1000;
	double fish_probability = 1.0f/(dwAllFishMul)*fMultiFishPro;

	bool bNotStock = true;
	bool bUseOld = true;
	int stock_count = 0;
	int stock_gold[20]={0};
	double stotck_pro[20]={0};
	stock_count=GetStockByFishKind(bUseOld,fishkind,stock_gold,stotck_pro);
	LONGLONG lStockT = g_game_config.g_stock_score_old;
	if(!bUseOld) lStockT=g_game_config.g_stock_Kind[fishkind];
	while ((--stock_count) >= 0)
	{
		if (lStockT >= stock_gold[stock_count])
		{
			bNotStock = false;
			if ( probability > (fish_probability *stotck_pro[stock_count]) )
			{
				if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
				{
					tagCatchFishRecord FishLog;
					//ZeroMemory(&FishLog,sizeof(FishLog));
					FishLog.dwGameID=pServerUserItem->GetGameID();
					FishLog.nBulletID=bullet_id;
					FishLog.nBulletOnlyID=bullet_onlyid;
					FishLog.enFishKind=fishkind;
					FishLog.lFishID=fish_id;
					FishLog.nWinScore=0;
					FishLog.nSoucrePro=probability;
					FishLog.nChangeAddPro=fish_probability;
					FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
					FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
					CDBAction::GetInstance().CatchFish(&FishLog);//����������ʧ�� ����
				}

				return false;
			}
			else
			{
				break;
			}
		}
	}

	//��С��涼����
	if (bNotStock)
	{
		//��ֹ���ô���
		if((rand()%10!=0) || (probability > fish_probability*0.5)) 
		{
			if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
			{
				tagCatchFishRecord FishLog;
				FishLog.dwGameID=pServerUserItem->GetGameID();
				FishLog.nBulletID=bullet_id;
				FishLog.nBulletOnlyID=bullet_onlyid;
				FishLog.enFishKind=fishkind;
				FishLog.lFishID=fish_id;
				FishLog.nWinScore=0;
				FishLog.nSoucrePro=probability;
				FishLog.nChangeAddPro=fish_probability;
				FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
				FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
				CDBAction::GetInstance().CatchFish(&FishLog);
			}

			return false;
		}
	}

	if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
	{
		tagCatchFishRecord FishLog;
		FishLog.dwGameID=pServerUserItem->GetGameID();
		FishLog.nBulletID=bullet_id;
		FishLog.nBulletOnlyID=bullet_onlyid;
		FishLog.enFishKind=fishkind;
		FishLog.lFishID=fish_id;
		if(FISH_CHAIN==fishkind || FISH_CATCH_SMAE==fishkind)
		{
			FishLog.lFishID=-pSeverFish->fish_tag;
		}
		FishLog.nWinScore=fish_score;
		FishLog.nSoucrePro=probability;
		FishLog.nChangeAddPro=fish_probability;
		FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
		FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
		CDBAction::GetInstance().CatchFish(&FishLog);//����������
	}

	return true;
}

bool TableFrameSink::NormalStockHitSingle(FishTraceInfo *pSeverFish,ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, SCORE fish_score, float fMultiFishPro)
{
	fMultiFishPro=1.0f;//ǰ�ڲ�����
	//��������
	if(pSeverFish->fish_kind>=FISH_KIND_14 && pSeverFish->fish_kind<=FISH_KIND_16) return false;
	if(FISH_CHAIN==pSeverFish->fish_kind || FISH_CATCH_SMAE==pSeverFish->fish_kind) return false;

	WORD chair_id=pServerUserItem->GetChairID();
	double probability = static_cast<double>((g_random_int_range(0,1000) + 1)) / 1000;
	double fish_probability = g_game_config.fish_capture_probability_[pSeverFish->fish_kind]*fMultiFishPro;
	if((FISH_YUQUN1 <= pSeverFish->fish_kind) && (FISH_YUZHEN>= pSeverFish->fish_kind))
	{
		fish_probability = g_game_config.fish_capture_probability_[pSeverFish->fish_tag]*fMultiFishPro;
	}

	////ͳһ��ƽ�����������
	////��������
	//WORD wFishKind=pSeverFish->fish_kind;
	//if(wFishKind>=FISH_YUQUN1 && wFishKind<=FISH_YUZHEN) wFishKind=pSeverFish->fish_tag;
	//if(pSeverFish->wFireCount[chair_id]<g_game_config.fish_fire_min_cnt[wFishKind])
	//{
	//	static double dRandX2[7] = {0.88, 0.91, 0.95, 0.93, 0.91, 0.98, 0.85};
	//	fish_probability*=dRandX2[rand()%7];
	//}
	////����������ע�� ��ʼ��С��ֵ�ſ����� ����С���
	//int nMaxHit=g_game_config.fish_fire_max_cnt[wFishKind];
	//if(pSeverFish->wFireCount[chair_id]>nMaxHit && nMaxHit>1)
	//{
	//	if(pSeverBullet->bullet_mulriple == g_game_config.min_bullet_multiple_)
	//	{
	//		static double dRandX3[7] = {1.1, 1.15, 1.25, 1.02, 1.01, 1.03, 1.04};
	//		fish_probability*=dRandX3[rand()%7];
	//	}
	//}

	////�¼Ӷ�1-8������Ѷȴ���
	//if (!pServerUserItem->IsAndroidUser())
	//{
	//	//ȫ�����ܹ�������ɢ �������
	//	static int nFishD = 0;			//���й�,10�ξ����߼�һ��
	//	static bool bRunFish = true;	//�����������Ƿ���
	//	static double dRand[7] = {0.8, 0.8, 0.9, 0.9, 1.0, 1.0, 1.0};

	//	if (bRunFish == false)
	//	{
	//		if (wFishKind >= FISH_KIND_2 && wFishKind <= FISH_KIND_9)
	//		{
	//			nFishD++;

	//			if (nFishD >= 10)
	//			{
	//				nFishD = 0;
	//				bRunFish = true;//10�κ��������߼��������߸������
	//			}
	//			else
	//			{
	//				fish_probability *= dRand[rand()%7];
	//			}
	//		}
	//	}

	//	if (bRunFish)
	//	{
	//		if (wFishKind >= FISH_KIND_2 && wFishKind <= FISH_KIND_9)
	//		{
	//			bRunFish = false;
	//		}
	//	}
	//}

	//ע��������־��¼
	SCORE lXiuZhen=pSeverBullet->bullet_mulriple;
	if((Bullet_Type_putongpao==pSeverBullet->bullet_kind) || (Bullet_Type_shandianpao==pSeverBullet->bullet_kind) || (Bullet_Type_zidanpao==pSeverBullet->bullet_kind)|| (Bullet_Type_sanguanpao==pSeverBullet->bullet_kind)) lXiuZhen=0;
	//////////////////////////////////////////////////////
	bool bNotStock = true;
	bool bUseOld = true;
	int stock_count = 0;
	int stock_gold[20]={0};
	double stotck_pro[20]={0};
	stock_count=GetStockByFishKind(bUseOld,pSeverFish->fish_kind,stock_gold,stotck_pro);
	LONGLONG lStockT = g_game_config.g_stock_score_old;
	if(!bUseOld) lStockT=g_game_config.g_stock_Kind[pSeverFish->fish_kind];
	while ((--stock_count) >= 0)
	{
		if (lStockT >= stock_gold[stock_count])
		{
			bNotStock = false;
			if ( probability > (fish_probability *stotck_pro[stock_count]) )
			{
				if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
				{
					tagCatchFishRecord FishLog;
					FishLog.dwGameID=pServerUserItem->GetGameID();
					FishLog.nBulletID=pSeverBullet->bullet_id;
					FishLog.nBulletOnlyID=pSeverBullet->bullet_idx_;
					FishLog.enFishKind=pSeverFish->fish_kind;
					FishLog.lFishID=pSeverFish->fish_id;
					FishLog.nWinScore=lXiuZhen;
					FishLog.nSoucrePro=probability;
					FishLog.nChangeAddPro=fish_probability;
					FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
					FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
					CDBAction::GetInstance().CatchFish(&FishLog);//����������ʧ�� ����
				}

				return false;
			}
			else
			{
				break;
			}
		}
	}

	//��С��涼����
	if (bNotStock)
	{
		//��ֹ���ô���
		if((rand()%10!=0) || (probability > fish_probability*0.5)) 
		{
			if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
			{
				tagCatchFishRecord FishLog;
				FishLog.dwGameID=pServerUserItem->GetGameID();
				FishLog.nBulletID=pSeverBullet->bullet_id;
				FishLog.nBulletOnlyID=pSeverBullet->bullet_idx_;
				FishLog.enFishKind=pSeverFish->fish_kind;
				FishLog.lFishID=pSeverFish->fish_id;
				FishLog.nWinScore=lXiuZhen;
				FishLog.nSoucrePro=probability;
				FishLog.nChangeAddPro=fish_probability;
				FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
				FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
				CDBAction::GetInstance().CatchFish(&FishLog);
			}

			return false;
		}
	}

	if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
	{
		tagCatchFishRecord FishLog;
		FishLog.dwGameID=pServerUserItem->GetGameID();
		FishLog.nBulletID=pSeverBullet->bullet_id;
		FishLog.nBulletOnlyID=pSeverBullet->bullet_idx_;
		FishLog.enFishKind=pSeverFish->fish_kind;
		FishLog.lFishID=pSeverFish->fish_id;
		FishLog.nWinScore=fish_score+lXiuZhen;
		FishLog.nSoucrePro=probability;
		FishLog.nChangeAddPro=fish_probability;
		FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
		FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
		CDBAction::GetInstance().CatchFish(&FishLog);//����������
	}

	return true;
}

BYTE TableFrameSink::BombStockHitTool(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, float fMultiFishPro)
{
	//�˴������ǡ�fish_tag��
	if (pSeverFish->fish_kind < FISH_KIND_14 || pSeverFish->fish_kind > FISH_KIND_16) return 0;

	WORD chair_id=pServerUserItem->GetChairID();
	double probability = static_cast<double>((g_random_int_range(0,1000) + 1)) / 1000;
	
	//����ͬ�����ʺʹ��� e.g ���ͳ�60���ӵ� ��1/60 * ����Э������
	BYTE cbGiveNum=0,cbFishCountPro=0;
	switch(pSeverFish->fish_kind)
	{
	case FISH_KIND_14:	//��ͷ������6��
		{
			cbGiveNum = g_random_int_range(15,30)+g_random_int_range(0,50);
			cbFishCountPro = g_random_int_range(15,30)+g_random_int_range(0,50);
		}
		break;
	case FISH_KIND_15:	//����ը��з3��
		{
			cbGiveNum = g_random_int_range(10,20)+g_random_int_range(0,40);
			cbFishCountPro = g_random_int_range(15,30)+g_random_int_range(0,50);
		}
		break;
	default:			//���з1��
		{
			cbGiveNum = g_random_int_range(15,30)+g_random_int_range(0,20);
			cbFishCountPro = g_random_int_range(15,30)+g_random_int_range(0,50);
		}
		break;
	}
	//���ⲻ�ɿ�����̫Ƶ��
	if(cbGiveNum>cbFishCountPro+10 && rand()%100>40)
	{
		cbGiveNum = cbFishCountPro-rand()%10;
	}
	double fish_probability = 1.0f/(cbFishCountPro)*fMultiFishPro;

	////ͳһ��ƽ�����������
	////��������
	//if(pSeverFish->wFireCount[chair_id]<g_game_config.fish_fire_min_cnt[pSeverFish->fish_kind])
	//{
	//	static double dRandX2[7] = {0.85, 0.86, 0.87, 0.88, 0.89, 0.92, 0.98};
	//	fish_probability*=dRandX2[rand()%7];
	//}

	bool bNotStock = true;
	bool bUseOld = true;
	int stock_count = 0;
	int stock_gold[20]={0};
	double stotck_pro[20]={0};
	stock_count=GetStockByFishKind(bUseOld,pSeverFish->fish_kind,stock_gold,stotck_pro);
	LONGLONG lStockT = g_game_config.g_stock_score_old;
	if(!bUseOld) lStockT=g_game_config.g_stock_Kind[pSeverFish->fish_kind];
	while ((--stock_count) >= 0)
	{
		if (lStockT >= stock_gold[stock_count])
		{
			bNotStock = false;
			if ( probability > (fish_probability *stotck_pro[stock_count]) )
			{
				if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
				{
					tagCatchFishRecord FishLog;
					FishLog.dwGameID=pServerUserItem->GetGameID();
					FishLog.nBulletID=pSeverBullet->bullet_id;
					FishLog.nBulletOnlyID=pSeverBullet->bullet_idx_;
					FishLog.enFishKind=pSeverFish->fish_kind;
					FishLog.lFishID=pSeverFish->fish_id;
					FishLog.nWinScore=0;
					FishLog.nSoucrePro=probability;
					FishLog.nChangeAddPro=fish_probability;
					FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
					FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
					CDBAction::GetInstance().CatchFish(&FishLog);//����������ʧ�� ����
				}

				return 0;
			}
			else
			{
				break;
			}
		}
	}

	//��С��涼����
	if (bNotStock)
	{
		//��ֹ���ô���
		if((rand()%10!=0) || (probability > fish_probability*0.5)) 
		{
			if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
			{
				tagCatchFishRecord FishLog;
				FishLog.dwGameID=pServerUserItem->GetGameID();
				FishLog.nBulletID=pSeverBullet->bullet_id;
				FishLog.nBulletOnlyID=pSeverBullet->bullet_idx_;
				FishLog.enFishKind=pSeverFish->fish_kind;
				FishLog.lFishID=pSeverFish->fish_id;
				FishLog.nWinScore=0;
				FishLog.nSoucrePro=probability;
				FishLog.nChangeAddPro=fish_probability;
				FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
				FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
				CDBAction::GetInstance().CatchFish(&FishLog);
			}

			return 0;
		}
	}

	if( GetNeedRecordDB(pServerUserItem->GetGameID()) )
	{
		tagCatchFishRecord FishLog;
		FishLog.dwGameID=pServerUserItem->GetGameID();
		FishLog.nBulletID=pSeverBullet->bullet_id;
		FishLog.nBulletOnlyID=pSeverBullet->bullet_idx_;
		FishLog.enFishKind=pSeverFish->fish_kind;
		FishLog.lFishID=pSeverFish->fish_id;
		FishLog.nWinScore=0;
		FishLog.nSoucrePro=probability;
		FishLog.nChangeAddPro=fish_probability;
		FishLog.lScoreCurrent=GetMeSore(pServerUserItem);
		FishLog.lNowKuCun = (bUseOld==false)?lStockT:-lStockT;
		CDBAction::GetInstance().CatchFish(&FishLog);//����������
	}

	return cbGiveNum;
}

bool TableFrameSink::BombStockHitSingle(FishTraceInfo *pSeverFish, ServerBulletInfo *pSeverBullet, IServerUserItem *pServerUserItem, float fMultiFishPro)
{
	//�˴������ǡ�fish_tag��
	//��������
	if((pSeverFish->fish_kind >= FISH_KIND_14 && pSeverFish->fish_kind <= FISH_KIND_16) || (pSeverFish->fish_kind>=FISH_CHAIN)) return false;

	//��������⴦��
	if(pSeverBullet->bullet_kind==Bullet_Type_diancipao) 
	{
		SCORE lChangeStock=g_game_config.g_stock_Kind[FISH_KIND_16]-g_game_config.bomb_stock_init_gold;
		float fChangePro=1.0 + (lChangeStock/5000000)*0.25;
		if(fChangePro>3.0) fChangePro=3.0;
		if(fChangePro<0.5) fChangePro=0.5;
		fMultiFishPro*=fChangePro;
	}

	WORD chair_id=pServerUserItem->GetChairID();
	double probability = static_cast<double>((g_random_int_range(0,1000) + 1)) / 1000;
	double fish_probability = g_game_config.fish_capture_probability_[pSeverFish->fish_kind]*fMultiFishPro;
	if((FISH_YUQUN1 <= pSeverFish->fish_kind) && (FISH_YUZHEN>= pSeverFish->fish_kind))
	{
		fish_probability = g_game_config.fish_capture_probability_[pSeverFish->fish_tag]*fMultiFishPro;
	}

	////ͳһ��ƽ�����������
	////��������
	//if(pSeverFish->wFireCount[chair_id]<g_game_config.fish_fire_min_cnt[pSeverFish->fish_kind])
	//{
	//	static double dRandX2[7] = {0.88, 0.91, 0.95, 0.93, 0.91, 0.98, 0.85};
	//	fish_probability*=dRandX2[rand()%7];
	//}

	////�¼Ӷ�1-8������Ѷȴ���
	//if (!pServerUserItem->IsAndroidUser())
	//{
	//	//ȫ�����ܹ�������ɢ �������
	//	static int nFishD = 0;			//���й�,10�ξ����߼�һ��
	//	static bool bRunFish = true;	//�����������Ƿ���
	//	static double dRand[7] = {0.8, 0.8, 0.9, 0.9, 1.0, 1.0, 1.0};

	//	if (bRunFish == false)
	//	{
	//		if (pSeverFish->fish_kind >= FISH_KIND_2 && pSeverFish->fish_kind <= FISH_KIND_9)
	//		{
	//			nFishD++;

	//			if (nFishD >= 10)
	//			{
	//				nFishD = 0;
	//				bRunFish = true;//10�κ��������߼��������߸������
	//			}
	//			else
	//			{
	//				fish_probability *= dRand[rand()%7];
	//			}
	//		}
	//	}

	//	if (bRunFish)
	//	{
	//		if (pSeverFish->fish_kind >= FISH_KIND_2 && pSeverFish->fish_kind <= FISH_KIND_9)
	//		{
	//			bRunFish = false;
	//		}
	//	}
	//}

	//ʹ������Ŀ��
	FishKind KunCunKind=FISH_KIND_14;//Ĭ����ͷ�ڿ��
	if(Bullet_Type_lianhuanzhadan == pSeverBullet->bullet_kind) KunCunKind=FISH_KIND_15;
	else if(Bullet_Type_diancipao == pSeverBullet->bullet_kind) KunCunKind=FISH_KIND_16;
	//////////////////////////////////////////////////////
	bool bNotStock = true;
	bool bUseOld = true;
	int stock_count = 0;
	int stock_gold[20]={0};
	double stotck_pro[20]={0};
	stock_count=GetStockByFishKind(bUseOld,KunCunKind,stock_gold,stotck_pro);
	LONGLONG lStockT = g_game_config.g_stock_score_old;
	if(!bUseOld) lStockT=g_game_config.g_stock_Kind[KunCunKind];
	while ((--stock_count) >= 0)
	{
		if (lStockT >= stock_gold[stock_count])
		{
			bNotStock = false;
			if ( probability > (fish_probability *stotck_pro[stock_count]) )
			{
				return false;
			}
			else
			{
				break;
			}
		}
	}

	//��С��涼����
	if (bNotStock)
	{
		//��ֹ���ô���
		if((rand()%10!=0) || (probability > fish_probability*0.5)) 
		{
			return false;
		}
	}

	return true;
}
//��ʼ������Ϸ��ʱ��
void TableFrameSink::StartAllGameTimer()
{
	table_frame_->SetGameTimer(IDI_UpdateLoop,kUpdateTime,-1,0);
}

//�ر�������Ϸ��ʱ��
void TableFrameSink::KillAllGameTimer()
{
	table_frame_->KillGameTimer(IDI_UpdateLoop);
}

//������Ϸ������Ϣ
bool TableFrameSink::SendGameConfig(IServerUserItem* server_user_item)
{
	CMD_S_GameConfig game_config;
	game_config.nExchange_ratio_userscore = g_game_config.exchange_ratio_userscore_;
	game_config.nExchange_ratio_fishscore = g_game_config.exchange_ratio_fishscore_;
	game_config.nExchange_count = g_game_config.exchange_count_;
	game_config.nMin_bullet_multiple = g_game_config.min_bullet_multiple_;
	game_config.nMax_bullet_multiple = g_game_config.max_bullet_multiple_;
	for (int i = 0; i < FISH_KIND_COUNT; ++i)
	{
		game_config.nFish_multiple[i] = g_game_config.fish_multiple_[i];
	}

	for (int i = 0; i < BULLET_KIND_COUNT; ++i)
	{
		game_config.nBullet_speed[i] = g_game_config.bullet_speed_[i];
	}

	return table_frame_->SendUserData(server_user_item, SUB_S_GAME_CONFIG, &game_config, sizeof(game_config));
}

LONGLONG TableFrameSink::GetUserGold(WORD wChairID, BYTE cbMethod)
{
	if(wChairID>=GAME_PLAYER) return 0;

	LONGLONG lRetGold=user_score[wChairID] - exchange_fish_score_[wChairID] * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;

	return lRetGold;
}

//��Ҫ�������
void TableFrameSink::DistributeFish(float delta_time)
{
	//����׶β��ܳ���
	if(m_cbNowYuZhenID!=0xff) return;

	//////////////////////////////////////////////////////////////////////////
	//--��ͳ��tagNewFishInfo��Ϣ �����ط�����Ҫ�ٶ���ά��cbFishCount �����ڻص�ʱ����
	int nAllCount=0;
	for (int i=0;i<FISH_KIND_COUNT;i++)
	{
		m_distribulte_fish[i].ResetPart();
	}
	for (int i=0;i<active_fish_trace_vector_.size();i++)
	{
		WORD wFishKind=active_fish_trace_vector_[i]->fish_kind;
		if(wFishKind>=FISH_KIND_COUNT || wFishKind==FISH_YUZHEN || wFishKind==FISH_YUQUN3 || wFishKind==FISH_YUQUN4 || wFishKind==FISH_YUQUN5) continue;

		nAllCount++;
		m_distribulte_fish[wFishKind].cbFishCount++;
	}
	for (int i=0;i<FISH_KIND_COUNT;i++)
	{
		if(m_distribulte_fish[i].cbCanNewTag==0) continue;

		if(i<FISH_KIND_14 || i>FISH_KIND_16) m_distribulte_fish[i].distribute_elapsed_ += delta_time;
		else
		{
			if(i==FISH_KIND_14+m_cbNowToolFish) m_distribulte_fish[i].distribute_elapsed_ += delta_time;
		}
	}
	//_tprintf("��������%d\n",nAllCount);

	//���ֵ��������������
	int nToolCnt=m_distribulte_fish[FISH_KIND_14].cbFishCount+m_distribulte_fish[FISH_KIND_15].cbFishCount+m_distribulte_fish[FISH_KIND_16].cbFishCount;
	if(nToolCnt<1)
	{
		for(int i=FISH_KIND_14;i<=FISH_KIND_16;i++)
		{
			if(m_distribulte_fish[i].cbCanNewTag==0) continue;

			//ע������ ���ܶ�� ����3�� ������Ͽͻ��˹̶���
			if(m_distribulte_fish[i].cbFishCount>=g_game_config.fish_new_max_cnt[i]) continue;
			if(m_distribulte_fish[i].distribute_elapsed_<g_game_config.distribute_interval[i]) continue;

			WORD wNewID=m_cbNowToolFish;
			if(FISH_KIND_14+wNewID!=i) continue;

			m_cbNowToolFish=(wNewID+1)%3;
			m_distribulte_fish[FISH_KIND_14].distribute_elapsed_=0;
			m_distribulte_fish[FISH_KIND_15].distribute_elapsed_=0;
			m_distribulte_fish[FISH_KIND_16].distribute_elapsed_=0;
			//_tprintf("�´γ��㣺%d\n",m_cbNowToolFish);

			FishKind fKind=static_cast<FishKind>(i);
			BuildFishTrace(1,fKind,fKind);
			break;
		}
	}
	if(nAllCount>MAX_FISH_SENCE) return;

	//һ�����
	for (int i=FISH_CATCH_SMAE;i>=0;i--)
	{
		//���ֵ����������
		if(FISH_KIND_14<=i && FISH_KIND_16>=i) continue;
		if(m_distribulte_fish[i].cbCanNewTag==0) continue;
		//ע������ ���ܶ�� ����3�� ������Ͽͻ��˹̶���
		if(m_distribulte_fish[i].cbFishCount>=g_game_config.fish_new_max_cnt[i]) continue;

		//������ �������
		bool bOne=false;
		bool bTwo=false;
		WORD wFishKind=i;
		if((wFishKind>=FISH_KIND_17 && wFishKind<=FISH_KIND_19)||(wFishKind>=FISH_KIND_22 && wFishKind<=FISH_KIND_24)||(wFishKind==FISH_CHAIN ||wFishKind==FISH_CATCH_SMAE))
		{
			bOne=(m_distribulte_fish[i].distribute_elapsed_>g_game_config.distribute_interval[i]);
		}
		else
		{
			//�������õ�ʱ������ ����������������Ŀ �����������Ҫ ��ֹ��������
			bTwo=((g_random_int()%20<8) &&(m_distribulte_fish[i].distribute_elapsed_>g_game_config.distribute_interval[i]));
		}

		if(bOne || bTwo)
		{
			//����ʱ��
			m_distribulte_fish[i].distribute_elapsed_=0;

			if(FISH_YUQUN1==i || FISH_YUQUN2==i || FISH_YUQUN3==i || FISH_YUQUN4==i)
			{
				BuildFishYuQun(i-FISH_YUQUN1);
			}
			else if(FISH_YUQUN5==i)
			{
				BuildFishYuQun(4);
			}
			else
			{
				//�������
				FishKind fKind=static_cast<FishKind>(i);
				int nOnceCnt=1;
				if(fKind>=FISH_KIND_1 && fKind<=FISH_KIND_9)
				{
					nOnceCnt=g_random_int_range(g_game_config.fish_new_min_cnt[i],g_game_config.fish_new_max_cnt[i]);
					if(nOnceCnt>2) nOnceCnt=rand()%2+1;
				}
				if(fKind==FISH_KIND_22) nOnceCnt=2;
				BuildFishTrace(nOnceCnt,fKind,fKind);
			}

			break;
		}
	}
}

//����ʱ����Ϣ
bool TableFrameSink::OnTimerSwitchScene()
{
	m_GameState=en_S_BuildSence;
	m_dwChangeSenceTm=GetTickCount();
	m_dwBuildSenceTm=GetTickCount();
	DWORD dwSenceID=((now_acitve_scene_kind_ + 1) % SCENE_KIND_COUNT);
	now_acitve_scene_kind_ = SceneKind(dwSenceID);

	if (now_acitve_scene_kind_ <= SCENE_KIND_3	)
	{
		m_Scene0Index ++;
		m_Scene0Index%=2;
	}
	
	//��������
	CMD_S_SwitchScene Switch_Scene;
	Switch_Scene.scene_kind = SceneKind(now_acitve_scene_kind_ + (m_Scene0Index<<16));
	Switch_Scene.nSceneStartTime=GetElapsedTm(m_dwChangeSenceTm);//�����Ѿ���ʼ��ʱ��
	
	//��������ʱ��������û�Ϊ��
	m_wDaoJuPaoUser=INVALID_CHAIR;

	table_frame_->SendTableData(INVALID_CHAIR, SUB_S_SWITCH_SCENE, &Switch_Scene, sizeof(CMD_S_SwitchScene));
	table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_SWITCH_SCENE, &Switch_Scene, sizeof(CMD_S_SwitchScene));

	//_tprintf("�����л�����=%d��tableid=%d\n",now_acitve_scene_kind_,table_frame_->GetTableID());//�������ȥ��
	return true;
}

//������Ϣ
bool TableFrameSink::SendTableData(WORD sub_cmdid, void* data, WORD data_size, IServerUserItem* only_user_item)
{
	if (only_user_item == NULL)
	{
		table_frame_->SendTableData(INVALID_CHAIR, sub_cmdid, data, data_size);
	}
	else
	{
		IServerUserItem* send_user_item = NULL;
		for (WORD i = 0; i < GAME_PLAYER; ++i)
		{
			send_user_item = table_frame_->GetServerUserItem(i);
			if (send_user_item == NULL) continue;
			if (send_user_item != only_user_item) continue;
			table_frame_->SendTableData(send_user_item->GetChairID(), sub_cmdid, data, data_size);
		}
	}
	table_frame_->SendLookonData(INVALID_CHAIR, sub_cmdid, data, data_size);
	return true;
}

//�һ����
bool TableFrameSink::OnSubExchangeFishScore(IServerUserItem* server_user_item, bool increase)
{
	WORD chair_id = server_user_item->GetChairID();

	CMD_S_ExchangeFishScore exchange_fish_score;
	exchange_fish_score.wChairID = chair_id;
	int exchangecount = g_game_config.exchange_count_;

	SCORE need_user_score = g_game_config.exchange_ratio_userscore_ * g_game_config.exchange_count_ / g_game_config.exchange_ratio_fishscore_;
	SCORE user_leave_score = user_score[chair_id] - exchange_fish_score_[chair_id] * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	if (increase)
	{
		if (need_user_score > user_leave_score)
		{
			exchangecount = user_leave_score * g_game_config.exchange_ratio_fishscore_ / g_game_config.exchange_ratio_userscore_;
		}
		fish_score_[chair_id] += exchangecount;
		exchange_fish_score_[chair_id] += exchangecount;
	}
	else
	{
		if (fish_score_[chair_id] <= 0) return true;
		
		exchange_fish_score_[chair_id] -= fish_score_[chair_id];
		fish_score_[chair_id] = 0;
	}

	exchange_fish_score.lNow_fish_score=fish_score_[chair_id];
	exchange_fish_score.lNow_user_score = GetUserGold(chair_id);

	//SendTableData(SUB_S_EXCHANGE_FISHSCORE, &exchange_fish_score, sizeof(exchange_fish_score), server_user_item->IsAndroidUser() ? NULL : server_user_item);
	SendTableData(SUB_S_EXCHANGE_FISHSCORE, &exchange_fish_score, sizeof(exchange_fish_score), NULL);
	//WriteScore(chair_id);

	TCHAR szTestRead[256]={0};
	_sntprintf(szTestRead,sizeof(szTestRead),TEXT("%s\\hw2Fish.ini"),m_szMoudleFilePath);
	int nRead=GetPrivateProfileInt(TEXT("TEST_NEED"),TEXT("OPEN"),0,szTestRead);
	if((increase==false) && (nRead==1))
	{
		g_cbTest=1;
		g_cbTestStyle=255;
		g_vecAllFish.clear();
		m_GameState=en_S_NewAcitveFish;

		OnTimerSwitchScene();
		g_cbTestStyle=GetPrivateProfileInt(TEXT("TEST_NEED"),TEXT("Style"),255,szTestRead);

		TCHAR szBuffer[1024]={0};
		GetPrivateProfileString(TEXT("TEST_NEED"),TEXT("Fish"),TEXT(""),szBuffer,1024,szTestRead);

		if(strlen(szBuffer)>1)
		{
			char *one =NULL; 
			one=strtok(szBuffer,",");
			while(one!= NULL)
			{
				int nTemp=atoi(one);
				g_vecAllFish.push_back(nTemp);

				one = strtok(NULL,",");
			}
		}
	}
	
	return true;
}

bool TableFrameSink::OnSubSanGuanFire(IServerUserItem* server_user_item, CMD_C_SanGuanFire *pMsg)
{
	int bullet_mul=pMsg->nBullet_mulriple;
	BulletKind bullet_kind=pMsg->bullet_kind;
	int nStartBulletID=pMsg->nStartBulletID;
	WORD chair_id = server_user_item->GetChairID();

	//����У��
	if ((Bullet_Type_sanguanpao!=bullet_kind)||nStartBulletID<0) return true;
	if (bullet_mul < g_game_config.min_bullet_multiple_ || bullet_mul > g_game_config.max_bullet_multiple_) return true;

	//����ӵ������Ƿ��������ֵ
	if (bullet_mul > g_game_config.min_bullet_multiple_ && bullet_mul < g_game_config.max_bullet_multiple_)
	{
		if(bullet_mul>=10 && bullet_mul<100)
		{
			if(bullet_mul%10>0) return true;
		}
		if(bullet_mul>=100 && bullet_mul<1000)
		{
			if(bullet_mul%100>0) return true;
		}
		if(bullet_mul>=1000 && bullet_mul<g_game_config.max_bullet_multiple_)
		{
			if((bullet_mul%1000 != 0)&&(bullet_mul%1000 != 900)) return true;
		}
	}
	g_nFireBulletTm[chair_id]=GetTickCount();

	for (int i=0;i<3;i++)
	{
		ServerBulletInfo *pBulletTT = GetBulletInfo(chair_id,nStartBulletID+i);
		if (pBulletTT!=0) return true;
	}

	//��֤�����ӵ��ķ���
	int bullet_score = bullet_mul*3;
	if (fish_score_[chair_id] < bullet_score) return true;

	//�������㻹ԭ��ʵ����
	int fResultMouseX[3];
	int fResultMouseY[3];
	float fStartX[3]={0};
	float fStartY[3]={0};
	for(int k=0;k<3;k++)
	{
		fResultMouseX[k]=pMsg->fMouseX[k];
		fResultMouseY[k]=pMsg->fMouseY[k];
		WORD wClientKey=g_wKeyClient[chair_id];
		int nUserIdKey=(server_user_item->GetUserID()*server_user_item->GetUserID()+server_user_item->GetUserID()/11)%1000000;
		fResultMouseX[k] += (int)( wClientKey + wClientKey * wClientKey + nUserIdKey + (nUserIdKey % wClientKey) * wClientKey);
		fResultMouseY[k] += (int)( wClientKey + wClientKey * wClientKey + nUserIdKey + (wClientKey / 100) * wClientKey);

		bool bValid=CalcAngle(chair_id,float(fResultMouseX[k])/1000000,float(fResultMouseY[k])/1000000,fStartX[k],fStartY[k]);
		if(!bValid || (fStartX==0 && fStartX==0)) return true;//���Ϸ��ٽ�ֵ���
	}

	BYTE cbAcitvieOk = INVALID_BYTE;
	ServerBulletInfo* bullet_info[3]={NULL,NULL,NULL};
	for (int i=0;i<3;i++)
	{
		bullet_info[i] = ActiveBulletInfo(chair_id,nStartBulletID+i);
		if(bullet_info[i]==0)
		{
			g_lMemWrongScore+=bullet_mul;
			cbAcitvieOk=i;
			break;
		}
	}
	if(cbAcitvieOk!=INVALID_BYTE)               //�������������һ���ӵ�����ʧ����ɾ���Ѿ������Ĳ�����
	{
		for (int i=0;i<cbAcitvieOk;i++)
		{
			FreeBulletInfo(chair_id,bullet_info[i]);
		}
		return true;
	}

	for (int t=0;t<3;t++)
	{
		bullet_info[t]->bCheat=false;
		bullet_info[t]->cbMaxUse=0;
		bool bCheat = m_tagFireSpeedCheck[chair_id].fire(false);
		bullet_info[t]->bCheat=bCheat;
		bullet_info[t]->cbBoomKind = INVALID_BYTE;
		bullet_info[t]->bullet_kind=Bullet_Type_sanguanpao;
		bullet_info[t]->bullet_mulriple=bullet_mul;
		bullet_info[t]->nLockFishID=0;

		BulletMoveManage.ActiveBullet(bullet_info[t]->bullet_id,chair_id,
			float(g_game_config.bullet_speed_[bullet_kind])/1000,fStartX[t],fStartY[t],float(fResultMouseX[t])/1000000,float(fResultMouseY[t])/1000000);
	}

	fish_score_[chair_id] -= bullet_score;
	g_lUserAllWin-=bullet_score;
	g_lFireScore+=bullet_score;
	m_lFireNoInKuCunGold[chair_id]+=bullet_score;
	m_lFireScore[chair_id] +=bullet_score;

	//���ſ����ڱ��뿼��������ұ����ʧ������
	if(server_bullet_info_vector_[chair_id].size()>=MAX_CONTAIN_BULLET_NUM/GAME_PLAYER)
	{
		int nOldCount[5]={0};
		for (int j=0;j<server_bullet_info_vector_[chair_id].size();j++)
		{
			if(server_bullet_info_vector_[chair_id][j]->cbMaxUse>0)
			{
				BulletKind TmpKind=server_bullet_info_vector_[chair_id][j]->bullet_kind;

				if(Bullet_Type_diancipao==TmpKind) nOldCount[1]++;
				else if(Bullet_Type_zuantoupao==TmpKind) nOldCount[2]++;
				else if(Bullet_Type_lianhuanzhadan==TmpKind) nOldCount[3]++;
				else nOldCount[0]++;
			}
			else nOldCount[4]++;
		}

		//��ʱ��Ϊ�̷�
		std::vector<ServerBulletInfo*>::iterator iter=server_bullet_info_vector_[chair_id].begin();
		ServerBulletInfo *bullet_tmp=*iter;
		ServerBulletInfo bullet_invalid;
		CopyMemory(&bullet_invalid,bullet_tmp,sizeof(ServerBulletInfo));
		FreeBulletInfo(chair_id,bullet_tmp);

		//ע��������־��¼
		int lXiuZhen=0;
		if((Bullet_Type_putongpao==bullet_invalid.bullet_kind) || (Bullet_Type_shandianpao==bullet_invalid.bullet_kind) || (Bullet_Type_zidanpao==bullet_invalid.bullet_kind) || (Bullet_Type_sanguanpao==bullet_invalid.bullet_kind)) lXiuZhen=bullet_invalid.bullet_mulriple;
		g_lReturnScore+=lXiuZhen;
		m_lFireIntoSystemGold[chair_id]+=lXiuZhen;

		if( GetNeedRecordDB(server_user_item->GetGameID()) )
		{
			if((Bullet_Type_putongpao==bullet_invalid.bullet_kind) || (Bullet_Type_shandianpao==bullet_invalid.bullet_kind) || (Bullet_Type_zidanpao==bullet_invalid.bullet_kind) || (Bullet_Type_sanguanpao==bullet_invalid.bullet_kind)) lXiuZhen=0;
			else lXiuZhen=bullet_invalid.bullet_mulriple;

			tagCatchFishRecord FishLog;
			FishLog.dwGameID=server_user_item->GetGameID();
			FishLog.nBulletID=bullet_invalid.bullet_id;
			FishLog.nBulletOnlyID=bullet_invalid.bullet_idx_;
			FishLog.enFishKind=300;
			FishLog.lFishID=300;
			FishLog.nWinScore=lXiuZhen;
			FishLog.nSoucrePro=-1.0f;
			FishLog.nChangeAddPro=-1.0f;
			FishLog.lScoreCurrent=GetMeSore(server_user_item);
			FishLog.lNowKuCun=-g_game_config.g_stock_score_old;
			CDBAction::GetInstance().CatchFish(&FishLog);//�̷ִ��� ����
		}
	}

	CMD_S_SanGuanFire sanGuan_fire;
	sanGuan_fire.bullet_kind = bullet_kind;
	CopyMemory(sanGuan_fire.fMouseX,fResultMouseX,sizeof(fResultMouseX));
	CopyMemory(sanGuan_fire.fMouseY,fResultMouseY,sizeof(fResultMouseY));

	sanGuan_fire.wChairID = server_user_item->GetChairID();
	sanGuan_fire.nBullet_mulriple = bullet_mul;
	sanGuan_fire.nStartBullet_id=nStartBulletID;

	sanGuan_fire.lNow_fish_score = fish_score_[chair_id];
	sanGuan_fire.lNow_user_score = GetUserGold(chair_id);

	SendTableData(SUB_S_SANGUAN_FIRE, &sanGuan_fire, sizeof(sanGuan_fire), NULL);

	WriteScore(chair_id);

	if( GetNeedRecordDB(server_user_item->GetGameID()) )
	{
		for(int m=0;m<3;m++)
		{
			bullet_info[m]->bullet_kind = sanGuan_fire.bullet_kind;
			bullet_info[m]->bullet_mulriple = sanGuan_fire.nBullet_mulriple;

			//ע��������־��¼
			int lXiuZhen=0;
			if(Bullet_Type_sanguanpao==bullet_info[m]->bullet_kind)
			{
				lXiuZhen=bullet_mul;
			}

			CDBAction::GetInstance().UserFire(server_user_item->GetGameID(), bullet_info[m]->bullet_id, bullet_info[m]->bullet_kind, lXiuZhen, bullet_info[m]->bullet_idx_, bullet_info[m]->cbMaxUse);

		}

	}

	return true;

}

//��ҿ���
bool TableFrameSink::OnSubUserFire(IServerUserItem * server_user_item, CMD_C_UserFire *pMsg)//BulletKind bullet_kind, float angle, int bullet_mul, int lock_fishid, bool bRepeat)
{
	int bullet_mul=pMsg->nBullet_mulriple;
	int lock_fishid=pMsg->nLock_fishid;
	BulletKind bullet_kind=pMsg->bullet_kind;//
	if(pMsg->nBulletID<0) return true;//��Ϊ�ͻ���Ԥ�ȷ������֤
	if(Bullet_Type_lianhuanzhadan==bullet_kind || Bullet_Type_sanguanpao==bullet_kind) return true;

	WORD chair_id = server_user_item->GetChairID();
	g_nFireBulletTm[chair_id]=GetTickCount();

	ServerBulletInfo *pBulletTT = GetBulletInfo(chair_id,pMsg->nBulletID);
	if (pBulletTT!=0)
	{
		//_tprintf("�ͻ��˷����ظ����ӵ�ID��%d\n",pMsg->nBulletID);
		return true;
	}

	//�������㻹ԭ��ʵ����
	int fResultMouseX=pMsg->fMouseX;
	int fResultMouseY=pMsg->fMouseY;
	WORD wClientKey=g_wKeyClient[chair_id];
	int nUserIdKey=(server_user_item->GetUserID()*server_user_item->GetUserID()+server_user_item->GetUserID()/11)%1000000;
	fResultMouseX += (int)( wClientKey + wClientKey * wClientKey + nUserIdKey + (nUserIdKey % wClientKey) * wClientKey);
	fResultMouseY += (int)( wClientKey + wClientKey * wClientKey + nUserIdKey + (wClientKey / 100) * wClientKey);

	float fStartX=0;
	float fStartY=0;
	bool bValid=CalcAngle(chair_id,float(fResultMouseX)/1000000,float(fResultMouseY)/1000000,fStartX,fStartY);
	if(!bValid || (fStartX==0 && fStartX==0)) return true;//���Ϸ��ٽ�ֵ���

	//��֤�����ӵ�
	if (pMsg->cbBoomKind!=INVALID_BYTE || Bullet_Type_yulei== pMsg->bullet_kind)
	{
		if(pMsg->cbBoomKind>=5) return false;
		if(m_pTorpedoInfo[chair_id].dwTorpedoCount[pMsg->cbBoomKind]<=0) return false; 
		if(m_pTorpedoInfo[chair_id].dwTorpedoScore[pMsg->cbBoomKind]<=0) return false; 
		if(m_dwTorpedoTime[chair_id]!=0 && GetElapsedTm(m_dwTorpedoTime[chair_id])<2000)  return false;
		bullet_mul=OnGetTorpedoScore(pMsg->cbBoomKind, m_pTorpedoInfo[chair_id].dwTorpedoScore[pMsg->cbBoomKind], m_pTorpedoInfo[chair_id].dwTorpedoCount[pMsg->cbBoomKind]);
		m_pTorpedoInfo[chair_id].dwTorpedoCount[pMsg->cbBoomKind]--;
		m_pTorpedoInfo[chair_id].dwTorpedoScore[pMsg->cbBoomKind]-=bullet_mul;

		m_dwTorpedoTime[chair_id] = GetTickCount();

		ServerBulletInfo* bullet_info = ActiveBulletInfo(chair_id, pMsg->nBulletID);
		if(bullet_info==0)  return true;

		bullet_info->bullet_kind = bullet_kind;
		bullet_info->bCheat=false;
		bullet_info->cbMaxUse=0;
		bullet_info->cbBoomKind = pMsg->cbBoomKind;
		//������⴦�� ��Ӱ��ͳ�� ����������ֵ�����ڴ���
		bullet_info->bullet_mulriple = 0;
		bullet_info->nLockFishID = bullet_mul;

		BulletMoveManage.ActiveBullet(bullet_info->bullet_id,chair_id,
			float(g_game_config.bullet_speed_[bullet_kind])/1000,fStartX,fStartY,float(fResultMouseX)/1000000,float(fResultMouseY)/1000000);

		CMD_S_UserFire user_fire;
		user_fire.bullet_kind = bullet_kind;
		user_fire.fMouseX = fResultMouseX;
		user_fire.fMouseY = fResultMouseY;
		user_fire.wChairID = server_user_item->GetChairID();
		user_fire.nBullet_mulriple = pMsg->nBullet_mulriple;
		user_fire.nLock_fishid = lock_fishid;
		user_fire.nBullet_id = bullet_info->bullet_id;
		user_fire.lNow_fish_score = fish_score_[chair_id];
		user_fire.lNow_user_score = GetUserGold(chair_id);
		user_fire.cbBoomKind = pMsg->cbBoomKind;
		user_fire.nBullet_index = pMsg->nBullet_index;
		SendTableData(SUB_S_USER_FIRE, &user_fire, sizeof(user_fire), NULL);

		return true;
	}

	//���ſ����ڱ��뿼��������ұ����ʧ������
	if(server_bullet_info_vector_[chair_id].size()>=MAX_CONTAIN_BULLET_NUM/GAME_PLAYER)
	{
		int nOldCount[5]={0};
		for (int j=0;j<server_bullet_info_vector_[chair_id].size();j++)
		{
			if(server_bullet_info_vector_[chair_id][j]->cbMaxUse>0)
			{
				BulletKind TmpKind=server_bullet_info_vector_[chair_id][j]->bullet_kind;

				if(Bullet_Type_diancipao==TmpKind) nOldCount[1]++;
				else if(Bullet_Type_zuantoupao==TmpKind) nOldCount[2]++;
				else if(Bullet_Type_lianhuanzhadan==TmpKind) nOldCount[3]++;
				else nOldCount[0]++;
			}
			else nOldCount[4]++;
		}
		//_tprintf("�ӵ���������%d�� �ӵ���%d,%d,%d,%d,%d\n",server_bullet_info_vector_[chair_id].size(),nOldCount[0],nOldCount[1],nOldCount[2],nOldCount[3],nOldCount[4]);//�������ȥ��
		//return true;

		//��ʱ��Ϊ�̷�
		std::vector<ServerBulletInfo*>::iterator iter=server_bullet_info_vector_[chair_id].begin();
		ServerBulletInfo *bullet_tmp=*iter;
		ServerBulletInfo bullet_invalid;
		CopyMemory(&bullet_invalid,bullet_tmp,sizeof(ServerBulletInfo));
		FreeBulletInfo(chair_id,bullet_tmp);

		//ע��������־��¼
		int lXiuZhen=0;
		if((Bullet_Type_putongpao==bullet_invalid.bullet_kind) || (Bullet_Type_shandianpao==bullet_invalid.bullet_kind) || (Bullet_Type_zidanpao==bullet_invalid.bullet_kind) || (Bullet_Type_sanguanpao==bullet_invalid.bullet_kind)) lXiuZhen=bullet_invalid.bullet_mulriple;
		g_lReturnScore+=lXiuZhen;
		m_lFireIntoSystemGold[chair_id]+=lXiuZhen;

		if( GetNeedRecordDB(server_user_item->GetGameID()) )
		{
			if((Bullet_Type_putongpao==bullet_invalid.bullet_kind) || (Bullet_Type_shandianpao==bullet_invalid.bullet_kind) || (Bullet_Type_zidanpao==bullet_invalid.bullet_kind) || (Bullet_Type_sanguanpao==bullet_invalid.bullet_kind)) lXiuZhen=0;
			else lXiuZhen=bullet_invalid.bullet_mulriple;

			tagCatchFishRecord FishLog;
			FishLog.dwGameID=server_user_item->GetGameID();
			FishLog.nBulletID=bullet_invalid.bullet_id;
			FishLog.nBulletOnlyID=bullet_invalid.bullet_idx_;
			FishLog.enFishKind=300;
			FishLog.lFishID=300;
			FishLog.nWinScore=lXiuZhen;
			FishLog.nSoucrePro=-1.0f;
			FishLog.nChangeAddPro=-1.0f;
			FishLog.lScoreCurrent=GetMeSore(server_user_item);
			FishLog.lNowKuCun=-g_game_config.g_stock_score_old;
			CDBAction::GetInstance().CatchFish(&FishLog);//�̷ִ��� ����
		}
	}

	//����У��
	if (bullet_mul < g_game_config.min_bullet_multiple_ || bullet_mul > g_game_config.max_bullet_multiple_)
	{
		//_tprintf("�ӵ���%d\n",bullet_mul);//�������ȥ��
		return true;
	}

	//����ӵ������Ƿ��������ֵ
	if (bullet_mul > g_game_config.min_bullet_multiple_ && bullet_mul < g_game_config.max_bullet_multiple_)
	{
		if(bullet_mul>=10 && bullet_mul<100)
		{
			if(bullet_mul%10>0) return true;
		}
		if(bullet_mul>=100 && bullet_mul<1000)
		{
			if(bullet_mul%100>0) return true;
		}
		if(bullet_mul>=1000 && bullet_mul<g_game_config.max_bullet_multiple_)
		{
			if((bullet_mul%1000 != 0)&&(bullet_mul%1000 != 900)) return true;
		}
	}
	if(bullet_kind>BULLET_KIND_COUNT) return true;
	
	//��������ģʽ��֤
	if(0==m_TimeFree[chair_id].dwFreeTime)
	{
		if(bullet_kind==Bullet_Type_xianshipao) return true;
	}
	else
	{
		if(bullet_kind!=Bullet_Type_xianshipao) return true;//�������������á�

		if(m_TimeFree[chair_id].mulriple != bullet_mul) return true;

		//������������ ǰ�÷����
		if(m_TimeFree[chair_id].wFireCount>35) return true;

		m_TimeFree[chair_id].wFireCount++;
	}

	//�������ӵ������ǻ�ȡ�ĵ����� ������֤����
	if( (Bullet_Type_diancipao==bullet_kind)||(Bullet_Type_zuantoupao==bullet_kind)||(Bullet_Type_lianhuanzhadan==bullet_kind) )
	{
		if(n_ToolsGet[chair_id].bullet_mulriple != bullet_mul) return true;

		//���� ��ֹ�޸�����
		n_ToolsGet[chair_id].bullet_mulriple = 0;
	}

	//����ģʽȡ�� �����ӵ���Ҽ���
	if (lock_fishid > 0)
	{
		FishTraceInfo *pLockFish = GetFishTraceInfo(lock_fishid);
		if(pLockFish && pLockFish->fish_kind<FISH_KIND_10) return true;
		////�����ж� ��������Ҫ���� ���ͻ����Ϸ�����󷵻����
		//if(pLockFish == NULL)
		//{
		//	_tprintf("������ID:%d\n",lock_fishid);
		//	//lock_fishid = -2; //�ͻ��˲������� �����޷��Ϸ� ����ûɾ��
		//}
	}else lock_fishid = -1;

	//���͵��ӵ�or��ѵ� ����֤����Ƿ�
	bool bNoCheck=((Bullet_Type_diancipao==bullet_kind)||(Bullet_Type_zuantoupao==bullet_kind)||(Bullet_Type_lianhuanzhadan==bullet_kind)||
		(m_TimeFree[chair_id].dwFreeTime!=0 && bullet_kind==Bullet_Type_xianshipao));

	if (!bNoCheck && fish_score_[chair_id] < bullet_mul)
	{
		//_tprintf("��ң�%I64d, �ӵ���%d\n",fish_score_[chair_id],bullet_mul);
		return true;
	}

	ServerBulletInfo* bullet_info = ActiveBulletInfo(chair_id,pMsg->nBulletID);
	if(bullet_info==0)//�ڴ�������� �ɿ��Ƿ������
	{
		g_lMemWrongScore+=bullet_mul;
		return true;
	}

	if(!bNoCheck) 
	{
		fish_score_[chair_id] -= bullet_mul;
		g_lUserAllWin-=bullet_mul;
		g_lFireScore+=bullet_mul;
		m_lFireNoInKuCunGold[chair_id]+=bullet_mul;
		m_lFireScore[chair_id] +=bullet_mul;
	}

	bullet_info->bCheat=false;
	bullet_info->cbMaxUse=0;
	bool bCheat = m_tagFireSpeedCheck[chair_id].fire(bullet_kind == Bullet_Type_xianshipao);
	bullet_info->bCheat=bCheat;
	bullet_info->cbBoomKind = INVALID_BYTE;
	
	//���ֵ����㱾���ֵ����
	if(Bullet_Type_diancipao== bullet_kind)
	{
		bullet_info->cbMaxUse=1;
		m_ToolsFree[chair_id].bullet_id=bullet_info->bullet_id;
	}
	else if(Bullet_Type_zuantoupao== bullet_kind)
	{
		bullet_info->cbMaxUse=6;
		m_ToolsFree[chair_id].cbMaxBullet=m_ToolsFree[chair_id].cbMaxBullet/6;
		m_ToolsFree[chair_id].bullet_id=bullet_info->bullet_id;
	}
	BulletMoveManage.ActiveBullet(bullet_info->bullet_id,chair_id,
		float(g_game_config.bullet_speed_[bullet_kind])/1000,fStartX,fStartY,float(fResultMouseX)/1000000,float(fResultMouseY)/1000000);

	CMD_S_UserFire user_fire;
	user_fire.bullet_kind = bullet_kind;
	user_fire.fMouseX = fResultMouseX;
	user_fire.fMouseY = fResultMouseY;
	user_fire.wChairID = server_user_item->GetChairID();
	user_fire.nBullet_mulriple = bullet_mul;
	user_fire.nLock_fishid = lock_fishid;
	user_fire.nBullet_id = bullet_info->bullet_id;
	user_fire.lNow_fish_score = fish_score_[chair_id];
	user_fire.lNow_user_score = GetUserGold(chair_id);
	user_fire.cbBoomKind = pMsg->cbBoomKind;
	user_fire.nBullet_index = pMsg->nBullet_index;

	SendTableData(SUB_S_USER_FIRE, &user_fire, sizeof(user_fire), NULL);

	bullet_info->nLockFishID=lock_fishid;
	bullet_info->bullet_kind = user_fire.bullet_kind;
	bullet_info->bullet_mulriple = user_fire.nBullet_mulriple;
	//_tprintf("----��ң�%I64d, �ӵ���%d\n",fish_score_[chair_id],bullet_mul);

	WriteScore(chair_id);
	if( GetNeedRecordDB(server_user_item->GetGameID()) )
	{
		//ע��������־��¼
		int lXiuZhen=0;
		if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=bullet_mul;

		CDBAction::GetInstance().UserFire(server_user_item->GetGameID(), bullet_info->bullet_id, bullet_info->bullet_kind, lXiuZhen, bullet_info->bullet_idx_, bullet_info->cbMaxUse);
	}

	return true;
}

//��׽����
bool TableFrameSink::OnSubCatchFish(IServerUserItem* server_user_item, CMD_C_CatchFish *pMsg)
{
	int fish_id=pMsg->nFish_id;
	int bullet_id=pMsg->nBullet_id;

	WORD chair_id = server_user_item->GetChairID();
	ServerBulletInfo* pDeleteBullet = GetBulletInfo(chair_id, bullet_id);
	if (pDeleteBullet == NULL)
	{
		//_tprintf("OnSubCatchFish�ӵ�������%d��%d\n",chair_id,bullet_id);//�������ȥ��
		return true;
	}
	//�յ���Ϣ,��ɾ���ⷢ�ӵ�......��Ҫ��������֤�켣 ���桾��ء�ɾ���ӵ�
	ServerBulletInfo bullet_info_cpy;
	memcpy(&bullet_info_cpy, pDeleteBullet, sizeof(bullet_info_cpy));
	ServerBulletInfo* bullet_info = &bullet_info_cpy;
	FishTraceInfo* pDeleteFish_trace = GetFishTraceInfo(fish_id,true);
	if (bullet_info->cbBoomKind!=INVALID_BYTE) return true; 

	//�㲻���ڷ���
	bool bRetScore=false;
	bRetScore = (pDeleteFish_trace==NULL);
	if(!bRetScore)
	{
		if(pDeleteFish_trace && pDeleteFish_trace->fish_kind>FISH_KIND_10)
		{
			if(bullet_info->nLockFishID>0 && bullet_info->nLockFishID!=pMsg->nFish_id) bRetScore=true;
		}
	}
	if (bRetScore)
	{
		//ע��������־��¼
		int lXiuZhen=0;
		if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=bullet_info->bullet_mulriple;

		FreeBulletInfo(chair_id, pDeleteBullet);
		//�򵽵��㲻���ڣ�������ҽ�� ���ûӰ��
		fish_score_[chair_id] += lXiuZhen;
		g_lUserAllWin+=lXiuZhen;
		g_lFireScore-=lXiuZhen;
		m_lRetrunScore[chair_id] += lXiuZhen;
		m_lFireNoInKuCunGold[chair_id]-=lXiuZhen;

		CMD_S_ReturnBulletScore tag_return;
		tag_return.wChairID = chair_id;
		tag_return.wReasonType = 1;
		//tag_return.lNow_fish_score =lXiuZhen;
		tag_return.lNow_fish_score =fish_score_[chair_id];
		tag_return.lNow_user_score = GetUserGold(chair_id);
		table_frame_->SendTableData(INVALID_CHAIR, SUB_S_RETURNBULLETSCORE, &tag_return, sizeof(tag_return));
		table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_RETURNBULLETSCORE, &tag_return, sizeof(tag_return));

		WriteScore(chair_id);

		if( GetNeedRecordDB(server_user_item->GetGameID()) )
		{
			if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=0;
			else lXiuZhen=bullet_info->bullet_mulriple;

			tagCatchFishRecord FishLog;
			FishLog.dwGameID=server_user_item->GetGameID();
			FishLog.nBulletID=bullet_id;
			FishLog.nBulletOnlyID=bullet_info->bullet_idx_;
			FishLog.enFishKind=100;
			FishLog.lFishID=fish_id;
			FishLog.nWinScore=lXiuZhen;
			FishLog.nSoucrePro=-1.0f;
			FishLog.nChangeAddPro=-1.0f;
			FishLog.lScoreCurrent=GetMeSore(server_user_item);
			FishLog.lNowKuCun=-g_game_config.g_stock_score_old;
			CDBAction::GetInstance().CatchFish(&FishLog);//�����㲻�� ����
		}

		return true;
	}

	//��ʱ��������Ϣ
	FishTraceInfo fish_trace_cpy;
	CopyMemory(&fish_trace_cpy,pDeleteFish_trace,sizeof(FishTraceInfo));
	FishTraceInfo* fish_trace_info=&fish_trace_cpy;

	//Ҫ��ɾ���ӵ�֮ǰ ���Ǻ��ڹ������
	if(fish_trace_info->fish_kind < FISH_KIND_10)
	{
		int fResultX=pMsg->nBulletX;
		int fResultY=pMsg->nBulletY;
		WORD wClientKey=g_wKeyClient[chair_id];
		int nUserIdKey=(server_user_item->GetUserID()*server_user_item->GetUserID()+server_user_item->GetUserID()/11)%1000000;
		fResultX += (int)( wClientKey + wClientKey * wClientKey + nUserIdKey + (nUserIdKey * (pMsg->nFish_id % 11) % wClientKey) * wClientKey);
		fResultY += (int)( wClientKey + wClientKey * wClientKey + nUserIdKey + (wClientKey * (pMsg->nBullet_id % 11) / 100) * wClientKey);

		//ֱ�Ӷ��� no initial
		tagCrossCheck CheckData;
		CheckData.wChairID=chair_id;
		CheckData.nBulletID=bullet_id;
		CheckData.nBulletX=fResultX;
		CheckData.nBulletY=fResultY;
		CheckData.fBulletStartX=float(pMsg->nBulletStartX)/1000000;
		CheckData.fBulletStartY=float(pMsg->nBulletStartY)/1000000;
		CheckData.nFishID=fish_id;
		CheckData.nFishX=pMsg->nFishX;
		CheckData.nFishY=pMsg->nFishY;
		CheckData.dwElapseTm=pMsg->dwMoveTime;
		CheckData.dwBulletTime=pMsg->dwBulletTime;
		CheckData.fMultiRation=1.0f;
		CheckData.cbCheckTag=0;

		//�����л�ʱ���� ����֤
		if(fish_trace_info->fish_kind==FISH_KIND_8 || fish_trace_info->fish_kind==FISH_KIND_11)
		{
			CheckData.cbCheckTag=1;
		}

		int nCheck=CollisionManage.IsCross(&CheckData);
		if(nCheck!=0)
		{
			//_tprintf(("δ��ײ��%d\n"),nCheck);//�������ȥ��
			m_dwInValidPlayerTm[chair_id]=GetTickCount();
		}
	}
	//������֤�����ɾ���ӵ�
	FreeBulletInfo(chair_id, pDeleteBullet);

	//������¼����ͼ
	int nFishX=fish_trace_info->fish_kind;
	if(FISH_YUQUN1==fish_trace_info->fish_kind || FISH_YUQUN2==fish_trace_info->fish_kind) nFishX=fish_trace_info->fish_tag;
	else if(FISH_CHAIN==fish_trace_info->fish_kind) nFishX=32+fish_trace_info->fish_tag;//FISH_KIND_1-FISH_KIND_9
	else if(FISH_CATCH_SMAE==fish_trace_info->fish_kind) nFishX=41+fish_trace_info->fish_tag;//FISH_KIND_1-FISH_KIND_13
	else if(FISH_YUZHEN==fish_trace_info->fish_kind) nFishX=fish_trace_info->fish_tag;
	if(nFishX<RECORD_FISH_NUM)
	{
		g_dwFireCountX[nFishX]++;
		g_lFireScoreX[nFishX]+=bullet_info->bullet_mulriple;
	}

	//�ӵ�����4�����Ϸ�
	if(GetElapsedTm(bullet_info->dwTicktCount)>240000)//bullet_info->bCheat
	{
		//ע��������־��¼
		int lXiuZhen=0;
		if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=bullet_info->bullet_mulriple;
		g_lReturnScore+=lXiuZhen;
		m_lFireIntoSystemGold[chair_id]+=lXiuZhen;

		if( GetNeedRecordDB(server_user_item->GetGameID()) )
		{
			if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=0;
			else lXiuZhen=bullet_info->bullet_mulriple;

			tagCatchFishRecord FishLog;
			FishLog.dwGameID=server_user_item->GetGameID();
			FishLog.nBulletID=bullet_info->bullet_id;
			FishLog.nBulletOnlyID=bullet_info->bullet_idx_;
			FishLog.enFishKind=400;
			FishLog.lFishID=(bullet_info->bCheat)?401:400;
			FishLog.nWinScore=lXiuZhen;
			FishLog.nSoucrePro=-1.0f;
			FishLog.nChangeAddPro=-1.0f;
			FishLog.lScoreCurrent=GetMeSore(server_user_item);
			FishLog.lNowKuCun=-g_game_config.g_stock_score_old;
			CDBAction::GetInstance().CatchFish(&FishLog);//�̷ִ��� ����
		}

		return true;
	}

	//2���Ӵ���״̬����
	if(m_dwInValidPlayerTm[chair_id]!=0 && GetElapsedTm(m_dwInValidPlayerTm[chair_id])>120000)
	{
		m_dwInValidPlayerTm[chair_id]=0;
	}
	float fLimitMultiPro=1.0f;
	if(m_dwInValidPlayerTm[chair_id]!=0) fLimitMultiPro=float(g_random_int_range(90,100))/100;
	
	BulletKind bullet_kind=bullet_info->bullet_kind;
	int bullet_mul=bullet_info->bullet_mulriple;

	if (bullet_mul < g_game_config.min_bullet_multiple_ || bullet_mul > g_game_config.max_bullet_multiple_)
	{
		g_dwUnNomal|=0x1000;
		return true;
	}

	//����ۼ�
	if (!server_user_item->IsAndroidUser())
	{
		bool bStock=((Bullet_Type_putongpao==bullet_kind)||(Bullet_Type_shandianpao==bullet_kind)||(Bullet_Type_zidanpao==bullet_kind)|| (Bullet_Type_sanguanpao==bullet_kind));
		if(bStock)
		{
			StockScore(fish_trace_info->fish_kind,bullet_mul,0,true);

			g_lEnterScore+=bullet_mul;
			m_lFireNoInKuCunGold[chair_id]-=bullet_mul;
		}
	}

	//��Ϸ���֤
	if (fish_trace_info->fish_kind >= FISH_KIND_COUNT) return true;
	if((m_cbNowYuZhenID!=0xff) && (FISH_YUZHEN!=fish_trace_info->fish_kind)) return true;//������֤
	if(FISH_YUZHEN==fish_trace_info->fish_kind)
	{
		if(fish_trace_info->fish_tag<FISH_KIND_3)
		{
			fLimitMultiPro*=g_random_double_range(0.8,1.1);//float(g_random_int_range(80,110))/100;//���rand�ȶ�
		}
		else if(fish_trace_info->fish_tag<FISH_KIND_9)
		{
			fLimitMultiPro*=g_random_double_range(0.9,1.05);//float(g_random_int_range(90,105))/100;
		}
		else
		{
			fLimitMultiPro*=g_random_double_range(0.92,1.0);//float(g_random_int_range(90,100))/100;
		}
	}


	//����ڿͻ���Ч����֧���л� ���˴���
	if(bullet_info->bullet_kind==Bullet_Type_xianshipao)
	{
		if(fish_trace_info->fish_kind>=FISH_KIND_14 && fish_trace_info->fish_kind<=FISH_KIND_16) return true;
	}

	//�ͻ���Ч����ͻ ��;���ܶ���ӵ��Ѿ��ڷ��������� �����Ч����֧���л�
	if(m_TimeFree[chair_id].dwFreeTime!=0)
	{
		if(fish_trace_info->fish_kind>=FISH_KIND_14 && fish_trace_info->fish_kind<=FISH_KIND_16) return true;

		if(m_TimeFree[chair_id].wFireCount>35) return true;
	}

	//���ֵ����� �����Բ�֧�ֶ����ڼ��ֻ������һ��
	if(m_dwToolFishTm[chair_id]!=0)
	{
		if(fish_trace_info->fish_kind>=FISH_KIND_14 && fish_trace_info->fish_kind<=FISH_KIND_16) return true;
	}

	//������ͳ�� �������� CatchGroup����Ҫ
	fish_trace_info->wFireCount[chair_id]++;

	//����ͳ��
	m_nTotalKillFishCount[chair_id]++;
	m_nKillFishCount[chair_id][fish_trace_info->fish_kind]++;

	int fish_multiple = g_game_config.fish_multiple_[fish_trace_info->fish_kind];
	SCORE fish_score = g_game_config.fish_multiple_[fish_trace_info->fish_kind] * bullet_mul;
	WORD wFishKind = fish_trace_info->fish_kind;
	if(wFishKind>=FISH_YUQUN1 && wFishKind<=FISH_YUZHEN) wFishKind=fish_trace_info->fish_tag;
	bool bRegionFish = ((wFishKind >= FISH_KIND_17 && wFishKind<=FISH_KIND_18)||(wFishKind >= FISH_KIND_20 && wFishKind<=FISH_SMALL_2_BIG3));
	if(bRegionFish)
	{
		fish_multiple = g_random_int_range(g_game_config.fish_multiple_[wFishKind], g_game_config.fish_multiple_max[wFishKind]);
		fish_score = fish_multiple * bullet_mul;
	}
	else if(FISH_YUQUN1<=fish_trace_info->fish_kind)
	{
		fish_multiple = g_game_config.fish_multiple_[fish_trace_info->fish_tag];
		fish_score=g_game_config.fish_multiple_[fish_trace_info->fish_tag] * bullet_mul;
	}

	BYTE cbGiveCount=0;
	if(0==g_cbTest)
	{
		//��������ִ�е���� ը���������汣��
		if (fish_trace_info->fish_kind >= FISH_KIND_14 && fish_trace_info->fish_kind <= FISH_KIND_16)
		{
			cbGiveCount = BombStockHitTool(fish_trace_info,bullet_info,server_user_item,fLimitMultiPro);
			if(0==cbGiveCount) return true;
		}
		//�������� ������ǰ�˳�
		else if (fish_trace_info->fish_kind == FISH_CHAIN)
		{
			DWORD dwAllFishMul=0;
			CMD_S_CatchGroupFish catch_sweep_fish;
			ZeroMemory(&catch_sweep_fish,sizeof(catch_sweep_fish));

			//���¼���
			int nFirstScore = g_game_config.fish_multiple_[fish_trace_info->fish_tag] * bullet_mul;
			fish_score=nFirstScore;

			catch_sweep_fish.nBullet_mul=bullet_mul;
			catch_sweep_fish.wGroupSytle=3;
			catch_sweep_fish.nCatch_fish_count=1;
			catch_sweep_fish.nCatch_fish_id[0]=fish_trace_info->fish_id;
			catch_sweep_fish.nfish_score[0]=fish_score;
			dwAllFishMul+=g_game_config.fish_multiple_[fish_trace_info->fish_tag];

			int nKill=0;
			int ndd=rand()%5+2;
			DWORD dwNowTm=GetTickCount();
			for (int f=0;f<active_fish_trace_vector_.size() && nKill<ndd;f++)
			{
				FishTraceInfo *pFishTraceTp=active_fish_trace_vector_[f];
				if(fish_trace_info->fish_id == pFishTraceTp->fish_id) continue;
				if(pFishTraceTp->fish_kind>FISH_KIND_13) continue;
				DWORD dwActiveTm=pFishTraceTp->build_tick;
				__int64 nElapseTm=GetElapsedTm(dwActiveTm);
				if((nElapseTm<3000) || (nElapseTm>15000)) continue;

				dwAllFishMul+=g_game_config.fish_multiple_[pFishTraceTp->fish_kind];
				int nTwoScore=g_game_config.fish_multiple_[pFishTraceTp->fish_kind] * bullet_mul;
				fish_score+=nTwoScore;

				++nKill;
				catch_sweep_fish.nfish_score[nKill]=nTwoScore;
				catch_sweep_fish.nCatch_fish_id[nKill]=pFishTraceTp->fish_id;
			}
			catch_sweep_fish.nCatch_fish_count+=nKill;

			bool bHitOk = NormalStockHitGroup(fish_trace_info,bullet_info,server_user_item,dwAllFishMul,fish_score,fLimitMultiPro);
			if(!bHitOk) return true;

			for (int k=0;k<catch_sweep_fish.nCatch_fish_count;k++)
			{
				FreeFishTrace(catch_sweep_fish.nCatch_fish_id[k]);
			}
			fish_score_[chair_id] += fish_score;
			g_lUserAllWin+=fish_score;
			m_lWinScore[chair_id] +=fish_score;

			catch_sweep_fish.wChairID = chair_id;
			catch_sweep_fish.lNow_fish_score = fish_score_[chair_id];
			catch_sweep_fish.lNow_user_score = GetUserGold(chair_id);

			table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_fish, sizeof(catch_sweep_fish));
			table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_fish, sizeof(catch_sweep_fish));

			WriteScore(chair_id);
			if (!server_user_item->IsAndroidUser())
			{
				StockScore(FISH_CHAIN,0,fish_score);
			}

			//����ʲô��
			m_nKillFishCount_Death[chair_id][FISH_CHAIN]++;						
			m_nTotalKillFishCount_Death[chair_id]++;
			if(nFishX<RECORD_FISH_NUM)
			{
				g_dwDeadCountX[nFishX]++;
				g_lCatchFishScoreX[nFishX]+=fish_score;
			}

			return true;
		}
		//������ ������ǰ�˳�
		else if (fish_trace_info->fish_kind == FISH_CATCH_SMAE)
		{
			DWORD dwAllFishMul=0;
			CMD_S_CatchGroupFish catch_sweep_fish;
			ZeroMemory(&catch_sweep_fish,sizeof(catch_sweep_fish));

			//���¼���
			int nFirstScore = g_game_config.fish_multiple_[fish_trace_info->fish_tag] * bullet_mul;
			fish_score=nFirstScore;

			DWORD dwFishMulti=g_game_config.fish_multiple_[fish_trace_info->fish_tag];
			dwAllFishMul+=dwFishMulti;
			catch_sweep_fish.nBullet_mul=bullet_mul;
			catch_sweep_fish.wGroupSytle=4;
			catch_sweep_fish.nCatch_fish_count=1;
			catch_sweep_fish.nCatch_fish_id[0]=fish_trace_info->fish_id;
			catch_sweep_fish.nfish_score[0]=fish_score;
			std::vector<FishTraceInfo*>::iterator iter;
			for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end();++iter)
			{
				FishTraceInfo* fish_trace_tmp = *iter;
				if(fish_trace_info->fish_id == fish_trace_tmp->fish_id) continue;
				bool bOne=(fish_trace_tmp->fish_kind==fish_trace_info->fish_tag);
				bool bTwo=false;
				if((fish_trace_tmp->fish_kind==FISH_YUQUN1) || (fish_trace_tmp->fish_kind==FISH_YUQUN2))
				{
					bTwo=(fish_trace_tmp->fish_tag==fish_trace_info->fish_tag);
					//if(bTwo) _tprintf("%d,%d\n",fish_trace_tmp->fish_id,fish_trace_tmp->fish_tag);
				}
				if (bOne || bTwo)
				{
					fish_score+=nFirstScore;
					int nD=catch_sweep_fish.nCatch_fish_count;
					catch_sweep_fish.nfish_score[nD]=nFirstScore;
					catch_sweep_fish.nCatch_fish_id[nD]=fish_trace_tmp->fish_id;

					dwAllFishMul+=dwFishMulti;
					catch_sweep_fish.nCatch_fish_count++;
				}
			}

			bool bHitOk = NormalStockHitGroup(fish_trace_info,bullet_info,server_user_item,dwAllFishMul,fish_score,fLimitMultiPro);
			if(!bHitOk) return true;

			for (int k=0;k<catch_sweep_fish.nCatch_fish_count;k++)
			{
				FreeFishTrace(catch_sweep_fish.nCatch_fish_id[k]);
			}
			fish_score_[chair_id] += fish_score;
			g_lUserAllWin+=fish_score;
			m_lWinScore[chair_id] +=fish_score;

			//
			catch_sweep_fish.wChairID = chair_id;
			catch_sweep_fish.lNow_fish_score = fish_score_[chair_id];
			catch_sweep_fish.lNow_user_score = GetUserGold(chair_id);

			table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_fish, sizeof(catch_sweep_fish));
			table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_fish, sizeof(catch_sweep_fish));
			//_tprintf("������Ⱥ��%d\n",catch_sweep_fish.nCatch_fish_count);

			WriteScore(chair_id);
			if (!server_user_item->IsAndroidUser())
			{
				StockScore(FISH_CATCH_SMAE,0,fish_score);
			}

			//����ʲô��
			m_nKillFishCount_Death[chair_id][FISH_CATCH_SMAE]++;						
			m_nTotalKillFishCount_Death[chair_id]++;
			if(nFishX<RECORD_FISH_NUM)
			{
				g_dwDeadCountX[nFishX]++;
				g_lCatchFishScoreX[nFishX]+=fish_score;
			}

			return true;
		}
		//��Ⱥ4�еķ�����
		else if (fish_trace_info->fish_kind == FISH_YUQUN4)
		{
			if(fish_trace_info->fish_king_id!=fish_id)
			{
				//��ͨ�������
				bool bHitOk = NormalStockHitSingle(fish_trace_info,bullet_info,server_user_item,fish_score,fLimitMultiPro);
				if(!bHitOk) return true;
			}		
		}
		//��������ִ�е���� ��������ʿ���
		else
		{
			if(bRegionFish)
			{
				bool bHitOk = NormalStockHitGroup(fish_trace_info,bullet_info,server_user_item,fish_multiple,fish_score,fLimitMultiPro);
				if(!bHitOk) return true;
			}
			else
			{
				bool bHitOk = NormalStockHitSingle(fish_trace_info,bullet_info,server_user_item,fish_score,fLimitMultiPro);
				if(!bHitOk) return true;
			}
		}
	}

	//���ֵ����㱾���ֵ����
	if(fish_trace_info->fish_kind>=FISH_KIND_14 && fish_trace_info->fish_kind<=FISH_KIND_16)
	{
		fish_score=0;
		//begin add by cxl
		m_wDaoJuPaoUser=chair_id;
		//end add by cxl
	}

	//����ʲô��
	m_nKillFishCount_Death[chair_id][fish_trace_info->fish_kind]++;						
	m_nTotalKillFishCount_Death[chair_id]++;
	if(nFishX<RECORD_FISH_NUM)
	{
		g_dwDeadCountX[nFishX]++;
		g_lCatchFishScoreX[nFishX]+=fish_score;
	}

	if(fish_score!=0)
	{
		fish_score_[chair_id] += fish_score;
		g_lUserAllWin+=fish_score;
		m_lWinScore[chair_id] +=fish_score;

		WriteScore(chair_id);
		if (!server_user_item->IsAndroidUser())
		{
			StockScore(fish_trace_info->fish_kind,0,fish_score);
		}
	}

	//���Ͳ�����
	CMD_S_CatchFish catch_fish;
	catch_fish.wChairID = chair_id;
	catch_fish.nFish_id = fish_id;
	catch_fish.nFish_score = fish_score;
	catch_fish.nBullet_mul = bullet_mul;
	catch_fish.lNow_fish_score = fish_score_[chair_id];
	catch_fish.lNow_user_score = GetUserGold(chair_id);
	table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_FISH, &catch_fish, sizeof(catch_fish));
	table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_FISH, &catch_fish, sizeof(catch_fish));

	//���ֵ��������⴦��
	if(fish_trace_info->fish_kind>=FISH_KIND_14 && fish_trace_info->fish_kind<=FISH_KIND_16)
	{
		m_ToolsFree[chair_id].cbMaxBullet=cbGiveCount;//����������ʷ�����ֵ����

		//����ը�����⴦��
		if(FISH_KIND_15==fish_trace_info->fish_kind)
		{
			ServerBulletInfo* pBullet_infoTmp = ActiveBulletInfo(chair_id,0);
			pBullet_infoTmp->cbMaxUse=3;
			m_ToolsFree[chair_id].cbMaxBullet=cbGiveCount/3;
			pBullet_infoTmp->bullet_mulriple=bullet_info->bullet_mulriple;
			pBullet_infoTmp->bullet_kind=Bullet_Type_lianhuanzhadan;
			pBullet_infoTmp->cbBoomKind = INVALID_BYTE;
			BulletMoveManage.ActiveBullet(pBullet_infoTmp->bullet_id,chair_id,0,0,0);

			if( GetNeedRecordDB(server_user_item->GetGameID()) )
			{
				CDBAction::GetInstance().UserFire(server_user_item->GetGameID(), pBullet_infoTmp->bullet_id, pBullet_infoTmp->bullet_kind, pBullet_infoTmp->bullet_mulriple, pBullet_infoTmp->bullet_idx_,pBullet_infoTmp->cbMaxUse);
			}

			m_ToolsFree[chair_id].bullet_id=pBullet_infoTmp->bullet_id;

			CMD_S_Bomb_Points BombPoints;
			BombPoints.wChairID=chair_id;
			for (int m=0;m<3;m++)
			{
				BombPoints.wPointX[m]=rand()%kResolutionWidth;
				BombPoints.wPointY[m]=rand()%kResolutionHeight;
			}
			table_frame_->SendTableData(INVALID_CHAIR, SUB_S_BOMB_POINTS, &BombPoints, sizeof(BombPoints));
			table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_BOMB_POINTS, &BombPoints, sizeof(BombPoints));
		}

		m_dwToolFishTm[chair_id]=1;

		//��ֹ�л�
		n_ToolsGet[chair_id].bullet_mulriple=bullet_info->bullet_mulriple;
		if(FISH_KIND_14==fish_trace_info->fish_kind) n_ToolsGet[chair_id].bullet_kind=Bullet_Type_zuantoupao;
		else if(FISH_KIND_15==fish_trace_info->fish_kind) n_ToolsGet[chair_id].bullet_kind=Bullet_Type_lianhuanzhadan;
		else n_ToolsGet[chair_id].bullet_kind=Bullet_Type_diancipao;
	}

	FreeFishTrace(fish_trace_info->fish_id);//�ر�ע�� ɾ����ȫ

	//������Ѱ�
	BuildXianShiFree(chair_id,bullet_info->bullet_kind,bullet_info->bullet_mulriple);

	return true;
}

//��׽������
bool TableFrameSink::OnSubCatchGroupFish(IServerUserItem* server_user_item, CMD_C_CatchGroupFish *pMsg)
{
	//���� ������ը�� ����� ��ͷ�ڡ�����
	WORD chair_id = server_user_item->GetChairID();
	ServerBulletInfo* pDeleteBullet = GetBulletInfo(chair_id, pMsg->nBullet_id);
	if (pDeleteBullet == NULL)
	{
		//_tprintf("OnSubCatchGroupFish ������ �ӵ�������%d��%d\n",chair_id,pMsg->nBullet_id);//�������ȥ��
		return true;
	}

	WORD wBulletKindT=pDeleteBullet->bullet_kind;
	//_tprintf("��%s�� ���Ͳ��������  �ӵ�ID��%d �����Ŀ:%d ʣ�������%d\n",server_user_item->GetAccounts(),pMsg->nBullet_id,pMsg->nCatch_fish_count,pDeleteBullet->cbMaxUse);

	//�ӵ��ж� ��������������ʱɾ��
	if(pDeleteBullet->cbMaxUse==0)
	{
		return true;
	}
	if(pDeleteBullet->cbBoomKind!=INVALID_BYTE)
	{
		return true; 
	}
	
	//������֤���� �����ֵ����ڿ��ߴ�����
	if(m_TimeFree[chair_id].dwFreeTime!=0)
	{
		return true;
	}
	if(wBulletKindT!=n_ToolsGet[chair_id].bullet_kind)
	{
		return true;
	}
	if((wBulletKindT!=Bullet_Type_diancipao) && (wBulletKindT!=Bullet_Type_zuantoupao) && (wBulletKindT!=Bullet_Type_lianhuanzhadan))
	{
		return true;
	}

	if(pDeleteBullet->cbMaxUse>=1) pDeleteBullet->cbMaxUse--;

	//�޸ĺ�ȫ����
	ServerBulletInfo bullet_info_cpy;
	memcpy(&bullet_info_cpy, pDeleteBullet, sizeof(bullet_info_cpy));
	ServerBulletInfo* bullet_info = &bullet_info_cpy;

	if(pDeleteBullet->cbMaxUse==0)
	{
		if(pDeleteBullet->bullet_kind==Bullet_Type_zuantoupao || pDeleteBullet->bullet_kind==Bullet_Type_lianhuanzhadan || pDeleteBullet->bullet_kind==Bullet_Type_diancipao)
			m_wDaoJuPaoUser = INVALID_CHAIR;

		FreeBulletInfo(chair_id,pDeleteBullet);
	}

	//�ӵ�����4�����Ϸ�
	if(GetElapsedTm(bullet_info->dwTicktCount)>240000)//bullet_info->bCheat
	{
		if( GetNeedRecordDB(server_user_item->GetGameID()) )
		{
			//ע��������־��¼
			int lXiuZhen=bullet_info->bullet_mulriple;
			if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=0;

			tagCatchFishRecord FishLog;
			FishLog.dwGameID=server_user_item->GetGameID();
			FishLog.nBulletID=bullet_info->bullet_id;
			FishLog.nBulletOnlyID=bullet_info->bullet_idx_;
			FishLog.enFishKind=500;
			FishLog.lFishID=(bullet_info->bCheat)?501:500;
			FishLog.nWinScore=lXiuZhen;
			FishLog.nSoucrePro=-1.0f;
			FishLog.nChangeAddPro=-1.0f;
			FishLog.lScoreCurrent=GetMeSore(server_user_item);
			FishLog.lNowKuCun=-g_game_config.g_stock_score_old;
			CDBAction::GetInstance().CatchFish(&FishLog);//�̷ִ��� ����
		}

		return true;
	}

	//�ͻ��˴�����������
	int bullet_mul=bullet_info->bullet_mulriple;
	BulletKind bullet_kind = bullet_info->bullet_kind;
	int catch_fish_count = pMsg->nCatch_fish_count;

	if(catch_fish_count>80 || catch_fish_count<=0)
	{
		//�������
		CMD_S_CatchGroupFish catch_except_sweep_result;//-û�д�����߳����������ҲҪ����Ϣ���ͻ���--��Ȼ�ͻ��˲����ؽ���̨
		memset(&catch_except_sweep_result, 0, sizeof(catch_except_sweep_result));
		catch_except_sweep_result.wChairID=chair_id;
		catch_except_sweep_result.nBullet_mul=bullet_mul;
		catch_except_sweep_result.wGroupCount = bullet_info->cbMaxUse;
		catch_except_sweep_result.nCatch_fish_count = 0;//0���ߴ���80���㶼��0��
		catch_except_sweep_result.lNow_fish_score = 0;
		catch_except_sweep_result.lNow_user_score = GetUserGold(chair_id);

		//0����ը�� 1����� 2��ͷ�� 3�������� 4������
		if(wBulletKindT==Bullet_Type_diancipao)
		{
			catch_except_sweep_result.wGroupSytle=1;
			m_ToolsFree[chair_id].TimeGold=0;
		}
		else if(wBulletKindT==Bullet_Type_zuantoupao)
		{
			catch_except_sweep_result.wGroupSytle=2;
			m_ToolsFree[chair_id].TimeGold+=0;
		}
		else if(wBulletKindT==Bullet_Type_lianhuanzhadan)
		{
			catch_except_sweep_result.wGroupSytle=0;
			m_ToolsFree[chair_id].TimeGold+=0;
		}

		if(catch_except_sweep_result.wGroupCount==0)
		{
			catch_except_sweep_result.nfish_score[59]=m_ToolsFree[chair_id].TimeGold;

			//����
			m_ToolsFree[chair_id].TimeGold=0;
			m_dwToolFishTm[chair_id]=GetTickCount();
			WORD wNowIndex=FISH_KIND_14+m_cbNowToolFish;
			m_distribulte_fish[wNowIndex].distribute_elapsed_=g_game_config.distribute_interval[wNowIndex]-40;
		}

		table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_except_sweep_result, sizeof(catch_except_sweep_result));
		table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_except_sweep_result, sizeof(catch_except_sweep_result));

		return true;
	}

	//---------------------------
	//���������ը����ķ���---����ķ���
	SCORE fish_score = 0;

	//�������
	CMD_S_CatchGroupFish catch_sweep_result;//--����Ҫ��¼Ⱥ����Ϣ��Ҫ���ݵ��ͻ�����
	memset(&catch_sweep_result, 0, sizeof(catch_sweep_result));
	catch_sweep_result.wChairID=chair_id;
	catch_sweep_result.nBullet_mul=bullet_mul;

	//���У�����ݰ���id�Ƿ��ظ� ע��ṹ���Ǳ䳤�յ��²���ֱ�ӿ���������
	CMD_C_CatchGroupFish catch_group;
	ZeroMemory(&catch_group,sizeof(catch_group));
	CopyMemory(&catch_group,pMsg,sizeof(CMD_C_CatchGroupFish));
	for (WORD k=0;k<catch_group.nCatch_fish_count;k++)
	{
		for (WORD j=k+1;j<catch_group.nCatch_fish_count;j++)
		{
			if(catch_group.nCatch_fish_id[k] == catch_group.nCatch_fish_id[j])
			{
				catch_group.nCatch_fish_id[j]=-1;
			}
		}
	}

	//
	INT nRealDead = 0;
	INT nMaxLimit = 0;
	for (DWORD i = 0; i < catch_fish_count; ++i)
	{
		if(++nMaxLimit > 200) break;//����ѭ���쳣

		//û��ȷ����������ȥ
		if((nRealDead<m_ToolsFree[chair_id].cbMaxBullet) && (i>=catch_fish_count-1))
		{
			i=0;
		}

		FishTraceInfo *fish_trace_infotemp=GetFishTraceInfo(catch_group.nCatch_fish_id[i],true);
		if (fish_trace_infotemp==0) //������
		{
			continue;
		}
		else if((fish_trace_infotemp->fish_kind >= FISH_KIND_14 && fish_trace_infotemp->fish_kind <= FISH_KIND_16) || (fish_trace_infotemp->fish_kind>=FISH_CHAIN))
		{
			//������ �������� ������ ���� pass
			continue;
		}
		else
		{
			//�����Ŀ����
			if(nRealDead > m_ToolsFree[chair_id].cbMaxBullet) break;

			FishKind fKindTTx=fish_trace_infotemp->fish_kind;

			//���������ж�
			float fLimitR = 1.0f;
			if(m_dwInValidPlayerTm[chair_id]!=0) fLimitR*=g_random_double_range(0.9,1.0);
			if(fKindTTx>=FISH_KIND_17 && fKindTTx<=FISH_KIND_19) fLimitR*=g_random_double_range(0.95,1.0);
			else if(fKindTTx>=FISH_KIND_20 && fKindTTx<=FISH_KIND_24) fLimitR*=g_random_double_range(0.8,0.98);
			bool bHitOk=BombStockHitSingle(fish_trace_infotemp,bullet_info,server_user_item,fLimitR);
			if(!bHitOk) continue;

			catch_sweep_result.nCatch_fish_id[ nRealDead ] = fish_trace_infotemp->fish_id;
			
			LONG lOneFish=g_game_config.fish_multiple_[fKindTTx] * bullet_mul;
			//��Ⱥ�������
			if(fKindTTx==FISH_YUQUN1 || fKindTTx==FISH_YUQUN2|| fKindTTx==FISH_YUQUN3|| fKindTTx==FISH_YUQUN4 || fKindTTx==FISH_YUQUN5)
			{
				fKindTTx=(FishKind)fish_trace_infotemp->fish_tag;
				lOneFish = g_game_config.fish_multiple_[fish_trace_infotemp->fish_tag] * bullet_mul;
			}
			fish_score += lOneFish;
			catch_sweep_result.nfish_score[nRealDead]=lOneFish;

			//����ʲô��
			m_nKillFishCount_Death[chair_id][fKindTTx]++;						
			m_nTotalKillFishCount_Death[chair_id]++;

			nRealDead++;

			FreeFishTrace(fish_trace_infotemp->fish_id);
		}
	}

	//
	if( GetNeedRecordDB(server_user_item->GetGameID()) )
	{
		tagCatchFishRecord FishLog;
		FishLog.dwGameID=server_user_item->GetGameID();
		FishLog.nBulletID=bullet_info->bullet_id;
		FishLog.nBulletOnlyID=bullet_info->bullet_idx_;
		FishLog.enFishKind=200;
		FishLog.lFishID=200;
		FishLog.nWinScore=fish_score;
		FishLog.nSoucrePro=0;
		FishLog.nChangeAddPro=0;
		FishLog.lScoreCurrent=GetMeSore(server_user_item);
		FishLog.lNowKuCun = -g_game_config.g_stock_score_old;
		CDBAction::GetInstance().CatchFish(&FishLog);
	}

	//��ķ���
	fish_score_[chair_id] += fish_score;
	g_lUserAllWin+=fish_score;
	m_lWinScore[chair_id] +=fish_score;
	WriteScore(chair_id);
	if (server_user_item->IsAndroidUser() == false)
	{
		FishKind fishkind=FISH_KIND_14;
		if(Bullet_Type_lianhuanzhadan==bullet_info->bullet_kind) fishkind=FISH_KIND_15;
		else if(Bullet_Type_diancipao==bullet_info->bullet_kind) fishkind=FISH_KIND_16;
		StockScore(fishkind,0,fish_score);
	}

	catch_sweep_result.wChairID = chair_id;
	catch_sweep_result.wGroupCount = bullet_info->cbMaxUse;
	catch_sweep_result.nCatch_fish_count = nRealDead;
	catch_sweep_result.lNow_fish_score = fish_score_[chair_id];
	catch_sweep_result.lNow_user_score = GetUserGold(chair_id);

	//0����ը�� 1����� 2��ͷ�� 3�������� 4������
	if(wBulletKindT==Bullet_Type_diancipao)
	{
		catch_sweep_result.wGroupSytle=1;
		m_ToolsFree[chair_id].TimeGold=fish_score;
	}
	else if(wBulletKindT==Bullet_Type_zuantoupao)
	{
		catch_sweep_result.wGroupSytle=2;
		m_ToolsFree[chair_id].TimeGold+=fish_score;
	}
	else if(wBulletKindT==Bullet_Type_lianhuanzhadan)
	{
		catch_sweep_result.wGroupSytle=0;
		m_ToolsFree[chair_id].TimeGold+=fish_score;
	}
	int nFishX=FISH_KIND_14;
	if(Bullet_Type_lianhuanzhadan==bullet_info->bullet_kind) nFishX=FISH_KIND_15;
	else if(Bullet_Type_diancipao==bullet_info->bullet_kind) nFishX=FISH_KIND_16;
	if(nFishX<RECORD_FISH_NUM)
	{
		g_dwDeadCountX[nFishX]++;
		g_lCatchFishScoreX[nFishX]+=fish_score;
	}
	
	if(catch_sweep_result.wGroupCount==0)
	{
		catch_sweep_result.nfish_score[59]=m_ToolsFree[chair_id].TimeGold;

		//����
		m_ToolsFree[chair_id].TimeGold=0;
		m_dwToolFishTm[chair_id]=GetTickCount();
		WORD wNowIndex=FISH_KIND_14+m_cbNowToolFish;
		m_distribulte_fish[wNowIndex].distribute_elapsed_=g_game_config.distribute_interval[wNowIndex]-40;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//_tprintf("ʣ�������%d\n",catch_sweep_result.wGroupCount);
	table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_result, sizeof(catch_sweep_result));
	table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH, &catch_sweep_result, sizeof(catch_sweep_result));

	//_tprintf("ok��Ⱥ  %d---������%d\n",catch_sweep_result.nCatch_fish_count,bullet_info->cbMaxUse);
	return true;
}

//��׽������
bool TableFrameSink::OnSubCatchFishKingGroupFish(IServerUserItem* server_user_item, CMD_C_CatchGroupFishKing *pMsg)
{
	//ֻ������������
	int bullet_id=pMsg->nBullet_id;
	int fishking_id=pMsg->nCatch_fishKing_id;
	int nCatchFishCount=pMsg->nCatch_fish_count;

	WORD chair_id = server_user_item->GetChairID();
	ServerBulletInfo* pDeleteBullet = GetBulletInfo(chair_id, bullet_id);
	if (pDeleteBullet == NULL) return true;
	if (pDeleteBullet->cbBoomKind!=INVALID_BYTE) return true; 

	//�յ���Ϣ,��ɾ���ⷢ�ӵ�......��Ҫ��������֤�켣 ���桾��ء�ɾ���ӵ�
	ServerBulletInfo bullet_info_cpy;
	memcpy(&bullet_info_cpy, pDeleteBullet, sizeof(bullet_info_cpy));
	ServerBulletInfo* bullet_info = &bullet_info_cpy;
	FishTraceInfo* pDeleteFish_trace = GetFishTraceInfo(fishking_id,true);
	
	//���������ڷ���
	bool bRetScore=false;
	bRetScore = (pDeleteFish_trace==NULL);
	if(!bRetScore)
	{
		if(pDeleteFish_trace && pDeleteFish_trace->fish_kind>FISH_KIND_10)//�������Ա�����
		{
			if(bullet_info->nLockFishID>0 && bullet_info->nLockFishID!=fishking_id) bRetScore=true;
		}
	}
	if(!bRetScore) bRetScore = (FISH_YUQUN4_KING==pDeleteFish_trace->fish_kind);
	if (bRetScore)
	{
		//ע��������־��¼
		int lXiuZhen=0;
		if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=bullet_info->bullet_mulriple;

		FreeBulletInfo(chair_id, pDeleteBullet);
		//�򵽵��㲻���ڣ�������ҽ�� ���ûӰ��
		fish_score_[chair_id] += lXiuZhen;
		g_lUserAllWin+=lXiuZhen;
		g_lFireScore-=lXiuZhen;
		m_lRetrunScore[chair_id] += lXiuZhen;
		m_lFireNoInKuCunGold[chair_id]-=lXiuZhen;

		CMD_S_ReturnBulletScore tag_return;
		tag_return.wChairID = chair_id;
		tag_return.wReasonType = 1;
		//tag_return.lNow_fish_score =lXiuZhen;
		tag_return.lNow_fish_score =fish_score_[chair_id];
		tag_return.lNow_user_score = GetUserGold(chair_id);
		table_frame_->SendTableData(INVALID_CHAIR, SUB_S_RETURNBULLETSCORE, &tag_return, sizeof(tag_return));
		table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_RETURNBULLETSCORE, &tag_return, sizeof(tag_return));

		WriteScore(chair_id);

		if( GetNeedRecordDB(server_user_item->GetGameID()) )
		{
			if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=0;
			else lXiuZhen=bullet_info->bullet_mulriple;

			tagCatchFishRecord FishLog;
			FishLog.dwGameID=server_user_item->GetGameID();
			FishLog.nBulletID=bullet_id;
			FishLog.nBulletOnlyID=bullet_info->bullet_idx_;
			FishLog.enFishKind=100;
			FishLog.lFishID=fishking_id;
			FishLog.nWinScore=lXiuZhen;
			FishLog.nSoucrePro=-1.0f;
			FishLog.nChangeAddPro=-1.0f;
			FishLog.lScoreCurrent=GetMeSore(server_user_item);
			FishLog.lNowKuCun=-g_game_config.g_stock_score_old;
			CDBAction::GetInstance().CatchFish(&FishLog);//�����㲻�� ����
		}

		return true;
	}

	//��ʱ����������Ϣ
	FishTraceInfo fish_trace_cpy;
	CopyMemory(&fish_trace_cpy,pDeleteFish_trace,sizeof(FishTraceInfo));
	FishTraceInfo* fish_trace_info=&fish_trace_cpy;

	//������֤�����ɾ���ӵ�
	FreeBulletInfo(chair_id, pDeleteBullet);

	//������¼����ͼ
	int nFishX=fish_trace_info->fish_kind;
	if(FISH_YUQUN1==fish_trace_info->fish_kind || FISH_YUQUN2==fish_trace_info->fish_kind ||
		FISH_YUQUN3==fish_trace_info->fish_kind || FISH_YUQUN4==fish_trace_info->fish_kind || FISH_YUQUN5==fish_trace_info->fish_kind) nFishX=fish_trace_info->fish_tag;
	else if(FISH_CHAIN==fish_trace_info->fish_kind) nFishX=32+fish_trace_info->fish_tag;//FISH_KIND_1-FISH_KIND_9
	else if(FISH_CATCH_SMAE==fish_trace_info->fish_kind) nFishX=41+fish_trace_info->fish_tag;//FISH_KIND_1-FISH_KIND_13
	else if(FISH_YUZHEN==fish_trace_info->fish_kind) nFishX=fish_trace_info->fish_tag;
	if(nFishX<RECORD_FISH_NUM)
	{
		g_dwFireCountX[nFishX]++;
		g_lFireScoreX[nFishX]+=bullet_info->bullet_mulriple;
	}

	//�ӵ�����4�����Ϸ�
	if(GetElapsedTm(bullet_info->dwTicktCount)>240000)//bullet_info->bCheat
	{
		//ע��������־��¼
		int lXiuZhen=0;
		if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=bullet_info->bullet_mulriple;
		g_lReturnScore+=lXiuZhen;
		m_lFireIntoSystemGold[chair_id]+=lXiuZhen;

		if( GetNeedRecordDB(server_user_item->GetGameID()) )
		{
			if((Bullet_Type_putongpao==bullet_info->bullet_kind) || (Bullet_Type_shandianpao==bullet_info->bullet_kind) || (Bullet_Type_zidanpao==bullet_info->bullet_kind) || (Bullet_Type_sanguanpao==bullet_info->bullet_kind)) lXiuZhen=0;
			else lXiuZhen=bullet_info->bullet_mulriple;

			tagCatchFishRecord FishLog;
			FishLog.dwGameID=server_user_item->GetGameID();
			FishLog.nBulletID=bullet_info->bullet_id;
			FishLog.nBulletOnlyID=bullet_info->bullet_idx_;
			FishLog.enFishKind=400;
			FishLog.lFishID=(bullet_info->bCheat)?401:400;
			FishLog.nWinScore=lXiuZhen;
			FishLog.nSoucrePro=-1.0f;
			FishLog.nChangeAddPro=-1.0f;
			FishLog.lScoreCurrent=GetMeSore(server_user_item);
			FishLog.lNowKuCun=-g_game_config.g_stock_score_old;
			CDBAction::GetInstance().CatchFish(&FishLog);//�̷ִ��� ����
		}

		return true;
	}

	//2���Ӵ���״̬����
	if(m_dwInValidPlayerTm[chair_id]!=0 && GetElapsedTm(m_dwInValidPlayerTm[chair_id])>120000)
	{
		m_dwInValidPlayerTm[chair_id]=0;
	}
	float fLimitMultiPro=1.0f;
	if(m_dwInValidPlayerTm[chair_id]!=0) fLimitMultiPro=float(g_random_int_range(90,100))/100;

	BulletKind bullet_kind=bullet_info->bullet_kind;
	int bullet_mul=bullet_info->bullet_mulriple;

	if (bullet_mul < g_game_config.min_bullet_multiple_ || bullet_mul > g_game_config.max_bullet_multiple_)
	{
		g_dwUnNomal|=0x1000;
		return true;
	}

	//�ӵ�����ۼ�
	if (!server_user_item->IsAndroidUser())
	{
		bool bStock=((Bullet_Type_putongpao==bullet_kind)||(Bullet_Type_shandianpao==bullet_kind)||(Bullet_Type_zidanpao==bullet_kind)|| (Bullet_Type_sanguanpao==bullet_kind));
		if(bStock)
		{
			StockScore(fish_trace_info->fish_kind,bullet_mul,0,true);

			g_lEnterScore+=bullet_mul;
			m_lFireNoInKuCunGold[chair_id]-=bullet_mul;
		}
	}

	//��Ϸ���֤
	if(fish_trace_info->fish_kind !=FISH_YUQUN4) return true;
	if (fish_trace_info->fish_id!=fish_trace_info->fish_king_id) return true;

	//�ͻ���Ч����ͻ ��;���ܶ���ӵ��Ѿ��ڷ��������� �����Ч����֧���л�
	if(m_TimeFree[chair_id].dwFreeTime!=0)
	{
		if(m_TimeFree[chair_id].wFireCount>35) return true;
	}

	//������ͳ�� �������� CatchGroup����Ҫ
	fish_trace_info->wFireCount[chair_id]++;

	//����ͳ��
	m_nTotalKillFishCount[chair_id]++;
	m_nKillFishCount[chair_id][fish_trace_info->fish_kind]++;

	int fish_multiple = g_game_config.fish_multiple_[fish_trace_info->fish_kind];
	SCORE fish_score = g_game_config.fish_multiple_[fish_trace_info->fish_kind] * bullet_mul;
	WORD wFishKind = fish_trace_info->fish_kind;
	if(wFishKind>=FISH_YUQUN1 && wFishKind<=FISH_YUZHEN) wFishKind=fish_trace_info->fish_tag;

	fish_multiple = g_game_config.fish_multiple_[fish_trace_info->fish_tag];
	fish_score=g_game_config.fish_multiple_[fish_trace_info->fish_tag] * bullet_mul;

	BYTE cbGiveCount=0;
	if(0==g_cbTest)
	{
		//��Ⱥ4�е�����
	    if (fish_trace_info->fish_kind == FISH_YUQUN4)
		{
			if(fish_trace_info->fish_king_id==fishking_id)
			{
				DWORD dwAllFishMul=0;
				CMD_S_CatchGroupFishKing catch_sweep_fish;
				ZeroMemory(&catch_sweep_fish,sizeof(catch_sweep_fish));

				//���¼���
				int nFirstScore = g_game_config.fish_multiple_[fish_trace_info->fish_tag] * bullet_mul;
				fish_score=nFirstScore;

				DWORD dwFishMulti=g_game_config.fish_multiple_[fish_trace_info->fish_tag];
				dwAllFishMul+=dwFishMulti;
				catch_sweep_fish.nBullet_mul=bullet_mul;
				catch_sweep_fish.nCatch_fish_count=1;
				catch_sweep_fish.nCatch_fish_id[0]=fish_trace_info->fish_id;
				catch_sweep_fish.nfish_score[0]=fish_score;
				//---
				for(int k=0;k<nCatchFishCount;k++)
				{
					FishTraceInfo* pSubFish_trace = GetFishTraceInfo(pMsg->nCatch_fish_id[k],true);
					if(!pSubFish_trace || pSubFish_trace->fish_id==fish_trace_info->fish_id)
					{
						continue;//�����Ѿ��������
					}

					if(pSubFish_trace && (pSubFish_trace->fish_kind==fish_trace_info->fish_kind || pSubFish_trace->fish_tag==fish_trace_info->fish_tag || pSubFish_trace->fish_kind==fish_trace_info->fish_tag ))
					{
						fish_score+=nFirstScore;
						int nD=catch_sweep_fish.nCatch_fish_count;
						catch_sweep_fish.nfish_score[nD]=nFirstScore;
						catch_sweep_fish.nCatch_fish_id[nD]=pSubFish_trace->fish_id;

						dwAllFishMul+=dwFishMulti;
						catch_sweep_fish.nCatch_fish_count++;

					}
				}
				//---

				bool bHitOk = NormalStockHitGroup(fish_trace_info,bullet_info,server_user_item,dwAllFishMul,fish_score,fLimitMultiPro);
				if(!bHitOk) return true;

				for (int k=0;k<catch_sweep_fish.nCatch_fish_count;k++)
				{
					FreeFishTrace(catch_sweep_fish.nCatch_fish_id[k]);
				}

				fish_score_[chair_id] += fish_score;
				g_lUserAllWin+=fish_score;
				m_lWinScore[chair_id] +=fish_score;

				catch_sweep_fish.wChairID = chair_id;
				catch_sweep_fish.lNow_fish_score = fish_score_[chair_id];
				catch_sweep_fish.lNow_user_score = GetUserGold(chair_id);
				catch_sweep_fish.nTotal_fish_score=fish_score;

				table_frame_->SendTableData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH_KING, &catch_sweep_fish, sizeof(catch_sweep_fish));
				table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_GROUP_FISH_KING, &catch_sweep_fish, sizeof(catch_sweep_fish));

				WriteScore(chair_id);
				if (!server_user_item->IsAndroidUser())
				{
					StockScore(FISH_YUQUN4,0,fish_score);
				}

				//����ʲô��
				m_nKillFishCount_Death[chair_id][FISH_YUQUN4]++;						
				m_nTotalKillFishCount_Death[chair_id]++;
				if(nFishX<RECORD_FISH_NUM)
				{
					g_dwDeadCountX[nFishX]++;
					g_lCatchFishScoreX[nFishX]+=fish_score;
				}

				return true;
			}

		}
	}

	return true;
}

//���״���
bool TableFrameSink::OnSubCatchFishTorpedo(IServerUserItem* server_user_item, CMD_C_CatchFish_Torpedo *pCatch_torpedo)
{
	//��֤�ӵ�
	WORD chair_id = server_user_item->GetChairID();
	ServerBulletInfo* pbullet_info = GetBulletInfo(chair_id, pCatch_torpedo->bullet_id);
	if (pbullet_info == NULL || Bullet_Type_yulei!=pbullet_info->bullet_kind) return true;

	//�յ���Ϣ,��ɾ���ⷢ�ӵ�...�Ժ��ٷ���ͬһ���ӵ����������Ч��...
	ServerBulletInfo bullet_info_cpy;
	memcpy(&bullet_info_cpy, pbullet_info, sizeof(bullet_info_cpy));
	FreeBulletInfo(chair_id, pbullet_info);

	m_nTempFishID[chair_id]=pCatch_torpedo->fish_id;
	BYTE cbTordedoKind = bullet_info_cpy.cbBoomKind;
	if (cbTordedoKind<=4)
	{
		if(false==server_user_item->WriteFishBombScore(cbTordedoKind, bullet_info_cpy.nLockFishID))
		{
			table_frame_->LimitGameAccount(server_user_item->GetUserID(), "");
			return true;
		}
	}

	return true;
}

//������
bool TableFrameSink::OnSubStockOperate(IServerUserItem* server_user_item, unsigned char operate_code)
{
	return true;
}

void TableFrameSink::BuildFishTrace(int fish_count, FishKind fish_kind_start, FishKind fish_kind_end)
{
	//#ifdef _DEBUG
	//	static DWORD dwTickt=0;
	//	if(GetTickCount()-dwTickt<20000) return;
	//	dwTickt=GetTickCount();
	//	fish_count=1;
	//#endif
	BYTE tcp_buffer[SOCKET_TCP_PACKET] = { 0 };
	WORD send_size = 0;
	CMD_S_FishTrace* fish_trace = reinterpret_cast<CMD_S_FishTrace*>(tcp_buffer);

	WORD wItemSize = sizeof(CMD_S_FishTrace);
	DWORD build_tick = GetTickCount();//��¼ʱ��

	int nDiWangTrace_id=g_random_int_range(0,12);
	for (int i = 0; i < fish_count; ++i)
	{
		if (send_size + wItemSize > wItemSize*1)//sizeof(tcp_buffer))//���Ͱ���С ����ͻ���json��������Ϊ1��һ��
		{
			table_frame_->SendTableData(INVALID_CHAIR, SUB_S_FISH_TRACE, tcp_buffer, send_size);
			table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_FISH_TRACE, tcp_buffer, send_size);
			send_size = 0;
			fish_trace = reinterpret_cast<CMD_S_FishTrace*>(tcp_buffer);//ע������
		}

		//��������ټ������ƥ������һ����
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = static_cast<FishKind>(fish_kind_start + (rand() + i) % (fish_kind_end - fish_kind_start + 1));
		fish_trace_info->build_tick = build_tick;

		//�� ·�� 0-11
		if(fish_trace_info->fish_kind==FISH_KIND_20) fish_trace_info->nTrace_id=g_random_int_range(0,12);
		//���� ·�� 0-11
		else if(fish_trace_info->fish_kind==FISH_KIND_24) fish_trace_info->nTrace_id=g_random_int_range(0,12);
		//����з ·�� 0-11
		else if(fish_trace_info->fish_kind==FISH_KIND_22) fish_trace_info->nTrace_id=(i==0?nDiWangTrace_id:(11-nDiWangTrace_id));
		//��צ����Ҫͬ�� 1,2,3����������
		else if(fish_trace_info->fish_kind==FISH_KIND_21)
		{
			//����������צ��
			BYTE cbNullX=0;                                                                          
			BYTE cbThree[3]={0};
			for (int y=0;y<3;y++)
			{
				if(m_cbBaZhuaYu[y]==0)
				{
					cbThree[cbNullX++]=y+1;
				}
			}
			int nTTID=cbThree[rand()%cbNullX];
			fish_trace_info->fish_tag=nTTID;
			if(nTTID>0 && nTTID<=3) m_cbBaZhuaYu[nTTID-1]=1;
		}
		//����·�� + ��ҹ���� ·�� 0-15
		else if((fish_trace_info->fish_kind>=FISH_KIND_17 && fish_trace_info->fish_kind<=FISH_KIND_19)||fish_trace_info->fish_kind==FISH_KIND_23)
		{
			fish_trace_info->nTrace_id=g_random_int_range(0,16);
		}
		else if(fish_trace_info->fish_kind>=FISH_SMALL_2_BIG1 && fish_trace_info->fish_kind<=FISH_SMALL_2_BIG3)
		{
			WORD wTraceID_=RandCardData();
			fish_trace_info->nTrace_id=m_wRandArray[wTraceID_];
		}
		else if(fish_trace_info->fish_kind==FISH_CHAIN)//ƽ�����
		{
			int fish_Tag=FISH_KIND_1;
			int nx=rand()%100;
			if(nx>=0 && nx<=40) fish_Tag=FISH_KIND_1;
			else if(nx<=50) fish_Tag=FISH_KIND_2;
			else if(nx<=60) fish_Tag=FISH_KIND_3;
			else if(nx<=70) fish_Tag=FISH_KIND_4;
			else if(nx<=80) fish_Tag=FISH_KIND_5;
			else if(nx<=88) fish_Tag=FISH_KIND_6;
			else if(nx<=92) fish_Tag=FISH_KIND_7;
			else if(nx<=96) fish_Tag=FISH_KIND_8;
			else if(nx<=99) fish_Tag=FISH_KIND_9;

			//����ų���ͬ�ŷ���
			for (int k=0;k<active_fish_trace_vector_.size();k++)
			{
				if(active_fish_trace_vector_[k]->fish_kind==FISH_CATCH_SMAE)
				{
					int nTagX=active_fish_trace_vector_[k]->fish_tag;
					if(fish_Tag==nTagX)
					{
						fish_Tag=(nTagX+1)%FISH_KIND_9;
					}
					break;
				}
			}

			WORD wTraceID_=RandCardData();
			fish_trace_info->nTrace_id=m_wRandArray[wTraceID_];

			fish_trace_info->fish_tag=fish_Tag;
		}
		else if(fish_trace_info->fish_kind==FISH_CATCH_SMAE)
		{
			int fish_Tag=FISH_KIND_1;
			int nx=g_random_int_range(0,100);
			if(nx>=0 && nx<=40) fish_Tag=FISH_KIND_1;
			else if(nx<=50) fish_Tag=FISH_KIND_2;
			else if(nx<=55) fish_Tag=FISH_KIND_3;
			else if(nx<=58) fish_Tag=FISH_KIND_4;
			else if(nx<=60) fish_Tag=FISH_KIND_5;
			else if(nx<=65) fish_Tag=FISH_KIND_6;
			else if(nx<=70) fish_Tag=FISH_KIND_7;
			else if(nx<=75) fish_Tag=FISH_KIND_8;
			else if(nx<=80) fish_Tag=FISH_KIND_9;
			else if(nx<=88) fish_Tag=FISH_KIND_10;
			else if(nx<=92) fish_Tag=FISH_KIND_11;
			else if(nx<=99) fish_Tag=FISH_KIND_12;

			//����ų���ͬ����������
			for (int k=0;k<active_fish_trace_vector_.size();k++)
			{
				if(active_fish_trace_vector_[k]->fish_kind==FISH_CHAIN)
				{
					int nTagX=active_fish_trace_vector_[k]->fish_tag;
					if(fish_Tag==nTagX)
					{
						fish_Tag=(nTagX+1)%FISH_KIND_13;
					}
					break;
				}
			}

			WORD wTraceID_=RandCardData();
			fish_trace_info->nTrace_id=m_wRandArray[wTraceID_];

			fish_trace_info->fish_tag=fish_Tag;
		}
		else if(fish_trace_info->fish_kind>=FISH_KIND_1 && fish_trace_info->fish_kind<=FISH_KIND_13)
		{
			WORD wTraceID_=RandCardData();
			fish_trace_info->nTrace_id=m_wRandArray[wTraceID_];
		}

		//��ͬ������ 10000*FishKind���� +  ����FishPath·���ļ�����id
		DWORD dwManuleTime=(fish_trace_info->fish_kind==FISH_KIND_20)?8000:20000;
		//��צ��ͻ���������������ȴ�10s
		if(FISH_KIND_21==fish_trace_info->fish_kind) dwManuleTime=30000;
		FishMoveManage.ActiveFish(fish_trace_info->fish_id,fish_trace_info->nTrace_id,dwManuleTime);//������ ��20s

		fish_trace->nFish_id= fish_trace_info->fish_id;
		fish_trace->nTrace_id = fish_trace_info->nTrace_id;
		fish_trace->nFish_Tag = fish_trace_info->fish_tag;
		fish_trace->fish_kind = fish_trace_info->fish_kind;

		//if(FISH_KIND_14<=fish_trace->fish_kind && FISH_KIND_16>=fish_trace->fish_kind)
		//{
		//	_tprintf("new fish id:%d   path:%d\n",fish_trace->nFish_id,fish_trace->nTrace_id);
		//}
		//if(FISH_KIND_21==fish_trace->fish_kind)
		//{
		//	_tprintf("new ��צ�� id:%d   path:%d\n",fish_trace->nFish_id,fish_trace->nTrace_id);
		//}
		//_tprintf("new fish id:%d   path:%d\n",fish_trace->nFish_id,fish_trace->nTrace_id);
		send_size += wItemSize;
		++fish_trace;
	}

	if (send_size > 0)
	{
		table_frame_->SendTableData(INVALID_CHAIR, SUB_S_FISH_TRACE, tcp_buffer, send_size);
		table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_FISH_TRACE, tcp_buffer, send_size);
	}
}

//��������1
void TableFrameSink::BuildSceneKind(bool bNoYuZhen)
{
	//���㷽��
	m_GameState=en_S_NewAcitveFish;
	WORD wSenceID = now_acitve_scene_kind_;
	ZeroMemory(m_distribulte_fish,sizeof(m_distribulte_fish));
	ZeroMemory(m_cbBaZhuaYu,sizeof(m_cbBaZhuaYu));
	ZeroMemory(m_dwToolFishTm,sizeof(m_dwToolFishTm));

	if(bNoYuZhen==false)
	{
		//����
		if(1)//rand()%100==0) //��������
		{
			BuildFishYuZhen();
			return;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/*
	��A��---���ռ��ʱ��---
	��B��---���+���ʱ�����+�����Ŀ����---
	��C��---�ض����� ����������ʱ�� һ��Ϊ20s ��ϡ�B������� �����Ͳ����������Ŀ��ƣ�����϶�����

			��boss:		1-3�� (һ��ֻ1��) ��+4��������  ��C��  �ų�[�� ��צ��]
			Сboss��	1��   FISH_KIND_17 or FISH_KIND_19  ��C��

			�ŷ磺		1�� �������пɱ䡿 ��C��
			���磺		1�� �������пɱ䡿 ��C��
			�����㣺	1��	3����������    ��C��
			
			�޴��㣺	1-4�� �������й̶���FISH_SMALL_2_BIG1 - FISH_SMALL_2_BIG3 	��B��
			
			���㣺		���1-4�� ������ͬʱ�����֡� ÿ��������� FISH_KIND_10-FISH_KIND_13	��B��

			----����������------------------------------------

			С�㣺		FISH_KIND_1-FISH_KIND_9 ��B��
			��Ⱥ��		��A��

			----ͨ��ʱ��+�������뼴��-------------------------

			==================================================
			����ʱ ȫ���ֹ��������ʱ������+rand()%5 ��ʼ����tagNewFishInfo
			==================================================
	*/
	
	WORD wTmpIndex=0;

	//Сboss FISH_KIND_17-FISH_KIND_19 ��ð�ʱ��
	wTmpIndex=FISH_KIND_17+rand()%3;
	m_distribulte_fish[wTmpIndex].cbCanNewTag=1;

	//������ FISH_CATCH_SMAE ��������Ƿ�������
	m_distribulte_fish[FISH_CATCH_SMAE].cbCanNewTag=1;

	//������ FISH_CHAIN ������ͻ�����ʾ��bug
	m_distribulte_fish[FISH_CHAIN].cbCanNewTag=1;

	//������ ���ֵ�����ֻ��ͬʱһ�� FISH_KIND_14-FISH_KIND_16 ������
	wTmpIndex=rand()%3;
	m_cbNowToolFish=wTmpIndex;
	m_distribulte_fish[FISH_KIND_14].cbCanNewTag=1;
	m_distribulte_fish[FISH_KIND_15].cbCanNewTag=1;
	m_distribulte_fish[FISH_KIND_16].cbCanNewTag=1;
	m_distribulte_fish[FISH_KIND_14].distribute_elapsed_=0;
	m_distribulte_fish[FISH_KIND_15].distribute_elapsed_=0;
	m_distribulte_fish[FISH_KIND_16].distribute_elapsed_=0;
	m_distribulte_fish[FISH_KIND_14+wTmpIndex].distribute_elapsed_=rand()%kNormolFishTime+10;

	//�޴��� 1-4��
	m_distribulte_fish[FISH_SMALL_2_BIG1+rand()%3].cbCanNewTag=1;

	//���� �������
	BYTE cbRand=rand()%3+1;
	for (int i=0;i<cbRand;i++)
	{
		WORD wIndex=FISH_KIND_10+(i+cbRand)%4;
		m_distribulte_fish[wIndex].cbCanNewTag=1;
	}

	//��Ⱥ FISH_YUQUN1-FISH_YUQUN2 ��ʱ��� ������FishTraceInfo�����ֶ�
	m_distribulte_fish[FISH_YUQUN1].cbCanNewTag=1;
	m_distribulte_fish[FISH_YUQUN2].cbCanNewTag=1;
	m_distribulte_fish[FISH_YUQUN3].cbCanNewTag=1;
	m_distribulte_fish[FISH_YUQUN4].cbCanNewTag=1;
	m_distribulte_fish[FISH_YUQUN5].cbCanNewTag=1;

	//��ͨ�� ������������
	for (int i=FISH_KIND_1;i<=FISH_KIND_9;i++)
	{
		m_distribulte_fish[i].cbCanNewTag=1;
	}
	//////////////////////////////////////////////////////////////////////////
	//��ͨ����
	switch(wSenceID)
	{
	case SCENE_KIND_1:
		{
			m_distribulte_fish[FISH_KIND_20].cbCanNewTag=1;
		}
		break;
	case SCENE_KIND_2:
		{
			m_distribulte_fish[FISH_KIND_20].cbCanNewTag=1;
		}
		break;
	case SCENE_KIND_3:	//��ҹ����
		{
			//��boss
			m_distribulte_fish[FISH_KIND_23].cbCanNewTag=1;
		}
		break;
	case SCENE_KIND_4:	//����з
		{
			//��boss
			m_distribulte_fish[FISH_KIND_22].cbCanNewTag=1;
		}
		break;
	case SCENE_KIND_5:	//ʷǰ����
		{
			//��boss
			m_distribulte_fish[FISH_KIND_24].cbCanNewTag=1;
		}
		break;
	case SCENE_KIND_6:	//���צ��
		{
			//��boss
			m_distribulte_fish[FISH_KIND_21].cbCanNewTag=1;
		}
		break;
	default:
		{
			ASSERT(FALSE && "û�еĳ���");
			return;
		}
	}

	return;
}

void TableFrameSink::BuildFishYuQun(WORD wYuQunStyleID)
{
	//����׶β��ܳ���Ⱥ
	if(m_cbNowYuZhenID!=0xff) return;

	//_tprintf("׼��������qun %ld\n",GetTickCount());
	int nSize=g_game_config.m_ConfigYuQun.size();
	if(nSize<=0) return;
	if(wYuQunStyleID>=nSize) return;

	tagYuQun *pData = g_game_config.m_ConfigYuQun[wYuQunStyleID];
	if(pData->wFishCount<=0) return;

	//��ʼ������
	CMD_S_FishTraceYuQun TraceStyle;
	ZeroMemory(&TraceStyle,sizeof(TraceStyle));
	DWORD build_tick = GetTickCount();

	int nRandomTemp=g_random_int_range(0,6);
	int nYuQun3FishKind=nRandomTemp>=3 ? FISH_KIND_3:FISH_KIND_1;
	int nYuQun4FishKind=FISH_KIND_1;
	if(nRandomTemp<=1)
	{
		nYuQun4FishKind=FISH_KIND_1;
	}
	else if(nRandomTemp<=3 && nRandomTemp>1)
	{
		nYuQun4FishKind=FISH_KIND_3;
	}
	else
	{
		nYuQun4FishKind=FISH_KIND_4;
	}
	int nYuKingId=g_random_int_range(40,60);

	for (int i = 0; i < pData->wFishCount; ++i)
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->build_tick = build_tick;
		if(i==0)//��ʼID
		{
			TraceStyle.nStartFishID=fish_trace_info->fish_id;
		}

		TraceStyle.wStyleID=pData->nStyleID;
        
		WORD wTraceID_=RandCardData();
		TraceStyle.nTraceID=m_wRandArray[wTraceID_];

		switch(wYuQunStyleID)
		{
		case 0:
			{
				TraceStyle.wFishKind=FISH_KIND_1;
				break;
			}
		case 1:
			{
				TraceStyle.wFishKind=FISH_KIND_4;
				break;
			}
		case 2:
			{
				TraceStyle.wFishKind=FishKind(nYuQun3FishKind);
				TraceStyle.nTraceID=g_random_int_range(0,6);
				break;
			}
		case 3:
			{
				TraceStyle.wFishKind=FishKind(nYuQun4FishKind);
				TraceStyle.nTraceID=g_random_int_range(0,3);
				break;
			}
		case 4:
			{
				TraceStyle.wFishKind=FISH_KIND_1;
				TraceStyle.nTraceID=0;
				break;
			}
		}

		fish_trace_info->fish_tag=TraceStyle.wFishKind;

		if(wYuQunStyleID==4)
		{
			fish_trace_info->fish_kind = (FishKind)(FISH_YUQUN1+5);
		}
		else
		{
			fish_trace_info->fish_kind = (FishKind)(FISH_YUQUN1+wYuQunStyleID);
		}

		if(wYuQunStyleID==3)
		{
			if(i==nYuKingId)//����40�������ļ���ȷ���˻���60���㣬�ٿͻ��˵ı������ٵ�3Ȧ���ϣ�֮���Է��ڵ�3Ȧ����Ϊ�˱�֤�ͻ��˵��㶼������ſ��Դ�����������Ȼ����˴����ͻ��˵����ݿͻ����Ҳ�����û��������
			{
				TraceStyle.wFishKing=FISH_YUQUN4_KING;
				TraceStyle.nFishKingID=nYuKingId;
				fish_trace_info->fish_king_id=fish_trace_info->fish_id;
			}
		}
		else
		{
			if(i==0)
			{
				TraceStyle.wFishKing=TraceStyle.wFishKind;
			}
		}

		//��ͬ������ 10000*FishKind���� +  ����FishPath·���ļ�����id
		FishMoveManage.ActiveFish(fish_trace_info->fish_id,TraceStyle.nTraceID,21000);//������+���� ��20s
	}
	//////////////////////////////////////////////////////////////////////////

	table_frame_->SendTableData(INVALID_CHAIR, SUB_S_FISH_TRACE_YUQUN, &TraceStyle, sizeof(CMD_S_FishTraceYuQun));
	table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_FISH_TRACE_YUQUN, &TraceStyle, sizeof(CMD_S_FishTraceYuQun));

	//_tprintf("������qun ok\n");
}

void TableFrameSink::BuildFishYuZhen()
{
	//_tprintf("׼���������� %ld\n",GetTickCount());

	int nSize=g_game_config.m_ConfigYuZhen.size();
	if(nSize<=0)
	{
		_tprintf("����û�з��ɹ�\n");//�������ȥ��
		m_cbNowYuZhenID=0xff;
		return;
	}

	BYTE cbYuZhenID=rand()%nSize;
	//���Դ����begin
	static BYTE cbR=0;
	cbYuZhenID=cbR;
	cbR=(cbR+1)%nSize;
	
	if(g_cbTestStyle!=255)
	{
		for (int i=0;i<nSize;i++)
		{
			if(g_game_config.m_ConfigYuZhen[i]->nStyleID==g_cbTestStyle)
			{
				cbYuZhenID=i;
				break;
			}
		}
	}
	//���Դ����over
	//
	tagYuZhen *pData = g_game_config.m_ConfigYuZhen[cbYuZhenID];

	//��ʼ������
	CMD_S_FishTraceYuZhen TraceStyle;
	ZeroMemory(&TraceStyle,sizeof(TraceStyle));
	TraceStyle.wStyleID=pData->nStyleID;//��ʵ����������ID

	//������������
	int nFirstID=-1;
	int nLastID=0;
	int nRealCount=0;
	DWORD build_tick = GetTickCount();
	for (int i=0;i<pData->ArrayKind.size();i++)
	{
		DWORD dwRegion=pData->ArrayRegion[i];
		DWORD wMinID = (dwRegion&0xFFFF0000)>>16;
		DWORD wMaxID = (dwRegion&0x0000FFFF);
		FishKind fishKind=(FishKind)(pData->ArrayKind[i]);

		if(wMinID>wMaxID)
		{
			_tprintf("���ô���\n");//�������ȥ��
			break;
		}
		for (int j=wMinID;j<=wMaxID;j++)
		{
			FishTraceInfo* fish_trace_info = ActiveFishTrace();
			if(fish_trace_info==0) continue;
			nRealCount++;
			nLastID=fish_trace_info->fish_id;
			if(nFirstID==-1) nFirstID=fish_trace_info->fish_id;

			fish_trace_info->fish_kind = FISH_YUZHEN;
			fish_trace_info->fish_tag = fishKind;
			fish_trace_info->build_tick = build_tick;

			//��ͬ������ 10000*FishKind���� +  ����FishPath·���ļ�����id
			FishMoveManage.ActiveFish(fish_trace_info->fish_id,-1,(pData->dwAllLifeTime+5)*1000);
		}
	}
	TraceStyle.nStartFishID=nFirstID;

	//_tprintf("��������%d  ��id��%d,%d����ʵ�ʳɹ���%d\n",TraceStyle.wStyleID,TraceStyle.nStartFishID,nLastID,nRealCount);//�������ȥ��
	table_frame_->SendTableData(INVALID_CHAIR, SUB_S_FISH_TRACE_YUZHEN, &TraceStyle, sizeof(CMD_S_FishTraceYuZhen));
	table_frame_->SendLookonData(INVALID_CHAIR, SUB_S_FISH_TRACE_YUZHEN, &TraceStyle, sizeof(CMD_S_FishTraceYuZhen));

	m_cbNowYuZhenID=cbYuZhenID;
	m_dwChangeScenceYuZhenTm=0;
	ZeroMemory(m_distribulte_fish,sizeof(m_distribulte_fish));
}

//�������ټ�
void TableFrameSink::OnTimeClearFishTrace()
{
	DWORD now_tick = GetTickCount();
	std::vector<FishTraceInfo*>::iterator iter;
	for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end();)
	{
		FishTraceInfo* fish_trace_info = *iter;
		DWORD dwStayTm=kFishAliveTime;
		if(fish_trace_info->fish_kind==FISH_YUZHEN) dwStayTm=180000;
		if(GetElapsedTm(fish_trace_info->build_tick)>=dwStayTm)
		{
			FishMoveManage.DeleteFish(fish_trace_info->fish_id);
			if(fish_trace_info->fish_kind==FISH_KIND_21)
			{
				int nTTID=fish_trace_info->fish_tag;
				if(nTTID>0 && nTTID<=3) m_cbBaZhuaYu[nTTID-1]=0;
				//_tprintf("xxxx��ʱ������ ��צ�� id:%d\n",fish_trace_info->fish_id);
			}
			iter = active_fish_trace_vector_.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

//�����ȫ����
void TableFrameSink::CalcScore(IServerUserItem* server_user_item)
{
	if (server_user_item == NULL) return;
	WORD chair_id = server_user_item->GetChairID();

	LONGLONG lScore = (fish_score_[chair_id] - exchange_fish_score_[chair_id]) * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;

	LONGLONG lChangeScore = lScore - m_lWriteScore[chair_id];

	DWORD dwPassTime = (DWORD)time(NULL) - m_dwSitTime[chair_id];
	m_dwSitTime[chair_id] = (DWORD)time(NULL);
	g_lWriteToDBScore+=lChangeScore;
	table_frame_->WriteUserScore(server_user_item, lChangeScore, 0, lChangeScore > 0 ? enScoreKind_Win : enScoreKind_Lost, dwPassTime, 0);

	m_lWriteScore[chair_id] = 0;

	fish_score_[chair_id] = 0;
	exchange_fish_score_[chair_id] = 0;
}

//200w�ڲ�ֵд��
void TableFrameSink::WriteScore(WORD chair_id)
{
	LONGLONG lScore = (fish_score_[chair_id] - exchange_fish_score_[chair_id]) * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;

	LONGLONG lChangeScore = lScore - m_lWriteScore[chair_id];
	int nBeilv = g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;
	LONGLONG lChangeScoreCfg = 2000000 * nBeilv;

	if(lChangeScore > lChangeScoreCfg || lChangeScore < -lChangeScoreCfg)
	{
		//
		if (lChangeScore >= 20000000 * nBeilv || lChangeScore <= -20000000 * nBeilv)
		{
			IServerUserItem * pIServerUserItem = table_frame_->GetServerUserItem(chair_id);
			if (pIServerUserItem != NULL)
			{
				CString strLog;
				strLog.Format("pIServerUserItem->GetAccounts()=%s, pIServerUserItem->GetUserID()=%d,pIServerUserItem->GetChairID()=%d,chair_id=%d,lChangeScore=%I64d",
					pIServerUserItem->GetAccounts(), pIServerUserItem->GetUserID(), pIServerUserItem->GetChairID(), chair_id, lChangeScore);
				CTraceService::TraceString(strLog, TraceLevel_Exception);
			}
			else
			{
				CString strLog;
				strLog.Format("chair_id=%d,lChangeScore=%I64d", chair_id, lChangeScore);
				CTraceService::TraceString(strLog, TraceLevel_Exception);
			}

			return;
		}

		DWORD dwPassTime = (DWORD)time(NULL) - m_dwSitTime[chair_id];
		m_dwSitTime[chair_id] = (DWORD)time(NULL);

		g_lWriteToDBScore+=lChangeScore;
		table_frame_->WriteUserScore(chair_id, lChangeScore, 0, lChangeScore > 0 ? enScoreKind_Win : enScoreKind_Lost, dwPassTime);
		m_lWriteScore[chair_id] += lChangeScore;
	}
}

//�ֶλ���
WORD TableFrameSink::RandCardData()
{
	if(m_wRandPathID==0)
	{
		WORD wRandTmp[PATH_SHARE_NUM];
		for (int i=0;i<PATH_SHARE_NUM;i++)
		{
			wRandTmp[i]=i;
		}

		WORD cbRegion=PATH_SHARE_NUM/6*5;

		//�ֶδ���
		WORD cbCardCount=cbRegion;
		if(cbCardCount>0)
		{
			WORD cbRandData[PATH_SHARE_NUM]={0};

			//����׼��
			WORD cbCardDataTemp[PATH_SHARE_NUM];
			CopyMemory(cbCardDataTemp,wRandTmp,sizeof(WORD)*cbCardCount);

			//�����˿�
			WORD cbRandCount=0,cbPosition=0;
			do
			{
				cbPosition=rand()%(cbCardCount-cbRandCount);
				cbRandData[cbRandCount++]=cbCardDataTemp[cbPosition];
				cbCardDataTemp[cbPosition]=cbCardDataTemp[cbCardCount-cbRandCount];
			} while (cbRandCount<cbCardCount);

			CopyMemory(m_wRandArray,cbRandData,sizeof(WORD)*cbCardCount);
		}
		cbCardCount=PATH_SHARE_NUM-cbRegion;
		if(cbCardCount>0)
		{
			WORD cbRandData[PATH_SHARE_NUM]={0};

			//����׼��
			WORD cbCardDataTemp[PATH_SHARE_NUM];
			CopyMemory(cbCardDataTemp,&(wRandTmp[cbRegion]),sizeof(WORD)*cbCardCount);

			//�����˿�
			WORD cbRandCount=0,cbPosition=0;
			do
			{
				cbPosition=rand()%(cbCardCount-cbRandCount);
				cbRandData[cbRandCount++]=cbCardDataTemp[cbPosition];
				cbCardDataTemp[cbPosition]=cbCardDataTemp[cbCardCount-cbRandCount];
			} while (cbRandCount<cbCardCount);

			CopyMemory(&(m_wRandArray[cbRegion]),cbRandData,sizeof(WORD)*cbCardCount);
		}

		m_wRandPathID=1;
		return 0;
	}

	WORD wRet=m_wRandPathID;
	m_wRandPathID=(m_wRandPathID+1)%PATH_SHARE_NUM;

	return wRet;
}

void TableFrameSink::DeleteLogFile(LPCTSTR lpPathName)
{
	if(lpPathName==0) return;

	CFileFind finder;
	CString tempPath;
	tempPath.Format("%s%s", lpPathName, "//*.*");
	BOOL bWork = finder.FindFile(tempPath);
	while(bWork)
	{		
		bWork = finder.FindNextFile();
		if(!finder.IsDots())
		{
			if(finder.IsDirectory())
			{
				DeleteLogFile(finder.GetFilePath());
			}
			else
			{
				DeleteFile(finder.GetFilePath());
			}
		}
	}
	finder.Close();
	RemoveDirectory(lpPathName);
}

LONGLONG TableFrameSink::GetMeSore(IServerUserItem *pServerUserItem)
{
	WORD chair_id = pServerUserItem->GetChairID();
	if (chair_id == INVALID_CHAIR) return -1;

	LONGLONG lScore = (fish_score_[chair_id] - exchange_fish_score_[chair_id]) * g_game_config.exchange_ratio_userscore_ / g_game_config.exchange_ratio_fishscore_;

	LONGLONG lChangeScore = lScore - m_lWriteScore[chair_id];

	return pServerUserItem->GetUserScore()->lScore + lChangeScore;
}

//�Ƿ���Ҫsqlite3��ס
bool TableFrameSink::GetNeedRecordDB(DWORD dwGameID)
{
	if(1 == g_nLogOutDB) return true;
	//�ر�ע�� ���ϲ����׿���
#ifdef _DEBUG
	return true;
#endif

	for (int i=0;i<g_RecordGameID.size();i++)
	{
		if(g_RecordGameID[i]==dwGameID) return true;
	}

	return false;
}

LONGLONG TableFrameSink::OnGetTorpedoScore(BYTE cbBoomKind, LONGLONG lTorpedoScore, DWORD dwTorpedoCount)
{
	if(dwTorpedoCount<=0) return 0;
	int nBoomGoldKind[5]={5,10,20,50,100};
	LONGLONG lCellNum=nBoomGoldKind[cbBoomKind];

	//�ҳ���ֵ0.8 1.2ת��Ϊ��� �������
	LONGLONG lMinGold=(LONGLONG)lCellNum*9200*dwTorpedoCount;
	LONGLONG lMaxGold=(LONGLONG)lCellNum*10800*dwTorpedoCount;
	LONGLONG lRetGold=(LONGLONG)lCellNum*10000;

	if(lTorpedoScore>=lMaxGold)
	{
		return lRetGold*108/100;
	}
	else if(lTorpedoScore<=lMinGold)
	{
		//��Ӽ���֮ǰ��0.2����
		if(lTorpedoScore<lMinGold) return lRetGold*80/100;

		return lRetGold*92/100;
	}
	else
	{
		lRetGold = lRetGold*(92+rand()%17)/100;

		if(lTorpedoScore-lRetGold<=(LONGLONG)lCellNum*9200*(dwTorpedoCount-1))
		{
			lRetGold=lCellNum*10000*92/100;
		}
		if(lTorpedoScore-lRetGold>=(LONGLONG)lCellNum*10800*(dwTorpedoCount-1))
		{
			lRetGold=lCellNum*10000*108/100;
		}
	}

	//���һ��ȫ��ʹ��
	if(lTorpedoScore-lRetGold<=lCellNum*10000*8/100)
	{
		lRetGold = lTorpedoScore;
	}

	return lRetGold;
}