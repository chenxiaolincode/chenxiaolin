#pragma once

#include <list>
#include <vector>

class TableFrameSink;//����ֱ�ӻص� ����̳з�ʽ
//////////////////////////////////////////////////////////////////////////

enum PathSyle
{
	en_Small=0,
	en_Big,
	en_Huge,
	en_Special,
	en_PathCount,
};

struct tagFishPath
{
	WORD										wPathID;									//�켣ID �㹻 ���ֲ���
	DWORD										dwTotalTickt;								//�̶����� ����������������ȥһ��ʱ��
	std::vector<DWORD>							TwoPoint;									//x==(TwoPoint>>16) y=(TwoPoint&0x0000FFFF)
};
typedef std::vector<tagFishPath *> ArrayFishPath;

class CPathShare
{
private:
	BYTE										m_cbNowReadStyle;							//��ǰ��ȡ����

public:
	ArrayFishPath								m_FishPathPoints[4];						//�켣�㼯�� 0 small 1big 2huge 3special

private:
	CPathShare();
	~CPathShare();

	//�ڲ�����
private:
	//ɨ��ָ���ļ�
	void ScanALlFile(LPCTSTR lpPathName);
	//�������
	bool LoadOnePathItem(LPCTSTR lpFileName);

public:
	bool LoadAllPath();
	tagFishPath * GetAllPointCount(PathSyle pathstyle, WORD wPathID);
	static CPathShare &GetInstance();
};
//////////////////////////////////////////////////////////////////////////

#define											MAX_CONTAIN_FISH_NUM	1200						//����� ���������������
//���ƶ�����


//��С���ṹ�� ģ����� ����Ҫ����FishStyle��FishGold����Ϣ
struct tagFishMove
{
	int											nID;											//��ID
	int											nPathID;										//-1û�� athSyle=x/10000 PathID=x%1000
	DWORD										dwActiveTime;									//����ʱ��
	DWORD										dwPathLineTime;									//�ܹ�����ʱ��
	//WORD										wRegion
};

typedef std::list<tagFishMove *> ArrayFishMove;
typedef std::list<tagFishMove *>::iterator IteratorFish;

//����ʵ�ֵ������� ȥ��Start��Update
class CFishMoveManage
{
public:
	int											m_nActiveIndex;									//��ѵ����
	ArrayFishMove								m_ActiveFish;									//�����
	tagFishMove									m_FishMoveData[MAX_CONTAIN_FISH_NUM];			//���500����

public:
	CFishMoveManage(void);
	~CFishMoveManage(void);

	//nPathID--��16λ�켣���� ��16λ�켣ID -1û·�� dwManueTimeǰ��û��·��ǰ��д
	bool ActiveFish(int nFishID, int nPathID, DWORD dwManueTime=0);								//��������Ϣ
	bool DeleteFish(int nFishID);																//-1 ȫ������
	int Update(int nDelFishID[], int &nDelCount);												//����ά��
	tagFishMove *GetFish(int nFishID);
};


//////////////////////////////////////////////////////////////////////////
//���淴�� ����Ҫ��С��� ������Ҫ�ۼ����
#define											MAX_CONTAIN_BULLET_NUM	500						//����ӵ�ÿ�����300


//�ӵ�����
struct tagBulletMove
{
	//����Ԫ�� �ڴ滻Ч��
	int											nID;
	WORD										wChairID;
	float										fSpeed;											//�����ٶ�

	//������ʱ���յ�
	float										fInitPosX;
	float										fInitPosY;
	float										fMouseX;
	float										fMouseY;
	DWORD										dwActiveTm;
};

typedef std::list<tagBulletMove *> ArrayBulletMove;
typedef std::list<tagBulletMove *>::iterator IteratorBullet;

class CBulletMoveManage
{
private:
	int											m_nActiveIndex;									//��ѵ����
	ArrayBulletMove								m_ActiveBullet;									//�����
	tagBulletMove								m_BulletMoveData[MAX_CONTAIN_BULLET_NUM];		//���600����

public:
	CBulletMoveManage(void);
	~CBulletMoveManage(void);

	//�����
	bool ActiveBullet(int nBulletID, WORD wChairID, float fSpeed, float fx, float fy, float fMouseX=0,float fMouseY=0);			//M_PI down/ M_PI_2 right 0 up/ M_PI+M_PI_2/left
	bool DeleteBullet(int nBulletID, WORD wChairID);															//-1 ȫ������
	tagBulletMove *GetBullet(int nBulletID, WORD wChairID);

	//һ���Լ��� nPosX1���
	bool GetBulletPoint(int &nPosXOut, int &nPosYOut, float fSpeed, DWORD dwElapseTime, float nPosX1, float nPosY1, float nPosX2, float nPosY2);
};


struct tagCrossCheck
{
	WORD										wChairID;
	int											nBulletID;										//�ӵ�id
	int											nBulletX;										//�ӵ�x
	int											nBulletY;										//�ӵ�y
	float										fBulletStartX;
	float										fBulletStartY;									//
	int											nFishID;										//��id
	int											nFishX;											//������
	int											nFishY;											//
	DWORD										dwElapseTm;										//��ʱ��
	DWORD										dwBulletTime;									//�ӵ�ʱ��
	float										fMultiRation;									//�۳˸���
	BYTE										cbCheckTag;										//255������λ�� 1����������·����� 3ֻ��֤��켣��������
};
class CCollisionManage
{
private:
	CFishMoveManage *m_pFish;
	CBulletMoveManage *m_pBullet;

public:
	CCollisionManage();
	~CCollisionManage();

public:
	void SetInputObject(CFishMoveManage *pFish, CBulletMoveManage *pBullet);

	//0��ײ
	int IsCross(tagCrossCheck *pCheckData);
};

#define PATH_SHARE	CPathShare::GetInstance()

////���µ����ǲ���ʹ��
//#define FISH_MANAGE CFishMoveManage::GetInstance()
//#define BULLET_MANAGE CBulletMoveManage::GetInstance()
//#define COLLISION_MANAGE CCollisionManage::GetInstance()