#ifndef GAME_CONFIG_H_
#define GAME_CONFIG_H_

#include "stdafx.h"
#include <vector>

//鱼群配置 同类鱼
struct tagYuQun
{
	BYTE nStyleID;					//鱼群标记id
	WORD wFishCount;				//一次产生鱼数
};

//鱼阵配置 多类鱼
struct tagYuZhen
{
	BYTE nStyleID;					//鱼阵标记id
	DWORD dwAllLifeTime;			//鱼阵总共持续时间
	std::vector<WORD> ArrayKind;	//鱼种类容器 鱼kindid
	std::vector<DWORD> ArrayRegion;	//0xAABBCCDD AABB=min_index BBCC=max_index
};

class GameConfig
{
public:
	GameConfig();
	~GameConfig();

	bool LoadGameConfig(const TCHAR szPath[SERVER_LEN],WORD wServerID,bool bFristLoad);

public:
	//////////////////////////////////////////////////////////////////////////
	//库存控制
	static SCORE													g_stock_Kind[FISH_KIND_COUNT];								//每种类型独立库存
	static SCORE													g_revenue_score;											//抽税库存
	static SCORE													g_stock_score_old;											//老方式库存

	//前13小倍率鱼独立库存
	int																stock_crucial_count_;										//
	int																stock_stock_min_range;										//按1倍计算20条鱼*(0+9900)/2
	int																stock_crucial_score_[20];									//
	double															stock_multi_probability_[20];								//
	SCORE															stock_init_gold;

	//最大支持动态配置20个 需重启(钻头炮,连环炸弹蟹,电磁蟹) 闪电鱼 旋风鱼 浮动倍率 公用基准
	int																bomb_stock_count_;											//
	int																bomb_stock_score_[20];										//
	double															bomb_stock_multi_pro_[20];									//累乘概率
	SCORE															bomb_stock_init_gold;

	//配置XML数据
	int																stock_init_gold_old;										//老库存方式
	int																stock_crucial_count_old;									//
	int																stock_crucial_score_old[20];								//
	double															stock_multi_probability_old[20];							//老库存方式

	//子弹配置
	int 															exchange_ratio_userscore_;									//金币和渔币的兑换(金币:渔币)
	int 															exchange_ratio_fishscore_;									//金币和渔币的兑换(金币:渔币)
	int 															exchange_count_;											//每次兑换数量
	int 															min_bullet_multiple_;										//最小子弹倍数
	int 															max_bullet_multiple_;										//最大子弹倍数
	int 															bullet_speed_[BULLET_KIND_COUNT];							//子弹速度

	//鱼配置
	BYTE															fish_new_min_cnt[FISH_KIND_COUNT];							//当前场景最少条数
	BYTE															fish_new_max_cnt[FISH_KIND_COUNT];							//当前场景最多条数 min=max时必须出min条
	WORD															fish_fire_min_cnt[FISH_KIND_COUNT];
	WORD															fish_fire_max_cnt[FISH_KIND_COUNT];
	int 															fish_multiple_[FISH_KIND_COUNT];							//鱼的倍数
	int																fish_multiple_max[FISH_KIND_COUNT];							//鱼的最大倍数 部分鱼才有
	int 															fish_hit_radius_[FISH_KIND_COUNT];							//鱼的击中半径
	double															fish_capture_probability_[FISH_KIND_COUNT];					//打鱼难度
	double															fish_capture_probability_Copy[FISH_KIND_COUNT];				//打鱼难度 - 备份
	float															distribute_interval[FISH_KIND_COUNT];						//出鱼间隔 单位：秒
	int																g_IsNotRand[FISH_KIND_COUNT];								//不参与是否参与随机难度
	double															g_dRandValue;												//随机值1.0+x表示概率上浮
	int																g_tTimeSecond;												//随机部分鱼概率时间秒数
	std::vector<tagYuQun*>											m_ConfigYuQun;												//鱼群
	std::vector<tagYuZhen*>											m_ConfigYuZhen;

private:
	static bool load_first_;
};

#endif  // GAME_CONFIG_H_
