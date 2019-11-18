
#include "stdafx.h"
#include "game_config.h"
#include "xml/rapidxml.hpp"
#include "xml/rapidxml_utils.hpp"
#include "xml/rapidxml_iterators.hpp"

#include <iterator>
#include <Shlwapi.h>
using namespace rapidxml;

bool GameConfig::load_first_ = true;

//税收和独立库存
SCORE GameConfig::g_revenue_score = 0;
SCORE GameConfig::g_stock_Kind[FISH_KIND_COUNT] = {0};
SCORE GameConfig::g_stock_score_old = 10000000;

GameConfig::GameConfig()
{
	g_dRandValue = 1.0;
	g_tTimeSecond = 3600;
	memset(distribute_interval, 0, sizeof(distribute_interval));
	memset(fish_capture_probability_, 0, sizeof(fish_capture_probability_));
	memset(fish_capture_probability_Copy, 0, sizeof(fish_capture_probability_Copy));//打鱼难度 - 备份

	ZeroMemory(fish_new_min_cnt,sizeof(fish_new_min_cnt));
	ZeroMemory(fish_new_max_cnt,sizeof(fish_new_max_cnt));
	ZeroMemory(fish_fire_min_cnt,sizeof(fish_fire_min_cnt));
	ZeroMemory(fish_fire_max_cnt,sizeof(fish_fire_max_cnt));

	exchange_count_=1;
	exchange_ratio_userscore_=1;
	exchange_ratio_fishscore_=1;

	stock_crucial_count_ = 0;
	memset(stock_crucial_score_, 0, sizeof(stock_crucial_score_));
	memset(stock_multi_probability_, 0, sizeof(stock_multi_probability_));

	bomb_stock_count_ = 0;
	memset(bomb_stock_score_, 0, sizeof(bomb_stock_score_));
	memset(bomb_stock_multi_pro_, 0, sizeof(bomb_stock_multi_pro_));

	stock_init_gold_old=10000000;
	stock_crucial_count_old=0;
	ZeroMemory(stock_crucial_score_old,sizeof(stock_crucial_score_old));
	ZeroMemory(stock_multi_probability_old,sizeof(stock_multi_probability_old));

	stock_init_gold=50000000;
	bomb_stock_init_gold=50000000;

	m_ConfigYuQun.clear();
	m_ConfigYuZhen.clear();
}

GameConfig::~GameConfig()
{
	std::vector<tagYuQun*>::iterator iterx1;
	for (iterx1 = m_ConfigYuQun.begin(); iterx1 != m_ConfigYuQun.end(); ++iterx1)
	{
		delete(*iterx1);
	}
	m_ConfigYuQun.clear();

	std::vector<tagYuZhen*>::iterator iterx2;
	for (iterx2 = m_ConfigYuZhen.begin(); iterx2 != m_ConfigYuZhen.end(); ++iterx2)
	{
		delete(*iterx2);
	}
	m_ConfigYuZhen.clear();
}

bool GameConfig::LoadGameConfig(const TCHAR szPath[SERVER_LEN],WORD wServerID,bool bFristLoad)
{
	TCHAR file_name[256]={0};
	sprintf(file_name,"%s\\hw2FishConfig\\config_%d.xml",szPath,wServerID);

	if(PathFileExists(file_name)==FALSE)
	{
		CTraceService::TraceString("文件不存在", TraceLevel_Exception);
		return false;
	}

	try
	{
		//错误读取统计
		WORD wErrorRead=0;

		bool bExecuteRand = false;
		int tTimeSecond_old = g_tTimeSecond;
		double dRandValue_old = g_dRandValue;

		rapidxml::file<> xml_file(file_name);
		rapidxml::xml_document<> xml_doc;
		xml_doc.parse<0>(xml_file.data());

		//! 获取根节点
		rapidxml::xml_node<>* xml_element = xml_doc.first_node("Config");
		if(xml_element==0)
		{
			return false;
		}

		int fish_count = 0, bullet_kind_count = 0;
		for (rapidxml::xml_node<>*xml_child = xml_element->first_node();xml_child; xml_child = xml_child->next_sibling())
		{
			//老方式库存
			if (!strcmp(xml_child->name(), "StockOld"))
			{
				xml_attribute<> *node1_attr1 = xml_child->first_attribute("InitBase");
				if (node1_attr1)
				{
					if(node1_attr1->name_size()>0)
					{
						if(bFristLoad)
						{
							stock_init_gold_old=atoi(node1_attr1->value());
							g_stock_score_old=stock_init_gold_old;
						}
					}
				}
				int nStockTmp=0;
				for (rapidxml::xml_node<>* xml_stock = xml_child->first_node(); xml_stock; xml_stock = xml_stock->next_sibling())
				{
					for (xml_attribute<> *xml_attr = xml_stock->first_attribute();
						xml_attr; xml_attr = xml_attr->next_attribute())
					{
						if(xml_attr->name_size()<=0) continue;

						if(!strcmp(xml_attr->name(),"stockScore"))
						{
							if(xml_attr->value_size()>0) stock_crucial_score_old[nStockTmp]=atoi(xml_attr->value());
							else wErrorRead++;
						}
						else if(!strcmp(xml_attr->name(),"MultiProbability"))
						{
							if(xml_attr->value_size()>0) stock_multi_probability_old[nStockTmp]=atof(xml_attr->value());
							else wErrorRead++;
						}
					}

					++nStockTmp;
					if (nStockTmp >= 20) break;
				}
				stock_crucial_count_old=nStockTmp;
			}

			//库存
			if (!strcmp(xml_child->name(), "Stock"))
			{
				xml_attribute<> *node1_attr1 = xml_child->first_attribute("InitBase");
				if(node1_attr1->name_size()>0)
				{
					if(bFristLoad)
					{
						stock_init_gold=_atoi64(node1_attr1->value());
						for(int f=0;f<13;f++) g_stock_Kind[f]=stock_init_gold;
					}
				}
				else wErrorRead++;

				xml_attribute<> *node1_attr2 = xml_child->first_attribute("MinRang");
				if(node1_attr2->name_size()>0)
				{
					stock_stock_min_range=atoi(node1_attr2->value());
				}
				else wErrorRead++;

				int nStockTmp=0; 
				for (rapidxml::xml_node<>* xml_stock = xml_child->first_node(); xml_stock; xml_stock = xml_stock->next_sibling())
				{
					for (xml_attribute<> *xml_attr = xml_stock->first_attribute();
						xml_attr; xml_attr = xml_attr->next_attribute())
					{
						if(xml_attr->name_size()<=0) continue;

						if(!strcmp(xml_attr->name(),"stockScore"))
						{
							if(xml_attr->value_size()>0) stock_crucial_score_[nStockTmp]=atoi(xml_attr->value());
							else wErrorRead++;
						}
						else if(!strcmp(xml_attr->name(),"MultiProbability"))
						{
							if(xml_attr->value_size()>0) stock_multi_probability_[nStockTmp]=atof(xml_attr->value());
							else wErrorRead++;
						}
					}

					++nStockTmp;
					if (nStockTmp >= 20) break;
				}
				stock_crucial_count_=nStockTmp;
			}
			else if (!strcmp(xml_child->name(), "BombStock"))
			{
				xml_attribute<> *node1_attr1 = xml_child->first_attribute("InitBase");
				if(node1_attr1->name_size()>0)
				{
					if(bFristLoad)
					{
						bomb_stock_init_gold=_atoi64(node1_attr1->value());
						for(int f=13;f<FISH_KIND_COUNT;f++) g_stock_Kind[f]=bomb_stock_init_gold;
					}
				}
				else wErrorRead++;

				int nStockTmp=0; 
				for (rapidxml::xml_node<>* xml_stock = xml_child->first_node(); xml_stock; xml_stock = xml_stock->next_sibling())
				{
					for (xml_attribute<> *xml_attr = xml_stock->first_attribute();
						xml_attr; xml_attr = xml_attr->next_attribute())
					{
						if(xml_attr->name_size()<=0) continue;

						if(!strcmp(xml_attr->name(),"stockScore"))
						{
							if(xml_attr->value_size()>0) bomb_stock_score_[nStockTmp]=atoi(xml_attr->value());
							else wErrorRead++;
						}
						else if(!strcmp(xml_attr->name(),"MultiProbability"))
						{
							if(xml_attr->value_size()>0) bomb_stock_multi_pro_[nStockTmp]=atof(xml_attr->value());
							else wErrorRead++;
						}
					}

					++nStockTmp;
					if (nStockTmp >= 10) break;
				}
				bomb_stock_count_=nStockTmp;
			}
			else if (!strcmp(xml_child->name(), "ScoreExchange"))
			{
				for (xml_attribute<> *xml_attr = xml_child->first_attribute();
					xml_attr; xml_attr = xml_attr->next_attribute())
				{
					if(xml_attr->name_size()<=0) continue;

					if(!strcmp(xml_attr->name(),"exchangeRatio"))
					{
						if(xml_attr->value_size()>0)
						{
							const char* attri = xml_attr->value();

							char* temp = NULL;
							exchange_ratio_userscore_ = strtol(attri, &temp, 10);
							exchange_ratio_fishscore_ = strtol(temp + 1, &temp, 10);
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"exchangeCount"))
					{
						if(xml_attr->value_size()>0) exchange_count_=atoi(xml_attr->value());
						else wErrorRead++;
					}
				}
			}
			else if (!strcmp(xml_child->name(), "Cannon"))
			{
				rapidxml::xml_attribute<>* xml_attr = xml_child->first_attribute("cannonMultiple");

				if(xml_attr && xml_attr->value_size()>0)
				{
					const char* attri = xml_attr->value();
					char* temp = NULL;
					min_bullet_multiple_ = strtol(attri, &temp, 10);
					max_bullet_multiple_ = strtol(temp + 1, &temp, 10);
				}
				else wErrorRead++;
			}
			//读配制数据 - 随机难度
			else if (!strcmp(xml_child->name(), "ProbabilityRand"))
			{
				//读取配制数据
				for (xml_attribute<> *xml_attr = xml_child->first_attribute();
					xml_attr; xml_attr = xml_attr->next_attribute())
				{
					if(xml_attr->name_size()<=0) continue;

					if(!strcmp(xml_attr->name(),"RandValue"))
					{
						if(xml_attr->value_size()>0)
						{
							g_dRandValue = atof(xml_attr->value());
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"TimeSecond"))
					{
						if(xml_attr->value_size()>0)
						{
							g_tTimeSecond = atoi(xml_attr->value());
						}
						else wErrorRead++;
					}
				}
				//
			}
			else if (!strcmp(xml_child->name(), "Fish"))
			{
				int fish_kind;

				//必须有kindid
				rapidxml::xml_attribute<>* xml_attr = xml_child->first_attribute("kind");
				if(xml_attr && xml_attr->value_size()>0) fish_kind = atoi(xml_attr->value());
				else
				{
					wErrorRead++;
					return false;
				}
				if (fish_kind >= FISH_KIND_COUNT || fish_kind < 0)
				{
					return false;
				}

				for (xml_attribute<> *xml_attr = xml_child->first_attribute();
					xml_attr; xml_attr = xml_attr->next_attribute())
				{
					if(xml_attr->name_size()<=0) continue;

					//////////////////////////////////////////////////////////////////////////
					if(!strcmp(xml_attr->name(),"multiple"))
					{
						if(xml_attr->value_size()>0)
						{
							const char* attri = xml_attr->value();
							if (fish_kind >= FISH_KIND_17 && fish_kind<=FISH_KIND_18)
							{
								char* temp = NULL;
								fish_multiple_[fish_kind] = strtol(attri, &temp, 10);
								fish_multiple_max[fish_kind] = strtol(temp + 1, &temp, 10);
							}
							else if (fish_kind >= FISH_KIND_20 && fish_kind<=FISH_SMALL_2_BIG3)
							{
								char* temp = NULL;
								fish_multiple_[fish_kind] = strtol(attri, &temp, 10);
								fish_multiple_max[fish_kind] = strtol(temp + 1, &temp, 10);
							}
							else
							{
								fish_multiple_[fish_kind] = atoi(xml_attr->value());
								fish_multiple_max[fish_kind] = fish_multiple_[fish_kind];
							}
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"hitCounts"))
					{
						if(xml_attr->value_size()>0)
						{
							char* temp = NULL;
							const char* attri = xml_attr->value();

							//m_distribulte_fish[fish_kind].cbMinNewCnt
							fish_fire_min_cnt[fish_kind] = strtol(attri, &temp, 10);
							fish_fire_max_cnt[fish_kind] = strtol(temp + 1, &temp, 10);
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"hitRadius"))
					{
						if(xml_attr->value_size()>0)
						{
							fish_hit_radius_[fish_kind] = atoi(xml_attr->value());
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"captureProbability"))
					{
						if(xml_attr->value_size()>0)
						{
							//第一次启动
							if (load_first_ == true)
							{
								fish_capture_probability_[fish_kind] = atof(xml_attr->value());
								fish_capture_probability_Copy[fish_kind] = atof(xml_attr->value());
							}
							else
							{
								//备份难度
								double dTmep = fish_capture_probability_Copy[fish_kind];
								fish_capture_probability_Copy[fish_kind] = atof(xml_attr->value());
								if (dTmep != fish_capture_probability_Copy[fish_kind]) bExecuteRand = true;
							}
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"distribute_region"))
					{
						if(xml_attr->value_size()>0)
						{
							char* temp = NULL;
							const char* attri = xml_attr->value();

							fish_new_min_cnt[fish_kind] = strtol(attri, &temp, 10);
							fish_new_max_cnt[fish_kind] = strtol(temp + 1, &temp, 10);
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"distribute_interval"))
					{
						if(xml_attr->value_size()>0)
						{
							distribute_interval[fish_kind] = atof(xml_attr->value());
						}
						else wErrorRead++;
					}
					else if(!strcmp(xml_attr->name(),"IsNotRand"))
					{
						if(xml_attr->value_size()>0)
						{
							//是否不参与随机
							int nTmep = g_IsNotRand[fish_kind];
							g_IsNotRand[fish_kind]=atoi(xml_attr->value());

							if (nTmep != g_IsNotRand[fish_kind]) bExecuteRand = true;
						}
						else wErrorRead++;
					}
				}

				++fish_count;
			}
			else if (!strcmp(xml_child->name(), "YuQun"))
			{
				if (load_first_ == true && m_ConfigYuQun.size()<=0)
				{
					for (rapidxml::xml_node<>* xml_node1 = xml_child->first_node(); xml_node1; xml_node1 = xml_node1->next_sibling())
					{
						int xwrong=0;
						tagYuQun *ItemTmp=new tagYuQun();
						for (xml_attribute<> *xml_attr = xml_node1->first_attribute();
							xml_attr; xml_attr = xml_attr->next_attribute())
						{
							if(xml_attr->name_size()<=0) continue;

							if(!strcmp(xml_attr->name(),"style"))
							{
								if(xml_attr->value_size()>0) ItemTmp->nStyleID=atoi(xml_attr->value());
								else {wErrorRead++; xwrong++;}
							}
							else if(!strcmp(xml_attr->name(),"oncecount"))
							{
								if(xml_attr->value_size()>0) ItemTmp->wFishCount=atof(xml_attr->value());
								else {wErrorRead++; xwrong++;}
							}
						}
						if(xwrong==0) m_ConfigYuQun.push_back(ItemTmp);
						else delete ItemTmp;
					}
				}
			}
			else if (!strcmp(xml_child->name(), "YuZhen"))
			{
				if (load_first_ == true && m_ConfigYuZhen.size()<=0)
				{
					for (rapidxml::xml_node<>* xml_node1 = xml_child->first_node(); xml_node1; xml_node1 = xml_node1->next_sibling())
					{
						int xwrong=0;
						tagYuZhen *ItemTmp=new tagYuZhen();
						xml_attribute<> *node1_attr1 = xml_node1->first_attribute("style");
						if(node1_attr1->name_size()>0)
						{
							ItemTmp->nStyleID=atoi(node1_attr1->value());
						}
						else {wErrorRead++; xwrong++;}

						xml_attribute<> *node1_attr2 = xml_node1->first_attribute("onlinetime");
						if(node1_attr2->name_size()>0)
						{
							ItemTmp->dwAllLifeTime=atoi(node1_attr2->value());
						}
						else {wErrorRead++; xwrong++;}

						for (rapidxml::xml_node<>* xml_node2 = xml_node1->first_node(); xml_node2; xml_node2 = xml_node2->next_sibling())
						{
							xml_attribute<> *node2_attr1 = xml_node2->first_attribute("fishkind");
							if(node2_attr1->name_size()>0)
							{
								int xd=atoi(node2_attr1->value());
								ItemTmp->ArrayKind.push_back(xd);
							}
							else {wErrorRead++; xwrong++;}

							xml_attribute<> *node2_attr2 = xml_node2->first_attribute("region");
							if(node2_attr2->name_size()>0)
							{
								const char* attri = node2_attr2->value();
								char* temp = NULL;
								DWORD one = strtol(attri, &temp, 10);
								DWORD two = strtol(temp + 1, &temp, 10);
								DWORD dwTmp=(one<<16)|(two&0xFFFF);
								ItemTmp->ArrayRegion.push_back(dwTmp);
								//_tprintf("鱼阵范围【%d-%d】\n",one,two);//上线务必去掉
							}
							else {wErrorRead++; xwrong++;}
						}

						//保存
						if(xwrong==0) m_ConfigYuZhen.push_back(ItemTmp);
						else delete ItemTmp;
					}
				}
			}
			else if (!strcmp(xml_child->name(), "Bullet"))
			{
				int bullet_kind;

				rapidxml::xml_attribute<>* xml_attr = xml_child->first_attribute("kind");
				if(xml_attr && xml_attr->value_size()>0) bullet_kind = atoi(xml_attr->value());
				else
				{
					wErrorRead++;
					return false;
				}
				if (bullet_kind >= BULLET_KIND_COUNT || bullet_kind < 0)
				{
					return false;
				}


				for (xml_attribute<> *xml_attr = xml_child->first_attribute();
					xml_attr; xml_attr = xml_attr->next_attribute())
				{
					if(xml_attr->name_size()<=0) continue;

					if(!strcmp(xml_attr->name(),"speed"))
					{
						if(xml_attr->value_size()>0)
						{
							bullet_speed_[bullet_kind] = atoi(xml_attr->value());
						}
						else wErrorRead++;
					}
				}

				++bullet_kind_count;
			}
		}

		////大配制有改变
		//if (tTimeSecond_old != g_tTimeSecond || nFishCount_old != g_nFishCount || dRandValue_old != g_dRandValue) bExecuteRand = true;

		////设置定时器
		//if (bExecuteRand == true)
		//{
		//	if (g_tTimeSecond > 0 && g_nFishCount > 0 && g_dRandValue > 0.0)
		//	{
		//		//启动时主动调用
		//		OnTimerMessage(IDI_FISHCAPTUREPROBABILITYRANDSETUP, 0);

		//		//修改难度定时器
		//		if (tTimeSecond_old != g_tTimeSecond && g_tTimeSecond > 0)
		//		{
		//			table_frame_->SetGameTimer(IDI_FISHCAPTUREPROBABILITYRANDSETUP, g_tTimeSecond * 1000, INVALID_DWORD, 0);
		//		}
		//	}
		//}

		if (fish_count != FISH_KIND_COUNT)
		{
			xml_doc.clear();
			return false;
		}
		if (bullet_kind_count != BULLET_KIND_COUNT)
		{
			xml_doc.clear();
			return false;
		}

		if(wErrorRead>0)
		{
			xml_doc.clear();
			CString strError;
			strError.Format(TEXT("未能读取配置的个数：%d\n"),wErrorRead);
			CTraceService::TraceString(strError, TraceLevel_Exception);
		}
	}
	catch (rapidxml::parse_error &e)
	{
		CString strError;
		strError = "读取XML错误";

		strError+= e.what();
		char *pError = e.where<char>();

		//足够找位置即可
		char szTemp[64]={0};
		if(pError)
		{
			_snprintf(szTemp,sizeof(szTemp),"%s",pError);
			szTemp[63]=0;
		}

		strError+=szTemp;
		CTraceService::TraceString(strError, TraceLevel_Exception);

		return false;
	}
	catch (...)
	{
		CTraceService::TraceString(_T("XML配置文件异常"), TraceLevel_Exception);
	}

	load_first_ = false;

	//static int xdf=0;
	//_tprintf("%08d--%d\n",++xdf,GetTickCount());

	return true;
}
