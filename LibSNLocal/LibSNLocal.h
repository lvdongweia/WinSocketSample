#pragma once; 

//------------------ LibSNLocal.h ----------------  
#ifndef LIBSNLOCAL_H
#define LIBSNLOCAL_H

// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� LIBSNLOCAL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// LIBSNLOCAL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef LIBSNLOCAL_EXPORTS
#define LIBSNLOCAL_API __declspec(dllexport)
#else
#define LIBSNLOCAL_API __declspec(dllimport)
#endif

// �����Ǵ� LibSNLocal.dll ������
class LIBSNLOCAL_API CLibSNLocal {
public:
	CLibSNLocal(void);
	// TODO: �ڴ�������ķ�����
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