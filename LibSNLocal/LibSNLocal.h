#pragma once; 

//------------------ LibSNLocal.h ----------------  
#ifndef LIBSNLOCAL_H
#define LIBSNLOCAL_H

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LIBSNLOCAL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LIBSNLOCAL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef LIBSNLOCAL_EXPORTS
#define LIBSNLOCAL_API __declspec(dllexport)
#else
#define LIBSNLOCAL_API __declspec(dllimport)
#endif

// 此类是从 LibSNLocal.dll 导出的
class LIBSNLOCAL_API CLibSNLocal {
public:
	CLibSNLocal(void);
	// TODO: 在此添加您的方法。
};

extern LIBSNLOCAL_API int nLibSNLocal;

LIBSNLOCAL_API int fnLibSNLocal(void);

enum SN_MSG_TYPE {
	RP_SYSCTRL_GET_CPU_ID_CMD           = 74, //0x4A    2   None 
	RP_SYSCTRL_GET_CPU_ID_CMD_REP       = 75, //0x4B    14  CPU_ID:96 bit,12 byte
         
	RP_SYSCTRL_SAVE_ROBOT_SN_CMD        = 76, //0x4C    20  SN:   18 byte
	RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP    = 77, //0x4D    3   int8            SUCCESS 0; FAILED -1
         
	RP_SYSCTRL_READ_ROBOT_SN_CMD        = 78, //0x4E    2   None 
	RP_SYSCTRL_READ_ROBOT_SN_CMD_REP    = 79, //0x4F    20  SN:   18 byte 
};

enum SN_LOCAL_ERROR {
	SAVE_SN_FAILED						= -100,
};

typedef bool (* CANCEL_CALLBACK)(void *lpObj);

LIBSNLOCAL_API int readCPUID(CANCEL_CALLBACK cancel_callback,
	void *cancel_prameter, char *o_cpuid);
LIBSNLOCAL_API int readSN(CANCEL_CALLBACK cancel_callback,
	void *cancel_prameter, char *o_sn);
LIBSNLOCAL_API int writeSN(CANCEL_CALLBACK cancel_callback,
	void *cancel_prameter, char *i_sn);

#endif /*LIBSNLOCAL_H*/