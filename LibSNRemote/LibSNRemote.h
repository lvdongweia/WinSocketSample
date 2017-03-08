#pragma once; 

//------------------ LibSNRemote.h ----------------  
#ifndef LIBSNREMOTE_H
#define LIBSNREMOTE_H

// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� LIBSNREMOTE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// LIBSNREMOTE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef LIBSNREMOTE_EXPORTS
#define LIBSNREMOTE_API __declspec(dllexport)
#else
#define LIBSNREMOTE_API __declspec(dllimport)
#endif

// �����Ǵ� LibSNRemote.dll ������
class LIBSNREMOTE_API CLibSNRemote {
public:
	CLibSNRemote(void);
	// TODO: �ڴ�������ķ�����
};

extern LIBSNREMOTE_API int nLibSNRemote;

LIBSNREMOTE_API int fnLibSNRemote(void);

#define SN_LEN		18
#define CPUID_LEN	12
LIBSNREMOTE_API bool isvalidSNRemote(char * i_cpuid, char * i_sn);
LIBSNREMOTE_API int generateSNRemote(char * i_cpuid, char * o_sn);

#endif /*LIBSNREMOTE_H*/