#pragma once; 

//------------------ LibSNRemote.h ----------------  
#ifndef LIBSNREMOTE_H
#define LIBSNREMOTE_H

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LIBSNREMOTE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LIBSNREMOTE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef LIBSNREMOTE_EXPORTS
#define LIBSNREMOTE_API __declspec(dllexport)
#else
#define LIBSNREMOTE_API __declspec(dllimport)
#endif

// 此类是从 LibSNRemote.dll 导出的
class LIBSNREMOTE_API CLibSNRemote {
public:
	CLibSNRemote(void);
	// TODO: 在此添加您的方法。
};

extern LIBSNREMOTE_API int nLibSNRemote;

LIBSNREMOTE_API int fnLibSNRemote(void);

#define SN_LEN		18
#define CPUID_LEN	12
LIBSNREMOTE_API bool isvalidSNRemote(char * i_cpuid, char * i_sn);
LIBSNREMOTE_API int generateSNRemote(char * i_cpuid, char * o_sn);

#endif /*LIBSNREMOTE_H*/