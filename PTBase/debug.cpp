#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include "ControlCAN.h"

using namespace std;

#define DLL_IMPLEMENT
#include "PTBase.h"
#include "debug.h"

typedef std::list<TCHAR *> UDT_LIST_DBG_STR;
UDT_LIST_DBG_STR DebugList;
static HANDLE DebugLock = INVALID_HANDLE_VALUE;   /* protection debug output */

static int count_list = 0;

void PushToDebugList(TCHAR *debug_str)
{
	WaitForSingleObject(DebugLock, INFINITE );
	DebugList.push_front(debug_str);
	count_list++;
	ReleaseMutex(DebugLock);
}

bool isDebugListEmpty()
{
	bool isEmpty = true;
	WaitForSingleObject(DebugLock, INFINITE );
	isEmpty = DebugList.empty();
	ReleaseMutex(DebugLock);
	return isEmpty;
}

void PopFromDebugList(TCHAR **debug_str_ptr)
{
	TCHAR *debug_str_local = NULL;
	WaitForSingleObject(DebugLock, INFINITE );
	if (!DebugList.empty())
	{
		*debug_str_ptr = DebugList.back();
		DebugList.pop_back();
		count_list--;
	}
	ReleaseMutex(DebugLock);
}

void CharToTchar (const char * _char, TCHAR * tchar)  
{  
    int iLength ;  
  
    iLength = MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, NULL, 0) ;  
    MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, tchar, iLength) ;  
} 

DLL_API bool PTDbgStr(LPCSTR lpszFormat, ...)
{
#if DEBUG
    va_list   args;
    int       nBuf;
    CHAR     *szBuffer = new CHAR[1024];
	TCHAR    *szWBuffer= new TCHAR[512];
    va_start(args, lpszFormat);
    nBuf = _vsnprintf_s(szBuffer, 1024, 1024, lpszFormat, args);
	CharToTchar(szBuffer, szWBuffer);
	delete[] szBuffer;
    if(nBuf > 0)
	{		
		PushToDebugList(szWBuffer);		
	}
    va_end(args);
#endif
	return true;
}

volatile BOOL bDebugRun=TRUE;
HANDLE hDebugThread;
DWORD DebugThreadID;

void DebugThreadFunc()
{
	while(bDebugRun)
	{
		TCHAR     *szWBuffer = NULL;
		PopFromDebugList(&szWBuffer);
		if (szWBuffer)
		{
			OutputDebugString(szWBuffer);
			delete[] szWBuffer;
		}
		Sleep(1);
	}
}

 void StartDebug()
{
	bDebugRun=TRUE;
	// TODO: Add your control notification handler code here
	hDebugThread=CreateThread(NULL,
	0,
	(LPTHREAD_START_ROUTINE)DebugThreadFunc,
	NULL,
	0,
	&DebugThreadID);
}
void StopDebug()
{
	while (!isDebugListEmpty()) Sleep(10);
	bDebugRun=FALSE;
}

void InitDebug()
{
	DebugLock = CreateMutex(NULL, FALSE, L"Mutex");  
	WaitForSingleObject(DebugLock, INFINITE );
	DebugList.clear();
	ReleaseMutex(DebugLock);
}

void UninitDebug()
{
	WaitForSingleObject(DebugLock, INFINITE );
	DebugList.clear();
	ReleaseMutex(DebugLock);
}