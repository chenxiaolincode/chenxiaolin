#include "StdAfx.h"
#include "ServerMoveManage.h"
#include <math.h>
#include "table_frame_sink.h"

//////////////////////////////////////////////////////////////////////////

//辅助信息
const int ResolutionWidth = kResolutionWidth;	//默认分辨率宽度
const int ResolutionHeight = kResolutionHeight;	//默认分辨率高度

//////////////////////////////////////////////////////////////////////////
//
//#pragma pack(push)
//#pragma pop(1)
struct tagPoints
{
	WORD wPointX;
	WORD wPointY;
};
//记录参数
struct tagRecordVer
{
	WORD							wVersion;							//配置版本 总运行时间
	WORD							wtagPointsCnt;						//文件大小
//	DWORD							dwTotalTickt;						//毫秒级别
};
//#pragma pack(pop)

/* 早期读写原型
if(1)
{
	CFile ResourceFile;
	if (ResourceFile.Open(strCurrentDir,CFile::modeWrite|CFile::modeCreate,NULL)==FALSE) throw 0;

	tagRecordVer verTemp;
	verTemp.wVersion=1234;
	verTemp.wtagPointsCnt=12;
	ResourceFile.Write(&verTemp,sizeof(tagRecordVer));

	tagPoints pointTmp[11];
	ZeroMemory(pointTmp,sizeof(pointTmp));
	for (int i=0;i<11;i++)
	{
		pointTmp[i].wPointX=i;
		pointTmp[i].wPointY=i*100;
	}
	ResourceFile.Write(pointTmp,sizeof(tagPoints)*11);

	tagPoints pointEx;
	pointEx.wPointX=99;
	pointEx.wPointY=999;
	ResourceFile.Write(&pointEx,sizeof(tagPoints)*1);

	ResourceFile.Flush();
	ResourceFile.Close();
}


if(1)
{
	CFile ResourceFile;
	if (ResourceFile.Open(strCurrentDir,CFile::modeRead,NULL)==FALSE) throw 0;

	ULONGLONG uLong=ResourceFile.GetLength();
	BOOL bRight=((uLong-sizeof(tagRecordVer))%sizeof(tagPoints)==0)?TRUE:FALSE;
	ASSERT(bRight);
	if(FALSE==bRight) throw 0;

	tagRecordVer VerCheck;
	ZeroMemory(&VerCheck,sizeof(VerCheck));
	ResourceFile.Read(&VerCheck,sizeof(tagRecordVer));
	CString strVer;
	strVer.Format(TEXT("%d"),VerCheck.wVersion);
	AfxMessageBox(strVer);

	UINT uReadSize=VerCheck.wtagPointsCnt;

	ResourceFile.Seek(sizeof(tagRecordVer),0);
	tagPoints *pReadData = new tagPoints[uReadSize];
	memset(pReadData,0,sizeof(tagPoints)*uReadSize);

	UINT uReadCount=ResourceFile.Read(pReadData,sizeof(tagPoints)*uReadSize);
	if (uReadCount!=sizeof(tagPoints)*VerCheck.wtagPointsCnt) throw 0;

	for (int i=0;i<uReadSize;i++)
	{
		int x=(pReadData+i)->wPointX;
		int y=(pReadData+i)->wPointY;
		int z=x+y;
		TRACE("%-02d--%d,%d\n",i,x,y);
	}

	if(pReadData)
	{
		delete []pReadData;
		pReadData=0;
	}

	ResourceFile.Close();
}
*/

CPathShare::CPathShare()
{
	m_cbNowReadStyle=0;
	for(int i=0;i<4;i++)
	{
		m_FishPathPoints[i].clear();
	}
}

CPathShare::~CPathShare()
{
	for (int i=0;i<4;i++)
	{
		std::vector<tagFishPath*>::iterator iter;
		for (iter = m_FishPathPoints[i].begin(); iter != m_FishPathPoints[i].end(); ++iter)
		{
			delete(*iter);
		}

		m_FishPathPoints[i].clear();
	}
}

CPathShare &CPathShare::GetInstance()
{
	static CPathShare tmp;
	return tmp;
}

bool CPathShare::LoadAllPath()
{
	static bool bLoad=false;
	if(bLoad==true) return true;
	bLoad=true;
	CString strCurrentDir;
	TCHAR szModuleFile[MAX_PATH]={0};
	GetModuleFileName(0, szModuleFile, MAX_PATH);
	//PathRemoveFileSpec(szModuleFile);
	//PathAddBackslash(szModuleFile);
	for (size_t i=lstrlen(szModuleFile)-1;i>0;i--)
	{
		if(szModuleFile[i]=='\\')
		{
			szModuleFile[i]=0;
			break;
		}
	}

	//加载samll路径
	m_cbNowReadStyle=0;
	strCurrentDir.Format(TEXT("%s\\hw2FishConfig\\small"),szModuleFile);
	ScanALlFile(strCurrentDir);

	//加载big路径
	m_cbNowReadStyle=1;
	strCurrentDir.Format(TEXT("%s\\hw2FishConfig\\big"),szModuleFile);
	ScanALlFile(strCurrentDir);

	//加载huge路径
	m_cbNowReadStyle=2;
	strCurrentDir.Format(TEXT("%s\\hw2FishConfig\\huge"),szModuleFile);
	ScanALlFile(strCurrentDir);

	//加载special路径
	m_cbNowReadStyle=3;
	strCurrentDir.Format(TEXT("%s\\hw2FishConfig\\special"),szModuleFile);
	ScanALlFile(strCurrentDir);


	return true;
}

tagFishPath * CPathShare::GetAllPointCount(PathSyle pathstyle, WORD wPathID)
{
	if(pathstyle>en_PathCount) return 0;

	for (size_t i=0;i<m_FishPathPoints[pathstyle].size();i++)
	{
		tagFishPath *pItem=m_FishPathPoints[pathstyle][i];
		if(wPathID != pItem->wPathID) continue;

		return pItem;
	}

	return 0;
}

void CPathShare::ScanALlFile(LPCTSTR lpPathName)
{
	if(lpPathName==0) return;

	CFileFind finder;
	CString tempPath;
	tempPath.Format("%s%s", lpPathName, "//*.txt");
	BOOL bWork = finder.FindFile(tempPath);
	while(bWork)
	{		
		bWork = finder.FindNextFile();
		if(!finder.IsDots())
		{
			if(finder.IsDirectory())
			{
				ScanALlFile(finder.GetFilePath());
			}
			else
			{
				if(m_cbNowReadStyle>=4) return;
				LoadOnePathItem(finder.GetFilePath());
			}
		}
	}
	finder.Close();
}

bool CPathShare::LoadOnePathItem(LPCTSTR lpFileName)
{
	if(NULL==lpFileName) return false;

	CString strFileName=lpFileName;

	CFile ResourceFile;
	if (ResourceFile.Open(strFileName,CFile::modeRead,NULL)==FALSE) return false;

	ULONGLONG uLong=ResourceFile.GetLength();
	BOOL bRight=((uLong-sizeof(tagRecordVer))%sizeof(tagPoints)==0)?TRUE:FALSE;
	ASSERT(bRight);
	if(FALSE==bRight) return false;

	tagRecordVer VerCheck;
	ZeroMemory(&VerCheck,sizeof(VerCheck));
	ResourceFile.Read(&VerCheck,sizeof(tagRecordVer));

	UINT uReadSize=VerCheck.wtagPointsCnt;
	//ResourceFile.Seek(sizeof(tagRecordVer),0);
	tagPoints *pReadData = new tagPoints[uReadSize];
	memset(pReadData,0,sizeof(tagPoints)*uReadSize);

	UINT uReadCount=ResourceFile.Read(pReadData,sizeof(tagPoints)*uReadSize);
	if (uReadCount!=sizeof(tagPoints)*VerCheck.wtagPointsCnt) return false;
	ResourceFile.Close();

	WORD wIndex=m_cbNowReadStyle;
	tagFishPath *pPathTmp=new tagFishPath();
	pPathTmp->dwTotalTickt=kNormolFishTime;//VerCheck.wVersion;//dwTotalTickt;

	int nBegin=strFileName.ReverseFind('/');
	if(nBegin==-1) nBegin=strFileName.ReverseFind('\\');
	if(nBegin!=-1) nBegin+=1;
	int nEnd=strFileName.ReverseFind('.');
	int nPathID=(WORD)m_FishPathPoints[wIndex].size();
	if(nBegin>0 && nEnd-nBegin>0)
	{
		CString strID=strFileName.Mid(nBegin,nEnd-nBegin);
		nPathID=atoi(strID);
	}
	pPathTmp->wPathID=nPathID;//(WORD)m_FishPathPoints[wIndex].size();
	for (size_t i=0;i<uReadSize;i++)
	{
		WORD x=(pReadData+i)->wPointX;
		WORD y=(pReadData+i)->wPointY;
		//TRACE("%-02d--%d,%d\n",i,x,y);

		DWORD dwTemp=(x<<16)|y;
		pPathTmp->TwoPoint.push_back(dwTemp);
	}
	m_FishPathPoints[wIndex].push_back(pPathTmp);

	if(pReadData)
	{
		delete []pReadData;
		pReadData=0;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
#define						UPDATE_TIME_FISH								30
#define						UPDATE_TIME_BULLET								30

//////////////////////////////////////////////////////////////////////////
CFishMoveManage::CFishMoveManage(void)
{
	m_nActiveIndex=0;

	m_ActiveFish.clear();
	ZeroMemory(m_FishMoveData,sizeof(m_FishMoveData));
}


CFishMoveManage::~CFishMoveManage(void)
{
	m_nActiveIndex=0;

	m_ActiveFish.clear();
}

bool CFishMoveManage::ActiveFish(int nFishID, int nPathID, DWORD dwManueTime)
{
	//最大维持500运行
	if(nFishID<0) return false;
	if(m_ActiveFish.size()>=MAX_CONTAIN_FISH_NUM) return false;

	//
	int nIndex=m_nActiveIndex;
	m_FishMoveData[nIndex].nID=nFishID;
	m_FishMoveData[nIndex].dwActiveTime=GetTickCount();
	m_FishMoveData[nIndex].nPathID=nPathID;
	m_FishMoveData[nIndex].dwPathLineTime=0;

	if(nPathID!=-1)
	{
		WORD wPathStyle=WORD(nPathID/10000);
		WORD wPathID=WORD(nPathID%1000);
	
		//tagFishPath *pPath=PATH_SHARE.GetAllPointCount(PathSyle(wPathStyle),wPathID);
		//ASSERT(pPath!=0);
		//if(pPath)
		//{
		//	//根据id约定规则 加减总时间即可
		//	m_FishMoveData[nIndex].dwPathLineTime=(pPath->dwTotalTickt/1000-rand()%5)*1000;
		//#ifdef _DEBUG
		//	m_FishMoveData[nIndex].dwPathLineTime=pPath->dwTotalTickt;
		//#endif
		//}
	}
	if(dwManueTime!=0)
	{
		m_FishMoveData[nIndex].dwPathLineTime=dwManueTime;
	}

	m_ActiveFish.push_back(&m_FishMoveData[nIndex]);

	m_nActiveIndex=(nIndex+1)%MAX_CONTAIN_FISH_NUM;

	return true;
}

bool CFishMoveManage::DeleteFish(int nFishID)
{
	if(nFishID==-1)
	{
		m_nActiveIndex=0;
		m_ActiveFish.clear();
		return true;
	}
	else
	{
		IteratorFish it=m_ActiveFish.begin();
		while (it!=m_ActiveFish.end())
		{
			tagFishMove *pItem=*it;
			if(pItem->nID==nFishID)
			{
				it = m_ActiveFish.erase(it);
				return true;
			}
			else it++;
		}
	}
	return false;
}

int CFishMoveManage::Update(int nDelFishID[], int &nDelCount)
{
	nDelCount=0;
	tagFishMove *pItem=0;
	for (IteratorFish it=m_ActiveFish.begin();it!=m_ActiveFish.end();)
	{
		pItem = *it;

		__int64 tick_count_elapsed=GetElapsedTm(pItem->dwActiveTime);

		//5s 误差回调删除
		if(pItem->dwPathLineTime!=0 && tick_count_elapsed>pItem->dwPathLineTime+500)
		{
			if(nDelCount>50) break;
			nDelFishID[nDelCount++]=pItem->nID;

			it=m_ActiveFish.erase(it);
		}
		else it++;
	}
	return nDelCount;
}

tagFishMove *CFishMoveManage::GetFish(int nFishID)
{
	tagFishMove *pItem=0;
	for (IteratorFish it=m_ActiveFish.begin();it!=m_ActiveFish.end();it++)
	{
		pItem = *it;

		if(pItem->nID==nFishID)
		{
			break;
		}
	}
	return pItem;
}

//////////////////////////////////////////////////////////////////////////
//子弹控制
//////////////////////////////////////////////////////////////////////////
CBulletMoveManage::CBulletMoveManage(void)
{
	m_nActiveIndex=0;
	m_ActiveBullet.clear();
	ZeroMemory(m_BulletMoveData,sizeof(m_BulletMoveData));
}


CBulletMoveManage::~CBulletMoveManage(void)
{
	m_nActiveIndex=0;
	m_ActiveBullet.clear();
}

bool CBulletMoveManage::ActiveBullet(int nBulletID, WORD wChairID, float fSpeed, float fx, float fy, float fMouseX, float fMouseY)
{
	//最大维持500运行
	if(nBulletID<0) return false;
	if(m_ActiveBullet.size()>=MAX_CONTAIN_BULLET_NUM) return false;

	int nIndex=m_nActiveIndex;
	m_BulletMoveData[nIndex].nID=nBulletID;
	m_BulletMoveData[nIndex].wChairID=wChairID;
	
	m_BulletMoveData[nIndex].fSpeed=fSpeed;
	m_BulletMoveData[nIndex].fInitPosX=fx;
	m_BulletMoveData[nIndex].fInitPosY=fy;
	m_BulletMoveData[nIndex].fMouseX=fMouseX;
	m_BulletMoveData[nIndex].fMouseY=fMouseY;
	m_BulletMoveData[nIndex].dwActiveTm=GetTickCount();
	m_ActiveBullet.push_back(&m_BulletMoveData[nIndex]);

	m_nActiveIndex=(nIndex+1)%MAX_CONTAIN_BULLET_NUM;

	return true;
}

bool CBulletMoveManage::DeleteBullet(int nBulletID, WORD wChairID)
{
	if(nBulletID==-1)
	{
		m_nActiveIndex=0;
		m_ActiveBullet.clear();
		return true;
	}
	else
	{
		IteratorBullet it=m_ActiveBullet.begin();
		while (it!=m_ActiveBullet.end())
		{
			tagBulletMove *pItem=*it;
			if(pItem->nID==nBulletID && pItem->wChairID==wChairID)
			{
				it = m_ActiveBullet.erase(it);
				return true;
			}
			else it++;
		}
	}
	return false;
}

tagBulletMove *CBulletMoveManage::GetBullet(int nBulletID, WORD wChairID)
{
	tagBulletMove *pItem=0;
	for (IteratorBullet it=m_ActiveBullet.begin();it!=m_ActiveBullet.end();it++)
	{
		pItem = *it;

		if(pItem->nID==nBulletID && pItem->wChairID==wChairID)
		{
			break;
		}
	}
	return pItem;
}

//nPosX1起点
bool CBulletMoveManage::GetBulletPoint(int &nPosXOut, int &nPosYOut, float fSpeed, DWORD dwElapseTime, float nPosX1, float nPosY1, float nPosX2, float nPosY2)
{
	//特殊点发射
	if(nPosX1==nPosX2 && nPosY1==nPosY2) return false;

	//nDirection 1234 对应 上下左右
	int nDirection=0;
	if(nPosX1==nPosX2)
	{
		if(nPosY1>nPosY2) nDirection=1;
		else nDirection=2;
	}
	else if(nPosY1==nPosY2)
	{
		if(nPosX1>nPosX2) nDirection=3;
		else nDirection=3;
	}

	//x,y方向折次数
	int nCountX=0,nCountY=0;
	//单位毫秒对应速度转换
	float nAllDistance=fSpeed*float(dwElapseTime);
	int nDistanceX=0;
	int nDistanceY=0;

	if(nDirection==0)
	{
		float x1=nPosX1,x2=nPosX2;
		float y1=nPosY1,y2=nPosY2;

		float distance = sqrtf((x1 - x2) *(x1 - x2) + (y1 - y2) *(y1 - y2));
		float cosin_value = (x2 - x1) / distance;
		float fRotate = acosf(cosin_value);
		if (y1 < y2) fRotate = 2 * M_PI - fRotate;
		nDistanceX=nPosX1+(int)(cos(fRotate)*nAllDistance);
		nDistanceY=nPosY1+(int)(-1*sin(fRotate)*float(nAllDistance));//浮点型无法还原 误差肯定存在

		if(nDistanceX<0) nDistanceX*=-1;
		if(nDistanceY<0) nDistanceY*=-1;
	}
	else
	{
		//两种情况需要加一次折返
		switch(nDirection)
		{
		case 1:
			{
				nDistanceX=nPosX1;
				nDistanceY=nPosY1+int(fSpeed*dwElapseTime);
			}
			break;
		case 2:
			{
				nDistanceX=nPosX1;
				nDistanceY=nPosY1+int(fSpeed*dwElapseTime);
			}
			break;
		case 3:
			{
				nDistanceY=nPosY1;
				nDistanceX=nPosX1+int(fSpeed*dwElapseTime);
			}
			break;
		case 4:
			{
				nDistanceY=nPosY1;
				nDistanceX=nPosX1+int(fSpeed*dwElapseTime);
			}
			break;
		}
		if(nDistanceX<0) nDistanceX*=-1;
		if(nDistanceY<0) nDistanceY*=-1;
	}

	//折返次数
	nCountX+=nDistanceX/kResolutionWidth;
	nCountY+=nDistanceY/kResolutionHeight;

	//取模剩余
	int nModX=0,nModY=0;
	nModX=nDistanceX%kResolutionWidth;
	nModY=nDistanceY%kResolutionHeight;

	if(1==nCountX%2)//奇数次折返
	{
		nPosXOut=kResolutionWidth-nModX;
	}
	else nPosXOut=nModX;

	if(1==nCountY%2)//奇数次折返
	{
		nPosYOut=kResolutionHeight-nModY;
	}
	else nPosYOut=nModY;
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
CCollisionManage::CCollisionManage(void)
{
	m_pFish=0;
	m_pBullet=0;
}


CCollisionManage::~CCollisionManage(void)
{
	m_pFish=0;
	m_pBullet=0;
}

void CCollisionManage::SetInputObject(CFishMoveManage *pFish, CBulletMoveManage *pBullet)
{
	m_pFish=pFish;
	m_pBullet=pBullet;
}

int CCollisionManage::IsCross(tagCrossCheck *pCheckData)
{
	return 0;//

	if(m_pFish==0 || m_pBullet==0) return -1;

	int nBulletID=pCheckData->nBulletID;
	WORD wChairID=pCheckData->wChairID;
	int nBulletX_C=pCheckData->nBulletX;
	int nBulletY_C=pCheckData->nBulletY;
	int nFishID=pCheckData->nFishID;
	DWORD dwElapseTm=pCheckData->dwElapseTm;
	int nFishX=pCheckData->nFishX;
	int nFishY=pCheckData->nFishY;

	pCheckData->fMultiRation=1.0f;//确认初始化

	tagFishMove *pFish=m_pFish->GetFish(nFishID);
	tagBulletMove *pBullet=m_pBullet->GetBullet(nBulletID,wChairID);
	if(pFish==0) return 10001;
	if(pBullet==0) return 10002;
	if(nBulletX_C<0 || nBulletX_C>ResolutionWidth || nBulletY_C<0 || nBulletY_C>ResolutionHeight)
	{
		pCheckData->fMultiRation=rand()%20*0.005;
		return 100;
	}

	//服务器容忍延时误差
	__int64 tick_count_elapsed=GetElapsedTm(pFish->dwActiveTime);
	int nElapsed=(int)tick_count_elapsed;
	if(std::abs(long(dwElapseTm-nElapsed))>8000)
	{
		pCheckData->fMultiRation=rand()%20*0.005;
		//_tprintf("鱼游时间误差太大：%d,%d\n",dwElapseTm,nElapsed);
		return 2;
	}

	//轨迹容忍误差判断
	if(pFish->nPathID==-1) return 0;
	WORD wPathStyle=WORD(pFish->nPathID/10000);
	WORD wPathID=WORD(pFish->nPathID%1000);
	tagFishPath *pPath=PATH_SHARE.GetAllPointCount(PathSyle(wPathStyle),wPathID);
	if(pPath==0)
	{
		pCheckData->fMultiRation=0.98;
		return 3;
	}

	if(pFish->dwPathLineTime>0)
	{
		int nPointsCount=(int)pPath->TwoPoint.size();
		int nPointIndex=nPointsCount*dwElapseTm/pFish->dwPathLineTime;
		if(nPointIndex>nPointsCount) return 4;

		int nIndexTmp=nPointIndex;
		if(nPointIndex==nPointsCount) nIndexTmp=nPointsCount-1;
		if(nIndexTmp<0) return 5;
		DWORD dwData=pPath->TwoPoint[nIndexTmp];
		short wPointX=short(dwData>>16);
		short wPointY=short(dwData&0x0000FFFF);

		//鱼轨迹容忍判断
		if(pCheckData->cbCheckTag!=255)
		{
			DWORD dwCross=(pCheckData->cbCheckTag==1)?200:50;
			if(abs(nFishX-wPointX)>50 || abs(nFishY-wPointY)>50)
			{
				pCheckData->fMultiRation=rand()%20*0.005;
				//_tprintf("wPathID:%d,帧数：%d，鱼坐标server：%d,%d    client:%d,%d，时间差：%d, ctime:%d\n",wPathID,nIndexTmp,wPointX,wPointY,nFishX,nFishY,dwElapseTm-nElapsed,dwElapseTm);//上线务必去掉
				return 6;
			}
			if(pCheckData->cbCheckTag==3) return 0;
		}

		//子弹轨迹容忍判断
		int nTmpX=0,nTmpY=0;
		DWORD dwTimeServer=GetElapsedTm(pBullet->dwActiveTm);
		DWORD dwTimeClient=pCheckData->dwBulletTime;
		DWORD dwSwanTm=(dwTimeServer>dwTimeClient)?(dwTimeServer-dwTimeClient):(dwTimeClient-dwTimeServer);
		if(dwSwanTm<2888)
		{
			float dXTmp=pBullet->fInitPosX-pCheckData->fBulletStartX;
			float dYTmp=pBullet->fInitPosY-pCheckData->fBulletStartY;
			//if(fabs(dXTmp)>3 || fabs(dYTmp)>3) return 88;
			if(dXTmp>3 || dXTmp<-3 || dYTmp>3 || dYTmp<-3)
			{
				//_tprintf("%5f,%5f,    %5f,%5f\n",pBullet->fInitPosX,pCheckData->fBulletStartX,pBullet->fInitPosY,pCheckData->fBulletStartY);//上线务必去掉
				return 88;
			}
			if(true==m_pBullet->GetBulletPoint(nTmpX,nTmpY,pBullet->fSpeed,dwTimeClient,pCheckData->fBulletStartX,pCheckData->fBulletStartY,pBullet->fMouseX,pBullet->fMouseY))
			{
				if(abs(nTmpX-nBulletX_C)>2 || abs(nTmpY-nBulletY_C)>2)
				{
					int nT=sqrtf((nTmpX - nBulletX_C) *(nTmpX - nBulletX_C) + (nTmpY - nBulletY_C) *(nTmpY - nBulletY_C));
					//_tprintf("子弹坐标:Server (%d,%d), Client (%d,%d)---%d  速度%5f,客户端时间：%d， \n",nTmpX,nTmpY,nBulletX_C,nBulletY_C,nT,pBullet->fSpeed,dwTimeClient);//上线务必去掉
					//_tprintf("子弹开始位置：Server (%5f,%5f),Client (%5f,%5f)  鼠标：%5f, %5f\n\n",
					//	pBullet->fInitPosX,pBullet->fInitPosY,pCheckData->fBulletStartX,pCheckData->fBulletStartY,pBullet->fMouseX,pBullet->fMouseY);//上线务必去掉
				}
			}
			//else _tprintf("计算失败\n");//上线务必去掉
		}
		else
		{
			//_tprintf("误差大 Server:%d, Client:%d\n",dwTimeServer,dwTimeClient);//上线务必去掉
		}
	}

	return 0;
}