
#ifndef CMD_FISH_H_
#define CMD_FISH_H_

//#pragma pack(1)

//////////////////////////////////////////////////////////////////////////
// ������

#define 										KIND_ID               							2016
#define 										GAME_NAME             							TEXT("����2")
#define 										GAME_PLAYER           							6

// �򵥵İ汾���
#define											GAME_VERSION									33

#ifndef SCORE
#define SCORE LONGLONG
#endif

//������Ϣ
const int kResolutionWidth = 1600;	//Ĭ�Ϸֱ��ʿ��
const int kResolutionHeight = 900;	//Ĭ�Ϸֱ��ʸ߶�
const int kBombRadius = 300;		//ը���뾶
const int kMaxChainFishCount = 6;	//�������������
const int kNormolFishTime = 20;		//Ĭ���㶼��20��ʱ��


#ifndef M_PI
#define M_PI    3.14159265358979323846f
#define M_PI_2  1.57079632679489661923f
#define M_PI_4  0.785398163397448309616f
#define M_1_PI  0.318309886183790671538f
#define M_2_PI  0.636619772367581343076f
#endif

//��
struct FPoint
{
	float x;
	float y;
};

//���Ƕȵĵ�
struct FPointAngle
{
	float x;
	float y;
	float angle;
};

//////////////////////////////////////////////////////////////////////////
// ��Ϸ����

/*
// ��λ��
-------------
    0   1   2

    5   4   3

��ת���ӽ�
	3	4	5

	2	1	0
-------------
*/
const float kChairDefaultAngle[GAME_PLAYER] = { M_PI, M_PI, M_PI, 0, 0, 0 };
const WORD kCannonCirlLen=180;//px
const WORD kChairCannon[GAME_PLAYER][2]=
{
	{350, 45},			{800,45},				{1250,45},	//0,1,2
	{1250,855},			{800,855},				{350,855},	//3,4,5
};

/*
	1.����ģʽ
		a.��������
	.	b.����籩
			������� ��ʱ��ѵ�����Ϸ ʱ������Ѳ��㣬�������л����ö��ⱶ����ʱ�䣬����ʱ��Ϸ����*����
	2.��ս�ƽ��
		������� ��ʱ��Ѷ�����Ϸ ̽�յ���  ���������ƽ��� ����ɻ�ø߶����
		�л�����ת�˱��䣬�л��ʽ��ַܷ�������Ӯȡ�������
*/
//��������
enum SceneKind
{
	SCENE_KIND_1 = 0,		//����2��ͨ1
	SCENE_KIND_2,			//����2��ͨ2
	SCENE_KIND_3,			//��ҹ����
	SCENE_KIND_4,			//����з
	SCENE_KIND_5,			//ʷǰ����
	SCENE_KIND_6,			//���צ��

	//SCENE_GOLD_WIN,		//��ս�ƽ�� //�ݸ���

	SCENE_KIND_COUNT
};

//�������
enum FishKind
{
	//С����13��			�����				��ϵͳĬ�ϡ�С����============== ����
	FISH_KIND_1 = 0,		//��β��			����*2
	FISH_KIND_2,			//ѩ��				����*3
	FISH_KIND_3,			//������			����*4
	FISH_KIND_4,			//������			����*5
	FISH_KIND_5,			//����				����*6
	FISH_KIND_6,			//ɺ����			����*7
	FISH_KIND_7,			//����				����*8
	FISH_KIND_8,			//����				����*9
	FISH_KIND_9,			//��Ϻ				����*10				���϶����������� �Զ���������

	FISH_KIND_10,			//������			����*12
	FISH_KIND_11,			//����				����*15
	FISH_KIND_12,			//����				����*18
	FISH_KIND_13,			//������			����*20				���϶���������  ͬ���㶼����

	//**********************************************************************************
	//���������
	FISH_KIND_14,			//��ͷ��		�ᴩ�㲢�м��ʲ��� ����һ��ʱ����Ա����и��ʲ���ը��Χ�ڵ���
	FISH_KIND_15,			//����ը��з    �м��ʲ���ը��Χ�ڵ���
	FISH_KIND_16,			//���з		�޶�ʱ���� �����Ƕȷ��� һ�����ʲ�����̷�Χ�ڵ���
	//**********************************************************************************

	//����
	FISH_KIND_17,			//��ͷ��			����*[20-60] ======== [30-100]
	FISH_KIND_18,			//����				����*[30-100] ======= [60-200]
	FISH_KIND_19,			//����				����*100 ============ 350
	FISH_KIND_20,			//�񱩻���			����*[100-250]======= [100-500]

	//boss�� ����*����
	FISH_KIND_21,			//���צ��		����*[100-500]======= [100-800]
	FISH_KIND_22,			//����з
	FISH_KIND_23,			//��ҹ����
	FISH_KIND_24,			//ʷǰ����

	//////////////////////////////////////////////////////////////////////////
	//����޴��� ����*���� �����Ķ�
	FISH_SMALL_2_BIG1,		//�޴���--ָ����FISH_KIND_3-FISH_KIND_5	����*[10-30] ======== [20-60]
	FISH_SMALL_2_BIG2,		//�޴���--ָ����FISH_KIND_3-FISH_KIND_5	����*[10-30] ======== [20-60]
	FISH_SMALL_2_BIG3,		//�޴���--ָ����FISH_KIND_3-FISH_KIND_5	����*[10-30] ======== [20-60]

	FISH_YUQUN1,             //(FISH_KIND_1-FISH_KIND_3)  ���Ǻ��ڸ�������������  ��ͬ����
	FISH_YUQUN2,             //(FISH_KIND_1-FISH_KIND_3)  ���Ǻ��ڸ�������������  ��ͬ����
	FISH_YUQUN3,             //(FISH_KIND_1-FISH_KIND_3)  ���Ǻ��ڸ�������������  ��ͬ����
	FISH_YUQUN4,             //(FISH_KIND_1-FISH_KIND_3)  ���Ǻ��ڸ�������������  ��ͬ����
	FISH_YUQUN4_KING,
	FISH_YUQUN5,
	FISH_CHAIN,				//(FISH_KIND_1-FISH_KIND_9) �� (FISH_KIND_1-FISH_KIND_9)
	FISH_CATCH_SMAE,		//(FISH_KIND_1-FISH_KIND_13) ������ֻд��FISH_KIND_12��
	FISH_YUZHEN,			//(FISH_KIND_1-FISH_KIND_13) ���͹̶� �����Ϳ��ܲ�ͬ���̶�

	FISH_KIND_COUNT
};
//FISH_CHAIN // ������ (FISH_KIND_1-FISH_KIND_9) �� (FISH_KIND_1-FISH_KIND_9) ��Ϊ�ڶ�����
//////////////////////////////////////////////////////////////////////////

/*
	A����������
		1.������Ͳ
		2.������Ͳ
		3.������Ͳ

	B����������
		1.��ͷ��
		2.����ը��
		3.�����

	C���������� ����ѡ�
		1.����籩��
		2.��ս�ƽ�Ǿ�����
*/
//��Ͳ
enum BulletKind
{
	//A��
	Bullet_Type_diancipao = 0,   	//�����
	Bullet_Type_putongpao,       	//��ͨ�� 
	Bullet_Type_shandianpao,     	//������
	Bullet_Type_xianshipao,			//����籩�� 10s���
	Bullet_Type_zidanpao,			//�ӵ���
	Bullet_Type_zuantoupao,			//��ͷ��
	Bullet_Type_lianhuanzhadan,		//����ը�� 
	Bullet_Type_yulei,				//����ը�� 
	Bullet_Type_sanguanpao,			//������
	BULLET_KIND_COUNT
};

const DWORD kBulletIonTime = 10;
const DWORD kLockTime = 10;

const int kMaxCatchFishCount = 2;


//////////////////////////////////////////////////////////////////////////
// ���������

#define SUB_S_GAME_CONFIG                   	100												//��Ϸ����
#define SUB_S_FISH_TRACE                    	101												//��Ĺ켣
#define SUB_S_EXCHANGE_FISHSCORE            	102												//�һ����
#define SUB_S_USER_FIRE                     	103												//��ҿ���
#define SUB_S_CATCH_FISH                    	104												//������Ⱥ
#define SUB_S_CATCH_GROUP_FISH					105												//���������
#define SUB_S_RETURNBULLETSCORE             	106												//�����ӵ����з���

#define SUB_S_BULLET_TOOL_TIMEOUT            	107												//���ڹ�ʱ
#define SUB_S_SWITCH_SCENE                  	108												//�л�����

#define SUB_S_CATCH_GROUP_FISH_KING				109												//��������

//#define SUB_S_SCENE_END                     	109												//��������
#define SUB_S_FISH_TRACE_YUQUN                 	110												//��Ⱥ
#define SUB_S_FISH_TRACE_YUZHEN                 111												//����
#define SUB_S_BOMB_POINTS						113												//����ը��λ��
#define SUB_S_ROTATE_BACK						114												//��צ����ת����
#define SUB_S_FREE_TIME_INFO					115												//�ڵ��л�֪ͨ ������籩
#define SUB_S_LOCK_TIMEOUT                  	120												//������ʱ
#define SUB_S_STOCK_OPERATE_RESULT          	121												//���������
#define SUB_S_RETURN_MSG_CHECK					122												//Э���鿴
#define SUB_S_SANGUAN_FIRE						124												//�����ڿ���

//��Ϸ״̬
struct CMD_S_GameStatus
{
	WORD										wDaoJuPaoUser;
	DWORD										dwGame_version;									//��Ϸ����С�汾����
	DWORD										dwTorpedoCount[5];								//���׸���
	SCORE										lNow_fish_score[GAME_PLAYER];					//��ǰ�������
	SCORE										lNow_user_score[GAME_PLAYER];					//��ǰ������Ͻ��
};

//��Ϸ����
struct CMD_S_GameConfig
{
	//��Ҷһ�����
	int 										nExchange_ratio_userscore;						//��һ���
	int 										nExchange_ratio_fishscore;						//���
	int 										nExchange_count;								//�һ�����

	//�ӵ���������
	int 										nMin_bullet_multiple;							//��С�ӵ�����
	int 										nMax_bullet_multiple;							//����ӵ�����

	//�����Ϣ �����������ú��·�
	int											nFish_multiple[FISH_KIND_COUNT];				//��ı���

	//�ӵ���Ϣ
	int											nBullet_speed[BULLET_KIND_COUNT];				//�ӵ��ٶ�

	//��һ�ڴ�һ�� �����õ� ��ֻ��һ��չ����ʽ ������һ�ִ����ϵĶ��ᣬ�����ǿ�����
	//int										net_radius[BULLET_KIND_COUNT];					//�����뾶
};

//��Ĺ켣 ��С���ṹ��
struct CMD_S_FishTrace
{
	int											nFish_id;										//��ID
	int											nTrace_id;										//�켣ID
	WORD										nFish_Tag;										//ָ��ͼƬ
	WORD										fish_kind;										//������
};

//�ֻ�����jason �򵥶�����
struct CMD_S_FishTraceYuQun
{
	WORD										wStyleID;										//��ȺID
	WORD										wFishKind;										//������ ͬһ��
	WORD										wFishKing;										//����
	int											nFishKingID;									//����ID
	int											nTraceID;										//��Ⱥ�켣id
	int											nStartFishID;									//��ʼid
};

struct CMD_S_FishTraceYuZhen
{
	WORD										wStyleID;										//����ID
	int											nStartFishID;									//��ʼid ���ʱif(fish_id_ <= 0) fish_id_ = 1;
};

//�һ����
struct CMD_S_ExchangeFishScore
{
	WORD										wChairID;										//�����λ
	SCORE										lNow_fish_score;								//��ǰ��ʾ���
	SCORE										lNow_user_score;								//��ǰ���Ͻ��
};

//��ҿ���
struct CMD_S_UserFire
{
	WORD										wChairID;										//�����λID
	int											nLock_fishid;									//�������ID
	int											nBullet_id;										//�ӵ�ID ���ͻ���ǿ�Ƹ�ֵ
	int											nBullet_mulriple;								//�ӵ�����
	int											fMouseX;
	int											fMouseY;
	int											nBullet_index;									//�����ڵ��ӵ���012�������ڶ���0
	BulletKind									bullet_kind;									//�ӵ�����
	
	//==nBullet_mulriple������������� �������ʹ�� ���Ǳ������� e.g����ӵ�
	SCORE										lNow_fish_score;								//��ǰ��ʾ���
	SCORE										lNow_user_score;								//��ǰ���Ͻ��
	BYTE										cbBoomKind;										//�������࣬�������ӵ�ΪINVALID_BYTE
};


//��������ڿ���
struct CMD_S_SanGuanFire
{
	WORD										wChairID;										//�����λID
	int											nStartBullet_id;								//�ӵ���ʼID
	int											nBullet_mulriple;								//�ӵ�����
	int											fMouseX[3];
	int											fMouseY[3];
	BulletKind									bullet_kind;									//�ӵ�����
	SCORE										lNow_fish_score;								//��ǰ��ʾ���
	SCORE										lNow_user_score;								//��ǰ���Ͻ��
};

//������
struct CMD_S_CatchFish
{
	WORD										wChairID;										//�����λID
	int											nFish_id;										//���ID �ó�����֤ FishKind fish_kind
	int											nFish_score;									//��� �������������ս��
	int											nBullet_mul;									//�����ֶ� �ͻ������BulletID���������� ������ֱ�ӷ�������
	SCORE										lNow_fish_score;								//��ǰ��ʾ���
	SCORE										lNow_user_score;								//��ǰ���Ͻ��
};

//������Ⱥ 3������ը�� 1�ε���� 3����ͷ�� 1���������� 1��������
struct CMD_S_CatchGroupFish
{
	WORD										wChairID;
	WORD										wGroupSytle;									//0����ը�� 1����� 2��ͷ�� 3�������� 4������,5��Ⱥ���е�����
	WORD										wGroupCount;									//����
	int											nCatch_fish_count;
	SCORE										lNow_fish_score;								//��ǰ��ʾ���
	SCORE										lNow_user_score;								//��ǰ���Ͻ��
	int											nBullet_mul;									//�����ֶ� �ͻ������BulletID���������� ������ֱ�ӷ�������
	int											nCatch_fish_id[60];								//���60�� �����ܹ���10�� [0]��������
	int											nfish_score[60];								//�����ҵ� ��ͷ�� ����[59]��ʾ��ǰ�ܹ���ý��
};

//��������
struct CMD_S_CatchGroupFishKing
{
	WORD										wChairID;
	int											nCatch_fish_count;
	SCORE										lNow_fish_score;								//��ǰ��ʾ���
	SCORE										lNow_user_score;								//��ǰ���Ͻ��
	SCORE										nTotal_fish_score;								//����ܹ�����
	int											nBullet_mul;									//�����ֶ� �ͻ������BulletID���������� ������ֱ�ӷ�������
	int											nCatch_fish_id[100];							//���100�� �����ܹ���10�� [0]��������
	int											nfish_score[100];								//ÿ�����Ӧ�÷���
};


//��������ʱ�䵽
struct CMD_S_BulletToolTimeout
{
	WORD										wChairID;										//�����Զ�����
};

//�л����� û��tag Ĭ�϶��Ƿ�������
struct CMD_S_SwitchScene
{
	SceneKind									scene_kind;
	DWORD										nSceneStartTime;//�³����Ѿ���ʼ��ʱ��

};

//�����ӵ�����
struct CMD_S_ReturnBulletScore
{
	WORD										wChairID;
	WORD										wReasonType;
	SCORE										lNow_fish_score;								//��ǰ��ʾ���
	SCORE										lNow_user_score;								//��ǰ���Ͻ��
};

//������ �ݲ�ʵ��
struct CMD_S_StockOperateResult
{
	BYTE										cbOperate_Code;
	TCHAR										szRetMSGManuel[128];							//�Զ�������
};

//���������������������λ��
struct CMD_S_Bomb_Points
{
	WORD										wChairID;
	WORD										wPointX[3];										//����ը��2����
	WORD										wPointY[3];
};

struct CMD_S_RotateBack
{
	WORD										wDegree;										//0-360
};

struct CMD_S_Free_Time
{
	WORD										wChairID;
	int											nBullet_mul;									//������ֵ
	BulletKind									wBulletKind;									//
};
//////////////////////////////////////////////////////////////////////////
// �ͻ�������

#define 										SUB_C_EXCHANGE_FISHSCORE            1			//�һ����
#define 										SUB_C_USER_FIRE                     2			//�û�����
#define 										SUB_C_CATCH_FISH                    3			//������
#define 										SUB_C_CATCH_GROUP_FISH              4			//�������
#define 										SUB_C_STOCK_OPERATE                 5			//������ �������� �ݲ�ʵ��
#define											SUB_C_CONTROL_WAI_GUA				6			//������¼ʹ��
#define											SUB_C_CATCH_FISH_TORPEDO			7			//���״���
#define											SUB_C_CATCH_GROUP_FISH_FISHKING		8			//��׽����
#define											SUB_C_SANGUAN_FIRE					9			//�����ڿ���������ڿ������ִ���



//�һ�����
struct CMD_C_ExchangeFishScore
{
	bool										bIncrease;										//��С��������Χ ��ֹ�κ�Ȩ��©��
};

//��ҿ��� ����ը������
struct CMD_C_UserFire
{
	int											fMouseX;
	int											fMouseY;
	int											nBullet_mulriple;								//�ӵ�����
	int											nLock_fishid;									//������ID ��һ����
	int											nBulletID;										//��Ϊ�ͻ�������
	int											nBullet_index;									//�����ڵ��ӵ���012�������ڶ���0
	BYTE										cbBoomKind;										//�������࣬0,1,2,3,4���������� �ӵ�ΪINVALID_BYTE
	BulletKind									bullet_kind;									//�ӵ�����
};

//�û������ڿ���
struct CMD_C_SanGuanFire
{
	int											fMouseX[3];
	int											fMouseY[3];
	int											nBullet_mulriple;
	int											nStartBulletID;									//�����ӵ�����ʼ�ӵ�ID
	BulletKind									bullet_kind;									//�ӵ�����
};


//������ �ų�����ը�� ����� ��ͷ��
struct CMD_C_CatchFish
{
	WORD										wChairID;										//���� ���ܴ��� ����ʣ���ӵ��������

	int											nFish_id;
	int											nBullet_id;										//��ѯȷ��int nBullet_mulriple BulletKind bullet_kind
	int											nBulletX;
	int											nBulletY;
	int											nBulletStartX;
	int											nBulletStartY;
	int											nFishX;
	int											nFishY;
	DWORD										dwMoveTime;										//���ζ�ʱ�� ���������ݿͻ���ֱ����֤
	DWORD										dwBulletTime;									//�ӵ��ζ�ʱ��
};

//������Ⱥ ����ը�� ����� ��ͷ��
struct CMD_C_CatchGroupFish
{
	WORD										wChairID;										//���� ���ܴ��� ����ʣ���ӵ��������

	int											nBullet_id;										//��ΪnBullet_id �ӵ�ID
	int											nCatch_fish_count;
	int											nFishPosX[80];									//
	int											nFishPosY[80];									//
	int											nCatch_fish_id[80];								//��ཱུ��Ϊ80��
	DWORD										dwMoveTime[80];									//���ζ�ʱ�� ���������ݿͻ���ֱ����֤
};

//��������
struct CMD_C_CatchGroupFishKing
{
	WORD										wChairID;										//���� ���ܴ��� ����ʣ���ӵ��������

	int											nBullet_id;										//��ΪnBullet_id �ӵ�ID
	int											nCatch_fishKing_id;									//����ID
	int											nCatch_fish_count;
	int											nFishPosX[100];									//
	int											nFishPosY[100];	
	int											nCatch_fish_id[100];								//��ཱུ��Ϊ130��
	DWORD										dwMoveTime[100];									//���ζ�ʱ�� ���������ݿͻ���ֱ����֤
};

//���״���
struct CMD_C_CatchFish_Torpedo
{
	int											fish_id;										
	int 										bullet_id;
};

//������
struct CMD_C_StockOperate
{
	BYTE										cbOperate_Code;									// 0��ѯ 1 ��� 2 ���� 3 ��ѯ��ˮ
};

struct CMD_C_ControlWaiGua
{
	BYTE										cbAddUser;										//�������� ��ɾ��
	DWORD										dwGameID;
};

//������ѯ Э����ؿ�����
struct CMD_C_MsgCheck
{
	TCHAR										szMessage[2048];
};

//#pragma pack()

#endif // CMD_FISH_H_