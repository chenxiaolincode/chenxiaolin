#pragma once

#include <list>
#include <vector>

class TableFrameSink;//方便直接回调 不需继承方式
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
	WORD										wPathID;									//轨迹ID 足够 名字不关
	DWORD										dwTotalTickt;								//固定毫秒 分配给单个鱼随机减去一段时间
	std::vector<DWORD>							TwoPoint;									//x==(TwoPoint>>16) y=(TwoPoint&0x0000FFFF)
};
typedef std::vector<tagFishPath *> ArrayFishPath;

class CPathShare
{
private:
	BYTE										m_cbNowReadStyle;							//当前读取类型

public:
	ArrayFishPath								m_FishPathPoints[4];						//轨迹点集合 0 small 1big 2huge 3special

private:
	CPathShare();
	~CPathShare();

	//内部函数
private:
	//扫描指定文件
	void ScanALlFile(LPCTSTR lpPathName);
	//逐个加载
	bool LoadOnePathItem(LPCTSTR lpFileName);

public:
	bool LoadAllPath();
	tagFishPath * GetAllPointCount(PathSyle pathstyle, WORD wPathID);
	static CPathShare &GetInstance();
};
//////////////////////////////////////////////////////////////////////////

#define											MAX_CONTAIN_FISH_NUM	1200						//最多鱼 考虑鱼阵设计问题
//鱼移动管理


//最小化结构体 模块独立 不需要冗余FishStyle和FishGold等信息
struct tagFishMove
{
	int											nID;											//鱼ID
	int											nPathID;										//-1没有 athSyle=x/10000 PathID=x%1000
	DWORD										dwActiveTime;									//激活时间
	DWORD										dwPathLineTime;									//总共给出时间
	//WORD										wRegion
};

typedef std::list<tagFishMove *> ArrayFishMove;
typedef std::list<tagFishMove *>::iterator IteratorFish;

//考虑实现的特殊性 去掉Start和Update
class CFishMoveManage
{
public:
	int											m_nActiveIndex;									//轮训激活
	ArrayFishMove								m_ActiveFish;									//活动的鱼
	tagFishMove									m_FishMoveData[MAX_CONTAIN_FISH_NUM];			//最大500即可

public:
	CFishMoveManage(void);
	~CFishMoveManage(void);

	//nPathID--高16位轨迹类型 低16位轨迹ID -1没路径 dwManueTime前期没有路径前手写
	bool ActiveFish(int nFishID, int nPathID, DWORD dwManueTime=0);								//激活鱼信息
	bool DeleteFish(int nFishID);																//-1 全部清理
	int Update(int nDelFishID[], int &nDelCount);												//清理维护
	tagFishMove *GetFish(int nFishID);
};


//////////////////////////////////////////////////////////////////////////
//镜面反射 这里要减小误差 反而需要累加求解
#define											MAX_CONTAIN_BULLET_NUM	500						//最多子弹每人最大300


//子弹管理
struct tagBulletMove
{
	//必须元素 内存换效率
	int											nID;
	WORD										wChairID;
	float										fSpeed;											//毫秒速度

	//辅助临时算终点
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
	int											m_nActiveIndex;									//轮训激活
	ArrayBulletMove								m_ActiveBullet;									//活动的鱼
	tagBulletMove								m_BulletMoveData[MAX_CONTAIN_BULLET_NUM];		//最大600即可

public:
	CBulletMoveManage(void);
	~CBulletMoveManage(void);

	//鱼控制
	bool ActiveBullet(int nBulletID, WORD wChairID, float fSpeed, float fx, float fy, float fMouseX=0,float fMouseY=0);			//M_PI down/ M_PI_2 right 0 up/ M_PI+M_PI_2/left
	bool DeleteBullet(int nBulletID, WORD wChairID);															//-1 全部清理
	tagBulletMove *GetBullet(int nBulletID, WORD wChairID);

	//一次性计算 nPosX1起点
	bool GetBulletPoint(int &nPosXOut, int &nPosYOut, float fSpeed, DWORD dwElapseTime, float nPosX1, float nPosY1, float nPosX2, float nPosY2);
};


struct tagCrossCheck
{
	WORD										wChairID;
	int											nBulletID;										//子弹id
	int											nBulletX;										//子弹x
	int											nBulletY;										//子弹y
	float										fBulletStartX;
	float										fBulletStartY;									//
	int											nFishID;										//鱼id
	int											nFishX;											//鱼坐标
	int											nFishY;											//
	DWORD										dwElapseTm;										//鱼时间
	DWORD										dwBulletTime;									//子弹时间
	float										fMultiRation;									//累乘概率
	BYTE										cbCheckTag;										//255放行鱼位置 1扩大特殊鱼路径误差 3只验证鱼轨迹其他过掉
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

	//0碰撞
	int IsCross(tagCrossCheck *pCheckData);
};

#define PATH_SHARE	CPathShare::GetInstance()

////以下单例是测试使用
//#define FISH_MANAGE CFishMoveManage::GetInstance()
//#define BULLET_MANAGE CBulletMoveManage::GetInstance()
//#define COLLISION_MANAGE CCollisionManage::GetInstance()