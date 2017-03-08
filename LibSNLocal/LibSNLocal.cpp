// LibSNLocal.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "LibSNLocal.h"

using namespace std;

#define DLL_IMPLEMENT
#include "pttype.h"
#include "robot_common_data.h"
#include "PTBase.h"
#include "debug.h"

// 这是导出变量的一个示例
LIBSNLOCAL_API int nLibSNLocal=0;

// 这是导出函数的一个示例。
LIBSNLOCAL_API int fnLibSNLocal(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 LibSNLocal.h
CLibSNLocal::CLibSNLocal()
{
	return;
}

static int InitLib()
{
	int ret = NO_PT_ERROR;

	ret = OpenDevice();
	if (ret < 0)
	{
		goto out;
	}

	InitPTBase();
	InitDebug();

	StartCanRecv();
	Sleep(10);
	StartDebug();
	Sleep(10);
out:
	return ret;
}

static void UninitLib()
{
	Sleep(10);
	StopCanRecv();
	Sleep(10);
	StopDebug();
	Sleep(20);
	UninitPTBase();
	Sleep(10);
	UninitDebug();
	Sleep(10);
	CloseDevice();
}

static int readCPUIDX(uint8_t src, uint8_t dst, IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_parameter, char *cpuid)
{
	int ret = NO_PT_ERROR;
	uint8_t data_received[41] = {0};

	/* 1. Send the cpuid query */
	uint8_t data[2] = {0};
	data[0] = RP_SYSCTRL_GET_CPU_ID_CMD;
	data[1] = 0x02;

	ret = SendCanData(MIDDLE, src, dst, data, 2);
	if (ret < 0)
	{
		goto out;
	}

	/* 2. Receive the cpuid to query */
	memset(data_received, 0, sizeof(data_received));
	ret = ReceiveCanData(dst, src, 14,
		RP_SYSCTRL_GET_CPU_ID_CMD_REP, data_received,
		3000, cancel_callback,
		cancel_parameter);
	if (ret < 0)
	{
		goto out;
	}

	if (cpuid)
		memcpy(cpuid, data_received + 2, 12);
out:
	return ret;
}

LIBSNLOCAL_API int readCPUID(IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_prameter, char *o_cpuid)
{
    int ret = NO_PT_ERROR;
	ret = InitLib();
	if (ret < 0)
	{
		goto init_lib_failed;
	}

	ret = readCPUIDX(RM_SYSCTRL_ID, RP_SYSCTRL_ID, cancel_callback, cancel_prameter, o_cpuid);
	if (ret < 0)
	{
		goto out;
	}
out:
	UninitLib();

init_lib_failed:
	return ret;
}

static int readSNX(uint8_t src, uint8_t dst, IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_parameter, char *sn)
{
	int ret = NO_PT_ERROR;
	uint8_t data_received[41] = {0};

	/* 1. Send the sn query */
	uint8_t data[2] = {0};
	data[0] = RP_SYSCTRL_READ_ROBOT_SN_CMD;
	data[1] = 0x02;

	ret = SendCanData(MIDDLE, src, dst, data, 2);
	if (ret < 0)
	{
		goto out;
	}

	/* 2. Receive the sn to query */
	memset(data_received, 0, sizeof(data_received));
	ret = ReceiveCanData(dst, src, 20,
		RP_SYSCTRL_READ_ROBOT_SN_CMD_REP, data_received,
		3000, cancel_callback,
		cancel_parameter);
	if (ret < 0)
	{
		goto out;
	}

	if (sn)
		memcpy(sn, data_received + 2, 18);
out:
	return ret;
}

LIBSNLOCAL_API int readSN(IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_prameter, char *o_sn)
{
    int ret = NO_PT_ERROR;
	ret = InitLib();
	if (ret < 0)
	{
		goto init_lib_failed;
	}

	ret = readSNX(RM_SYSCTRL_ID, RP_SYSCTRL_ID, cancel_callback, cancel_prameter, o_sn);
	if (ret < 0)
	{
		goto out;
	}
out:
	UninitLib();

init_lib_failed:
	return ret;
}


static int writeSNX(uint8_t src, uint8_t dst, IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_parameter, char *sn)
{
	int ret = NO_PT_ERROR;
	uint8_t data_received[41] = {0};

	/* 1. Send the sn to save */
	uint8_t data[20] = {0};
	data[0] = RP_SYSCTRL_SAVE_ROBOT_SN_CMD;
	data[1] = 0x14;
	memcpy(data + 2, sn, 18);

	ret = SendCanData(MIDDLE, src, dst, data, 20);
	if (ret < 0)
	{
		goto out;
	}

	/* 2. Receive the response of sn to save */
	memset(data_received, 0, sizeof(data_received));
	ret = ReceiveCanData(dst, src, 3,
		RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP, data_received,
		3000, cancel_callback,
		cancel_parameter);
	if (ret < 0)
	{
		goto out;
	}

	if (data_received[2] != 0)
		ret = SAVE_SN_FAILED;

out:
	return ret;
}


LIBSNLOCAL_API int writeSN(IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_prameter, char *i_sn)
{
    int ret = NO_PT_ERROR;
	ret = InitLib();
	if (ret < 0)
	{
		goto init_lib_failed;
	}

	ret = writeSNX(RM_SYSCTRL_ID, RP_SYSCTRL_ID, cancel_callback, cancel_prameter, i_sn);
	if (ret < 0)
	{
		goto out;
	}
out:
	UninitLib();

init_lib_failed:
	return ret;
}