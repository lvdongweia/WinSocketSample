// LibSNRemote.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "LibSNRemote.h"
#include "snRemote.h"

// ���ǵ���������һ��ʾ��
LIBSNREMOTE_API int nLibSNRemote=0;

// ���ǵ���������һ��ʾ����
LIBSNREMOTE_API int fnLibSNRemote(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� LibSNRemote.h
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