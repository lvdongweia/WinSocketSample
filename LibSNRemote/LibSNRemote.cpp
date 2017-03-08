// LibSNRemote.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "LibSNRemote.h"
#include "snRemote.h"

// 这是导出变量的一个示例
LIBSNREMOTE_API int nLibSNRemote=0;

// 这是导出函数的一个示例。
LIBSNREMOTE_API int fnLibSNRemote(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 LibSNRemote.h
CLibSNRemote::CLibSNRemote()
{
	return;
}

LIBSNREMOTE_API bool isvalidSNRemote(char * i_cpuid, char * i_sn)
{
	return isvalidSN(i_cpuid, i_sn);
}

LIBSNREMOTE_API int generateSNRemote(char * i_cpuid, char * o_sn)
{
	return generateSN(i_cpuid, o_sn);
}