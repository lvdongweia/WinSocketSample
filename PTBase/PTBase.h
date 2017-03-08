//------------------ PTBase.h ----------------  
#ifndef PTBASE_H
#define PTBASE_H

#pragma once;  

//该宏完成在dll项目内部使用__declspec(dllexport)导出  
//在dll项目外部使用时，用__declspec(dllimport)导入  
//宏DLL_IMPLEMENT在simpledll.cpp中定义  
#ifdef DLL_IMPLEMENT  
#define DLL_API __declspec(dllexport)  
#else  
#define DLL_API __declspec(dllimport)  
#endif  

#include "pttype.h"

typedef bool (* IS_CANCEL_CALLBACK)(void *lpObj);

enum RmSysctrlCanPriority {
    HIGH	= 0,
    MIDDLE	= 3,
    LOW		= 6
};

enum PT_BASE_ERROR
{
	NO_PT_ERROR				=  0,
	OPEN_DEVICE_FAILED		= -1,
	INIT_CAN_FAILED			= -2,
	START_CAN_FAILED		= -3,
	CLOSE_DEVICE_FAILED		= -4,
	INVALID_PARAMETER		= -5,
	DEVICE_NOT_OPEN			= -6,
	SEND_ERROR				= -7,
	PACKET_IS_INCOMPLETE	= -8,
	RECEIVE_ERROR			= -9,
	RECEIVE_PACKET_FAILED	= -10,
	RECEIVE_PACKET_TIMEOUT	= -11,
	RECEIVE_PACKET_CANCELED = -12,
	NO_CAN_OBJ				= -13,
	SWITCH_MODE_FAILED		= -14,
};

DLL_API void InitPTBase();
DLL_API void UninitPTBase();
DLL_API int OpenDevice();
DLL_API int CloseDevice();
DLL_API void StartCanRecv();
DLL_API void StopCanRecv();
DLL_API int SendCanData(int priority, int src_id, int dest_id,
	const void *pdata, int len);
DLL_API int ReceiveCanData(uint8_t expect_src_id, uint8_t expect_dest_id,
	uint8_t expect_msg_len, uint8_t expect_msg_type, 
	void *o_pdata, int tolerable_waiting_time,
	IS_CANCEL_CALLBACK is_cancel_func,
	void *canel_parameter);


enum PT_MODE
{
	INVALID_MODE =  -1,
	FACTORY_MODE =   0,
    NORMAL_MODE  =   1,
};

struct SubSysVersion
{
    uint8_t sub_sys_id;
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    uint16_t building_year;
    uint8_t building_month;
    uint8_t building_day;
    uint8_t rom_id;
};

enum CommonSysctrlMsg {
	COMMON_SYSCTRL_VERSION_QUERY				= 26,  //0x1A    2
    COMMON_SYSCTRL_VERSION_QUERY_REP			= 27,  //0x1B    11 uint8*9       	Please see following
		/*
		*send_data=COMMON_SYSCTRL_VERSION_QUERY_REP;
		*(send_data+1)=0x0B;                                //command length
		*(send_data+2)=sub-sys-id;               	        //sub-system ID
		*(send_data+3)=major;                	            //major version number
		*(send_data+4)=minor;              	                //minor version number
		*(send_data+5)=revision; 	                        //revision number
		*(send_data+6)=year>>8;
		*(send_data+7)=year;                                //building year
		*(send_data+8)=month;         	                    //building month
		*(send_data+9)=day;                                 //building day
		*(send_data+10)=rom-id;            	                //rom id
		*/
	COMMON_SYSCTRL_PROD_MODE_QUERY				= 28, //0x1C    2  
	COMMON_SYSCTRL_PROD_MODE_QUERY_REP			= 29, //0x1D    3  0: FACTORY_MODE 1: NORMAL_MODE           
	COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD			= 30, //0x1E    2
	COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD_REP		= 31, //0x1F    3  0: Successful -1: failed  
};

DLL_API int PTGetMode(uint8_t src, uint8_t dst, IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_parameter, PT_MODE *mode);

DLL_API int PTSwitchMode(uint8_t src, uint8_t dst, IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_parameter);

enum CAN_CHANNEL {
	CHANNEL_ONE = 0,
	CHANNEL_TWO = 1,
};
DLL_API int PTSelectCanChannel(enum CAN_CHANNEL can_channel);
#endif /* PTBase_H */
