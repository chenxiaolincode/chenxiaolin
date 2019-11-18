#include "StdAfx.h"
#include "table_frame_sink.h"
#include "game_service_manager.h"
//#include "android_useritem_sink.h"
#include "DBAction.h"
//////////////////////////////////////////////////////////////////////////

//ȫ�ֱ���
static CGameServiceManager			g_GameServiceManager;				//�������

//////////////////////////////////////////////////////////////////////////

//���캯��
CGameServiceManager::CGameServiceManager(void)
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 

	//_CrtSetBreakAlloc(20022); 
#endif

	//��������
	m_GameServiceAttrib.wKindID=KIND_ID;
	m_GameServiceAttrib.wChairCount=GAME_PLAYER;
	lstrcpyn(m_GameServiceAttrib.szKindName,GAME_NAME,CountArray(m_GameServiceAttrib.szKindName));
	lstrcpyn(m_GameServiceAttrib.szDescription,TEXT("����2��Ϸ�������"),CountArray(m_GameServiceAttrib.szDescription));
	m_GameServiceAttrib.cbJoinInGame = FALSE;

	lstrcpyn(m_GameServiceAttrib.szDataBaseName,TEXT( "QPTreasureDB" ),CountArray(m_GameServiceAttrib.szDataBaseName));	
	lstrcpyn(m_GameServiceAttrib.szClientModuleName,TEXT("hw2Fish.exe"),CountArray(m_GameServiceAttrib.szClientModuleName));
	lstrcpyn(m_GameServiceAttrib.szServerModuleName,TEXT("hw2Fish_server.dll"),CountArray(m_GameServiceAttrib.szServerModuleName));

	return;
}

//��������
CGameServiceManager::~CGameServiceManager(void)
{
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	LOG_FILE_X.m_bNowPrintf=true;
	LOG_FILE_EX_X.m_bNowPrintf=true;
	LOG_FILE_TWO("��������");
	LOG_FILE_EX_TWO("��������");

	CDBAction::GetInstance().Close();
}

//�ӿڲ�ѯ
void * __cdecl CGameServiceManager::QueryInterface(const IID & Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IGameServiceManager,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IGameServiceManager,Guid,dwQueryVer);
	return NULL;
}

//������Ϸ��
void * __cdecl CGameServiceManager::CreateTableFrameSink(const IID & Guid, DWORD dwQueryVer)
{

	//��������
	TableFrameSink * pTableFrameSink=NULL;
	try
	{
		pTableFrameSink=new TableFrameSink();
		if (pTableFrameSink==NULL) throw TEXT("����ʧ��");
		void * pObject=pTableFrameSink->QueryInterface(Guid,dwQueryVer);
		if (pObject==NULL) throw TEXT("�ӿڲ�ѯʧ��");
		return pObject;
	}
	catch (...) {}

	//�������
	SafeDelete(pTableFrameSink);

	return NULL;
}

//��������
VOID * __cdecl CGameServiceManager::CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer)
{
	////��������
	//CAndroidUserItemSink * pAndroidUserItemSink=NULL;

	//try
	//{
	//	//��������
	//	pAndroidUserItemSink=new CAndroidUserItemSink();
	//	if (pAndroidUserItemSink==NULL) throw TEXT("����ʧ��");

	//	//��ѯ�ӿ�
	//	void * pObject=pAndroidUserItemSink->QueryInterface(Guid,dwQueryVer);
	//	if (pObject==NULL) throw TEXT("�ӿڲ�ѯʧ��");

	//	return pObject;
	//}
	//catch (...) {}

	////ɾ������
	//SafeDelete(pAndroidUserItemSink);

	return NULL;
}


//��ȡ����
void __cdecl CGameServiceManager::GetGameServiceAttrib(tagGameServiceAttrib & GameServiceAttrib)
{
	GameServiceAttrib=m_GameServiceAttrib;
	return;
}

//�����޸�
bool __cdecl CGameServiceManager::RectifyServiceOption(tagGameServiceOption * pGameServiceOption)
{
	//Ч�����
	ASSERT(pGameServiceOption!=NULL);
	if (pGameServiceOption==NULL) return false;

	//�������ݿ�
	SYSTEMTIME sys; 
	GetLocalTime(&sys);
	CDBAction::GetInstance().SetService(pGameServiceOption->wKindID, pGameServiceOption->wServerID, sys);

	//��Ԫ����
	pGameServiceOption->lCellScore=__max(1L,pGameServiceOption->lCellScore);

	//��������
	if (pGameServiceOption->wServerType==GAME_GENRE_GOLD)
	{
		/*pGameServiceOption->lLessScore=__max(pGameServiceOption->lCellScore*32L,pGameServiceOption->lLessScore);*/
	}

	//��������
	//if (pGameServiceOption->lRestrictScore!=0L)
	//{
	//	pGameServiceOption->lRestrictScore=__max(pGameServiceOption->lRestrictScore,pGameServiceOption->lLessScore);
	//}

	return true;
}


//////////////////////////////////////////////////////////////////////////

//����������
extern "C" __declspec(dllexport) void * __cdecl CreateGameServiceManager(const GUID & Guid, DWORD dwInterfaceVer)
{
	return g_GameServiceManager.QueryInterface(Guid,dwInterfaceVer);
}

//////////////////////////////////////////////////////////////////////////
