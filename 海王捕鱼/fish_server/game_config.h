#ifndef GAME_CONFIG_H_
#define GAME_CONFIG_H_

#include "stdafx.h"
#include <vector>

//��Ⱥ���� ͬ����
struct tagYuQun
{
	BYTE nStyleID;					//��Ⱥ���id
	WORD wFishCount;				//һ�β�������
};

//�������� ������
struct tagYuZhen
{
	BYTE nStyleID;					//������id
	DWORD dwAllLifeTime;			//�����ܹ�����ʱ��
	std::vector<WORD> ArrayKind;	//���������� ��kindid
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
	//������
	static SCORE													g_stock_Kind[FISH_KIND_COUNT];								//ÿ�����Ͷ������
	static SCORE													g_revenue_score;											//��˰���
	static SCORE													g_stock_score_old;											//�Ϸ�ʽ���

	//ǰ13С������������
	int																stock_crucial_count_;										//
	int																stock_stock_min_range;										//��1������20����*(0+9900)/2
	int																stock_crucial_score_[20];									//
	double															stock_multi_probability_[20];								//
	SCORE															stock_init_gold;

	//���֧�ֶ�̬����20�� ������(��ͷ��,����ը��з,���з) ������ ������ �������� ���û�׼
	int																bomb_stock_count_;											//
	int																bomb_stock_score_[20];										//
	double															bomb_stock_multi_pro_[20];									//�۳˸���
	SCORE															bomb_stock_init_gold;

	//����XML����
	int																stock_init_gold_old;										//�Ͽ�淽ʽ
	int																stock_crucial_count_old;									//
	int																stock_crucial_score_old[20];								//
	double															stock_multi_probability_old[20];							//�Ͽ�淽ʽ

	//�ӵ�����
	int 															exchange_ratio_userscore_;									//��Һ���ҵĶһ�(���:���)
	int 															exchange_ratio_fishscore_;									//��Һ���ҵĶһ�(���:���)
	int 															exchange_count_;											//ÿ�ζһ�����
	int 															min_bullet_multiple_;										//��С�ӵ�����
	int 															max_bullet_multiple_;										//����ӵ�����
	int 															bullet_speed_[BULLET_KIND_COUNT];							//�ӵ��ٶ�

	//������
	BYTE															fish_new_min_cnt[FISH_KIND_COUNT];							//��ǰ������������
	BYTE															fish_new_max_cnt[FISH_KIND_COUNT];							//��ǰ����������� min=maxʱ�����min��
	WORD															fish_fire_min_cnt[FISH_KIND_COUNT];
	WORD															fish_fire_max_cnt[FISH_KIND_COUNT];
	int 															fish_multiple_[FISH_KIND_COUNT];							//��ı���
	int																fish_multiple_max[FISH_KIND_COUNT];							//�������� ���������
	int 															fish_hit_radius_[FISH_KIND_COUNT];							//��Ļ��а뾶
	double															fish_capture_probability_[FISH_KIND_COUNT];					//�����Ѷ�
	double															fish_capture_probability_Copy[FISH_KIND_COUNT];				//�����Ѷ� - ����
	float															distribute_interval[FISH_KIND_COUNT];						//������ ��λ����
	int																g_IsNotRand[FISH_KIND_COUNT];								//�������Ƿ��������Ѷ�
	double															g_dRandValue;												//���ֵ1.0+x��ʾ�����ϸ�
	int																g_tTimeSecond;												//������������ʱ������
	std::vector<tagYuQun*>											m_ConfigYuQun;												//��Ⱥ
	std::vector<tagYuZhen*>											m_ConfigYuZhen;

private:
	static bool load_first_;
};

#endif  // GAME_CONFIG_H_
