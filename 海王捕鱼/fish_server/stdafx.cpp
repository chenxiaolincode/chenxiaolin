// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// fish_server.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

__int64 GetElapsedTm(IN DWORD dwTicktTm)
{
	__int64 tick_count_elapsed=0;
	__int64 tick_count = GetTickCount();
	__int64 tick_count_last=dwTicktTm;
	if (tick_count >= tick_count_last)
	{
		tick_count_elapsed = (tick_count - tick_count_last);
	}
	else
	{
		__int64 tickCountLarge = tick_count + 0x100000000I64;
		tick_count_elapsed = tickCountLarge - tick_count_last;
	}
	return tick_count_elapsed;
}
//
//__int64 GetDelateTm()
//{
//	static __int64 tick_count_last=GetTickCount();
//
//	__int64 tick_count_elapsed=0;
//	__int64 tick_count = GetTickCount();
//	if (tick_count >= tick_count_last)
//	{
//		tick_count_elapsed = (tick_count - tick_count_last);
//	}
//	else
//	{
//		__int64 tickCountLarge = tick_count + 0x100000000I64;
//		tick_count_elapsed = tickCountLarge - tick_count_last;
//	}
//	tick_count_last=tick_count;
//
//	return tick_count_elapsed;
//}