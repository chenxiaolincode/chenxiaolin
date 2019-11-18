#include "StdAfx.h"
#include "table_frame_sink.h"
#include "game_service_manager.h"
//#include "android_useritem_sink.h"
#include "DBAction.h"
//////////////////////////////////////////////////////////////////////////

//全局变量
static CGameServiceManager			g_GameServiceManager;				//管理变量

//////////////////////////////////////////////////////////////////////////

//构造函数
CGameServiceManager::CGameServiceManager(void)
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 

	//_CrtSetBreakAlloc(20022); 
#endif

	//设置属性
	m_GameServiceAttrib.wKindID=KIND_ID;
	m_GameServiceAttrib.wChairCount=GAME_PLAYER;
	lstrcpyn(m_GameServiceAttrib.szKindName,GAME_NAME,CountArray(m_GameServiceAttrib.szKindName));
	lstrcpyn(m_GameServiceAttrib.szDescription,TEXT("海王2游戏服务组件"),CountArray(m_GameServiceAttrib.szDescription));
	m_GameServiceAttrib.cbJoinInGame = FALSE;

	lstrcpyn(m_GameServiceAttrib.szDataBaseName,TEXT( "QPTreasureDB" ),CountArray(m_GameServiceAttrib.szDataBaseName));	
	lstrcpyn(m_GameServiceAttrib.szClientModuleName,TEXT("hw2Fish.exe"),CountArray(m_GameServiceAttrib.szClientModuleName));
	lstrcpyn(m_GameServiceAttrib.szServerModuleName,TEXT("hw2Fish_server.dll"),CountArray(m_GameServiceAttrib.szServerModuleName));

	return;
}

//析构函数
CGameServiceManager::~CGameServiceManager(void)
{
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	LOG_FILE_X.m_bNowPrintf=true;
	LOG_FILE_EX_X.m_bNowPrintf=true;
	LOG_FILE_TWO("总输出完毕");
	LOG_FILE_EX_TWO("总输出完毕");

	CDBAction::GetInstance().Close();
}

//接口查询
void * __cdecl CGameServiceManager::QueryInterface(const IID & Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IGameServiceManager,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IGameServiceManager,Guid,dwQueryVer);
	return NULL;
}

//创建游戏桌
void * __cdecl CGameServiceManager::CreateTableFrameSink(const IID & Guid, DWORD dwQueryVer)
{

	//建立对象
	TableFrameSink * pTableFrameSink=NULL;
	try
	{
		pTableFrameSink=new TableFrameSink();
		if (pTableFrameSink==NULL) throw TEXT("创建失败");
		void * pObject=pTableFrameSink->QueryInterface(Guid,dwQueryVer);
		if (pObject==NULL) throw TEXT("接口查询失败");
		return pObject;
	}
	catch (...) {}

	//清理对象
	SafeDelete(pTableFrameSink);

	return NULL;
}

//创建机器
VOID * __cdecl CGameServiceManager::CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer)
{
	////变量定义
	//CAndroidUserItemSink * pAndroidUserItemSink=NULL;

	//try
	//{
	//	//建立对象
	//	pAndroidUserItemSink=new CAndroidUserItemSink();
	//	if (pAndroidUserItemSink==NULL) throw TEXT("创建失败");

	//	//查询接口
	//	void * pObject=pAndroidUserItemSink->QueryInterface(Guid,dwQueryVer);
	//	if (pObject==NULL) throw TEXT("接口查询失败");

	//	return pObject;
	//}
	//catch (...) {}

	////删除对象
	//SafeDelete(pAndroidUserItemSink);

	return NULL;
}


//获取属性
void __cdecl CGameServiceManager::GetGameServiceAttrib(tagGameServiceAttrib & GameServiceAttrib)
{
	GameServiceAttrib=m_GameServiceAttrib;
	return;
}

//参数修改
bool __cdecl CGameServiceManager::RectifyServiceOption(tagGameServiceOption * pGameServiceOption)
{
	//效验参数
	ASSERT(pGameServiceOption!=NULL);
	if (pGameServiceOption==NULL) return false;

	//创建数据库
	SYSTEMTIME sys; 
	GetLocalTime(&sys);
	CDBAction::GetInstance().SetService(pGameServiceOption->wKindID, pGameServiceOption->wServerID, sys);

	//单元积分
	pGameServiceOption->lCellScore=__max(1L,pGameServiceOption->lCellScore);

	//积分下限
	if (pGameServiceOption->wServerType==GAME_GENRE_GOLD)
	{
		/*pGameServiceOption->lLessScore=__max(pGameServiceOption->lCellScore*32L,pGameServiceOption->lLessScore);*/
	}

	//积分上限
	//if (pGameServiceOption->lRestrictScore!=0L)
	//{
	//	pGameServiceOption->lRestrictScore=__max(pGameServiceOption->lRestrictScore,pGameServiceOption->lLessScore);
	//}

	return true;
}


//////////////////////////////////////////////////////////////////////////

//建立对象函数
extern "C" __declspec(dllexport) void * __cdecl CreateGameServiceManager(const GUID & Guid, DWORD dwInterfaceVer)
{
	return g_GameServiceManager.QueryInterface(Guid,dwInterfaceVer);
}

//////////////////////////////////////////////////////////////////////////
