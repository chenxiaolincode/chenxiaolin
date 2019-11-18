
#ifndef CMD_FISH_H_
#define CMD_FISH_H_

//#pragma pack(1)

//////////////////////////////////////////////////////////////////////////
// 服务定义

#define 										KIND_ID               							2016
#define 										GAME_NAME             							TEXT("海王2")
#define 										GAME_PLAYER           							6

// 简单的版本检测
#define											GAME_VERSION									33

#ifndef SCORE
#define SCORE LONGLONG
#endif

//辅助信息
const int kResolutionWidth = 1600;	//默认分辨率宽度
const int kResolutionHeight = 900;	//默认分辨率高度
const int kBombRadius = 300;		//炸弹半径
const int kMaxChainFishCount = 6;	//闪电鱼最大连数
const int kNormolFishTime = 20;		//默认鱼都游20秒时间


#ifndef M_PI
#define M_PI    3.14159265358979323846f
#define M_PI_2  1.57079632679489661923f
#define M_PI_4  0.785398163397448309616f
#define M_1_PI  0.318309886183790671538f
#define M_2_PI  0.636619772367581343076f
#endif

//点
struct FPoint
{
	float x;
	float y;
};

//带角度的点
struct FPointAngle
{
	float x;
	float y;
	float angle;
};

//////////////////////////////////////////////////////////////////////////
// 游戏定义

/*
// 座位号
-------------
    0   1   2

    5   4   3

旋转主视角
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
	1.常规模式
		a.正常流程
	.	b.烈焰风暴
			随机进入 限时免费单人游戏 时限内免费捕鱼，过程中有机会获得额外倍数及时间，结算时游戏分数*倍数
	2.决战黄金城
		随机进入 限时免费多人游戏 探照灯找  帝王鲸，黄金鱼 捕获可获得高额分数
		有机会获得转运宝箱，有机率将总分翻倍，或赢取额外分数
*/
//场景类型
enum SceneKind
{
	SCENE_KIND_1 = 0,		//海王2普通1
	SCENE_KIND_2,			//海王2普通2
	SCENE_KIND_3,			//暗夜炬兽
	SCENE_KIND_4,			//帝王蟹
	SCENE_KIND_5,			//史前巨鳄
	SCENE_KIND_6,			//深海八爪鱼

	//SCENE_GOLD_WIN,		//决战黄金城 //暂搁置

	SCENE_KIND_COUNT
};

//鱼的种类
enum FishKind
{
	//小倍数13个			鱼民称				【系统默认】小倍率============== 大倍率
	FISH_KIND_1 = 0,		//凤尾鱼			倍率*2
	FISH_KIND_2,			//雪鱼				倍率*3
	FISH_KIND_3,			//蝴蝶鱼			倍率*4
	FISH_KIND_4,			//尖嘴鱼			倍率*5
	FISH_KIND_5,			//河豚				倍率*6
	FISH_KIND_6,			//珊瑚鱼			倍率*7
	FISH_KIND_7,			//鲶鱼				倍率*8
	FISH_KIND_8,			//海星				倍率*9
	FISH_KIND_9,			//龙虾				倍率*10				以上都可闪电连锁 自动连续捕鱼

	FISH_KIND_10,			//灯笼鱼			倍率*12
	FISH_KIND_11,			//海龟				倍率*15
	FISH_KIND_12,			//旗鱼				倍率*18
	FISH_KIND_13,			//蝙蝠鱼			倍率*20				以上都可旋风鱼  同种鱼都吸入

	//**********************************************************************************
	//特殊道具鱼
	FISH_KIND_14,			//钻头炮		贯穿鱼并有几率捕鱼 反弹一段时间后自爆并有概率捕获爆炸范围内的鱼
	FISH_KIND_15,			//连环炸弹蟹    有几率捕获爆炸范围内的鱼
	FISH_KIND_16,			//电磁蟹		限定时间内 调整角度发射 一定概率捕获射程范围内的鱼
	//**********************************************************************************

	//大鱼
	FISH_KIND_17,			//锤头鲨			倍率*[20-60] ======== [30-100]
	FISH_KIND_18,			//鲨鱼				倍率*[30-100] ======= [60-200]
	FISH_KIND_19,			//海豚				倍率*100 ============ 350
	FISH_KIND_20,			//狂暴火龙			倍率*[100-250]======= [100-500]

	//boss鱼 倍率*浮动
	FISH_KIND_21,			//深海八爪鱼		倍率*[100-500]======= [100-800]
	FISH_KIND_22,			//帝王蟹
	FISH_KIND_23,			//暗夜炬兽
	FISH_KIND_24,			//史前巨鳄

	//////////////////////////////////////////////////////////////////////////
	//特殊巨大化鱼 倍率*浮动 单独改动
	FISH_SMALL_2_BIG1,		//巨大化鱼--指定鱼FISH_KIND_3-FISH_KIND_5	倍率*[10-30] ======== [20-60]
	FISH_SMALL_2_BIG2,		//巨大化鱼--指定鱼FISH_KIND_3-FISH_KIND_5	倍率*[10-30] ======== [20-60]
	FISH_SMALL_2_BIG3,		//巨大化鱼--指定鱼FISH_KIND_3-FISH_KIND_5	倍率*[10-30] ======== [20-60]

	FISH_YUQUN1,             //(FISH_KIND_1-FISH_KIND_3)  考虑后期概率特例化需求  相同的鱼
	FISH_YUQUN2,             //(FISH_KIND_1-FISH_KIND_3)  考虑后期概率特例化需求  相同的鱼
	FISH_YUQUN3,             //(FISH_KIND_1-FISH_KIND_3)  考虑后期概率特例化需求  相同的鱼
	FISH_YUQUN4,             //(FISH_KIND_1-FISH_KIND_3)  考虑后期概率特例化需求  相同的鱼
	FISH_YUQUN4_KING,
	FISH_YUQUN5,
	FISH_CHAIN,				//(FISH_KIND_1-FISH_KIND_9) 连 (FISH_KIND_1-FISH_KIND_9)
	FISH_CATCH_SMAE,		//(FISH_KIND_1-FISH_KIND_13) 但代码只写到FISH_KIND_12了
	FISH_YUZHEN,			//(FISH_KIND_1-FISH_KIND_13) 阵型固定 鱼类型可能不同但固定

	FISH_KIND_COUNT
};
//FISH_CHAIN // 闪电鱼 (FISH_KIND_1-FISH_KIND_9) 连 (FISH_KIND_1-FISH_KIND_9) 改为第二属性
//////////////////////////////////////////////////////////////////////////

/*
	A。常规类型
		1.正常炮筒
		2.加速炮筒
		3.锁定炮筒

	B。变种类型
		1.钻头炮
		2.连环炸弹
		3.电磁炮

	C。无属性炮 【免费】
		1.烈焰风暴炮
		2.决战黄金城镜子炮
*/
//炮筒
enum BulletKind
{
	//A类
	Bullet_Type_diancipao = 0,   	//电磁炮
	Bullet_Type_putongpao,       	//普通炮 
	Bullet_Type_shandianpao,     	//锁定炮
	Bullet_Type_xianshipao,			//烈焰风暴炮 10s免费
	Bullet_Type_zidanpao,			//子弹炮
	Bullet_Type_zuantoupao,			//钻头炮
	Bullet_Type_lianhuanzhadan,		//连环炸弹 
	Bullet_Type_yulei,				//连环炸弹 
	Bullet_Type_sanguanpao,			//三管炮
	BULLET_KIND_COUNT
};

const DWORD kBulletIonTime = 10;
const DWORD kLockTime = 10;

const int kMaxCatchFishCount = 2;


//////////////////////////////////////////////////////////////////////////
// 服务端命令

#define SUB_S_GAME_CONFIG                   	100												//游戏配置
#define SUB_S_FISH_TRACE                    	101												//鱼的轨迹
#define SUB_S_EXCHANGE_FISHSCORE            	102												//兑换鱼币
#define SUB_S_USER_FIRE                     	103												//玩家开火
#define SUB_S_CATCH_FISH                    	104												//捕获鱼群
#define SUB_S_CATCH_GROUP_FISH					105												//捕获多条鱼
#define SUB_S_RETURNBULLETSCORE             	106												//返回子弹击中分数

#define SUB_S_BULLET_TOOL_TIMEOUT            	107												//大炮过时
#define SUB_S_SWITCH_SCENE                  	108												//切换场景

#define SUB_S_CATCH_GROUP_FISH_KING				109												//捕获鱼王

//#define SUB_S_SCENE_END                     	109												//场景结束
#define SUB_S_FISH_TRACE_YUQUN                 	110												//鱼群
#define SUB_S_FISH_TRACE_YUZHEN                 111												//鱼阵
#define SUB_S_BOMB_POINTS						113												//连环炸弹位置
#define SUB_S_ROTATE_BACK						114												//八爪鱼旋转场景
#define SUB_S_FREE_TIME_INFO					115												//炮弹切换通知 仅烈焰风暴
#define SUB_S_LOCK_TIMEOUT                  	120												//锁定过时
#define SUB_S_STOCK_OPERATE_RESULT          	121												//库存操作结果
#define SUB_S_RETURN_MSG_CHECK					122												//协助查看
#define SUB_S_SANGUAN_FIRE						124												//三管炮开火

//游戏状态
struct CMD_S_GameStatus
{
	WORD										wDaoJuPaoUser;
	DWORD										dwGame_version;									//游戏定义小版本需求
	DWORD										dwTorpedoCount[5];								//鱼雷个数
	SCORE										lNow_fish_score[GAME_PLAYER];					//当前鱼币数量
	SCORE										lNow_user_score[GAME_PLAYER];					//当前玩家身上金币
};

//游戏配置
struct CMD_S_GameConfig
{
	//渔币兑换比例
	int 										nExchange_ratio_userscore;						//玩家积分
	int 										nExchange_ratio_fishscore;						//渔币
	int 										nExchange_count;								//兑换数量

	//子弹倍数区间
	int 										nMin_bullet_multiple;							//最小子弹倍数
	int 										nMax_bullet_multiple;							//最大子弹倍数

	//鱼的信息 后期隐藏重置后下发
	int											nFish_multiple[FISH_KIND_COUNT];				//鱼的倍数

	//子弹信息
	int											nBullet_speed[BULLET_KIND_COUNT];				//子弹速度

	//因一炮打一鱼 所以拿掉 网只是一种展现形式 这里是一种大方向上的定夺，不考虑可扩充
	//int										net_radius[BULLET_KIND_COUNT];					//渔网半径
};

//鱼的轨迹 最小化结构体
struct CMD_S_FishTrace
{
	int											nFish_id;										//鱼ID
	int											nTrace_id;										//轨迹ID
	WORD										nFish_Tag;										//指定图片
	WORD										fish_kind;										//鱼类型
};

//手机兼容jason 则单独定义
struct CMD_S_FishTraceYuQun
{
	WORD										wStyleID;										//鱼群ID
	WORD										wFishKind;										//鱼类型 同一种
	WORD										wFishKing;										//鱼王
	int											nFishKingID;									//鱼王ID
	int											nTraceID;										//鱼群轨迹id
	int											nStartFishID;									//开始id
};

struct CMD_S_FishTraceYuZhen
{
	WORD										wStyleID;										//鱼阵ID
	int											nStartFishID;									//开始id 溢出时if(fish_id_ <= 0) fish_id_ = 1;
};

//兑换鱼分
struct CMD_S_ExchangeFishScore
{
	WORD										wChairID;										//玩家座位
	SCORE										lNow_fish_score;								//当前显示渔分
	SCORE										lNow_user_score;								//当前身上金币
};

//玩家开火
struct CMD_S_UserFire
{
	WORD										wChairID;										//玩家座位ID
	int											nLock_fishid;									//锁定鱼的ID
	int											nBullet_id;										//子弹ID 给客户端强制赋值
	int											nBullet_mulriple;								//子弹倍数
	int											fMouseX;
	int											fMouseY;
	int											nBullet_index;									//三管炮的子弹有012，其他炮都是0
	BulletKind									bullet_kind;									//子弹类型
	
	//==nBullet_mulriple正常情况下冗余 特殊情况使用 固是必须属性 e.g免费子弹
	SCORE										lNow_fish_score;								//当前显示渔分
	SCORE										lNow_user_score;								//当前身上金币
	BYTE										cbBoomKind;										//鱼雷种类，非鱼雷子弹为INVALID_BYTE
};


//玩家三管炮开火
struct CMD_S_SanGuanFire
{
	WORD										wChairID;										//玩家座位ID
	int											nStartBullet_id;								//子弹起始ID
	int											nBullet_mulriple;								//子弹倍数
	int											fMouseX[3];
	int											fMouseY[3];
	BulletKind									bullet_kind;									//子弹类型
	SCORE										lNow_fish_score;								//当前显示渔分
	SCORE										lNow_user_score;								//当前身上金币
};

//捕获鱼
struct CMD_S_CatchFish
{
	WORD										wChairID;										//玩家座位ID
	int											nFish_id;										//鱼的ID 得出后验证 FishKind fish_kind
	int											nFish_score;									//鱼分 服务器控制最终结果
	int											nBullet_mul;									//多余字段 客户端设计BulletID可能消亡了 服务器直接发更合理
	SCORE										lNow_fish_score;								//当前显示渔分
	SCORE										lNow_user_score;								//当前身上金币
};

//捕获鱼群 3次连环炸弹 1次电磁炮 3次钻头炮 1次闪电连锁 1次旋风鱼
struct CMD_S_CatchGroupFish
{
	WORD										wChairID;
	WORD										wGroupSytle;									//0连环炸弹 1电磁炮 2钻头炮 3闪电连锁 4旋风鱼,5鱼群中中的鱼王
	WORD										wGroupCount;									//次数
	int											nCatch_fish_count;
	SCORE										lNow_fish_score;								//当前显示渔分
	SCORE										lNow_user_score;								//当前身上金币
	int											nBullet_mul;									//多余字段 客户端设计BulletID可能消亡了 服务器直接发更合理
	int											nCatch_fish_id[60];								//最多60条 连锁总公共10条 [0]连锁自身
	int											nfish_score[60];								//连环砸蛋 钻头炮 利用[59]表示当前总共获得金币
};

//捕获鱼王
struct CMD_S_CatchGroupFishKing
{
	WORD										wChairID;
	int											nCatch_fish_count;
	SCORE										lNow_fish_score;								//当前显示渔分
	SCORE										lNow_user_score;								//当前身上金币
	SCORE										nTotal_fish_score;								//鱼的总共分数
	int											nBullet_mul;									//多余字段 客户端设计BulletID可能消亡了 服务器直接发更合理
	int											nCatch_fish_id[100];							//最多100条 连锁总公共10条 [0]连锁自身
	int											nfish_score[100];								//每条鱼对应得分数
};


//道具类型时间到
struct CMD_S_BulletToolTimeout
{
	WORD										wChairID;										//过期自动发射
};

//切换场景 没有tag 默认都是非特殊鱼
struct CMD_S_SwitchScene
{
	SceneKind									scene_kind;
	DWORD										nSceneStartTime;//新场景已经开始的时间

};

//返回子弹积分
struct CMD_S_ReturnBulletScore
{
	WORD										wChairID;
	WORD										wReasonType;
	SCORE										lNow_fish_score;								//当前显示渔分
	SCORE										lNow_user_score;								//当前身上金币
};

//库存操作 暂不实现
struct CMD_S_StockOperateResult
{
	BYTE										cbOperate_Code;
	TCHAR										szRetMSGManuel[128];							//自定义内容
};

//服务器决定随机三个定点位置
struct CMD_S_Bomb_Points
{
	WORD										wChairID;
	WORD										wPointX[3];										//连环炸弹2个点
	WORD										wPointY[3];
};

struct CMD_S_RotateBack
{
	WORD										wDegree;										//0-360
};

struct CMD_S_Free_Time
{
	WORD										wChairID;
	int											nBullet_mul;									//具体炮值
	BulletKind									wBulletKind;									//
};
//////////////////////////////////////////////////////////////////////////
// 客户端命令

#define 										SUB_C_EXCHANGE_FISHSCORE            1			//兑换鱼分
#define 										SUB_C_USER_FIRE                     2			//用户开火
#define 										SUB_C_CATCH_FISH                    3			//捕获鱼
#define 										SUB_C_CATCH_GROUP_FISH              4			//捕获多条
#define 										SUB_C_STOCK_OPERATE                 5			//库存操作 特殊需求 暂不实现
#define											SUB_C_CONTROL_WAI_GUA				6			//仅仅记录使用
#define											SUB_C_CATCH_FISH_TORPEDO			7			//鱼雷打鱼
#define											SUB_C_CATCH_GROUP_FISH_FISHKING		8			//捕捉鱼王
#define											SUB_C_SANGUAN_FIRE					9			//三管炮开火跟其他炮开火区分处理



//兑换积分
struct CMD_C_ExchangeFishScore
{
	bool										bIncrease;										//最小化操作范围 防止任何权限漏洞
};

//玩家开火 连环炸弹不走
struct CMD_C_UserFire
{
	int											fMouseX;
	int											fMouseY;
	int											nBullet_mulriple;								//子弹倍数
	int											nLock_fishid;									//锁定鱼ID 不一定有
	int											nBulletID;										//改为客户端流程
	int											nBullet_index;									//三管炮的子弹有012，其他炮都是0
	BYTE										cbBoomKind;										//鱼雷种类，0,1,2,3,4，代表鱼雷 子弹为INVALID_BYTE
	BulletKind									bullet_kind;									//子弹类型
};

//用户三管炮开火
struct CMD_C_SanGuanFire
{
	int											fMouseX[3];
	int											fMouseY[3];
	int											nBullet_mulriple;
	int											nStartBulletID;									//三颗子弹的起始子弹ID
	BulletKind									bullet_kind;									//子弹类型
};


//捕获鱼 排除连环炸弹 电磁炮 钻头炮
struct CMD_C_CatchFish
{
	WORD										wChairID;										//无用 加密处理 根据剩余子弹规律组合

	int											nFish_id;
	int											nBullet_id;										//查询确定int nBullet_mulriple BulletKind bullet_kind
	int											nBulletX;
	int											nBulletY;
	int											nBulletStartX;
	int											nBulletStartY;
	int											nFishX;
	int											nFishY;
	DWORD										dwMoveTime;										//鱼游动时间 服务器根据客户端直接验证
	DWORD										dwBulletTime;									//子弹游动时间
};

//捕获鱼群 连环炸弹 电磁炮 钻头炮
struct CMD_C_CatchGroupFish
{
	WORD										wChairID;										//无用 加密处理 根据剩余子弹规律组合

	int											nBullet_id;										//改为nBullet_id 子弹ID
	int											nCatch_fish_count;
	int											nFishPosX[80];									//
	int											nFishPosY[80];									//
	int											nCatch_fish_id[80];								//最多降低为80条
	DWORD										dwMoveTime[80];									//鱼游动时间 服务器根据客户端直接验证
};

//捕获鱼王
struct CMD_C_CatchGroupFishKing
{
	WORD										wChairID;										//无用 加密处理 根据剩余子弹规律组合

	int											nBullet_id;										//改为nBullet_id 子弹ID
	int											nCatch_fishKing_id;									//鱼王ID
	int											nCatch_fish_count;
	int											nFishPosX[100];									//
	int											nFishPosY[100];	
	int											nCatch_fish_id[100];								//最多降低为130条
	DWORD										dwMoveTime[100];									//鱼游动时间 服务器根据客户端直接验证
};

//鱼雷打鱼
struct CMD_C_CatchFish_Torpedo
{
	int											fish_id;										
	int 										bullet_id;
};

//库存操作
struct CMD_C_StockOperate
{
	BYTE										cbOperate_Code;									// 0查询 1 清除 2 增加 3 查询抽水
};

struct CMD_C_ControlWaiGua
{
	BYTE										cbAddUser;										//非零增加 零删除
	DWORD										dwGameID;
};

//回馈查询 协助风控看数据
struct CMD_C_MsgCheck
{
	TCHAR										szMessage[2048];
};

//#pragma pack()

#endif // CMD_FISH_H_